#include "texture_manager.h"
#include "rlgl.h"

// Global instances
TextureManager* g_TextureManager = nullptr;
ShaderManager* g_ShaderManager = nullptr;

// Texture file paths
static const char* TEXTURE_PATHS[TEX_COUNT] = {
    "assets/textures/wall_concrete.png",
    "assets/textures/wall_brick.png",
    "assets/textures/wall_metal.png",
    "assets/textures/floor_tile.png",
    "assets/textures/floor_concrete.png",
    "assets/textures/floor_wood.png",
    "assets/textures/floor_carpet.png",
    "assets/textures/ceiling_tile.png",
    "assets/textures/door_wood.png",
    "assets/textures/door_metal.png",
    "assets/textures/road_asphalt.png",
    "assets/textures/grass.png",
    "assets/textures/dirt.png",
    "assets/textures/tree_bark.png",
    "assets/textures/tree_leaves.png",
    "assets/textures/building_exterior.png",
    "assets/textures/window_glass.png",
    "assets/textures/roof_shingles.png",
    "assets/textures/bush_leaves.png",
    "assets/textures/flower_petals.png",
    "assets/textures/sky.png"
};

// =============================================================================
// TEXTURE MANAGER IMPLEMENTATION
// =============================================================================

TextureManager::TextureManager() {
    fallbackTexture = { 0 };
}

TextureManager::~TextureManager() {
    Unload();
}

void TextureManager::Initialize() {
    TraceLog(LOG_INFO, "Initializing Texture Manager...");
    
    // Create fallback texture first
    CreateFallbackTexture();
    
    // Load all textures
    for (int i = 0; i < TEX_COUNT; i++) {
        TextureID id = (TextureID)i;
        
        // Try to load from file
        if (!LoadTextureFile(id, TEXTURE_PATHS[i])) {
            TraceLog(LOG_WARNING, "Failed to load texture: %s - Using procedural fallback", TEXTURE_PATHS[i]);
            
            // Create procedural texture as fallback
            Texture2D procTex = CreateProceduralTexture(id);
            textures[id] = procTex;
            loadedStatus[id] = true; // Mark as loaded (even if procedural)
        }
    }
    
    TraceLog(LOG_INFO, "Texture Manager initialized. Loaded %d/%d textures from files.", 
             (int)loadedStatus.size(), TEX_COUNT);
}

bool TextureManager::LoadTextureFile(TextureID id, const char* filename) {
    if (FileExists(filename)) {
        Texture2D tex = LoadTexture(filename);
        if (tex.id > 0) {
            // Enable trilinear filtering for better quality
            SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
            SetTextureWrap(tex, TEXTURE_WRAP_REPEAT);
            
            textures[id] = tex;
            loadedStatus[id] = true;
            TraceLog(LOG_INFO, "Loaded texture: %s", filename);
            return true;
        }
    }
    
    loadedStatus[id] = false;
    return false;
}

void TextureManager::CreateFallbackTexture() {
    // Create a 64x64 purple/magenta checkerboard pattern
    const int size = 64;
    Image img = GenImageChecked(size, size, size/8, size/8, MAGENTA, PURPLE);
    fallbackTexture = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(fallbackTexture, TEXTURE_FILTER_POINT);
}

Texture2D TextureManager::CreateProceduralTexture(TextureID id) {
    const int size = 256;
    Image img;
    
    switch (id) {
        case TEX_WALL_CONCRETE:
            img = GenImageColor(size, size, Color{180, 180, 185, 255});
            ImageDrawRectangleLines(&img, Rectangle{0, 0, (float)size, (float)size}, 2, Color{140, 140, 145, 255});
            break;
            
        case TEX_WALL_BRICK:
            img = GenImageColor(size, size, Color{150, 80, 60, 255});
            for (int y = 0; y < size; y += 32) {
                for (int x = 0; x < size; x += 64) {
                    int offset = (y / 32) % 2 ? 32 : 0;
                    ImageDrawRectangle(&img, x + offset, y, 60, 28, Color{120, 60, 45, 255});
                    ImageDrawRectangleLines(&img, Rectangle{(float)(x + offset), (float)y, 60, 28}, 2, Color{100, 50, 35, 255});
                }
            }
            break;
            
        case TEX_WALL_METAL:
            img = GenImageColor(size, size, Color{100, 100, 110, 255});
            for (int i = 0; i < size; i += 16) {
                ImageDrawLine(&img, 0, i, size, i, Color{80, 80, 90, 255});
            }
            break;
            
        case TEX_FLOOR_TILE:
            img = GenImageChecked(size, size, size/4, size/4, Color{200, 200, 200, 255}, Color{180, 180, 180, 255});
            break;
            
        case TEX_FLOOR_CONCRETE:
            img = GenImageColor(size, size, Color{120, 120, 125, 255});
            break;
            
        case TEX_FLOOR_WOOD:
            img = GenImageColor(size, size, Color{139, 90, 43, 255});
            for (int i = 0; i < size; i += 32) {
                ImageDrawLine(&img, 0, i, size, i, Color{110, 70, 30, 255});
            }
            break;
            
        case TEX_FLOOR_CARPET:
            img = GenImageColor(size, size, Color{140, 60, 60, 255});
            break;
            
        case TEX_CEILING_TILE:
            img = GenImageChecked(size, size, size/8, size/8, Color{240, 240, 240, 255}, Color{230, 230, 230, 255});
            break;
            
        case TEX_DOOR_WOOD:
            img = GenImageColor(size, size, Color{101, 67, 33, 255});
            ImageDrawRectangle(&img, size/4, size/4, size/2, size/2, Color{120, 80, 40, 255});
            break;
            
        case TEX_DOOR_METAL:
            img = GenImageColor(size, size, Color{90, 90, 95, 255});
            break;
            
        case TEX_ROAD_ASPHALT:
            img = GenImageColor(size, size, Color{60, 60, 65, 255});
            break;
            
        case TEX_GRASS:
            img = GenImageColor(size, size, Color{50, 140, 50, 255});
            for (int i = 0; i < 100; i++) {
                int x = rand() % size;
                int y = rand() % size;
                ImageDrawPixel(&img, x, y, Color{40, 120, 40, 255});
            }
            break;
            
        case TEX_DIRT:
            img = GenImageColor(size, size, Color{139, 90, 43, 255});
            break;
            
        case TEX_TREE_BARK:
            img = GenImageColor(size, size, Color{101, 67, 33, 255});
            break;
            
        case TEX_TREE_LEAVES:
            img = GenImageColor(size, size, Color{34, 139, 34, 255});
            break;
            
        case TEX_BUILDING_EXTERIOR:
            img = GenImageColor(size, size, Color{120, 120, 130, 255});
            break;
            
        case TEX_WINDOW_GLASS:
            img = GenImageColor(size, size, Color{135, 206, 235, 180});
            break;
            
        case TEX_ROOF_SHINGLES:
            img = GenImageColor(size, size, Color{80, 50, 50, 255});
            break;
            
        case TEX_BUSH_LEAVES:
            img = GenImageColor(size, size, Color{46, 125, 50, 255});
            break;
            
        case TEX_FLOWER_PETALS:
            img = GenImageColor(size, size, Color{255, 105, 180, 255});
            break;
            
        case TEX_SKY:
            img = GenImageGradientLinear(size, size, 90, Color{135, 206, 235, 255}, Color{70, 130, 180, 255});
            break;
            
        default:
            img = GenImageColor(size, size, GRAY);
            break;
    }
    
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(tex, TEXTURE_WRAP_REPEAT);
    
    return tex;
}

Texture2D TextureManager::GetTexture(TextureID id) {
    if (textures.find(id) != textures.end() && textures[id].id > 0) {
        return textures[id];
    }
    return fallbackTexture;
}

bool TextureManager::IsLoaded(TextureID id) {
    return loadedStatus[id];
}

Texture2D TextureManager::GetFallbackTexture() {
    return fallbackTexture;
}

void TextureManager::Reload() {
    Unload();
    Initialize();
}

void TextureManager::Unload() {
    for (auto& pair : textures) {
        if (pair.second.id > 0) {
            UnloadTexture(pair.second);
        }
    }
    textures.clear();
    loadedStatus.clear();
    
    if (fallbackTexture.id > 0) {
        UnloadTexture(fallbackTexture);
        fallbackTexture = { 0 };
    }
}

// =============================================================================
// SHADER MANAGER IMPLEMENTATION
// =============================================================================

ShaderManager::ShaderManager() {
    lightingShader = { 0 };
    shaderLoaded = false;
}

ShaderManager::~ShaderManager() {
    Unload();
}

void ShaderManager::Initialize() {
    TraceLog(LOG_INFO, "Initializing Shader Manager...");
    
    // Try to load custom shaders
    if (FileExists("assets/shaders/lighting.vs") && FileExists("assets/shaders/lighting.fs")) {
        lightingShader = LoadShader("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
        
        if (lightingShader.id > 0) {
            shaderLoaded = true;
            TraceLog(LOG_INFO, "Loaded custom lighting shader");
        }
    }
    
    // Fallback to default shader if custom shaders not found
    if (!shaderLoaded) {
        TraceLog(LOG_WARNING, "Custom shaders not found, using default lighting");
        lightingShader = LoadShaderFromMemory(nullptr, nullptr); // Default shader
        shaderLoaded = false;
    }
    
    if (shaderLoaded && lightingShader.id > 0) {
        // Get uniform locations
        viewPosLoc = GetShaderLocation(lightingShader, "viewPos");
        lightPosLoc = GetShaderLocation(lightingShader, "lightPos");
        lightColorLoc = GetShaderLocation(lightingShader, "lightColor");
        lightIntensityLoc = GetShaderLocation(lightingShader, "lightIntensity");
        ambientColorLoc = GetShaderLocation(lightingShader, "ambientColor");
        ambientIntensityLoc = GetShaderLocation(lightingShader, "ambientIntensity");
        flashlightPosLoc = GetShaderLocation(lightingShader, "flashlightPos");
        flashlightDirLoc = GetShaderLocation(lightingShader, "flashlightDir");
        flashlightColorLoc = GetShaderLocation(lightingShader, "flashlightColor");
        flashlightIntensityLoc = GetShaderLocation(lightingShader, "flashlightIntensity");
        flashlightCutoffLoc = GetShaderLocation(lightingShader, "flashlightCutoff");
        flashlightOuterCutoffLoc = GetShaderLocation(lightingShader, "flashlightOuterCutoff");
        flashlightEnabledLoc = GetShaderLocation(lightingShader, "flashlightEnabled");
        fogColorLoc = GetShaderLocation(lightingShader, "fogColor");
        fogDensityLoc = GetShaderLocation(lightingShader, "fogDensity");
        fogStartLoc = GetShaderLocation(lightingShader, "fogStart");
        fogEndLoc = GetShaderLocation(lightingShader, "fogEnd");
        
        // Set default lighting values
        Vector3 ambientColor = { 0.2f, 0.2f, 0.3f };
        float ambientIntensity = 0.3f;
        Vector3 lightColor = { 1.0f, 0.95f, 0.8f };
        float lightIntensity = 0.6f;
        Vector3 fogColor = { 0.02f, 0.04f, 0.06f };
        float fogDensity = 0.8f;
        float fogStart = 15.0f;
        float fogEnd = 50.0f;
        
        SetShaderValue(lightingShader, ambientColorLoc, &ambientColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, ambientIntensityLoc, &ambientIntensity, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lightingShader, lightColorLoc, &lightColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, lightIntensityLoc, &lightIntensity, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lightingShader, fogColorLoc, &fogColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lightingShader, fogStartLoc, &fogStart, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lightingShader, fogEndLoc, &fogEnd, SHADER_UNIFORM_FLOAT);
        
        // Flashlight defaults
        float cutoff = cosf(12.5f * DEG2RAD);
        float outerCutoff = cosf(17.5f * DEG2RAD);
        SetShaderValue(lightingShader, flashlightCutoffLoc, &cutoff, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lightingShader, flashlightOuterCutoffLoc, &outerCutoff, SHADER_UNIFORM_FLOAT);
    }
}

Shader ShaderManager::GetLightingShader() {
    return lightingShader;
}

void ShaderManager::UpdateLighting(const Camera3D& camera, Vector3 lightPos, bool flashlightOn, Vector3 flashlightPos, Vector3 flashlightDir, float flashlightIntensity) {
    if (!shaderLoaded || lightingShader.id == 0) return;
    
    // Update camera position
    SetShaderValue(lightingShader, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
    
    // Update main light position (sun/moon)
    SetShaderValue(lightingShader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
    
    // Update flashlight
    int enabled = flashlightOn ? 1 : 0;
    SetShaderValue(lightingShader, flashlightEnabledLoc, &enabled, SHADER_UNIFORM_INT);
    
    if (flashlightOn) {
        Vector3 flashColor = { 1.0f, 0.95f, 0.8f };
        SetShaderValue(lightingShader, flashlightPosLoc, &flashlightPos, SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, flashlightDirLoc, &flashlightDir, SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, flashlightColorLoc, &flashColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, flashlightIntensityLoc, &flashlightIntensity, SHADER_UNIFORM_FLOAT);
    }
}

void ShaderManager::Unload() {
    if (lightingShader.id > 0) {
        UnloadShader(lightingShader);
        lightingShader = { 0 };
    }
    shaderLoaded = false;
}

// =============================================================================
// GLOBAL INITIALIZATION
// =============================================================================

void InitializeRenderingSystems() {
    g_TextureManager = new TextureManager();
    g_TextureManager->Initialize();
    
    g_ShaderManager = new ShaderManager();
    g_ShaderManager->Initialize();
    
    TraceLog(LOG_INFO, "Rendering systems initialized");
}

void CleanupRenderingSystems() {
    if (g_TextureManager) {
        delete g_TextureManager;
        g_TextureManager = nullptr;
    }
    
    if (g_ShaderManager) {
        delete g_ShaderManager;
        g_ShaderManager = nullptr;
    }
    
    TraceLog(LOG_INFO, "Rendering systems cleaned up");
}