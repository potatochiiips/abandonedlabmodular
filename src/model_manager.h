#pragma once
#include "globals.h"
#include <map>
#include <string>

// Model IDs for different items
enum ModelID {
    MODEL_PISTOL,
    MODEL_M16,
    MODEL_FLASHLIGHT,
    MODEL_WATER_BOTTLE,
    MODEL_LAB_KEY,
    MODEL_WOOD,
    MODEL_STONE,
    MODEL_POTATO_CHIPS,
    MODEL_MAGAZINE,
    MODEL_M16_MAGAZINE,
    MODEL_COUNT
};

// Model data structure
struct ModelData {
    Model model;
    bool loaded;
    Vector3 scale;
    Vector3 offset;
    Vector3 rotation;
    std::string filename;
};

// Model manager class
class ModelManager {
public:
    ModelManager();
    ~ModelManager();
    
    // Initialize and load all models
    void Initialize();
    
    // Get a model by ID (returns fallback if missing)
    Model GetModel(ModelID id);
    
    // Get model data (includes transform info)
    const ModelData* GetModelData(ModelID id);
    
    // Check if a model is loaded
    bool IsLoaded(ModelID id);
    
    // Reload all models
    void Reload();
    
    // Unload all models
    void Unload();
    
    // Draw a model with proper transforms
    void DrawModel(ModelID id, Vector3 position, Vector3 forward, Vector3 right, Vector3 up, Color tint = WHITE);
    
private:
    std::map<ModelID, ModelData> models;
    Model fallbackModel;
    
    // Create fallback model (simple cube)
    void CreateFallbackModel();
    
    // Load individual model with error handling
    bool LoadModelFile(ModelID id, const char* filename, Vector3 scale = {1.0f, 1.0f, 1.0f}, 
                       Vector3 offset = {0.0f, 0.0f, 0.0f}, Vector3 rotation = {0.0f, 0.0f, 0.0f});
    
    // Create simple procedural model as fallback
    Model CreateProceduralModel(ModelID id);
    
    // Apply textures from texture manager
    void ApplyTexturesToModel(Model& model, ModelID id);
};

// Global model manager instance
extern ModelManager* g_ModelManager;

// Initialize model system
void InitializeModelSystem();

// Cleanup model system
void CleanupModelSystem();