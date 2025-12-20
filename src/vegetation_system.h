#pragma once
#include "globals.h"

// Vegetation stats
struct VegetationStats {
    VegetationType type;
    const char* name;
    float minScale;
    float maxScale;
    float density;
    int modelId;
    bool castsShadow;
    float windEffect; // 0.0 to 1.0
};

// Vegetation manager
class VegetationManager {
public:
    VegetationManager();
    ~VegetationManager();
    
    // Initialize vegetation system
    void Initialize();
    
    // Generate vegetation for entire world
    void GenerateVegetation(TerrainManager* terrain);
    
    // Update visible vegetation (LOD, culling)
    void Update(const Camera3D& camera);
    
    // Draw visible vegetation
    void Draw(const Camera3D& camera);
    
    // Clear all vegetation
    void ClearAll();
    
    // Cleanup
    void Unload();
    
    // Set vegetation density (0.0 to 1.0)
    void SetDensity(float density);
    
private:
    std::vector<VegetationInstance> instances;
    VegetationStats stats[VEG_TYPE_COUNT];
    float currentDensity;
    unsigned int vegetationSeed;
    
    // Initialize vegetation stats
    void InitializeStats();
    
    // Generate vegetation in area
    void GenerateInArea(float x, float z, float size, const BiomeData& biome, TerrainManager* terrain);
    
    // Check if should spawn vegetation at position
    bool ShouldSpawnVegetation(VegetationType type, float x, float z, const BiomeData& biome);
    
    // Draw single vegetation instance
    void DrawInstance(const VegetationInstance& instance, const Camera3D& camera);
    
    // Frustum culling check
    bool IsInFrustum(const VegetationInstance& instance, const Camera3D& camera);
};

// Global vegetation manager
extern VegetationManager* g_VegetationManager;

// Initialize vegetation system
void InitializeVegetationSystem();

// Cleanup vegetation system
void CleanupVegetationSystem();
