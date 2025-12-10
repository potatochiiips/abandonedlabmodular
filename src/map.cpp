#include "globals.h"
#include "map.h"
#include "texture_manager.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include <rlgl.h> 
#include <player.h> 
// Global instances
MapData g_MapData;
MapPlayerState g_MapPlayer; 
std::vector<Door> doors;
std::vector<Building> buildings;
int currentFloor = -1;
int currentBuildingIndex = -1;
static int idCounter = 1;

// Tile definitions for legacy system
#define ROAD_TILE '='
#define GRASS_TILE '"'
#define BUILDING_TILE 'B'
#define TREE_TILE 'T'
#define LIGHT_TILE 'L'
#define BUSH_TILE 'b'
#define FLOWER_TILE 'f'

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

    // Main entrance (south center)
    int doorX = W / 2;
    it.tiles[(H - 1) * W + doorX] = IT_DOOR;

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

    it.tiles[(h - 1) * w + (w / 2)] = IT_DOOR;
    it.tiles[1 * w + 1] = IT_BED;
    it.spawns.push_back({ 1,1,"pillow" });

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
    b.footprint = { x,y,w,h };
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

    if (InBounds(m, ex, ey)) m.tiles[ey * m.width + ex] = WT_ROAD;
    m.buildings.push_back(b);
}

// Main map generation
void GenerateMapData(MapData& m) {
    m.width = MAP_WIDTH;
    m.height = MAP_HEIGHT;
    m.tiles.assign(m.width * m.height, WT_GRASS);
    m.buildings.clear();

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

    // Set map start state: spawn player inside lab cryo room
    m.startInsideInterior = true;
    m.startInteriorId = "lab_detailed_01";
}

// Initialize player from map start
void InitializePlayerFromMapStart(MapData& m, Player& p) {
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
bool EnterInterior(MapData& m, Player& p, int buildingId) {
    auto it = std::find_if(m.buildings.begin(), m.buildings.end(),
        [&](const Building& b) { return b.id == buildingId; });
    if (it == m.buildings.end()) return false;

    auto f = m.interiors.find(it->interiorId);
    if (f == m.interiors.end()) return false;

    const Interior& inter = f->second;
    p.insideInterior = true;
    p.currentInteriorId = inter.id;
    p.currentBuildingId = it->id;

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
bool ExitInterior(MapData& m, Player& p) {
    if (!p.insideInterior) return false;

    auto it = std::find_if(m.buildings.begin(), m.buildings.end(),
        [&](const Building& b) { return b.id == p.currentBuildingId; });
    if (it == m.buildings.end()) return false;

    p.worldX = it->entranceX;
    p.worldY = it->entranceY;
    p.insideInterior = false;
    p.currentInteriorId.clear();
    p.currentBuildingId = 0;
    return true;
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
            case WT_GRASS: map[y][x] = GRASS_TILE; break;
            case WT_ROAD: map[y][x] = ROAD_TILE; break;
            case WT_CONCRETE: map[y][x] = '.'; break;
            case WT_BUILDING_FOOTPRINT: map[y][x] = BUILDING_TILE; break;
            case WT_SUBURB: map[y][x] = GRASS_TILE; break;
            case WT_FARMLAND: map[y][x] = GRASS_TILE; break;
            default: map[y][x] = GRASS_TILE; break;
            }
        }
    }
}

// =============================================================================
// MINIMAP DRAWING (Enhanced to show interior/exterior state)
// =============================================================================

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
                minimapX + (int)(g_MapPlayer.interiorX * cellSize),
                minimapY + 20 + (int)(g_MapPlayer.interiorY * cellSize)
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
                case '~': col = Color{ 30, 60, 120, 255 }; break; // Water
                case BUILDING_TILE: col = Color{ 100, 100, 120, 255 }; break;
                case ROAD_TILE: col = Color{ 80, 80, 80, 255 }; break;
                case GRASS_TILE: col = Color{ 30, 120, 30, 200 }; break;
                case '.': col = Color{ 90, 90, 95, 255 }; break; // Concrete
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

// Placeholder implementations for other legacy functions
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
    // Simplified - draw basic geometry based on legacy map
    Texture2D grassTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_GRASS) : Texture2D{ 0 };
    Texture2D roadTex = g_TextureManager ? g_TextureManager->GetTexture(TEX_ROAD_ASPHALT) : Texture2D{ 0 };

    for (int r = 0; r < MAP_SIZE; ++r) {
        for (int c = 0; c < MAP_SIZE; ++c) {
            Vector3 pos = Vector3{ (float)c, 0.0f, (float)r };

            if (map[r][c] == BUILDING_TILE) continue;

            Texture2D floorTex = grassTex;
            if (map[r][c] == ROAD_TILE) floorTex = roadTex;

            if (floorTex.id > 0) {
                DrawCubeTexture(floorTex, Vector3{ (float)c, 0.0f, (float)r },
                    1.0f, 0.05f, 1.0f, WHITE);
            }
        }
    }
}

void DrawDoor(const Door& door) {
    // Placeholder
}

void UpdateDoors(float deltaTime) {
    // Placeholder
}

Door* GetNearestDoor(Vector3 playerPos, float maxDistance) {
    return nullptr;
}

bool IsAABBInFrustum(const Camera3D& camera, const AABB& box) {
    float distance = Vector3Distance(camera.position,
        Vector3{ (box.min.x + box.max.x) * 0.5f,
                (box.min.y + box.max.y) * 0.5f,
                (box.min.z + box.max.z) * 0.5f });
    return distance < 100.0f;
}

// Helper to draw textured cube
extern void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);