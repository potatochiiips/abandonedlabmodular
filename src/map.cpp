#include "globals.h"
#include "map.h"
#include "texture_manager.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include <rlgl.h>

// Global instances
MapData g_MapData;
MapPlayerState g_MapPlayer;
std::vector<Door> doors;
std::vector<Building> buildings;
int currentFloor = -1;
int currentBuildingIndex = -1;
static int idCounter = 1;

// Helper to draw textured cubes (forward declaration)
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);

// =============================================================================
// NEW MAP SYSTEM IMPLEMENTATION
// =============================================================================

static inline bool InBounds(const MapData& m, int x, int y) {
    return x >= 0 && y >= 0 && x < m.width && y < m.height;
}

static inline void FillRectWorld(MapData& m, int x, int y, int w, int h, int tile) {
    for (int yy = 0; yy < h; yy++) {
        for (int xx = 0; xx < w; xx++) {
            int tx = x + xx, ty = y + yy;
            if (InBounds(m, tx, ty)) m.tiles[ty * m.width + tx] = tile;
        }
    }
}

static int RNG(int lo, int hi) {
    return lo + (rand() % (hi - lo + 1));
}

// Create detailed laboratory interior
static Interior MakeLabDetailedInterior(const std::string& id) {
    const int W = 36;
    const int H = 28;
    Interior it;
    it.width = W;
    it.height = H;
    it.id = id;
    it.tiles.assign(W * H, IT_FLOOR);

    // Outer walls
    for (int x = 0; x < W; x++) {
        it.tiles[0 * W + x] = IT_WALL;
        it.tiles[(H - 1) * W + x] = IT_WALL;
    }
    for (int y = 0; y < H; y++) {
        it.tiles[y * W + 0] = IT_WALL;
        it.tiles[y * W + (W - 1)] = IT_WALL;
    }

    // Main entrance (south center) - EXIT DOOR
    int doorX = W / 2;
    it.tiles[(H - 1) * W + doorX] = IT_DOOR;
    it.doorX = doorX;
    it.doorY = H - 1;

    // Cryo chamber: 12x10 at (2,2)
    int c_x = 2, c_y = 2, c_w = 12, c_h = 10;
    for (int x = c_x; x < c_x + c_w; x++) {
        it.tiles[c_y * W + x] = IT_WALL;
        it.tiles[(c_y + c_h - 1) * W + x] = IT_WALL;
    }
    for (int y = c_y; y < c_y + c_h; y++) {
        it.tiles[y * W + c_x] = IT_WALL;
        it.tiles[y * W + (c_x + c_w - 1)] = IT_WALL;
    }
    it.tiles[(c_y + c_h / 2) * W + (c_x + c_w - 1)] = IT_DOOR;

    // Cryo props
    int cryoX = c_x + 4, cryoY = c_y + 3;
    it.tiles[cryoY * W + cryoX] = IT_CRYOPOD_BROKEN;
    it.tiles[cryoY * W + (cryoX + 1)] = IT_CONSOLE;
    it.tiles[(cryoY + 1) * W + cryoX] = IT_COOLANT_PUDDLE;
    it.tiles[(cryoY + 2) * W + (cryoX + 2)] = IT_BROKEN_GLASS;

    it.spawns.push_back({ cryoX + 1, cryoY, "terminal_log_cryo" });
    it.spawns.push_back({ cryoX + 1, cryoY + 1, "small_medkit" });
    it.playerSpawnX = cryoX + 2;
    it.playerSpawnY = cryoY + 1;

    // Specimen Analysis area: 14x10 at (15,2)
    int s_x = 15, s_y = 2, s_w = 14, s_h = 10;
    for (int x = s_x; x < s_x + s_w; x++) {
        it.tiles[s_y * W + x] = IT_WALL;
        it.tiles[(s_y + s_h - 1) * W + x] = IT_WALL;
    }
    for (int y = s_y; y < s_y + s_h; y++) {
        it.tiles[y * W + s_x] = IT_WALL;
        it.tiles[y * W + (s_x + s_w - 1)] = IT_WALL;
    }
    it.tiles[(s_y + s_h - 1) * W + (s_x + s_w / 2)] = IT_DOOR;

    for (int bx = s_x + 2; bx < s_x + s_w - 2; bx += 4) {
        for (int by = s_y + 2; by < s_y + s_h - 2; by += 3) {
            it.tiles[by * W + bx] = IT_BENCH;
            it.tiles[by * W + (bx + 1)] = IT_CONSOLE;
            it.spawns.push_back({ bx, by, "microscope" });
            it.spawns.push_back({ bx + 1, by, "sample_tube" });
        }
    }

    return it;
}

// Simple house interior
static Interior MakeHouseInterior(const std::string& id, int variant = 0) {
    int w = 10 + (variant % 3), h = 8 + (variant % 2);
    Interior it;
    it.width = w;
    it.height = h;
    it.id = id;
    it.tiles.assign(w * h, IT_FLOOR);

    for (int x = 0; x < w; x++) {
        it.tiles[0 * w + x] = IT_WALL;
        it.tiles[(h - 1) * w + x] = IT_WALL;
    }
    for (int y = 0; y < h; y++) {
        it.tiles[y * w + 0] = IT_WALL;
        it.tiles[y * w + (w - 1)] = IT_WALL;
    }

    // Exit door at south center
    it.tiles[(h - 1) * w + (w / 2)] = IT_DOOR;
    it.doorX = w / 2;
    it.doorY = h - 1;

    it.tiles[1 * w + 1] = IT_BED;
    it.spawns.push_back({ 1, 1, "pillow" });

    it.playerSpawnX = w / 2;
    it.playerSpawnY = h / 2;

    return it;
}

// Create all interiors
static void CreateInteriors(MapData& m) {
    m.interiors.clear();
    m.interiors["lab_detailed_01"] = MakeLabDetailedInterior("lab_detailed_01");
    m.interiors["house_small_01"] = MakeHouseInterior("house_small_01", 0);
    m.interiors["house_small_02"] = MakeHouseInterior("house_small_02", 1);
}

// Road generators
static void GenerateRoadGrid(MapData& m, int startY, int endY, int xStart) {
    for (int y = startY; y < endY; y++) {
        for (int x = xStart; x < m.width; x++) {
            if ((x - xStart) % 16 == 0 || (y - startY) % 12 == 0) {
                if (InBounds(m, x, y)) m.tiles[y * m.width + x] = WT_ROAD;
            }
        }
    }
}

// Place building helper
static void PlaceBuilding(MapData& m, int x, int y, int w, int h, BuildingType btype,
    const std::string& interiorId, int& idCounter) {
    FillRectWorld(m, x, y, w, h, WT_BUILDING_FOOTPRINT);

    Building b;
    b.footprint = { x, y, w, h };
    b.type = btype;
    b.interiorId = interiorId;
    b.id = idCounter++;

    int ex = x + w / 2;
    int ey = y + h - 1;
    if (!InBounds(m, ex, ey)) {
        ex = x + w / 2;
        ey = y;
    }
    b.entranceX = ex;
    b.entranceY = ey;
    b.position = Vector3{ (float)ex, 0.0f, (float)ey };
    b.floor = 0;

    if (InBounds(m, ex, ey)) m.tiles[ey * m.width + ex] = WT_ROAD;
    m.buildings.push_back(b);

    // Create entrance door for this building
    Door entranceDoor;
    entranceDoor.position = Vector3{ (float)ex, 0.0f, (float)ey };
    entranceDoor.normal = Vector3{ 0, 0, -1 }; // Faces inward
    entranceDoor.buildingId = b.id;
    entranceDoor.isInteriorDoor = false;
    doors.push_back(entranceDoor);
}

// Main map generation
void GenerateMapData(MapData& m) {
    m.width = MAP_WIDTH;
    m.height = MAP_HEIGHT;
    m.tiles.assign(m.width * m.height, WT_GRASS);
    m.buildings.clear();
    doors.clear(); // Clear existing doors

    CreateInteriors(m);

    // Ocean left (15%)
    int oceanW = (int)(m.width * 0.15f);
    for (int y = 0; y < m.height; y++) {
        for (int x = 0; x < oceanW; x++) {
            m.tiles[y * m.width + x] = WT_WATER;
        }
    }

    // Central lake
    int lakeW = (int)(m.width * 0.10f);
    int lakeH = (int)(m.height * 0.08f);
    int lakeX = m.width / 2 - lakeW / 2;
    int lakeY = m.height / 2 - lakeH / 2;
    for (int y = lakeY; y < lakeY + lakeH; y++) {
        for (int x = lakeX; x < lakeX + lakeW; x++) {
            if (InBounds(m, x, y)) m.tiles[y * m.width + x] = WT_WATER;
        }
    }

    // City band (top 35%)
    int cityY0 = 0, cityY1 = (int)(m.height * 0.35f);
    for (int y = cityY0; y < cityY1; y++) {
        for (int x = oceanW; x < m.width; x++) {
            m.tiles[y * m.width + x] = WT_CONCRETE;
        }
    }
    GenerateRoadGrid(m, cityY0 + 2, cityY1 - 2, oceanW + 2);

    // Place laboratory (north-central)
    int idCounter = 1;
    int labW = 34, labH = 26;
    int labX = oceanW + (int)(m.width * 0.32f);
    int labY = (int)(m.height * 0.10f);
    PlaceBuilding(m, labX, labY, labW, labH, BTYPE_LABORATORY, "lab_detailed_01", idCounter);

    // Suburb houses
    int suburbY0 = cityY1;
    int hx = oceanW + 20;
    int hy = suburbY0 + 8;
    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 4; c++) {
            int px = hx + c * 24;
            int py = hy + r * 18;
            PlaceBuilding(m, px, py, 10, 8, BTYPE_HOUSE,
                (r % 2 == 0 ? "house_small_01" : "house_small_02"), idCounter);
        }
    }

    // Create interior exit doors
    for (const auto& building : m.buildings) {
        const Interior* interior = GetInterior(m, building.interiorId);
        if (interior && interior->doorX >= 0 && interior->doorY >= 0) {
            Door exitDoor;
            exitDoor.position = Vector3{ (float)interior->doorX, 0.0f, (float)interior->doorY };
            exitDoor.normal = Vector3{ 0, 0, 1 }; // Faces outward
            exitDoor.buildingId = building.id;
            exitDoor.isInteriorDoor = true;
            doors.push_back(exitDoor);
        }
    }

    // Set map start state: spawn player inside lab cryo room
    m.startInsideInterior = true;
    m.startInteriorId = "lab_detailed_01";
}

// Initialize player from map start
void InitializePlayerFromMapStart(MapData& m, MapPlayerState& p) {
    if (m.startInsideInterior) {
        const Interior* inter = GetInterior(m, m.startInteriorId);
        if (inter) {
            p.insideInterior = true;
            p.currentInteriorId = inter->id;
            p.currentBuildingId = 0;

            for (const Building& b : m.buildings) {
                if (b.interiorId == inter->id) {
                    p.currentBuildingId = b.id;
                    break;
                }
            }

            if (inter->playerSpawnX >= 0 && inter->playerSpawnY >= 0) {
                p.interiorX = inter->playerSpawnX;
                p.interiorY = inter->playerSpawnY;
            }
            else {
                p.interiorX = 2;
                p.interiorY = 2;
            }
            return;
        }
    }

    // Fallback world spawn
    p.insideInterior = false;
    p.worldX = m.width / 2;
    p.worldY = m.height / 2;
}

// Get interior by ID
const Interior* GetInterior(const MapData& m, const std::string& id) {
    auto f = m.interiors.find(id);
    if (f == m.interiors.end()) return nullptr;
    return &f->second;
}

// Enter interior
bool EnterInterior(MapData& m, MapPlayerState& p, int buildingId) {
    auto it = std::find_if(m.buildings.begin(), m.buildings.end(),
        [&](const Building& b) { return b.id == buildingId; });
    if (it == m.buildings.end()) return false;

    auto f = m.interiors.find(it->interiorId);
    if (f == m.interiors.end()) return false;

    const Interior& inter = f->second;
    p.insideInterior = true;
    p.currentInteriorId = inter.id;
    p.currentBuildingId = it->id;
    p.worldX = it->entranceX;
    p.worldY = it->entranceY;

    if (inter.playerSpawnX >= 0 && inter.playerSpawnY >= 0) {
        p.interiorX = inter.playerSpawnX;
        p.interiorY = inter.playerSpawnY;
    }
    else {
        p.interiorX = 2;
        p.interiorY = 2;
    }
    return true;
}

// Exit interior
bool ExitInterior(MapData& m, MapPlayerState& p) {
    if (!p.insideInterior) return false;

    auto it = std::find_if(m.buildings.begin(), m.buildings.end(),
        [&](const Building& b) { return b.id == p.currentBuildingId; });
    if (it == m.buildings.end()) return false;

    p.worldX = it->entranceX;
    p.worldY = it->entranceY + 1; // Spawn just outside door
    p.insideInterior = false;
    p.currentInteriorId.clear();
    p.currentBuildingId = 0;
    return true;
}

// =============================================================================
// 3D RENDERING - NEW IMPLEMENTATION
// =============================================================================

void Draw3DWorld(const MapData& mapData, const MapPlayerState& playerState) {
    if (playerState.insideInterior) {
        // Draw interior
        const Interior* interior = GetInterior(mapData, playerState.currentInteriorId);
        if (interior) {
            Draw3DInterior(*interior);
        }
    }
    else {
        // Draw exterior world
        Texture2D grassTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_GRASS) : Texture2D{ 0 };
        Texture2D roadTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_ROAD_ASPHALT) : Texture2D{ 0 };
        Texture2D buildingTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_BUILDING_EXTERIOR) : Texture2D{ 0 };
        Texture2D waterTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_GRASS) : Texture2D{ 0 };

        // Draw ground tiles
        for (int z = 0; z < mapData.height; z++) {
            for (int x = 0; x < mapData.width; x++) {
                int tile = mapData.tiles[z * mapData.width + x];

                Texture2D floorTex = grassTex;
                if (tile == WT_ROAD || tile == WT_CONCRETE) floorTex = roadTex;
                else if (tile == WT_WATER) floorTex = waterTex;

                if (floorTex.id > 0) {
                    DrawCubeTexture(floorTex, Vector3{ (float)x, 0.0f, (float)z },
                        1.0f, 0.05f, 1.0f, WHITE);
                }
            }
        }

        // Draw buildings
        for (const auto& building : mapData.buildings) {
            // Draw building walls
            for (int z = building.footprint.y; z < building.footprint.y + building.footprint.h; z++) {
                for (int x = building.footprint.x; x < building.footprint.x + building.footprint.w; x++) {
                    // Draw outer walls only on perimeter
                    bool isPerimeter = (x == building.footprint.x || x == building.footprint.x + building.footprint.w - 1 ||
                        z == building.footprint.y || z == building.footprint.y + building.footprint.h - 1);

                    // Don't draw wall at entrance
                    bool isEntrance = (x == building.entranceX && z == building.entranceY);

                    if (isPerimeter && !isEntrance) {
                        if (buildingTex.id > 0) {
                            DrawCubeTexture(buildingTex, Vector3{ (float)x, WALL_HEIGHT / 2.0f, (float)z },
                                1.0f, WALL_HEIGHT, 1.0f, WHITE);
                        }
                        else {
                            DrawCube(Vector3{ (float)x, WALL_HEIGHT / 2.0f, (float)z },
                                1.0f, WALL_HEIGHT, 1.0f, Color{ 120, 120, 130, 255 });
                        }
                    }
                }
            }

            // Draw roof
            Vector3 roofCenter = Vector3{
                building.footprint.x + building.footprint.w / 2.0f,
                CEILING_HEIGHT,
                building.footprint.y + building.footprint.h / 2.0f
            };
            DrawCube(roofCenter, (float)building.footprint.w, 0.2f, (float)building.footprint.h, Color{ 80, 50, 50, 255 });
        }

        // Draw exterior doors
        for (const auto& door : doors) {
            if (!door.isInteriorDoor) {
                DrawDoor(door);
            }
        }
    }
}

void Draw3DInterior(const Interior& interior) {
    Texture2D wallTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_WALL_CONCRETE) : Texture2D{ 0 };
    Texture2D floorTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_FLOOR_TILE) : Texture2D{ 0 };
    Texture2D doorTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_DOOR_METAL) : Texture2D{ 0 };

    for (int y = 0; y < interior.height; y++) {
        for (int x = 0; x < interior.width; x++) {
            int tile = interior.tiles[y * interior.width + x];
            Vector3 pos = Vector3{ (float)x, 0.0f, (float)y };

            // Draw floor for all non-empty tiles
            if (tile != IT_EMPTY && floorTex.id > 0) {
                DrawCubeTexture(floorTex, Vector3{ (float)x, 0.0f, (float)y },
                    1.0f, 0.05f, 1.0f, WHITE);
            }

            // Draw walls
            if (tile == IT_WALL) {
                if (wallTex.id > 0) {
                    DrawCubeTexture(wallTex, Vector3{ (float)x, WALL_HEIGHT / 2.0f, (float)y },
                        1.0f, WALL_HEIGHT, 1.0f, WHITE);
                }
                else {
                    DrawCube(Vector3{ (float)x, WALL_HEIGHT / 2.0f, (float)y },
                        1.0f, WALL_HEIGHT, 1.0f, Color{ 180, 180, 185, 255 });
                }
            }

            // Draw props
            switch (tile) {
            case IT_CRYOPOD_BROKEN:
                DrawCube(Vector3{ (float)x, 0.5f, (float)y }, 0.8f, 1.0f, 0.8f, Color{ 100, 150, 200, 255 });
                break;
            case IT_CONSOLE:
                DrawCube(Vector3{ (float)x, 0.4f, (float)y }, 0.6f, 0.8f, 0.6f, Color{ 80, 120, 160, 255 });
                break;
            case IT_BENCH:
                DrawCube(Vector3{ (float)x, 0.4f, (float)y }, 0.8f, 0.8f, 0.4f, Color{ 140, 120, 100, 255 });
                break;
            case IT_BED:
                DrawCube(Vector3{ (float)x, 0.3f, (float)y }, 0.9f, 0.6f, 0.9f, Color{ 180, 160, 140, 255 });
                break;
            }
        }
    }

    // Draw ceiling
    Vector3 ceilingCenter = Vector3{
        interior.width / 2.0f,
        CEILING_HEIGHT,
        interior.height / 2.0f
    };
    DrawCube(ceilingCenter, (float)interior.width, 0.1f, (float)interior.height, Color{ 240, 240, 240, 255 });

    // Draw interior doors
    for (const auto& door : doors) {
        if (door.isInteriorDoor) {
            DrawDoor(door);
        }
    }
}

void DrawDoor(const Door& door) {
    Texture2D doorTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_DOOR_METAL) : Texture2D{ 0 };

    Color doorColor = door.isLocked ? Color{ 150, 50, 50, 255 } : Color{ 100, 100, 110, 255 };

    if (!door.isOpen) {
        if (doorTex.id > 0) {
            DrawCubeTexture(doorTex, Vector3{ door.position.x, DOOR_HEIGHT / 2.0f, door.position.z },
                0.2f, DOOR_HEIGHT, 1.0f, doorColor);
        }
        else {
            DrawCube(Vector3{ door.position.x, DOOR_HEIGHT / 2.0f, door.position.z },
                0.2f, DOOR_HEIGHT, 1.0f, doorColor);
        }

        // Draw door frame
        DrawCube(Vector3{ door.position.x, DOOR_HEIGHT + 0.1f, door.position.z },
            0.3f, 0.2f, 1.2f, Color{ 80, 80, 85, 255 });
    }
}

void UpdateDoors(float deltaTime) {
    for (auto& door : doors) {
        if (door.isOpen && door.openProgress < 1.0f) {
            door.openProgress += deltaTime * 2.0f;
            if (door.openProgress >= 1.0f) {
                door.openProgress = 1.0f;
            }
        }
        else if (!door.isOpen && door.openProgress > 0.0f) {
            door.openProgress -= deltaTime * 2.0f;
            if (door.openProgress <= 0.0f) {
                door.openProgress = 0.0f;
            }
        }
    }
}

Door* GetNearestDoor(Vector3 playerPos, float maxDistance) {
    Door* nearest = nullptr;
    float minDist = maxDistance;

    for (auto& door : doors) {
        // Calculate door position based on whether player is inside
        Vector3 doorPos = door.position;
        if (g_MapPlayer.insideInterior && door.isInteriorDoor) {
            // Use interior coordinates for interior doors
            doorPos = Vector3{ door.position.x, playerPos.y, door.position.z };
        }
        else if (!g_MapPlayer.insideInterior && !door.isInteriorDoor) {
            // Use world coordinates for exterior doors
            doorPos = Vector3{ door.position.x, playerPos.y, door.position.z };
        }
        else {
            continue; // Skip doors not in current context
        }

        float dist = Vector3Distance(playerPos, doorPos);
        if (dist < minDist) {
            minDist = dist;
            nearest = &door;
        }
    }

    return nearest;
}

// Wall collision detection
bool CheckWallCollision(Vector3 position, float radius, const MapData& mapData, const MapPlayerState& playerState) {
    if (playerState.insideInterior) {
        const Interior* interior = GetInterior(mapData, playerState.currentInteriorId);
        if (!interior) return false;

        int gridX = (int)floorf(position.x);
        int gridZ = (int)floorf(position.z);
 
        // Check 3x3 area around player
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                int checkX = gridX + dx;
                int checkZ = gridZ + dz;

                if (checkX >= 0 && checkX < interior->width &&
                    checkZ >= 0 && checkZ < interior->height) {
                    int tile = interior->tiles[checkZ * interior->width + checkX];

                    if (tile == IT_WALL) {
                        Vector3 tileCenter = Vector3{ (float)checkX + 0.5f, position.y, (float)checkZ + 0.5f };
                        float dist = Vector3Distance(
                            Vector3{ position.x, position.y, position.z },
                            Vector3{ tileCenter.x, position.y, tileCenter.z }
                        );
                        if (dist < radius + 0.5f) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    else {
        // Check world collisions
        int gridX = (int)floorf(position.x);
        int gridZ = (int)floorf(position.z);

        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                int checkX = gridX + dx;
                int checkZ = gridZ + dz;

                if (checkX >= 0 && checkX < mapData.width &&
                    checkZ >= 0 && checkZ < mapData.height) {

                    // Check if position is inside a building footprint
                    for (const auto& building : mapData.buildings) {
                        bool insideBuilding = checkX >= building.footprint.x &&
                            checkX < building.footprint.x + building.footprint.w &&
                            checkZ >= building.footprint.y &&
                            checkZ < building.footprint.y + building.footprint.h;

                        if (insideBuilding) {
                            // Only collide with perimeter walls, not entrance
                            bool isPerimeter = (checkX == building.footprint.x ||
                                checkX == building.footprint.x + building.footprint.w - 1 ||
                                checkZ == building.footprint.y ||
                                checkZ == building.footprint.y + building.footprint.h - 1);

                            bool isEntrance = (checkX == building.entranceX && checkZ == building.entranceY);

                            if (isPerimeter && !isEntrance) {
                                Vector3 tileCenter = Vector3{ (float)checkX + 0.5f, position.y, (float)checkZ + 0.5f };
                                float dist = Vector3Distance(
                                    Vector3{ position.x, position.y, position.z },
                                    Vector3{ tileCenter.x, position.y, tileCenter.z }
                                );

                                if (dist < radius + 0.5f) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

// =============================================================================
// LEGACY COMPATIBILITY LAYER
// =============================================================================

void GenerateMap(char map[MAP_SIZE][MAP_SIZE]) {
    // Generate new map data
    GenerateMapData(g_MapData);
    InitializePlayerFromMapStart(g_MapData, g_MapPlayer);

    // Convert to legacy format for minimap display
    for (int y = 0; y < MAP_SIZE && y < g_MapData.height; y++) {
        for (int x = 0; x < MAP_SIZE && x < g_MapData.width; x++) {
            int idx = y * g_MapData.width + x;
            int tile = g_MapData.tiles[idx];

            switch (tile) {
            case WT_WATER: map[y][x] = '~'; break;
            case WT_GRASS: map[y][x] = '"'; break;
            case WT_ROAD: map[y][x] = '='; break;
            case WT_CONCRETE: map[y][x] = '.'; break;
            case WT_BUILDING_FOOTPRINT: map[y][x] = 'B'; break;
            case WT_SUBURB: map[y][x] = '"'; break;
            case WT_FARMLAND: map[y][x] = '"'; break;
            default: map[y][x] = '"'; break;
            }
        }
    }
}

void DrawMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw,
    int minimapX, int minimapY, int minimapW, int minimapH,
    bool largeMap, int screenH) {
    DrawRectangle(minimapX, minimapY, minimapW, minimapH, Color{ 0, 0, 0, 180 });
    DrawRectangleLines(minimapX, minimapY, minimapW, minimapH, PIPBOY_GREEN);

    int viewRange = largeMap ? 20 : 15;
    float cellSize = (float)minimapW / (viewRange * 2);

    // Inside building - show interior layout
    if (g_MapPlayer.insideInterior) {
        const Interior* interior = GetInterior(g_MapData, g_MapPlayer.currentInteriorId);
        if (interior) {
            DrawText("INTERIOR", minimapX + 5, minimapY + 5, 12, PIPBOY_GREEN);

            // Draw interior tiles
            for (int y = 0; y < interior->height; y++) {
                for (int x = 0; x < interior->width; x++) {
                    int tile = interior->tiles[y * interior->width + x];
                    Color col = PIPBOY_DIM;

                    switch (tile) {
                    case IT_WALL: col = Color{ 90, 90, 90, 255 }; break;
                    case IT_DOOR: col = Color{ 200, 170, 60, 255 }; break;
                    case IT_FLOOR: col = Color{ 100, 100, 100, 255 }; break;
                    case IT_CRYOPOD_BROKEN: col = Color{ 255, 100, 100, 255 }; break;
                    case IT_CONSOLE: col = Color{ 100, 200, 255, 255 }; break;
                    }

                    int drawX = minimapX + (int)(x * cellSize);
                    int drawY = minimapY + 20 + (int)(y * cellSize);

                    if (drawX >= minimapX && drawX < minimapX + minimapW &&
                        drawY >= minimapY && drawY < minimapY + minimapH) {
                        DrawRectangle(drawX, drawY, (int)ceilf(cellSize), (int)ceilf(cellSize), col);
                    }
                }
            }

            // Draw player position in interior
            Vector2 playerMapPos = {
                minimapX + (float)(g_MapPlayer.interiorX * cellSize),
                minimapY + 20 + (float)(g_MapPlayer.interiorY * cellSize)
            };
            DrawCircleV(playerMapPos, fmaxf(3.0f, cellSize * 0.8f), Color{ 255, 50, 50, 255 });
        }
    }
    // Outside - show world map
    else {
        int playerX = (int)playerPos.x;
        int playerZ = (int)playerPos.z;

        for (int r = -viewRange; r < viewRange; ++r) {
            for (int c = -viewRange; c < viewRange; ++c) {
                int worldX = playerX + c;
                int worldZ = playerZ + r;

                if (worldX < 0 || worldX >= MAP_SIZE || worldZ < 0 || worldZ >= MAP_SIZE) continue;

                Color col = PIPBOY_DIM;
                switch (map[worldZ][worldX]) {
                case '~': col = Color{ 30, 60, 120, 255 }; break;
                case 'B': col = Color{ 100, 100, 120, 255 }; break;
                case '=': col = Color{ 80, 80, 80, 255 }; break;
                case '"': col = Color{ 30, 120, 30, 200 }; break;
                case '.': col = Color{ 90, 90, 95, 255 }; break;
                }

                int drawX = minimapX + (int)((c + viewRange) * cellSize);
                int drawY = minimapY + (int)((r + viewRange) * cellSize);
                DrawRectangle(drawX, drawY, (int)ceilf(cellSize), (int)ceilf(cellSize), col);
            }
        }

        Vector2 centerPos = { minimapX + minimapW / 2.0f, minimapY + minimapH / 2.0f };
        DrawCircleV(centerPos, fmaxf(3.0f, cellSize * 0.8f), Color{ 255, 50, 50, 255 });
    }
}

void DrawMapMenu(int screenW, int screenH, char map[MAP_SIZE][MAP_SIZE], Vector3 cameraPos, float zoom) {
    const int menuW = screenW - 200;
    const int menuH = screenH - 120;
    const int menuX = 100;
    const int menuY = 60;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);
    DrawText("MAP", menuX + 20, menuY + 10, 30, PIPBOY_GREEN);

    DrawMinimap(map, cameraPos, 0, menuX + 20, menuY + 60, menuW - 40, menuH - 80, true, screenH);
}

void DrawMapGeometry(char map[MAP_SIZE][MAP_SIZE]) {
    // Use new 3D drawing system
    Draw3DWorld(g_MapData, g_MapPlayer);
}

bool IsAABBInFrustum(const Camera3D& camera, const AABB& box) {
    float distance = Vector3Distance(camera.position,
        Vector3{ (box.min.x + box.max.x) * 0.5f,
                (box.min.y + box.max.y) * 0.5f,
                (box.min.z + box.max.z) * 0.5f });
    return distance < 100.0f;
}