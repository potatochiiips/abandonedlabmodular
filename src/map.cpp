#include "globals.h"
#include "map.h"
#include "texture_manager.h"
#include "model_manager.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include <rlgl.h>
#include "rendering.h"
#include "enhanced_map_system.h"

// Global instance definitions
UpgradedMapRenderer* g_UpgradedMapRenderer = nullptr;

#define MAP_WIDTH MAP_SIZE
#define MAP_HEIGHT MAP_SIZE
#define WALL_HEIGHT 3.0f
#define DOOR_HEIGHT 2.5f
#define CEILING_HEIGHT 3.0f
#define FLOOR_HEIGHT 4.0f

UpgradedMapRenderer::UpgradedMapRenderer() : nextPropId(1) {
}

UpgradedMapRenderer::~UpgradedMapRenderer() {
    Cleanup();
}

void UpgradedMapRenderer::GenerateEnhancedWorld() {
    TraceLog(LOG_INFO, "Generating Game world with seed: %u", g_WorldSettings.seed);

    worldRenderers.clear();

    CreateTerrainLayer();
    if (g_WorldSettings.generateWater) CreateWaterFeatures();
    if (g_WorldSettings.generateVegetation) CreateDecorativeElements();

    TraceLog(LOG_INFO, "Enhanced world generated: %d renderers", (int)worldRenderers.size());
}

void UpgradedMapRenderer::CreateTerrainLayer() {
    TraceLog(LOG_INFO, "Creating Game terrain...");

    int terrainGridSize = 64;
    float tileSize = 512.0f / terrainGridSize;

    for (int z = 0; z < terrainGridSize; z++) {
        for (int x = 0; x < terrainGridSize; x++) {
            float worldX = x * tileSize;
            float worldZ = z * tileSize;

            float noiseVal = GetNoiseValue(worldX / 100.0f, worldZ / 100.0f, g_WorldSettings);
            float temperatureNoise = GetNoiseValue(worldX / 200.0f + 1000, worldZ / 200.0f, g_WorldSettings);
            float humidityNoise = GetNoiseValue(worldX / 200.0f, worldZ / 200.0f + 1000, g_WorldSettings);

            BiomeType biome = GetBiomeFromNoise(noiseVal, temperatureNoise, humidityNoise);
            BiomeInfo biomeInfo = GetBiomeInfo(biome);

            auto terrainRenderer = std::make_unique<MeshRenderer>();
            terrainRenderer->mesh = GenMeshPlane(tileSize, tileSize, 4, 4);
            // FIXED: MatrixTranslate requires 3 parameters (x, y, z)
            terrainRenderer->transform = MatrixTranslate(worldX, noiseVal * g_WorldSettings.scale, worldZ);
            terrainRenderer->castShadows = false;
            terrainRenderer->receiveShadows = true;
            terrainRenderer->enabled = true;
            terrainRenderer->tint = biomeInfo.groundColor;

            worldRenderers.push_back(std::move(terrainRenderer));
        }
    }

    TraceLog(LOG_INFO, "Terrain layer created with %d tiles", terrainGridSize * terrainGridSize);
}

void UpgradedMapRenderer::CreateBiomes() {
    // Biomes are generated procedurally in CreateTerrainLayer
}

void UpgradedMapRenderer::CreateRoadNetwork() {
    // Roads can be added later if needed
}

void UpgradedMapRenderer::CreateBuildingsClusteredByZone() {
    TraceLog(LOG_INFO, "Creating clustered buildings...");

    if (!g_EnhancedMapSystem) {
        TraceLog(LOG_WARNING, "Enhanced map system not available for building generation");
        return;
    }

    int totalBuildings = 0;
    for (int zone = ZONE_CENTRAL_CITY; zone <= ZONE_HARBOR; zone++) {
        auto zoneBuildings = g_EnhancedMapSystem->GetBuildingsInZone((WorldZone)zone);

        for (const auto* building : zoneBuildings) {
            if (!building) continue;

            auto buildingRenderer = std::make_unique<MeshRenderer>();
            buildingRenderer->mesh = GenMeshCube(building->size.x, building->size.y, building->size.z);
            buildingRenderer->transform = MatrixTranslate(building->position.x, building->size.y / 2.0f, building->position.z);
            buildingRenderer->castShadows = true;
            buildingRenderer->receiveShadows = true;
            buildingRenderer->enabled = true;

            // Color by building type
            switch (building->type) {
                case BLDG_SKYSCRAPER: buildingRenderer->tint = Color{ 80, 80, 100, 255 }; break;
                case BLDG_LABORATORY: buildingRenderer->tint = Color{ 120, 140, 160, 255 }; break;
                case BLDG_HOSPITAL: buildingRenderer->tint = Color{ 210, 210, 210, 255 }; break;
                case BLDG_OFFICE_BUILDING: buildingRenderer->tint = Color{ 110, 110, 130, 255 }; break;
                case BLDG_WAREHOUSE: buildingRenderer->tint = Color{ 100, 100, 110, 255 }; break;
                case BLDG_FACTORY: buildingRenderer->tint = Color{ 70, 70, 90, 255 }; break;
                default: buildingRenderer->tint = Color{ 120, 120, 120, 255 }; break;
            }

            worldRenderers.push_back(std::move(buildingRenderer));

            // Add roof
            auto roofRenderer = std::make_unique<MeshRenderer>();
            roofRenderer->mesh = GenMeshCube(building->size.x * 1.05f, 0.3f, building->size.z * 1.05f);
            roofRenderer->transform = MatrixTranslate(building->position.x, building->size.y + 0.15f, building->position.z);
            roofRenderer->castShadows = true;
            roofRenderer->receiveShadows = true;
            roofRenderer->tint = Color{ 60, 40, 30, 255 };

            worldRenderers.push_back(std::move(roofRenderer));
            totalBuildings++;
        }
    }

    TraceLog(LOG_INFO, "Created %d buildings", totalBuildings);
}

void UpgradedMapRenderer::CreateWaterFeatures() {
    TraceLog(LOG_INFO, "Creating water features...");

    int waterGridSize = 32;
    float tileSize = 512.0f / waterGridSize;

    for (int z = 0; z < waterGridSize; z++) {
        for (int x = 0; x < waterGridSize; x++) {
            float worldX = x * tileSize;
            float worldZ = z * tileSize;

            float noiseVal = GetNoiseValue(worldX / 100.0f, worldZ / 100.0f, g_WorldSettings);

            if (noiseVal < 0.25f) {
                auto waterRenderer = std::make_unique<MeshRenderer>();
                waterRenderer->mesh = GenMeshPlane(tileSize, tileSize, 2, 2);
                // FIXED: MatrixTranslate requires 3 parameters (x, y, z)
                waterRenderer->transform = MatrixTranslate(worldX, noiseVal * g_WorldSettings.scale + 0.5f, worldZ);
                waterRenderer->castShadows = false;
                waterRenderer->receiveShadows = false;
                waterRenderer->tint = Color{ 20, 80, 140, 200 };

                worldRenderers.push_back(std::move(waterRenderer));
            }
        }
    }

    TraceLog(LOG_INFO, "Water features created");
}

void UpgradedMapRenderer::CreateDecorativeElements() {
    TraceLog(LOG_INFO, "Creating decorative elements...");

    int decorCount = 50;
    for (int i = 0; i < decorCount; i++) {
        float x = (float)(rand() % 512);
        float z = (float)(rand() % 512);
        float noiseVal = GetNoiseValue(x / 100.0f, z / 100.0f, g_WorldSettings);

        if (noiseVal > 0.3f && noiseVal < 0.7f) {
            Vector3 propPos = { x, noiseVal * g_WorldSettings.scale + 2.0f, z };

            if (g_ModelManager && g_ModelManager->IsLoaded(MODEL_CRATE)) {
                AddProp(propPos, MODEL_CRATE, 0.6f);
            }
        }
    }

    TraceLog(LOG_INFO, "Decorative elements created");
}

void UpgradedMapRenderer::Initialize() {
    TraceLog(LOG_INFO, "Initializing Map Renderer...");
    InitializeMaterials();
}

void UpgradedMapRenderer::InitializeMaterials() {
    TraceLog(LOG_INFO, "Map Renderer initialized");
}

std::shared_ptr<UpgradedMaterial> UpgradedMapRenderer::CreateOpaqueMaterial(TextureID texture, Color tint) {
    return nullptr;
}

std::shared_ptr<UpgradedMaterial> UpgradedMapRenderer::CreateTransparentMaterial(TextureID texture, Color tint) {
    return nullptr;
}

void UpgradedMapRenderer::GenerateWorldGeometry(const MapData& mapData) {
    TraceLog(LOG_INFO, "Generating world geometry from map data...");
    worldRenderers.clear();
    TraceLog(LOG_INFO, "World geometry cleared (using enhanced world generation)");
}

void UpgradedMapRenderer::GenerateInteriorGeometry(const Interior& interior) {
    TraceLog(LOG_INFO, "Generating interior geometry for: %s", interior.id.c_str());

    interiorRenderers.clear();

    for (int y = 0; y < interior.height; y++) {
        for (int x = 0; x < interior.width; x++) {
            int tile = interior.tiles[y * interior.width + x];
            Vector3 pos = { (float)x, 0.0f, (float)y };

            // Floor
            if (tile != IT_EMPTY) {
                auto floorRenderer = std::make_unique<MeshRenderer>();
                floorRenderer->mesh = GenMeshCube(1.0f, 0.1f, 1.0f);
                Vector3 floorPos = pos;
                floorPos.y = 0.05f;
                floorRenderer->transform = MatrixTranslate(floorPos.x, floorPos.y, floorPos.z);
                floorRenderer->castShadows = false;
                floorRenderer->receiveShadows = true;
                floorRenderer->tint = Color{ 120, 120, 120, 255 };
                interiorRenderers.push_back(std::move(floorRenderer));
            }

            // Walls
            if (tile == IT_WALL) {
                auto wallRenderer = std::make_unique<MeshRenderer>();
                wallRenderer->mesh = GenMeshCube(1.0f, WALL_HEIGHT, 1.0f);
                Vector3 wallPos = pos;
                wallPos.y = WALL_HEIGHT / 2.0f;
                wallRenderer->transform = MatrixTranslate(wallPos.x, wallPos.y, wallPos.z);
                wallRenderer->castShadows = true;
                wallRenderer->receiveShadows = true;
                wallRenderer->tint = Color{ 180, 180, 180, 255 };
                interiorRenderers.push_back(std::move(wallRenderer));
            }

            // Props using model manager
            if (g_ModelManager) {
                Vector3 propPos = { (float)x, 1.0f, (float)y };
                ModelID modelId = MODEL_PISTOL;
                bool shouldDraw = false;

                switch (tile) {
                case IT_CRYOPOD_BROKEN: modelId = MODEL_CRYOPOD; shouldDraw = true; break;
                case IT_CONSOLE: modelId = MODEL_CONSOLE_TERMINAL; shouldDraw = true; break;
                case IT_BED: modelId = MODEL_BED; shouldDraw = true; break;
                case IT_DESK: modelId = MODEL_DESK; shouldDraw = true; break;
                case IT_CHAIR: modelId = MODEL_CHAIR; shouldDraw = true; break;
                case IT_TABLE: modelId = MODEL_TABLE; shouldDraw = true; break;
                case IT_SHELF: modelId = MODEL_SHELF; shouldDraw = true; break;
                case IT_LOCKER: modelId = MODEL_LOCKER; shouldDraw = true; break;
                case IT_CABINET: modelId = MODEL_CABINET; shouldDraw = true; break;
                case IT_CRATE: modelId = MODEL_CRATE; shouldDraw = true; break;
                }

                if (shouldDraw && g_ModelManager->IsLoaded(modelId)) {
                    AddProp(propPos, modelId, 1.0f);
                }
            }
        }
    }

    // Ceiling
    auto ceilingRenderer = std::make_unique<MeshRenderer>();
    ceilingRenderer->mesh = GenMeshCube((float)interior.width, 0.1f, (float)interior.height);
    ceilingRenderer->transform = MatrixTranslate(interior.width / 2.0f, CEILING_HEIGHT, interior.height / 2.0f);
    ceilingRenderer->castShadows = false;
    ceilingRenderer->receiveShadows = false;
    ceilingRenderer->tint = Color{ 200, 200, 200, 255 };
    interiorRenderers.push_back(std::move(ceilingRenderer));

    TraceLog(LOG_INFO, "Interior geometry generated: %d renderers", (int)interiorRenderers.size());
}

void UpgradedMapRenderer::CreateFloor(Vector3 position, float size, TextureID texture) {
    // Stub for compatibility
}

void UpgradedMapRenderer::CreateWall(Vector3 position, TextureID texture) {
    // Stub for compatibility
}

void UpgradedMapRenderer::CreateWaterTile(Vector3 position) {
    // Stub for compatibility
}

void UpgradedMapRenderer::CreateRoadTile(Vector3 position) {
    // Stub for compatibility
}

void UpgradedMapRenderer::CreateDoor(const Door& door) {
    // Stub for compatibility
}

void UpgradedMapRenderer::CreateBuilding(const Building& building) {
    // Stub for compatibility
}

void UpgradedMapRenderer::AddProp(Vector3 position, ModelID modelId, float scale) {
    if (!g_ModelManager || !g_ModelManager->IsLoaded(modelId)) return;

    Model model = g_ModelManager->GetModel(modelId);
    if (model.meshCount == 0) return;

    auto propRenderer = std::make_unique<MeshRenderer>();
    propRenderer->mesh = model.meshes[0];

    Matrix scaleMatrix = MatrixScale(scale, scale, scale);
    Matrix translation = MatrixTranslate(position.x, position.y, position.z);
    propRenderer->transform = MatrixMultiply(scaleMatrix, translation);

    propRenderer->castShadows = true;
    propRenderer->receiveShadows = true;
    propRenderer->enabled = true;
    propRenderer->tint = WHITE;

    propRenderers.push_back(std::move(propRenderer));
}

void UpgradedMapRenderer::Cleanup() {
    worldRenderers.clear();
    interiorRenderers.clear();
    propRenderers.clear();
    doorRenderers.clear();
}

std::vector<MeshRenderer*> UpgradedMapRenderer::GetActiveRenderers() {
    std::vector<MeshRenderer*> active;

    if (g_MapPlayer.insideInterior) {
        for (auto& renderer : interiorRenderers) {
            active.push_back(renderer.get());
        }
        for (auto& renderer : propRenderers) {
            active.push_back(renderer.get());
        }
    } else {
        for (auto& renderer : worldRenderers) {
            active.push_back(renderer.get());
        }
    }

    for (auto& renderer : doorRenderers) {
        active.push_back(renderer.get());
    }

    return active;
}

void UpgradedMapRenderer::Update(float deltaTime, const Camera3D& camera) {
    UpdateVisibility(camera);
}

void UpgradedMapRenderer::UpdateVisibility(const Camera3D& camera) {
    Vector3 camPos = camera.position;
    float viewDistance = 200.0f;

    auto checkVisibility = [&](std::unique_ptr<MeshRenderer>& renderer) {
        Vector3 objPos = { renderer->transform.m12, renderer->transform.m13, renderer->transform.m14 };
        float distance = Vector3Distance(camPos, objPos);
        renderer->enabled = (distance < viewDistance);
    };

    for (auto& renderer : worldRenderers) checkVisibility(renderer);
    for (auto& renderer : interiorRenderers) checkVisibility(renderer);
    for (auto& renderer : propRenderers) checkVisibility(renderer);
}

void UpgradedMapRenderer::UpdateDoor(int doorId, float openProgress) {
    // Door animation implementation
}

void UpgradedMapRenderer::RemoveProp(int propId) {
    // Prop removal implementation
}

// Noise generation functions
static float Hash(uint32_t n) {
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 789221U) + 1376312589U;
    return 1.0f - ((float)(n & 0x7fffffffU) / 1073741824.0f);
}

static float LerpNoise(float a, float b, float t) {
    return a + (b - a) * (3.0f - t * 2.0f) * t * t;
}

static float PerlinNoise(float x, float y, uint32_t seed) {
    int xi = (int)floorf(x);
    int yi = (int)floorf(y);
    float xf = x - xi;
    float yf = y - yi;

    float n00 = Hash(seed + xi + yi * 73856093U);
    float n10 = Hash(seed + (xi + 1) + yi * 73856093U);
    float n01 = Hash(seed + xi + (yi + 1) * 73856093U);
    float n11 = Hash(seed + (xi + 1) + (yi + 1) * 73856093U);

    float nx0 = LerpNoise(n00, n10, xf);
    float nx1 = LerpNoise(n01, n11, xf);
    return LerpNoise(nx0, nx1, yf);
}

static float GetNoiseValue(float x, float y, const WorldSettings& settings) {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = settings.frequency;
    float maxValue = 0.0f;

    for (int i = 0; i < settings.octaves; i++) {
        value += PerlinNoise(x * frequency, y * frequency, settings.seed + i) * amplitude;
        maxValue += amplitude;
        amplitude *= settings.persistence;
        frequency *= settings.lacunarity;
    }

    return value / maxValue;
}

static BiomeType GetBiomeFromNoise(float noise, float temperature, float humidity) {
    if (noise > 0.7f) return BIOME_MOUNTAIN;
    if (noise > 0.55f && humidity > 0.5f) return BIOME_FOREST;
    if (noise > 0.55f) return BIOME_PLAINS;
    if (noise > 0.4f && temperature < 0.2f) return BIOME_SNOW;
    if (noise > 0.35f && humidity > 0.7f) return BIOME_SWAMP;
    if (noise > 0.25f && temperature > 0.8f) return BIOME_DESERT;
    return BIOME_OCEAN;
}

static BiomeInfo GetBiomeInfo(BiomeType type) {
    BiomeInfo info = {};
    switch (type) {
        case BIOME_PLAINS:
            info.type = BIOME_PLAINS;
            info.groundColor = Color{ 100, 160, 80, 255 };
            info.decorColor = Color{ 80, 140, 60, 255 };
            info.temperature = 0.5f;
            info.humidity = 0.5f;
            info.name = "Plains";
            break;
        case BIOME_FOREST:
            info.type = BIOME_FOREST;
            info.groundColor = Color{ 80, 140, 60, 255 };
            info.decorColor = Color{ 60, 120, 40, 255 };
            info.temperature = 0.5f;
            info.humidity = 0.8f;
            info.name = "Forest";
            break;
        case BIOME_MOUNTAIN:
            info.type = BIOME_MOUNTAIN;
            info.groundColor = Color{ 120, 100, 80, 255 };
            info.decorColor = Color{ 100, 80, 60, 255 };
            info.temperature = 0.2f;
            info.humidity = 0.3f;
            info.name = "Mountain";
            break;
        case BIOME_DESERT:
            info.type = BIOME_DESERT;
            info.groundColor = Color{ 200, 180, 100, 255 };
            info.decorColor = Color{ 180, 160, 80, 255 };
            info.temperature = 0.9f;
            info.humidity = 0.1f;
            info.name = "Desert";
            break;
        case BIOME_OCEAN:
            info.type = BIOME_OCEAN;
            info.groundColor = Color{ 20, 80, 140, 255 };
            info.decorColor = Color{ 30, 100, 160, 255 };
            info.temperature = 0.4f;
            info.humidity = 1.0f;
            info.name = "Ocean";
            break;
        case BIOME_SNOW:
            info.type = BIOME_SNOW;
            info.groundColor = Color{ 240, 240, 240, 255 };
            info.decorColor = Color{ 200, 200, 200, 255 };
            info.temperature = -0.2f;
            info.humidity = 0.4f;
            info.name = "Snow";
            break;
        case BIOME_SWAMP:
            info.type = BIOME_SWAMP;
            info.groundColor = Color{ 90, 110, 80, 255 };
            info.decorColor = Color{ 70, 90, 60, 255 };
            info.temperature = 0.6f;
            info.humidity = 0.9f;
            info.name = "Swamp";
            break;
        default:
            info = GetBiomeInfo(BIOME_PLAINS);
            break;
    }
    return info;
}