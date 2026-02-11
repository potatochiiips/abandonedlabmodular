#include "rendering.h"
#include "model_manager.h"
#include "texture_manager.h"
#include "rlgl.h"

RenderingPipeline* g_RenderingPipeline = nullptr;
UpgradedPipeline* g_UpgradedPipeline = nullptr;

// ============================================================================
// RenderingPipeline Implementation
// ============================================================================

RenderingPipeline::RenderingPipeline() {
    fallbackModel = { 0 };
}

RenderingPipeline::~RenderingPipeline() {
    Cleanup();
}

void RenderingPipeline::Initialize() {
    TraceLog(LOG_INFO, "Initializing Rendering Pipeline...");
    CreateFallbackModel();
    TraceLog(LOG_INFO, "Rendering Pipeline initialized");
}

void RenderingPipeline::CreateFallbackModel() {
    Mesh mesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    fallbackModel = LoadModelFromMesh(mesh);
    fallbackModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = MAGENTA;
}

Model RenderingPipeline::LoadModel(int modelId) {
    if (modelCache.find(modelId) != modelCache.end()) {
        if (modelCache[modelId].isLoaded) {
            modelCache[modelId].referenceCount++;
            return modelCache[modelId].model;
        }
    }

    // Load from model manager if available
    if (g_ModelManager && g_ModelManager->IsLoaded((ModelID)modelId)) {
        Model model = g_ModelManager->GetModel((ModelID)modelId);
        if (model.meshCount > 0) {
            CachedModel cached;
            cached.model = model;
            cached.referenceCount = 1;
            cached.isLoaded = true;
            modelCache[modelId] = cached;
            TraceLog(LOG_INFO, "Loaded model %d into cache", modelId);
            return model;
        }
    }

    // Return fallback if load fails
    return fallbackModel;
}

void RenderingPipeline::UnloadModel(int modelId) {
    if (modelCache.find(modelId) == modelCache.end()) return;

    modelCache[modelId].referenceCount--;
    if (modelCache[modelId].referenceCount <= 0) {
        if (modelCache[modelId].isLoaded) {
            modelCache[modelId].isLoaded = false;
            TraceLog(LOG_INFO, "Unloaded model %d from cache", modelId);
        }
    }
}

bool RenderingPipeline::IsModelLoaded(int modelId) const {
    if (modelCache.find(modelId) == modelCache.end()) return false;
    return modelCache.at(modelId).isLoaded && modelCache.at(modelId).referenceCount > 0;
}

Model RenderingPipeline::GetModel(int modelId) {
    if (modelCache.find(modelId) != modelCache.end() && modelCache[modelId].isLoaded) {
        return modelCache[modelId].model;
    }
    return fallbackModel;
}

void RenderingPipeline::BeginFrame() {
    // Frame setup if needed
}

void RenderingPipeline::EndFrame() {
    // Frame cleanup if needed
}

void RenderingPipeline::RenderMesh(const MeshRenderer& renderer, Color tint) {
    if (!renderer.enabled) return;

    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(renderer.transform));
    
    // FIXED: Use texture manager if available
    Material mat = LoadMaterialDefault();
    
    // Apply custom material if available
    if (renderer.material) {
        // Material already has textures set, use it
        mat = LoadMaterialDefault();
        Texture2D tex = renderer.material->GetTexture("texture0");
        if (tex.id > 0) {
            mat.maps[MATERIAL_MAP_DIFFUSE].texture = tex;
        }
        Color matColor = renderer.material->GetColor("colDiffuse");
        mat.maps[MATERIAL_MAP_DIFFUSE].color = matColor;
    }
    else {
        // Use texture manager for default textures
        if (g_TextureManager) {
            Texture2D floorTex = g_TextureManager->GetTexture(TEX_FLOOR_CONCRETE);
            mat.maps[MATERIAL_MAP_DIFFUSE].texture = floorTex;
        }
        mat.maps[MATERIAL_MAP_DIFFUSE].color = tint;
    }
    
    DrawMesh(renderer.mesh, mat, MatrixIdentity());
    
    rlPopMatrix();
}

void RenderingPipeline::RenderModel(const Vector3& position, int modelId, float scale, Color tint) {
    if (!IsModelLoaded(modelId)) {
        LoadModel(modelId);
    }

    Model model = GetModel(modelId);
    if (model.meshCount == 0) return;

    Matrix scaleMatrix = MatrixScale(scale, scale, scale);
    Matrix translation = MatrixTranslate(position.x, position.y, position.z);
    Matrix transform = MatrixMultiply(scaleMatrix, translation);

    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(transform));
    
    for (int i = 0; i < model.meshCount; i++) {
        Material mat = model.materials[model.meshMaterial[i]];
        
        // FIXED: Apply texture manager textures as fallback
        if (g_TextureManager && mat.maps[MATERIAL_MAP_DIFFUSE].texture.id == 0) {
            mat.maps[MATERIAL_MAP_DIFFUSE].texture = g_TextureManager->GetTexture(TEX_BUILDING_EXTERIOR);
        }
        
        mat.maps[MATERIAL_MAP_DIFFUSE].color = tint;
        DrawMesh(model.meshes[i], mat, MatrixIdentity());
    }
    
    rlPopMatrix();
}

void RenderingPipeline::ClearUnusedModels() {
    auto it = modelCache.begin();
    while (it != modelCache.end()) {
        if (it->second.referenceCount <= 0 && it->second.isLoaded) {
            it = modelCache.erase(it);
            TraceLog(LOG_INFO, "Cleared unused model from cache");
        }
        else {
            ++it;
        }
    }
}

int RenderingPipeline::GetCachedModelCount() const {
    int count = 0;
    for (const auto& pair : modelCache) {
        if (pair.second.isLoaded) count++;
    }
    return count;
}

void RenderingPipeline::Cleanup() {
    modelCache.clear();
    if (fallbackModel.meshCount > 0) {
        UnloadModel((int)fallbackModel.meshCount);
    }
    TraceLog(LOG_INFO, "Rendering Pipeline cleaned up");
}

// ============================================================================
// UpgradedPipeline Implementation
// ============================================================================

UpgradedPipeline::UpgradedPipeline() 
    : ambientColor(Color{200, 200, 200, 255}), ambientIntensity(0.8f),
      fogEnabled(false), fogColor(Color{180, 180, 180, 255}), fogDensity(0.01f) {
}

UpgradedPipeline::~UpgradedPipeline() {
    Cleanup();
}

void UpgradedPipeline::Initialize() {
    TraceLog(LOG_INFO, "Initializing Upgraded Rendering Pipeline...");
    TraceLog(LOG_INFO, "Upgraded Rendering Pipeline initialized");
}

void UpgradedPipeline::Cleanup() {
    lights.clear();
    TraceLog(LOG_INFO, "Upgraded Rendering Pipeline cleaned up");
}

void UpgradedPipeline::SetAmbientLight(Color color, float intensity) {
    ambientColor = color;
    ambientIntensity = Clamp(intensity, 0.0f, 2.0f);
}

void UpgradedPipeline::SetFog(bool enabled, Color color, float density) {
    fogEnabled = enabled;
    fogColor = color;
    fogDensity = density;
}

void UpgradedPipeline::AddLight(const UpgradedLight& light) {
    if (lights.size() < 8) {  // Limit to 8 lights
        lights.push_back(light);
    }
}

void UpgradedPipeline::ClearLights() {
    lights.clear();
}

Shader UpgradedPipeline::CreateUnlitShader() const {
    // Return a basic unlit shader
    // For now, return default material shader
    return LoadShaderFromMemory(nullptr, nullptr);
}

Shader UpgradedPipeline::CreateLitShader() const {
    // Return a lit shader with lighting support
    return LoadShaderFromMemory(nullptr, nullptr);
}

void UpgradedPipeline::BeginFrame() {
    // Setup frame state
}

void UpgradedPipeline::EndFrame() {
    // Cleanup frame state
}

void UpgradedPipeline::RenderMesh(const MeshRenderer& renderer, Color tint) {
    if (!renderer.enabled) return;

    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(renderer.transform));
    
    Material mat = LoadMaterialDefault();
    
    // Apply custom material if available
    if (renderer.material) {
        Texture2D tex = renderer.material->GetTexture("texture0");
        if (tex.id > 0) {
            mat.maps[MATERIAL_MAP_DIFFUSE].texture = tex;
        }
        Color matColor = renderer.material->GetColor("colDiffuse");
        mat.maps[MATERIAL_MAP_DIFFUSE].color = matColor;
    }
    else {
        mat.maps[MATERIAL_MAP_DIFFUSE].color = tint;
    }
    
    DrawMesh(renderer.mesh, mat, MatrixIdentity());
    
    rlPopMatrix();
}

void UpgradedPipeline::RenderModel(const Vector3& position, int modelId, float scale, Color tint) {
    if (!g_RenderingPipeline) return;
    g_RenderingPipeline->RenderModel(position, modelId, scale, tint);
}

void InitializeRenderingPipeline() {
    g_RenderingPipeline = new RenderingPipeline();
    g_RenderingPipeline->Initialize();
    
    g_UpgradedPipeline = new UpgradedPipeline();
    g_UpgradedPipeline->Initialize();
}

void CleanupRenderingPipeline() {
    if (g_RenderingPipeline) {
        delete g_RenderingPipeline;
        g_RenderingPipeline = nullptr;
    }
    if (g_UpgradedPipeline) {
        delete g_UpgradedPipeline;
        g_UpgradedPipeline = nullptr;
    }
}