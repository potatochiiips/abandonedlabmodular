#include "globals.h"
#include "map.h"
#include "texture_manager.h"
#include "model_manager.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include <rlgl.h>

// Global instance definitions
UpgradedMapRenderer* g_UpgradedMapRenderer = nullptr;
std::vector<Door> doors;
MapPlayerState g_MapPlayer;
MapData g_MapData;
// Forward declaration
struct Player;

// Define MAP_WIDTH/HEIGHT for map.cpp
#define MAP_WIDTH MAP_SIZE
#define MAP_HEIGHT MAP_SIZE

// Wall heights
#define WALL_HEIGHT 3.0f
#define DOOR_HEIGHT 2.5f
#define CEILING_HEIGHT 3.0f
#define FLOOR_HEIGHT 4.0f  // Distance between floors

UpgradedMapRenderer::UpgradedMapRenderer() : nextPropId(1) {
}

UpgradedMapRenderer::~UpgradedMapRenderer() {
    Cleanup();
}

void UpgradedMapRenderer::Initialize() {
    TraceLog(LOG_INFO, "Initializing Map Renderer...");
    InitializeMaterials();
}

void UpgradedMapRenderer::InitializeMaterials() {
    // Wall material
    wallMaterial = CreateOpaqueMaterial(TEX_WALL_CONCRETE, WHITE);

    // Floor material
    floorMaterial = CreateOpaqueMaterial(TEX_FLOOR_TILE, WHITE);

    // Door material
    doorMaterial = CreateOpaqueMaterial(TEX_DOOR_METAL, Color{ 100, 100, 110, 255 });

    // Concrete material
    concreteMaterial = CreateOpaqueMaterial(TEX_FLOOR_CONCRETE, Color{ 120, 120, 125, 255 });

    // Grass material
    grassMaterial = CreateOpaqueMaterial(TEX_GRASS, Color{ 50, 140, 50, 255 });

    // Water material (transparent)
    waterMaterial = CreateTransparentMaterial(TEX_GRASS, Color{ 30, 60, 120, 180 });
    waterMaterial->SetRenderQueue(QUEUE_TRANSPARENT);

    // Glass material (transparent)
    glassMaterial = CreateTransparentMaterial(TEX_WINDOW_GLASS, Color{ 135, 206, 235, 100 });
    glassMaterial->SetRenderQueue(QUEUE_TRANSPARENT);

    TraceLog(LOG_INFO, "Map materials initialized");
}

std::shared_ptr<UpgradedMaterial> UpgradedMapRenderer::CreateOpaqueMaterial(TextureID texture, Color tint) {
    auto material = std::make_shared<UpgradedMaterial>(g_UpgradedPipeline->CreateStandardShader());

    if (g_TextureManager) {
        material->SetTexture("texture0", g_TextureManager->GetTexture(texture));
    }

    material->SetColor("colDiffuse", tint);
    material->SetRenderQueue(QUEUE_GEOMETRY);

    return material;
}

std::shared_ptr<UpgradedMaterial> UpgradedMapRenderer::CreateTransparentMaterial(TextureID texture, Color tint) {
    auto material = std::make_shared<UpgradedMaterial>(g_UpgradedPipeline->CreateTransparentShader());

    if (g_TextureManager) {
        material->SetTexture("texture0", g_TextureManager->GetTexture(texture));
    }

    material->SetColor("colDiffuse", tint);
    material->SetRenderQueue(QUEUE_TRANSPARENT);

    return material;
}

void UpgradedMapRenderer::GenerateWorldGeometry(const MapData& mapData) {
    TraceLog(LOG_INFO, "Generating world geometry...");

    worldRenderers.clear();

    // Generate terrain/ground
    for (int z = 0; z < mapData.height; z++) {
        for (int x = 0; x < mapData.width; x++) {
            int tile = mapData.tiles[z * mapData.width + x];
            Vector3 pos = { (float)x, 0.0f, (float)z };

            switch (tile) {
            case WT_GRASS:
                CreateFloor(pos, 1.0f, TEX_GRASS);
                break;

            case WT_ROAD:
            case WT_CONCRETE:
                CreateRoadTile(pos);
                break;

            case WT_WATER:
                CreateWaterTile(pos);
                break;

            case WT_BUILDING_FOOTPRINT:
                // Buildings handled separately
                CreateFloor(pos, 1.0f, TEX_FLOOR_CONCRETE);
                break;
            }
        }
    }

    // Generate buildings
    for (const Building& building : mapData.buildings) {
        CreateBuilding(building);
    }

    // Generate exterior doors
    for (const Door& door : doors) {
        if (!door.isInteriorDoor) {
            CreateDoor(door);
        }
    }

    TraceLog(LOG_INFO, "World geometry generated: %d renderers", (int)worldRenderers.size());
}

void UpgradedMapRenderer::GenerateInteriorGeometry(const Interior& interior) {
    TraceLog(LOG_INFO, "Generating interior geometry...");

    interiorRenderers.clear();

    for (int y = 0; y < interior.height; y++) {
        for (int x = 0; x < interior.width; x++) {
            int tile = interior.tiles[y * interior.width + x];
            Vector3 pos = { (float)x, 0.0f, (float)y };

            // Floor
            if (tile != IT_EMPTY) {
                CreateFloor(pos, 1.0f, TEX_FLOOR_TILE);
            }

            // Walls
            if (tile == IT_WALL) {
                CreateWall(pos, TEX_WALL_CONCRETE);
            }

            // Props using model manager
            if (g_ModelManager) {
                Vector3 propPos = { (float)x, 0.0f, (float)y };

                switch (tile) {
                case IT_CRYOPOD_BROKEN:
                    AddProp(propPos, MODEL_CRYOPOD, 1.0f);
                    break;
                case IT_CONSOLE:
                    AddProp(propPos, MODEL_CONSOLE_TERMINAL, 1.0f);
                    break;
                case IT_BED:
                    AddProp(propPos, MODEL_BED, 1.0f);
                    break;
                case IT_DESK:
                    AddProp(propPos, MODEL_DESK, 1.0f);
                    break;
                case IT_CHAIR:
                    AddProp(propPos, MODEL_CHAIR, 1.0f);
                    break;
                case IT_TABLE:
                    AddProp(propPos, MODEL_TABLE, 1.0f);
                    break;
                case IT_SHELF:
                    AddProp(propPos, MODEL_SHELF, 1.0f);
                    break;
                case IT_LOCKER:
                    AddProp(propPos, MODEL_LOCKER, 1.0f);
                    break;
                case IT_CABINET:
                    AddProp(propPos, MODEL_CABINET, 1.0f);
                    break;
                case IT_CRATE:
                    AddProp(propPos, MODEL_CRATE, 1.0f);
                    break;
                }
            }
        }
    }

    // Interior ceiling
    Vector3 ceilingCenter = {
        interior.width / 2.0f,
        CEILING_HEIGHT,
        interior.height / 2.0f
    };

    auto ceilingRenderer = std::make_unique<MeshRenderer>();
    ceilingRenderer->mesh = GenMeshCube((float)interior.width, 0.1f, (float)interior.height);
    ceilingRenderer->material = CreateOpaqueMaterial(TEX_CEILING_TILE, Color{ 240, 240, 240, 255 });
    ceilingRenderer->transform = MatrixTranslate(ceilingCenter.x, ceilingCenter.y, ceilingCenter.z);
    ceilingRenderer->castShadows = false;
    ceilingRenderer->receiveShadows = false;

    interiorRenderers.push_back(std::move(ceilingRenderer));

    // Interior doors
    for (const Door& door : doors) {
        if (door.isInteriorDoor) {
            CreateDoor(door);
        }
    }

    TraceLog(LOG_INFO, "Interior geometry generated: %d renderers", (int)interiorRenderers.size());
}

void UpgradedMapRenderer::CreateWall(Vector3 position, TextureID texture) {
    auto renderer = std::make_unique<MeshRenderer>();
    renderer->mesh = GenMeshCube(1.0f, WALL_HEIGHT, 1.0f);

    renderer->material = CreateOpaqueMaterial(texture, WHITE);

    Vector3 wallPos = position;
    wallPos.y = WALL_HEIGHT / 2.0f;
    renderer->transform = MatrixTranslate(wallPos.x, wallPos.y, wallPos.z);

    renderer->castShadows = true;
    renderer->receiveShadows = true;

    if (g_MapPlayer.insideInterior) {
        interiorRenderers.push_back(std::move(renderer));
    }
    else {
        worldRenderers.push_back(std::move(renderer));
    }
}

void UpgradedMapRenderer::CreateFloor(Vector3 position, float size, TextureID texture) {
    auto renderer = std::make_unique<MeshRenderer>();
    renderer->mesh = GenMeshCube(size, 0.1f, size);

    renderer->material = CreateOpaqueMaterial(texture, WHITE);

    Vector3 floorPos = position;
    floorPos.y = 0.05f;
    renderer->transform = MatrixTranslate(floorPos.x, floorPos.y, floorPos.z);

    renderer->castShadows = false;
    renderer->receiveShadows = true;

    if (g_MapPlayer.insideInterior) {
        interiorRenderers.push_back(std::move(renderer));
    }
    else {
        worldRenderers.push_back(std::move(renderer));
    }
}

void UpgradedMapRenderer::CreateBuilding(const Building& building) {
    // Draw building walls (perimeter only)
    for (int z = building.footprint.y; z < building.footprint.y + building.footprint.h; z++) {
        for (int x = building.footprint.x; x < building.footprint.x + building.footprint.w; x++) {
            bool isPerimeter = (x == building.footprint.x ||
                x == building.footprint.x + building.footprint.w - 1 ||
                z == building.footprint.y ||
                z == building.footprint.y + building.footprint.h - 1);

            bool isEntrance = (x == building.entranceX && z == building.entranceY);

            if (isPerimeter && !isEntrance) {
                CreateWall(Vector3{ (float)x, 0.0f, (float)z }, TEX_BUILDING_EXTERIOR);
            }
        }
    }

    // Building roof
    auto roofRenderer = std::make_unique<MeshRenderer>();
    roofRenderer->mesh = GenMeshCube((float)building.footprint.w, 0.2f, (float)building.footprint.h);

    roofRenderer->material = CreateOpaqueMaterial(TEX_ROOF_SHINGLES, Color{ 80, 50, 50, 255 });

    Vector3 roofCenter = {
        building.footprint.x + building.footprint.w / 2.0f,
        CEILING_HEIGHT,
        building.footprint.y + building.footprint.h / 2.0f
    };
    roofRenderer->transform = MatrixTranslate(roofCenter.x, roofCenter.y, roofCenter.z);

    roofRenderer->castShadows = true;
    roofRenderer->receiveShadows = true;

    worldRenderers.push_back(std::move(roofRenderer));
}

void UpgradedMapRenderer::CreateDoor(const Door& door) {
    auto doorRenderer = std::make_unique<MeshRenderer>();
    doorRenderer->mesh = GenMeshCube(0.2f, DOOR_HEIGHT, 1.0f);

    Color doorColor = door.isLocked ? Color{ 150, 50, 50, 255 } : Color{ 100, 100, 110, 255 };
    doorRenderer->material = CreateOpaqueMaterial(TEX_DOOR_METAL, doorColor);

    Vector3 doorPos = door.position;
    doorPos.y = DOOR_HEIGHT / 2.0f;
    doorRenderer->transform = MatrixTranslate(doorPos.x, doorPos.y, doorPos.z);

    doorRenderer->castShadows = true;
    doorRenderer->receiveShadows = true;

    doorRenderers.push_back(std::move(doorRenderer));
}

void UpgradedMapRenderer::CreateWaterTile(Vector3 position) {
    auto renderer = std::make_unique<MeshRenderer>();
    renderer->mesh = GenMeshCube(1.0f, 0.2f, 1.0f);

    renderer->material = waterMaterial;

    Vector3 waterPos = position;
    waterPos.y = 0.1f;
    renderer->transform = MatrixTranslate(waterPos.x, waterPos.y, waterPos.z);

    renderer->castShadows = false;
    renderer->receiveShadows = false;

    worldRenderers.push_back(std::move(renderer));
}

void UpgradedMapRenderer::CreateRoadTile(Vector3 position) {
    auto renderer = std::make_unique<MeshRenderer>();
    renderer->mesh = GenMeshCube(1.0f, 0.05f, 1.0f);

    renderer->material = CreateOpaqueMaterial(TEX_ROAD_ASPHALT, Color{ 60, 60, 65, 255 });

    Vector3 roadPos = position;
    roadPos.y = 0.025f;
    renderer->transform = MatrixTranslate(roadPos.x, roadPos.y, roadPos.z);

    renderer->castShadows = false;
    renderer->receiveShadows = true;

    worldRenderers.push_back(std::move(renderer));
}

void UpgradedMapRenderer::AddProp(Vector3 position, ModelID modelId, float scale) {
    if (!g_ModelManager || !g_ModelManager->IsLoaded(modelId)) return;

    auto propRenderer = std::make_unique<MeshRenderer>();
    Model model = g_ModelManager->GetModel(modelId);

    if (model.meshCount > 0) {
        propRenderer->mesh = model.meshes[0];

        propRenderer->material = CreateOpaqueMaterial(TEX_WALL_CONCRETE, WHITE);

        Matrix scaleMatrix = MatrixScale(scale, scale, scale);
        Matrix translation = MatrixTranslate(position.x, position.y, position.z);
        propRenderer->transform = MatrixMultiply(scaleMatrix, translation);

        propRenderer->castShadows = true;
        propRenderer->receiveShadows = true;

        propRenderers.push_back(std::move(propRenderer));
    }
}

void UpgradedMapRenderer::UpdateDoor(int doorId, float openProgress) {
    // Update door position based on open progress
    // This would animate doors opening/closing
}

void UpgradedMapRenderer::RemoveProp(int propId) {
    // Remove a prop by ID
}

std::vector<MeshRenderer*> UpgradedMapRenderer::GetActiveRenderers() {
    std::vector<MeshRenderer*> active;

    if (g_MapPlayer.insideInterior) {
        // Return interior renderers
        for (auto& renderer : interiorRenderers) {
            active.push_back(renderer.get());
        }
        for (auto& renderer : propRenderers) {
            active.push_back(renderer.get());
        }
    }
    else {
        // Return world renderers
        for (auto& renderer : worldRenderers) {
            active.push_back(renderer.get());
        }
    }

    // Always add doors
    for (auto& renderer : doorRenderers) {
        active.push_back(renderer.get());
    }

    return active;
}

void UpgradedMapRenderer::Update(float deltaTime, const Camera3D& camera) {
    UpdateVisibility(camera);
}

void UpgradedMapRenderer::UpdateVisibility(const Camera3D& camera) {
    // Simple frustum culling based on distance
    Vector3 camPos = camera.position;
    float viewDistance = 100.0f;

    auto checkVisibility = [&](std::unique_ptr<MeshRenderer>& renderer) {
        Vector3 objPos = { renderer->transform.m12, renderer->transform.m13, renderer->transform.m14 };
        float distance = Vector3Distance(camPos, objPos);
        renderer->enabled = (distance < viewDistance);
        };

    for (auto& renderer : worldRenderers) checkVisibility(renderer);
    for (auto& renderer : interiorRenderers) checkVisibility(renderer);
    for (auto& renderer : propRenderers) checkVisibility(renderer);
}

void UpgradedMapRenderer::Cleanup() {
    worldRenderers.clear();
    interiorRenderers.clear();
    propRenderers.clear();
    doorRenderers.clear();
}

// Global instance
extern UpgradedMapRenderer* g_UpgradedMapRenderer;

// Item spawn structure
struct ItemSpawn {
    int x;
    int y;
    std::string itemType;
};

// Player state in map system
struct MapPlayerState {
    bool insideInterior;
    std::string currentInteriorId;
    int currentBuildingId;
    int worldX;
    int worldY;
    int interiorX;
    int interiorY;

    MapPlayerState() : insideInterior(false), currentBuildingId(0),
        worldX(0), worldY(0), interiorX(0), interiorY(0) {
    }
};

// Global map data
extern MapData g_MapData;
extern MapPlayerState g_MapPlayer;

// Global building and door management
extern std::vector<Door> doors;
extern std::vector<Building> buildings;
extern int currentFloor;
extern int currentBuildingIndex;

// Map generation functions
void GenerateMapData(MapData& m);
void InitializePlayerFromMapStart(MapData& m, MapPlayerState& p);
bool EnterInterior(MapData& m, MapPlayerState& p, int buildingId);
bool ExitInterior(MapData& m, MapPlayerState& p);
const Interior* GetInterior(const MapData& m, const std::string& id);

// Legacy compatibility functions
void GenerateMap(char map[MAP_SIZE][MAP_SIZE]);
void DrawMapMenu(int screenW, int screenH, char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw);
void DrawMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw, int minimapX, int minimapY, int minimapW, int minimapH, bool largeMap, int screenH);
void DrawMapGeometry(char map[MAP_SIZE][MAP_SIZE]);

// Enhanced world functions - NEW 3D DRAWING
void Draw3DWorld(const MapData& mapData, const MapPlayerState& playerState);
void Draw3DInterior(const Interior& interior);
void DrawDoor(const Door& door);
void UpdateDoors(float deltaTime);
Door* GetNearestDoor(Vector3 playerPos, float maxDistance);

// Collision detection
bool CheckWallCollision(Vector3 position, float radius, const MapData& mapData, const MapPlayerState& playerState);

// Frustum culling helper
struct AABB {
    Vector3 min;
    Vector3 max;
};

bool IsAABBInFrustum(const Camera3D& camera, const AABB& box);