#include "model_manager.h"
#include "texture_manager.h"
#include "rlgl.h"

// Global instance
ModelManager* g_ModelManager = nullptr;

// Model file paths
static const char* MODEL_PATHS[MODEL_COUNT] = {
    "assets/models/pistol.glb",
    "assets/models/m16.glb",
    "assets/models/flashlight.glb",
    "assets/models/water_bottle.glb",
    "assets/models/lab_key.glb",
    "assets/models/wood.glb",
    "assets/models/stone.glb",
    "assets/models/potato_chips.glb",
    "assets/models/magazine.glb",
    "assets/models/m16_magazine.glb"
};

// Model scales and offsets for proper display
struct ModelTransform {
    Vector3 scale;
    Vector3 offset;
    Vector3 rotation;
};

static const ModelTransform MODEL_TRANSFORMS[MODEL_COUNT] = {
    {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},    // Pistol
    {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},    // M16
    {{0.8f, 0.8f, 0.8f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},    // Flashlight
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},    // Water bottle
    {{0.3f, 0.3f, 0.3f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},    // Lab key
    {{0.6f, 0.6f, 0.6f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},    // Wood
    {{0.4f, 0.4f, 0.4f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},    // Stone
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},    // Potato chips
    {{0.6f, 0.6f, 0.6f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},    // Magazine
    {{0.6f, 0.6f, 0.6f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}     // M16 magazine
};

ModelManager::ModelManager() {
    fallbackModel = { 0 };
}

ModelManager::~ModelManager() {
    Unload();
}

void ModelManager::Initialize() {
    TraceLog(LOG_INFO, "Initializing Model Manager...");
    
    // Create fallback model first
    CreateFallbackModel();
    
    // Load all models
    for (int i = 0; i < MODEL_COUNT; i++) {
        ModelID id = (ModelID)i;
        
        // Try to load from glTF/glb file
        if (!LoadModelFile(id, MODEL_PATHS[i], 
                          MODEL_TRANSFORMS[i].scale,
                          MODEL_TRANSFORMS[i].offset,
                          MODEL_TRANSFORMS[i].rotation)) {
            TraceLog(LOG_WARNING, "Failed to load model: %s - Using procedural fallback", MODEL_PATHS[i]);
            
            // Create procedural model as fallback
            Model procModel = CreateProceduralModel(id);
            ModelData data;
            data.model = procModel;
            data.loaded = true;
            data.scale = MODEL_TRANSFORMS[i].scale;
            data.offset = MODEL_TRANSFORMS[i].offset;
            data.rotation = MODEL_TRANSFORMS[i].rotation;
            data.filename = MODEL_PATHS[i];
            models[id] = data;
        }
    }
    
    TraceLog(LOG_INFO, "Model Manager initialized. Loaded %d/%d models from files.", 
             (int)models.size(), MODEL_COUNT);
}

bool ModelManager::LoadModelFile(ModelID id, const char* filename, Vector3 scale, Vector3 offset, Vector3 rotation) {
    if (FileExists(filename)) {
        Model model = LoadModel(filename);
        if (model.meshCount > 0) {
            // Apply textures from texture manager
            ApplyTexturesToModel(model, id);
            
            ModelData data;
            data.model = model;
            data.loaded = true;
            data.scale = scale;
            data.offset = offset;
            data.rotation = rotation;
            data.filename = filename;
            
            models[id] = data;
            TraceLog(LOG_INFO, "Loaded model: %s", filename);
            return true;
        } else {
            UnloadModel(model);
        }
    }
    
    return false;
}

void ModelManager::ApplyTexturesToModel(Model& model, ModelID id) {
    if (!g_TextureManager) return;
    
    // Apply appropriate textures based on model type
    Texture2D texture = { 0 };
    
    switch (id) {
        case MODEL_PISTOL:
        case MODEL_M16:
        case MODEL_MAGAZINE:
        case MODEL_M16_MAGAZINE:
            texture = g_TextureManager->GetTexture(TEX_WALL_METAL);
            break;
            
        case MODEL_FLASHLIGHT:
            texture = g_TextureManager->GetTexture(TEX_WALL_METAL);
            break;
            
        case MODEL_WATER_BOTTLE:
            texture = g_TextureManager->GetTexture(TEX_WINDOW_GLASS);
            break;
            
        case MODEL_LAB_KEY:
            texture = g_TextureManager->GetTexture(TEX_WALL_METAL);
            break;
            
        case MODEL_WOOD:
            texture = g_TextureManager->GetTexture(TEX_FLOOR_WOOD);
            break;
            
        case MODEL_STONE:
            texture = g_TextureManager->GetTexture(TEX_WALL_CONCRETE);
            break;
            
        case MODEL_POTATO_CHIPS:
            texture = g_TextureManager->GetTexture(TEX_FLOOR_CARPET);
            break;
            
        default:
            texture = g_TextureManager->GetTexture(TEX_WALL_CONCRETE);
            break;
    }
    
    // Apply texture to all materials in the model
    if (texture.id > 0) {
        for (int i = 0; i < model.materialCount; i++) {
            model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
        }
    }
}

void ModelManager::CreateFallbackModel() {
    // Create a simple cube mesh
    Mesh mesh = GenMeshCube(0.1f, 0.1f, 0.1f);
    fallbackModel = LoadModelFromMesh(mesh);
    
    // Apply magenta texture for easy identification
    if (g_TextureManager) {
        fallbackModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = 
            g_TextureManager->GetFallbackTexture();
    }
}

Model ModelManager::CreateProceduralModel(ModelID id) {
    Mesh mesh;
    
    switch (id) {
        case MODEL_PISTOL:
        case MODEL_M16:
            mesh = GenMeshCube(0.05f, 0.05f, 0.15f);
            break;
            
        case MODEL_FLASHLIGHT:
            mesh = GenMeshCylinder(0.015f, 0.015f, 0.12f, 16);
            break;
            
        case MODEL_WATER_BOTTLE:
            mesh = GenMeshCylinder(0.03f, 0.03f, 0.15f, 12);
            break;
            
        case MODEL_LAB_KEY:
            mesh = GenMeshCube(0.01f, 0.05f, 0.02f);
            break;
            
        case MODEL_WOOD:
            mesh = GenMeshCube(0.05f, 0.05f, 0.08f);
            break;
            
        case MODEL_STONE:
            mesh = GenMeshSphere(0.03f, 8, 8);
            break;
            
        case MODEL_POTATO_CHIPS:
            mesh = GenMeshCube(0.06f, 0.08f, 0.03f);
            break;
            
        case MODEL_MAGAZINE:
        case MODEL_M16_MAGAZINE:
            mesh = GenMeshCube(0.03f, 0.05f, 0.06f);
            break;
            
        default:
            mesh = GenMeshCube(0.05f, 0.05f, 0.05f);
            break;
    }
    
    Model model = LoadModelFromMesh(mesh);
    
    // Apply textures
    ApplyTexturesToModel(model, id);
    
    return model;
}

Model ModelManager::GetModel(ModelID id) {
    if (models.find(id) != models.end() && models[id].model.meshCount > 0) {
        return models[id].model;
    }
    return fallbackModel;
}

const ModelData* ModelManager::GetModelData(ModelID id) {
    if (models.find(id) != models.end()) {
        return &models[id];
    }
    return nullptr;
}

bool ModelManager::IsLoaded(ModelID id) {
    return models.find(id) != models.end() && models[id].loaded;
}

void ModelManager::DrawModel(ModelID id, Vector3 position, Vector3 forward, Vector3 right, Vector3 up, Color tint) {
    const ModelData* data = GetModelData(id);
    if (!data) return;
    
    // Apply transforms
    Vector3 scaledPos = position;
    scaledPos = Vector3Add(scaledPos, Vector3Scale(forward, data->offset.z));
    scaledPos = Vector3Add(scaledPos, Vector3Scale(right, data->offset.x));
    scaledPos = Vector3Add(scaledPos, Vector3Scale(up, data->offset.y));
    
    // Calculate rotation matrix from forward, right, up vectors
    Matrix transform = MatrixIdentity();
    
    // Apply model rotation
    if (data->rotation.x != 0.0f) transform = MatrixMultiply(transform, MatrixRotateX(data->rotation.x * DEG2RAD));
    if (data->rotation.y != 0.0f) transform = MatrixMultiply(transform, MatrixRotateY(data->rotation.y * DEG2RAD));
    if (data->rotation.z != 0.0f) transform = MatrixMultiply(transform, MatrixRotateZ(data->rotation.z * DEG2RAD));
    
    // Apply orientation from camera vectors
    Matrix orientation = {
        right.x, right.y, right.z, 0.0f,
        up.x, up.y, up.z, 0.0f,
        forward.x, forward.y, forward.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    transform = MatrixMultiply(transform, orientation);
    
    // Apply scale
    transform = MatrixMultiply(MatrixScale(data->scale.x, data->scale.y, data->scale.z), transform);
    
    // Apply translation
    transform = MatrixMultiply(transform, MatrixTranslate(scaledPos.x, scaledPos.y, scaledPos.z));
    
    // Draw the model
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(transform));
    DrawModel(data->model, Vector3Zero(), 1.0f, tint);
    rlPopMatrix();
}

void ModelManager::Reload() {
    Unload();
    Initialize();
}

void ModelManager::Unload() {
    for (auto& pair : models) {
        if (pair.second.model.meshCount > 0) {
            UnloadModel(pair.second.model);
        }
    }
    models.clear();
    
    if (fallbackModel.meshCount > 0) {
        UnloadModel(fallbackModel);
        fallbackModel = { 0 };
    }
}

// Global initialization
void InitializeModelSystem() {
    g_ModelManager = new ModelManager();
    g_ModelManager->Initialize();
    TraceLog(LOG_INFO, "Model system initialized");
}

void CleanupModelSystem() {
    if (g_ModelManager) {
        delete g_ModelManager;
        g_ModelManager = nullptr;
    }
    TraceLog(LOG_INFO, "Model system cleaned up");
}