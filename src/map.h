#pragma once
#include "globals.h"
#include "rendering.h"
#include <vector>
#include <memory>

class UpgradedMapRenderer {
public:
    UpgradedMapRenderer();
    ~UpgradedMapRenderer();
    
    void Initialize();
    void GenerateWorldGeometry(const MapData& mapData);
    void GenerateInteriorGeometry(const Interior& interior);
    void Update(float deltaTime, const Camera3D& camera);
    void Cleanup();
    
    // Get all renderers for the pipeline
    std::vector<MeshRenderer*> GetActiveRenderers();
    
    // Dynamic updates
    void UpdateDoor(int doorId, float openProgress);
    void AddProp(Vector3 position, ModelID modelId, float scale = 1.0f);
    void RemoveProp(int propId);
    
private:
    // Render components
    std::vector<std::unique_ptr<MeshRenderer>> worldRenderers;
    std::vector<std::unique_ptr<MeshRenderer>> interiorRenderers;
    std::vector<std::unique_ptr<MeshRenderer>> propRenderers;
    std::vector<std::unique_ptr<MeshRenderer>> doorRenderers;
    
    // Materials cache
    std::shared_ptr<UpgradedMaterial> wallMaterial;
    std::shared_ptr<UpgradedMaterial> floorMaterial;
    std::shared_ptr<UpgradedMaterial> doorMaterial;
    std::shared_ptr<UpgradedMaterial> concreteMaterial;
    std::shared_ptr<UpgradedMaterial> grassMaterial;
    std::shared_ptr<UpgradedMaterial> waterMaterial;
    std::shared_ptr<UpgradedMaterial> glassMaterial;
    
    // Geometry generation
    void CreateWall(Vector3 position, TextureID texture);
    void CreateFloor(Vector3 position, float size, TextureID texture);
    void CreateBuilding(const Building& building);
    void CreateDoor(const Door& door);
    void CreateWaterTile(Vector3 position);
    void CreateRoadTile(Vector3 position);
    
    // Material creation
    void InitializeMaterials();
    std::shared_ptr<UpgradedMaterial> CreateOpaqueMaterial(TextureID texture, Color tint);
    std::shared_ptr<UpgradedMaterial> CreateTransparentMaterial(TextureID texture, Color tint);
    
    // Culling and LOD
    void UpdateVisibility(const Camera3D& camera);
    
    int nextPropId;
};
