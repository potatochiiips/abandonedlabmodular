#include "globals.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

// Tile definitions
#define ROAD_TILE '='
#define GRASS_TILE '"'
#define BUILDING_TILE 'B'
#define TREE_TILE 'T'
#define LIGHT_TILE 'L'

// Building config
#define ROAD_SPACING 16
#define ROAD_DEVIATION 8
#define BUILDING_FOOTPRINT_W 6
#define BUILDING_FOOTPRINT_H 4
#define BUILDING_CHANCE 70

// ----------------- Building Interior -----------------
void generate_building_interior(char interior_map[20][20]) {
    for (int r = 0; r < 20; ++r)
        for (int c = 0; c < 20; ++c)
            interior_map[r][c] = (r == 0 || r == 19 || c == 0 || c == 19) ? TILE_WALL : TILE_FLOOR;

    interior_map[10][19] = TILE_DOOR; // exit door
}

// ----------------- Terrain -----------------
void initialize_terrain(char map[WORLD_SIZE][WORLD_SIZE]) {
    for (int r = 0; r < WORLD_SIZE; ++r)
        for (int c = 0; c < WORLD_SIZE; ++c)
            map[r][c] = GRASS_TILE;
}

void generate_roads(char map[WORLD_SIZE][WORLD_SIZE]) {
    for (int i = 0; i < WORLD_SIZE; i += ROAD_SPACING)
        for (int c = 0; c < WORLD_SIZE; ++c) {
            map[i][c] = ROAD_TILE;
            map[c][i] = ROAD_TILE;
        }

    for (int i = ROAD_DEVIATION; i < WORLD_SIZE; i += ROAD_SPACING)
        for (int c = 0; c < WORLD_SIZE; ++c)
            if (std::rand() % 10 < 9) map[i][c] = ROAD_TILE;
}

// ----------------- Structures -----------------
void place_buildings_and_features(char map[WORLD_SIZE][WORLD_SIZE]) {
    int step_r = BUILDING_FOOTPRINT_H + 2;
    int step_c = BUILDING_FOOTPRINT_W + 2;

    for (int r = 1; r < WORLD_SIZE - step_r; r += step_r) {
        for (int c = 1; c < WORLD_SIZE - step_c; c += step_c) {
            bool near_road = map[r - 1][c] == ROAD_TILE || map[r][c - 1] == ROAD_TILE;
            if (near_road && (std::rand() % 100 < BUILDING_CHANCE)) {
                for (int br = r; br < r + BUILDING_FOOTPRINT_H; ++br)
                    for (int bc = c; bc < c + BUILDING_FOOTPRINT_W; ++bc)
                        map[br][bc] = BUILDING_TILE;
            }
        }
    }

    // Trees/lights near roads/buildings
    for (int r = 1; r < WORLD_SIZE - 1; ++r) {
        for (int c = 1; c < WORLD_SIZE - 1; ++c) {
            bool adjacent = map[r + 1][c] == ROAD_TILE || map[r - 1][c] == ROAD_TILE || map[r][c + 1] == BUILDING_TILE || map[r][c - 1] == BUILDING_TILE;
            if (map[r][c] == GRASS_TILE && adjacent) {
                if (std::rand() % 100 < 2) map[r][c] = LIGHT_TILE;
                else if (std::rand() % 100 < 5) map[r][c] = TREE_TILE;
            }
        }
    }
}

// ----------------- Main Map Generation -----------------
void GenerateMap(char map[WORLD_SIZE][WORLD_SIZE]) {
    std::srand((unsigned int)time(NULL));
    initialize_terrain(map);
    generate_roads(map);
    place_buildings_and_features(map);

    // World border
    for (int i = 0; i < WORLD_SIZE; ++i) {
        map[0][i] = GRASS_TILE;
        map[WORLD_SIZE - 1][i] = GRASS_TILE;
        map[i][0] = GRASS_TILE;
        map[i][WORLD_SIZE - 1] = GRASS_TILE;
    }

    map[WORLD_SIZE - 1][WORLD_SIZE / 2] = TILE_DOOR;
}

// ----------------- 2D Map Marker -----------------
void DrawMapMarker(Vector2 mapPos, float size, Color col) {
    Vector2 tip = Vector2{ mapPos.x, mapPos.y - size };
    Vector2 left = Vector2{ mapPos.x - size * 0.6f, mapPos.y + size * 0.5f };
    Vector2 right = Vector2{ mapPos.x + size * 0.6f, mapPos.y + size * 0.5f };
    DrawTriangle(tip, left, right, col);
}

// ----------------- Minimap Rendering -----------------
void DrawMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw, int minimapX, int minimapY, int minimapW, int minimapH, bool largeMap, int screenH) {
    (void)yaw;       // unused parameter
    (void)screenH;   // unused parameter

    // Background
    DrawRectangle(minimapX, minimapY, minimapW, minimapH, Color{ 0, 0, 0, 180 });
    DrawRectangleLines(minimapX, minimapY, minimapW, minimapH, PIPBOY_GREEN);

    // Calculate visible range
    int viewRange = largeMap ? 20 : 15;
    float cellSize = (float)minimapW / (viewRange * 2);

    int playerX = (int)playerPos.x;
    int playerZ = (int)playerPos.z;

    // Draw map tiles
    for (int r = -viewRange; r < viewRange; ++r) {
        for (int c = -viewRange; c < viewRange; ++c) {
            int worldX = playerX + c;
            int worldZ = playerZ + r;

            if (worldX < 0 || worldX >= MAP_SIZE || worldZ < 0 || worldZ >= MAP_SIZE) continue;

            Color col = PIPBOY_DIM;
            switch (map[worldZ][worldX]) {
            case TILE_WALL: col = Color{ 90, 90, 90, 255 }; break;
            case TILE_DOOR: col = Color{ 200, 170, 60, 255 }; break;
            case BUILDING_TILE: col = Color{ 100, 100, 120, 255 }; break;
            case ROAD_TILE: col = Color{ 80, 80, 80, 255 }; break;
            case TREE_TILE: col = Color{ 10, 80, 10, 255 }; break;
            case LIGHT_TILE: col = Color{ 255, 255, 180, 255 }; break;
            case GRASS_TILE: col = Color{ 30, 120, 30, 200 }; break;
            }

            int drawX = minimapX + (int)((c + viewRange) * cellSize);
            int drawY = minimapY + (int)((r + viewRange) * cellSize);
            DrawRectangle(drawX, drawY, (int)ceilf(cellSize), (int)ceilf(cellSize), col);
        }
    }

    // Draw player marker in center
    Vector2 centerPos = { minimapX + minimapW / 2.0f, minimapY + minimapH / 2.0f };
    DrawMapMarker(centerPos, fmaxf(3.0f, cellSize * 0.8f), Color{ 255, 50, 50, 255 });
}

// ----------------- 2D Map Menu Rendering -----------------
void DrawMapMenu(int screenW, int screenH, char map[MAP_SIZE][MAP_SIZE], Vector3 cameraPos, float zoom) {
    (void)zoom;
    const int menuW = screenW - 200;
    const int menuH = screenH - 120;
    const int menuX = 100;
    const int menuY = 60;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);
    DrawText("MAP", menuX + 20, menuY + 10, 30, PIPBOY_GREEN);

    int mapAreaX = menuX + 20;
    int mapAreaY = menuY + 60;
    int mapAreaW = menuW - 40;
    int mapAreaH = menuH - 80;

    float cellW = (float)mapAreaW / MAP_SIZE;
    float cellH = (float)mapAreaH / MAP_SIZE;

    for (int r = 0; r < MAP_SIZE; ++r) {
        for (int c = 0; c < MAP_SIZE; ++c) {
            Color col = PIPBOY_DIM;
            switch (map[r][c]) {
            case TILE_WALL: col = Color{ 90,90,90,255 }; break;
            case TILE_DOOR: col = Color{ 200,170,60,255 }; break;
            case BUILDING_TILE: col = Color{ 100,100,120,255 }; break;
            case ROAD_TILE: col = Color{ 80,80,80,255 }; break;
            case TREE_TILE: col = Color{ 10,80,10,255 }; break;
            case LIGHT_TILE: col = Color{ 255,255,180,255 }; break;
            case GRASS_TILE: col = Color{ 30,120,30,200 }; break;
            }
            int x = mapAreaX + (int)floorf(c * cellW);
            int y = mapAreaY + (int)floorf(r * cellH);
            int w = (int)ceilf(cellW);
            int h = (int)ceilf(cellH);
            DrawRectangle(x, y, w, h, col);
        }
    }

    // Draw player marker as triangle
    float px = cameraPos.x;
    float pz = cameraPos.z;
    if (px >= 0 && px < MAP_SIZE && pz >= 0 && pz < MAP_SIZE) {
        Vector2 mapPos = Vector2{ mapAreaX + px * cellW, mapAreaY + pz * cellH };
        DrawMapMarker(mapPos, fmaxf(2.0f, cellW * 0.6f), Color{ 255,50,50,220 });
    }
}

// ----------------- 3D Map Geometry -----------------
void DrawMapGeometry(char map[MAP_SIZE][MAP_SIZE]) {
    for (int r = 0; r < MAP_SIZE; ++r) {
        for (int c = 0; c < MAP_SIZE; ++c) {
            Vector3 pos = Vector3{ (float)c,0.5f,(float)r };
            if (map[r][c] == TILE_WALL || map[r][c] == BUILDING_TILE) {
                Color col = (map[r][c] == TILE_WALL) ? Color{ 100,100,105,255 } : Color{ 120,120,130,255 };
                DrawCubeV(pos, Vector3{ 1.0f,1.0f,1.0f }, col);
                DrawCubeWiresV(pos, Vector3{ 1.0f,1.0f,1.0f }, Color{ 40,40,45,255 });
            }
            else if (map[r][c] == TILE_FLOOR || map[r][c] == GRASS_TILE) {
                Vector3 floorSize = Vector3{ 1.0f,0.05f,1.0f };
                Color floorCol = (map[r][c] == TILE_FLOOR) ? Color{ 60,65,60,255 } : Color{ 50,100,50,255 };
                DrawCubeV(Vector3{ (float)c,0.0f,(float)r }, floorSize, floorCol);
            }
            else if (map[r][c] == TILE_DOOR) {
                DrawCubeV(pos, Vector3{ 1.0f,1.0f,0.1f }, Color{ 120,80,50,255 });
            }
        }
    }
}