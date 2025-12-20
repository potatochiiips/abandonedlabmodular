#include "terrain_system.h"
#include "texture_manager.h"
#include <cstdlib>
#include <cmath>

// Global instance
TerrainManager* g_TerrainManager = nullptr;

// Perlin noise implementation
PerlinNoise::PerlinNoise() {
    // Initialize permutation table
    for (int i = 0; i < 256; i++) p[i] = i;
    for (int i = 255; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = p[i];
        p[i] = p[j];
        p[j] = temp;
    }
    for (int i = 0; i < 256; i++) p[256 + i] = p[i];
}

PerlinNoise::PerlinNoise(unsigned int seed) {
    srand(seed);
    for (int i = 0; i < 256; i++) p[i] = i;
    for (int i = 255; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = p[i];
        p[i] = p[j];
        p[j] = temp;
    }
    for (int i = 0; i < 256; i++) p[256 + i] = p[i];
}

float PerlinNoise::Noise(float x, float y) const {
    // Standard 2D Perlin implementation
    int X = (int)floorf(x) & 255;
    int Y = (int)floorf(y) & 255;
    
    x -= floorf(x);
    y -= floorf(y);
    
    float u = Fade(x);
    float v = Fade(y);
    
    int A = p[X] + Y;
    int B = p[X+1] + Y;
    
    return Lerp(v, 
        Lerp(u, Grad(p[A], x, y, 0), Grad(p[B], x-1, y, 0)),
        Lerp(u, Grad(p[A+1], x, y-1, 0), Grad(p[B+1], x-1, y-1, 0))
    );
}

float PerlinNoise::Fade(float t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::Lerp(float t, float a, float b) const {
    return a + t * (b - a);
}

float PerlinNoise::Grad(int hash, float x, float y, float z) const {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// TerrainManager implementation
TerrainManager::TerrainManager() : seed(0) {
    lastCameraPos = Vector3{ 0, 0, 0 };
}

TerrainManager::~TerrainManager() {
    Unload();
}

void TerrainManager::Initialize(unsigned int seed) {
    this->seed = seed;
    heightNoise = PerlinNoise(seed);
    moistureNoise = PerlinNoise(seed + 1);
    temperatureNoise = PerlinNoise(seed + 2);
    
    TraceLog(LOG_INFO, "Terrain Manager initialized with seed %u", seed);
}

void TerrainManager::GenerateTerrain() {
    TraceLog(LOG_INFO, "Generating terrain...");
    
    for (int z = 0; z < TERRAIN_CHUNKS_Z; z++) {
        for (int x = 0; x < TERRAIN_CHUNKS_X; x++) {
            GenerateChunk(x, z);
        }
    }
    
    TraceLog(LOG_INFO, "Terrain generation complete");
}

void TerrainManager::GenerateChunk(int chunkX, int chunkZ) {
    TerrainChunk& chunk = chunks[chunkZ][chunkX];
    chunk.chunkX = chunkX;
    chunk.chunkZ = chunkZ;
    chunk.position = Vector3{ 
        (float)(chunkX * TERRAIN_CHUNK_SIZE), 
        0.0f, 
        (float)(chunkZ * TERRAIN_CHUNK_SIZE) 
    };
    
    // Generate heights using multiple octaves
    for (int z = 0; z <= TERRAIN_CHUNK_SIZE; z++) {
        for (int x = 0; x <= TERRAIN_CHUNK_SIZE; x++) {
            float worldX = (chunkX * TERRAIN_CHUNK_SIZE + x) * 0.05f;
            float worldZ = (chunkZ * TERRAIN_CHUNK_SIZE + z) * 0.05f;
            
            // Multiple octaves for realistic terrain
            float height = 0.0f;
            float amplitude = 1.0f;
            float frequency = 1.0f;
            
            for (int octave = 0; octave < 6; octave++) {
                height += heightNoise.Noise(worldX * frequency, worldZ * frequency) * amplitude;
                amplitude *= 0.5f;
                frequency *= 2.0f;
            }
            
            // Scale and offset
            height = height * TERRAIN_HEIGHT_SCALE + TERRAIN_BASE_HEIGHT;
            
            // Clamp to reasonable values
            height = fmaxf(0.0f, fminf(100.0f, height));
            
            chunk.heights[z][x] = height;
        }
    }
    
    // Generate mesh
    chunk.mesh = GenerateChunkMesh(chunkX, chunkZ);
    chunk.model = LoadModelFromMesh(chunk.mesh);
    chunk.loaded = true;
    chunk.visible = true;
}

Mesh TerrainManager::GenerateChunkMesh(int chunkX, int chunkZ) {
    const TerrainChunk& chunk = chunks[chunkZ][chunkX];
    const int size = TERRAIN_CHUNK_SIZE;
    
    // Calculate vertices
    int vertexCount = (size + 1) * (size + 1);
    int triangleCount = size * size * 2;
    
    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    
    mesh.vertices = (float*)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short*)MemAlloc(triangleCount * 3 * sizeof(unsigned short));
    
    // Generate vertices
    int vIndex = 0;
    for (int z = 0; z <= size; z++) {
        for (int x = 0; x <= size; x++) {
            mesh.vertices[vIndex * 3 + 0] = (float)x;
            mesh.vertices[vIndex * 3 + 1] = chunk.heights[z][x];
            mesh.vertices[vIndex * 3 + 2] = (float)z;
            
            mesh.texcoords[vIndex * 2 + 0] = (float)x / size;
            mesh.texcoords[vIndex * 2 + 1] = (float)z / size;
            
            vIndex++;
        }
    }
    
    // Calculate normals
    for (int i = 0; i < vertexCount * 3; i++) mesh.normals[i] = 0.0f;
    
    // Generate indices and calculate normals
    int tIndex = 0;
    for (int z = 0; z < size; z++) {
        for (int x = 0; x < size; x++) {
            int topLeft = z * (size + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (size + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            // Triangle 1
            mesh.indices[tIndex++] = topLeft;
            mesh.indices[tIndex++] = bottomLeft;
            mesh.indices[tIndex++] = topRight;
            
            // Triangle 2
            mesh.indices[tIndex++] = topRight;
            mesh.indices[tIndex++] = bottomLeft;
            mesh.indices[tIndex++] = bottomRight;
        }
    }
    
    // Upload mesh
    UploadMesh(&mesh, false);
    
    return mesh;
}

float TerrainManager::GetHeightAt(float worldX, float worldZ) {
    int chunkX = (int)(worldX / TERRAIN_CHUNK_SIZE);
    int chunkZ = (int)(worldZ / TERRAIN_CHUNK_SIZE);
    
    if (chunkX < 0 || chunkX >= TERRAIN_CHUNKS_X || chunkZ < 0 || chunkZ >= TERRAIN_CHUNKS_Z) {
        return 0.0f;
    }
    
    TerrainChunk& chunk = chunks[chunkZ][chunkX];
    
    float localX = worldX - (chunkX * TERRAIN_CHUNK_SIZE);
    float localZ = worldZ - (chunkZ * TERRAIN_CHUNK_SIZE);
    
    int x0 = (int)floorf(localX);
    int z0 = (int)floorf(localZ);
    int x1 = x0 + 1;
    int z1 = z0 + 1;
    
    if (x1 > TERRAIN_CHUNK_SIZE) x1 = TERRAIN_CHUNK_SIZE;
    if (z1 > TERRAIN_CHUNK_SIZE) z1 = TERRAIN_CHUNK_SIZE;
    
    float fx = localX - x0;
    float fz = localZ - z0;
    
    // Bilinear interpolation
    float h00 = chunk.heights[z0][x0];
    float h10 = chunk.heights[z0][x1];
    float h01 = chunk.heights[z1][x0];
    float h11 = chunk.heights[z1][x1];
    
    float h0 = h00 * (1 - fx) + h10 * fx;
    float h1 = h01 * (1 - fx) + h11 * fx;
    
    return h0 * (1 - fz) + h1 * fz;
}

BiomeData TerrainManager::GetBiomeAt(float worldX, float worldZ) {
    BiomeData biome;
    
    biome.elevation = GetHeightAt(worldX, worldZ) / TERRAIN_HEIGHT_SCALE;
    biome.temperature = temperatureNoise.Noise(worldX * 0.01f, worldZ * 0.01f);
    biome.moisture = moistureNoise.Noise(worldX * 0.02f, worldZ * 0.02f);
    
    biome.groundColor = CalculateBiomeColor(biome);
    
    // Calculate densities based on biome
    if (biome.elevation > 0.7f) {
        // Mountains - sparse vegetation
        biome.treeDensity = 0.1f;
        biome.rockDensity = 0.5f;
    } else if (biome.moisture > 0.6f && biome.temperature > 0.0f) {
        // Forest
        biome.treeDensity = 0.8f;
        biome.rockDensity = 0.1f;
    } else {
        // Plains
        biome.treeDensity = 0.3f;
        biome.rockDensity = 0.2f;
    }
    
    return biome;
}

Color TerrainManager::CalculateBiomeColor(const BiomeData& biome) {
    if (biome.elevation > 0.8f) {
        // Mountain peaks - gray/white
        return Color{ 180, 180, 185, 255 };
    } else if (biome.elevation > 0.6f) {
        // Rocky hills - brown/gray
        return Color{ 120, 100, 80, 255 };
    } else if (biome.moisture > 0.6f) {
        // Lush - green
        return Color{ 50, 140, 50, 255 };
    } else if (biome.moisture > 0.3f) {
        // Grassland - light green
        return Color{ 100, 160, 80, 255 };
    } else {
        // Dry - tan/brown
        return Color{ 160, 140, 100, 255 };
    }
}

void TerrainManager::Update(const Camera3D& camera) {
    UpdateChunkVisibility(camera);
}

void TerrainManager::UpdateChunkVisibility(const Camera3D& camera) {
    // Simple distance-based visibility
    float viewDist = 500.0f; // View distance in meters
    
    for (int z = 0; z < TERRAIN_CHUNKS_Z; z++) {
        for (int x = 0; x < TERRAIN_CHUNKS_X; x++) {
            TerrainChunk& chunk = chunks[z][x];
            
            Vector3 chunkCenter = Vector3{
                chunk.position.x + TERRAIN_CHUNK_SIZE * 0.5f,
                0.0f,
                chunk.position.z + TERRAIN_CHUNK_SIZE * 0.5f
            };
            
            float dist = Vector3Distance(camera.position, chunkCenter);
            chunk.visible = (dist < viewDist);
        }
    }
}

void TerrainManager::Draw() {
    for (int z = 0; z < TERRAIN_CHUNKS_Z; z++) {
        for (int x = 0; x < TERRAIN_CHUNKS_X; x++) {
            TerrainChunk& chunk = chunks[z][x];
            
            if (!chunk.loaded || !chunk.visible) continue;
            
            DrawModel(chunk.model, chunk.position, 1.0f, WHITE);
        }
    }
}

void TerrainManager::Unload() {
    for (int z = 0; z < TERRAIN_CHUNKS_Z; z++) {
        for (int x = 0; x < TERRAIN_CHUNKS_X; x++) {
            TerrainChunk& chunk = chunks[z][x];
            if (chunk.loaded) {
                UnloadModel(chunk.model);
                chunk.loaded = false;
            }
        }
    }
}

TerrainChunk* TerrainManager::GetChunkAt(int chunkX, int chunkZ) {
    if (chunkX >= 0 && chunkX < TERRAIN_CHUNKS_X && chunkZ >= 0 && chunkZ < TERRAIN_CHUNKS_Z) {
        return &chunks[chunkZ][chunkX];
    }
    return nullptr;
}

void InitializeTerrainSystem(unsigned int seed) {
    g_TerrainManager = new TerrainManager();
    g_TerrainManager->Initialize(seed);
    g_TerrainManager->GenerateTerrain();
    TraceLog(LOG_INFO, "Terrain system initialized");
}

void CleanupTerrainSystem() {
    if (g_TerrainManager) {
        delete g_TerrainManager;
        g_TerrainManager = nullptr;
    }
    TraceLog(LOG_INFO, "Terrain system cleaned up");
}