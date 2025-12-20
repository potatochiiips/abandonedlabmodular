#pragma once
#include "globals.h"

// Skybox manager class
class SkyboxManager {
public:
    SkyboxManager();
    ~SkyboxManager();
    
    // Initialize and load skybox textures
    void Initialize();
    
    // Draw skybox around camera
    void Draw(const Camera3D& camera);
    
    // Reload skybox textures
    void Reload();
    
    // Cleanup
    void Unload();
    
    // Check if skybox is loaded
    bool IsLoaded() const { return skyboxLoaded; }
    
private:
    Texture2D skyboxTextures[6]; // right, left, top, bottom, front, back
    Mesh skyboxMesh;
    Material skyboxMaterial;
    bool skyboxLoaded;
    
    // Create procedural skybox if textures not found
    void CreateProceduralSkybox();
    
    // Load individual skybox face
    bool LoadSkyboxFace(int index, const char* filename);
};

// Global skybox manager
extern SkyboxManager* g_SkyboxManager;

// Initialize skybox system
void InitializeSkyboxSystem();

// Cleanup skybox system
void CleanupSkyboxSystem();
