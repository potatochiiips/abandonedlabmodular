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
    MODEL_KNIFE,
    MODEL_CRYOPOD,
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

    void Initialize();
    Model GetModel(ModelID id);
    const ModelData* GetModelData(ModelID id);
    bool IsLoaded(ModelID id);
    void Reload();
    void Unload();
    void DrawModel(ModelID id, Vector3 position, Vector3 forward, Vector3 right, Vector3 up, Color tint = WHITE);

private:
    std::map<ModelID, ModelData> models;
    Model fallbackModel;

    void CreateFallbackModel();
    Vector3 CalculateAutoScale(const Model& model);
    bool LoadModelFile(ModelID id, const char* filename);
    Model CreateProceduralModel(ModelID id);
    void ApplyTexturesToModel(Model& model, ModelID id);
};

extern ModelManager* g_ModelManager;
void InitializeModelSystem();
void CleanupModelSystem();