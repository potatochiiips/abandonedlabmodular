#include "globals.h"
#include <cstdlib>
#include <cmath>
#include <ctime>

// Tile definitions for hybrid map
#define ROAD_TILE '='
#define GRASS_TILE '"'
#define BUILDING_TILE 'B'
#define TREE_TILE 'T'
#define LIGHT_TILE 'L'

// Building placement configuration
#define ROAD_SPACING 16
#define ROAD_DEVIATION 8
#define BUILDING_FOOTPRINT_W 6
#define BUILDING_FOOTPRINT_H 4
#define BUILDING_CHANCE 70  // % chance

// ----------------- Micro-Level Building Interior -----------------
void generate_building_interior(char (*interior_map)[20]) {
    for (int r = 0; r < 20; ++r) {
        for (int c = 0; c < 20; ++c) {
            if (r == 0 || r == 19 || c == 0 || c == 19) interior_map[r][c] = TILE_WALL;
            else interior_map[r][c] = TILE_FLOOR;
        }
    }
    interior_map[10][19] = TILE_DOOR;  // Door/exit
}

// ----------------- Macro-Level Terrain -----------------
void initialize_terrain(char (*map)[WORLD_SIZE]) {
    for (int r = 0; r < WORLD_SIZE; ++r)
        for (int c = 0; c < WORLD_SIZE; ++c)
            map[r][c] = GRASS_TILE;
}

void generate_roads(char (*map)[WORLD_SIZE]) {
    for (int i = 0; i < WORLD_SIZE; i += ROAD_SPACING)
        for (int c = 0; c < WORLD_SIZE; ++c) {
            map[i][c] = ROAD_TILE;
            map[c][i] = ROAD_TILE;
        }
    for (int i = ROAD_DEVIATION; i < WORLD_SIZE; i += ROAD_SPACING)
        for (int c = 0; c < WORLD_SIZE; ++c)
            if (std::rand() % 10 < 9) map[i][c] = ROAD_TILE;
}

// ----------------- Meso-Level Structures -----------------
void place_buildings_and_features(char (*map)[WORLD_SIZE]) {
    int step_r = BUILDING_FOOTPRINT_H + 2;
    int step_c = BUILDING_FOOTPRINT_W + 2;

    
        for (int r = 1; r < WORLD_SIZE - step_r; r += step_r) {
            for (int c = 1; c < WORLD_SIZE - step_c; c += step_c) {
                bool near_road = (map[r - 1][c] == ROAD_TILE || map[r][c - 1] == ROAD_TILE);
                if (near_road && (std::rand() % 100 < BUILDING_CHANCE)) {
                    for (int br = r; br < r + BUILDING_FOOTPRINT_H; ++br)
                        for (int bc = c; bc < c + BUILDING_FOOTPRINT_W; ++bc)
                            map[br][bc] = BUILDING_TILE;
                }
            }
        }

    // Trees and lights along roads/buildings
    for (int r = 1; r < WORLD_SIZE - 1; ++r) {
        for (int c = 1; c < WORLD_SIZE - 1; ++c) {
            bool adjacent_to_structure = map[r + 1][c] == ROAD_TILE || map[r - 1][c] == ROAD_TILE || map[r][c + 1] == BUILDING_TILE || map[r][c - 1] == BUILDING_TILE;
            if (map[r][c] == GRASS_TILE && adjacent_to_structure) {
                if (std::rand() % 100 < 2) map[r][c] = LIGHT_TILE;
                else if (std::rand() % 100 < 5) map[r][c] = TREE_TILE;
            }
        }
    }
    ```

}

// ----------------- Main Map Generation -----------------
void GenerateMap(char (* const map)[WORLD_SIZE]) {
    std::srand((unsigned int)time(NULL));

    ```
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
    ```

}

// ----------------- 2D Map Menu Rendering -----------------
void DrawMapMenu(int screenW, int screenH, char (* const map)[MAP_SIZE], Vector3 cameraPos, float zoom) {
    (void)zoom;

    ```
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

    float cellW = (float)mapAreaW / (float)MAP_SIZE;
    float cellH = (float)mapAreaH / (float)MAP_SIZE;

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

    float px = cameraPos.x;
    float pz = cameraPos.z;
    if (px >= 0 && px < MAP_SIZE && pz >= 0 && pz < MAP_SIZE) {
        float fx = mapAreaX + px * cellW;
        float fy = mapAreaY + pz * cellH;
        float markerW = fmaxf(2.0f, cellW * 0.6f);
        float markerH = fmaxf(2.0f, cellH * 0.6f);
        DrawRectangle((int)(fx - markerW / 2.0f), (int)(fy - markerH / 2.0f), (int)markerW, (int)markerH, Color{ 255,50,50,220 });
    }
    ```

}

// ----------------- Minimap Rendering -----------------
void DrawMinimap(char (* const map)[MAP_SIZE], Vector3 cameraPos, float yawDegrees, int x, int y, int w, int h, bool showBorder, int highlight) {
    (void)highlight;

    ```
        int mmW = w > 0 ? w : 160;
    int mmH = h > 0 ? h : 160;
    DrawRectangle(x, y, mmW, mmH, Color{ 0,0,0,200 });
    if (showBorder) DrawRectangleLines(x, y, mmW, mmH, PIPBOY_GREEN);

    float cellW = (float)mmW / (float)MAP_SIZE;
    float cellH = (float)mmH / (float)MAP_SIZE;

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
            case GRASS_TILE: col = Color{ 20,90,20,180 }; break;
            }
            int rx = x + (int)floorf(c * cellW);
            int ry = y + (int)floorf(r * cellH);
            int rw = (int)ceilf(cellW);
            int rh = (int)ceilf(cellH);
            DrawRectangle(rx, ry, rw, rh, col);
        }
    }

    float px = cameraPos.x;
    float pz = cameraPos.z;
    if (px >= 0 && px < MAP_SIZE && pz >= 0 && pz < MAP_SIZE) {
        float fx = x + px * cellW;
        float fy = y + pz * cellH;
        float markerSize = fmaxf(3.0f, fminf(cellW, cellH) * 0.8f);
        float rad = yawDegrees * DEG2RAD;
        Vector2 tip = { fx + cosf(rad) * markerSize * 1.5f, fy + sinf(rad) * markerSize * 1.5f };
        Vector2 base1 = { fx - cosf(rad) * markerSize * 0.5f - sinf(rad) * markerSize * 0.7f, fy - sinf(rad) * markerSize * 0.5f + cosf(rad) * markerSize * 0.7f };
        Vector2 base2 = { fx - cosf(rad) * markerSize * 0.5f + sinf(rad) * markerSize * 0.7f, fy - sinf(rad) * markerSize * 0.5f - cosf(rad) * markerSize * 0.7f };
        DrawTriangle(tip, base1, base2, Color{ 255,80,80,240 });
        DrawTriangleLines(tip, base1, base2, Color{ 200,40,40,255 });
        DrawCircle((int)fx, (int)fy, markerSize * 0.3f, Color{ 255,200,200,200 });
    }
    ```

}

// ----------------- 3D Map Geometry -----------------
void DrawMapGeometry(char (* const map)[MAP_SIZE]) {
    for (int r = 0; r < MAP_SIZE; ++r) {
        for (int c = 0; c < MAP_SIZE; ++c) {
            Vector3 pos = { (float)c,0.5f,(float)r };
            if (map[r][c] == TILE_WALL || map[r][c] == BUILDING_TILE) {
                Color col = (map[r][c] == TILE_WALL) ? Color{ 100,100,105,255 } : Color{ 120,120,130,255 };
                DrawCubeV(pos, (Vector3) { 1.0f, 1.0f, 1.0f }, col);
                DrawCubeWiresV(pos, (Vector3) { 1.0f, 1.0f, 1.0f }, Color{ 40,40,45,255 });
            }
            else if (map[r][c] == TILE_FLOOR || map[r][c] == GRASS_TILE) {
                Vector3 floorSize = { 1.0f,0.05f,1.0f };
                Color floorCol = (map[r][c] == TILE_FLOOR) ? Color{ 60,65,60,255 } : Color{ 50,100,50,255 };
                DrawCubeV((Vector3) { (float)c, 0.0f, (float)r }, floorSize, floorCol);
            }
            else if (map[r][c] == TILE_DOOR) {
                DrawCubeV(pos, (Vector3) { 1.0f, 1.0f, 0.1f }, Color{ 120,80,50,255 });
            }
        }
    }
}
