#pragma once
#include "globals.h"
#include <map>
#include <string>

// Texture IDs for different surface types
enum TextureID {
    TEX_WALL_CONCRETE,
    TEX_WALL_BRICK,
    TEX_WALL_METAL,
    TEX_FLOOR_TILE,
    TEX_FLOOR_CONCRETE,
    TEX_FLOOR_WOOD,
    TEX_FLOOR_CARPET,
    TEX_CEILING_TILE,
    TEX_DOOR_WOOD,
    TEX_DOOR_METAL,
    TEX_ROAD_ASPHALT,
    TEX_GRASS,
    TEX_DIRT,
    TEX_TREE_BARK,
    TEX_TREE_LEAVES,
    TEX_BUILDING_EXTERIOR,
    TEX_WINDOW_GLASS,
    TEX_ROOF_SHINGLES,
    TEX_BUSH_LEAVES,
    TEX_FLOWER_PETALS,
    TEX_SKY,
    TEX_COUNT
};

// Texture manager class
class TextureManager {
public:
    TextureManager();
    ~TextureManager();
    
    // Initialize and load all textures
    void Initialize();
    
    // Get a texture by ID (returns fallback if missing)
    Texture2D GetTexture(TextureID id);
    
    // Check if a texture is loaded
    bool IsLoaded(TextureID id);
    
    // Reload all textures
    void Reload();
    
    // Unload all textures
    void Unload();
    
    // Get fallback texture
    Texture2D GetFallbackTexture();
    
private:
    std::map<TextureID, Texture2D> textures;
    std::map<TextureID, bool> loadedStatus;
    Texture2D fallbackTexture;
    
    // Create procedural fallback texture
    void CreateFallbackTexture();
    
    // Load individual texture with error handling
    bool LoadTextureFile(TextureID id, const char* filename);
    
    // Create simple procedural texture as fallback for specific types
    Texture2D CreateProceduralTexture(TextureID id);
};

// Global texture manager instance
extern TextureManager* g_TextureManager;

// Shader manager class
class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();
    
    // Initialize and load shaders
    void Initialize();
    
    // Get the lighting shader
    Shader GetLightingShader();
    
    // Update shader uniforms
    void UpdateLighting(const Camera3D& camera, Vector3 lightPos, bool flashlightOn, Vector3 flashlightPos, Vector3 flashlightDir, float flashlightIntensity);
    
    // Unload shaders
    void Unload();
    
private:
    Shader lightingShader;
    bool shaderLoaded;
    
    // Shader uniform locations
    int viewPosLoc;
    int lightPosLoc;
    int lightColorLoc;
    int lightIntensityLoc;
    int ambientColorLoc;
    int ambientIntensityLoc;
    int flashlightPosLoc;
    int flashlightDirLoc;
    int flashlightColorLoc;
    int flashlightIntensityLoc;
    int flashlightCutoffLoc;
    int flashlightOuterCutoffLoc;
    int flashlightEnabledLoc;
    int fogColorLoc;
    int fogDensityLoc;
    int fogStartLoc;
    int fogEndLoc;
};

// Global shader manager instance
extern ShaderManager* g_ShaderManager;

// Initialize both managers
void InitializeRenderingSystems();

// Cleanup both managers
void CleanupRenderingSystems();