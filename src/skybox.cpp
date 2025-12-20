#include "skybox.h"
#include "rlgl.h"

// Global instance
SkyboxManager* g_SkyboxManager = nullptr;

// Skybox texture paths
static const char* SKYBOX_PATHS[6] = {
    "assets/textures/skybox/right.png",   // +X
    "assets/textures/skybox/left.png",    // -X
    "assets/textures/skybox/top.png",     // +Y
    "assets/textures/skybox/bottom.png",  // -Y
    "assets/textures/skybox/front.png",   // +Z
    "assets/textures/skybox/back.png"     // -Z
};

SkyboxManager::SkyboxManager() {
    for (int i = 0; i < 6; i++) {
        skyboxTextures[i] = { 0 };
    }
    skyboxMesh = { 0 };
    skyboxMaterial = { 0 };
    skyboxLoaded = false;
}

SkyboxManager::~SkyboxManager() {
    Unload();
}

void SkyboxManager::Initialize() {
    TraceLog(LOG_INFO, "Initializing Skybox Manager...");
    
    // Try to load skybox textures
    bool allLoaded = true;
    for (int i = 0; i < 6; i++) {
        if (!LoadSkyboxFace(i, SKYBOX_PATHS[i])) {
            allLoaded = false;
            break;
        }
    }
    
    // If textures not found, create procedural skybox
    if (!allLoaded) {
        TraceLog(LOG_WARNING, "Skybox textures not found, creating procedural skybox");
        CreateProceduralSkybox();
    }
    
    // Create skybox cube mesh
    skyboxMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    
    // Create material for skybox
    skyboxMaterial = LoadMaterialDefault();
    
    // Disable backface culling for skybox
    skyboxMaterial.maps[MATERIAL_MAP_DIFFUSE].texture = skyboxTextures[0];
    
    skyboxLoaded = true;
    TraceLog(LOG_INFO, "Skybox Manager initialized");
}

bool SkyboxManager::LoadSkyboxFace(int index, const char* filename) {
    if (FileExists(filename)) {
        Texture2D tex = LoadTexture(filename);
        if (tex.id > 0) {
            SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
            SetTextureWrap(tex, TEXTURE_WRAP_CLAMP);
            skyboxTextures[index] = tex;
            TraceLog(LOG_INFO, "Loaded skybox face %d: %s", index, filename);
            return true;
        }
    }
    return false;
}

void SkyboxManager::CreateProceduralSkybox() {
    const int size = 512;
    
    // Create gradient skybox textures
    for (int i = 0; i < 6; i++) {
        Image img;
        
        // Different colors for each face
        Color topColor = Color{135, 206, 235, 255};    // Sky blue
        Color bottomColor = Color{70, 130, 180, 255};  // Darker blue
        
        if (i == 2) { // Top face - lighter
            img = GenImageColor(size, size, topColor);
        } else if (i == 3) { // Bottom face - ground color
            img = GenImageColor(size, size, Color{80, 70, 60, 255});
        } else { // Side faces - gradient
            img = GenImageGradientLinear(size, size, 90, topColor, bottomColor);
        }
        
        // Add some noise for atmosphere
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (rand() % 100 < 5) {
                    ImageDrawPixel(&img, x, y, Color{255, 255, 255, 200});
                }
            }
        }
        
        skyboxTextures[i] = LoadTextureFromImage(img);
        UnloadImage(img);
        SetTextureFilter(skyboxTextures[i], TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(skyboxTextures[i], TEXTURE_WRAP_CLAMP);
    }
}

void SkyboxManager::Draw(const Camera3D& camera) {
    if (!skyboxLoaded) return;
    
    // Disable depth test so skybox is always behind everything
    rlDisableDepthTest();
    rlDisableBackfaceCulling();
    
    // Draw skybox centered on camera (but doesn't move with camera)
    // Scale skybox to be very large
    float skyboxSize = 1000.0f;
    
    // Save current matrix
    rlPushMatrix();
    
    // Position skybox at camera location (but ignore camera rotation for translation)
    rlTranslatef(camera.position.x, camera.position.y, camera.position.z);
    rlScalef(skyboxSize, skyboxSize, skyboxSize);
    
    // Draw each face of the skybox with correct texture
    rlBegin(RL_QUADS);
    
    // Right face (+X)
    rlSetTexture(skyboxTextures[0].id);
    rlColor4ub(255, 255, 255, 255);
    rlNormal3f(1.0f, 0.0f, 0.0f);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.5f, -0.5f, -0.5f);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.5f, -0.5f, 0.5f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.5f, 0.5f, 0.5f);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.5f, 0.5f, -0.5f);
    
    // Left face (-X)
    rlSetTexture(skyboxTextures[1].id);
    rlNormal3f(-1.0f, 0.0f, 0.0f);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-0.5f, -0.5f, 0.5f);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(-0.5f, -0.5f, -0.5f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(-0.5f, 0.5f, -0.5f);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-0.5f, 0.5f, 0.5f);
    
    // Top face (+Y)
    rlSetTexture(skyboxTextures[2].id);
    rlNormal3f(0.0f, 1.0f, 0.0f);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-0.5f, 0.5f, -0.5f);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.5f, 0.5f, -0.5f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.5f, 0.5f, 0.5f);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-0.5f, 0.5f, 0.5f);
    
    // Bottom face (-Y)
    rlSetTexture(skyboxTextures[3].id);
    rlNormal3f(0.0f, -1.0f, 0.0f);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-0.5f, -0.5f, 0.5f);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.5f, -0.5f, 0.5f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.5f, -0.5f, -0.5f);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-0.5f, -0.5f, -0.5f);
    
    // Front face (+Z)
    rlSetTexture(skyboxTextures[4].id);
    rlNormal3f(0.0f, 0.0f, 1.0f);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-0.5f, -0.5f, 0.5f);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(0.5f, -0.5f, 0.5f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(0.5f, 0.5f, 0.5f);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-0.5f, 0.5f, 0.5f);
    
    // Back face (-Z)
    rlSetTexture(skyboxTextures[5].id);
    rlNormal3f(0.0f, 0.0f, -1.0f);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(0.5f, -0.5f, -0.5f);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(-0.5f, -0.5f, -0.5f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(-0.5f, 0.5f, -0.5f);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(0.5f, 0.5f, -0.5f);
    
    rlEnd();
    rlSetTexture(0);
    
    rlPopMatrix();
    
    // Re-enable depth test
    rlEnableDepthTest();
    rlEnableBackfaceCulling();
}

void SkyboxManager::Reload() {
    Unload();
    Initialize();
}

void SkyboxManager::Unload() {
    for (int i = 0; i < 6; i++) {
        if (skyboxTextures[i].id > 0) {
            UnloadTexture(skyboxTextures[i]);
            skyboxTextures[i] = { 0 };
        }
    }
    
    if (skyboxMesh.vertexCount > 0) {
        UnloadMesh(skyboxMesh);
        skyboxMesh = { 0 };
    }
    
    if (skyboxMaterial.maps != nullptr) {
        UnloadMaterial(skyboxMaterial);
        skyboxMaterial = { 0 };
    }
    
    skyboxLoaded = false;
}

// Global initialization
void InitializeSkyboxSystem() {
    g_SkyboxManager = new SkyboxManager();
    g_SkyboxManager->Initialize();
    TraceLog(LOG_INFO, "Skybox system initialized");
}

void CleanupSkyboxSystem() {
    if (g_SkyboxManager) {
        delete g_SkyboxManager;
        g_SkyboxManager = nullptr;
    }
    TraceLog(LOG_INFO, "Skybox system cleaned up");
}
