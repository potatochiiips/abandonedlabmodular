#pragma once
#include "globals.h"

// Perlin noise implementation for terrain generation
class PerlinNoise {
public:
    PerlinNoise();
    PerlinNoise(unsigned int seed);
    
    float Noise(float x, float y, float z) const;
    float Noise(float x, float y) const;
    
private:
    int p[512];
    
    float Fade(float t) const;
    float Lerp(float t, float a, float b) const;
    float Grad(int hash, float x, float y, float z) const;
};

// Terrain generation and management
class TerrainManager {
public:
    TerrainManager();
    ~TerrainManager();
    
    // Initialize terrain system
    void Initialize(unsigned int seed = 12345);
    
    // Generate all terrain chunks
    void GenerateTerrain();
    
    // Update visible chunks based on camera position
    void Update(const Camera3D& camera);
    
    // Draw visible terrain chunks
    void Draw();
    
    // Get height at world position
    float GetHeightAt(float worldX, float worldZ);
    
    // Get biome data at position
    BiomeData GetBiomeAt(float worldX, float worldZ);
    
    // Cleanup
    void Unload();
    
    // Get terrain chunk for collision
    TerrainChunk* GetChunkAt(int chunkX, int chunkZ);
    
private:
    TerrainChunk chunks[TERRAIN_CHUNKS_X][TERRAIN_CHUNKS_Z];
    PerlinNoise heightNoise;
    PerlinNoise moistureNoise;
    PerlinNoise temperatureNoise;
    unsigned int seed;
    Vector3 lastCameraPos;
    
    // Generate single terrain chunk
    void GenerateChunk(int chunkX, int chunkZ);
    
    // Create mesh for chunk
    Mesh GenerateChunkMesh(int chunkX, int chunkZ);
    
    // Update chunk visibility
    void UpdateChunkVisibility(const Camera3D& camera);
    
    // Calculate biome color
    Color CalculateBiomeColor(const BiomeData& biome);
};

// Global terrain manager
extern TerrainManager* g_TerrainManager;

// Initialize terrain system
void InitializeTerrainSystem(unsigned int seed = 12345);

// Cleanup terrain system
void CleanupTerrainSystem();
