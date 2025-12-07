#include "globals.h"
#include <cstdlib>
#include <cmath>

// Generate a simple map: border walls, interior floor with some random walls.
// Assuming these are defined elsewhere (e.g., in a header file)
// #define MAP_SIZE 100
// #define TILE_WALL '#'
// #define TILE_FLOOR '.'
// #define TILE_DOOR '+'
// --- MAP & TILE DEFINITIONS ---
//#define WORLD_SIZE 128     // Reduced for a more manageable example, but still large
//#define TILE_DOOR '+'      // Primary world entry/exit
#define ROAD_TILE '='
#define GRASS_TILE '"'
#define BUILDING_TILE 'B'
#define TREE_TILE 'T'
#define LIGHT_TILE 'L'

// --- BUILDING & PLACEMENT CONFIG ---
#define ROAD_SPACING 16      // Major roads every 16 tiles
#define ROAD_DEVIATION 8     // Minor roads offset by 8 tiles
#define BUILDING_FOOTPRINT_W 6 // Width of building footprint
#define BUILDING_FOOTPRINT_H 4 // Height of building footprint
#define BUILDING_CHANCE 70   // % chance to place a building next to a road

// --- HELPER FUNCTION: Generates a 20x20 interior (Micro-Level Concept) ---
// In a real application, this function would be called when the player enters a 'B' tile.
void generate_building_interior(char (*interior_map)[20]) {
    // Conceptual: Uses Binary Space Partitioning (BSP) to create rooms.
    // We only simulate the result here: a simple 20x20 room with an exit.

    // Fill the interior with walls and floors
    for (int r = 0; r < 20; ++r) {
        for (int c = 0; c < 20; ++c) {
            if (r == 0 || r == 19 || c == 0 || c == 19) {
                interior_map[r][c] = '#'; // Interior Wall
            }
            else {
                interior_map[r][c] = '.'; // Interior Floor
            }
        }
    }
    // Place an interior door/exit
    interior_map[10][19] = 'D';
}


// --- MACRO-LEVEL: Initialize Terrain and Roads ---

// Step 1: Initialize the world with grass/terrain
void initialize_terrain(char (*map)[WORLD_SIZE]) {
    for (int r = 0; r < WORLD_SIZE; ++r) {
        for (int c = 0; c < WORLD_SIZE; ++c) {
            map[r][c] = GRASS_TILE;
        }
    }
}

// Step 2: Generate a structured road network (Randomized Grid)
void generate_roads(char (*map)[WORLD_SIZE]) {
    // Generate major horizontal and vertical roads
    for (int i = 0; i < WORLD_SIZE; i += ROAD_SPACING) {
        for (int c = 0; c < WORLD_SIZE; ++c) {
            map[i][c] = ROAD_TILE; // Horizontal
            map[c][i] = ROAD_TILE; // Vertical
        }
    }

    // Add minor, offset roads for realism
    for (int i = ROAD_DEVIATION; i < WORLD_SIZE; i += ROAD_SPACING) {
        for (int c = 0; c < WORLD_SIZE; ++c) {
            // Apply a small random chance to skip road segments (for rural feel)
            if (std::rand() % 10 < 9) {
                map[i][c] = ROAD_TILE;
            }
        }
    }
}


// --- MESO-LEVEL: Placing Buildings and Features ---

void place_buildings_and_features(char (*map)[WORLD_SIZE]) {

    // 1. Place buildings
    // Iterate in steps slightly larger than the building size to ensure gaps
    int step_r = BUILDING_FOOTPRINT_H + 2;
    int step_c = BUILDING_FOOTPRINT_W + 2;

    for (int r = 1; r < WORLD_SIZE - step_r; r += step_r) {
        for (int c = 1; c < WORLD_SIZE - step_c; c += step_c) {

            // Check if the current plot (r, c) is adjacent to a road
            bool near_road = (map[r - 1][c] == ROAD_TILE || map[r][c - 1] == ROAD_TILE);

            if (near_road && (std::rand() % 100 < BUILDING_CHANCE)) {
                // Place the building footprint
                for (int br = r; br < r + BUILDING_FOOTPRINT_H; ++br) {
                    for (int bc = c; bc < c + BUILDING_FOOTPRINT_W; ++bc) {
                        map[br][bc] = BUILDING_TILE;
                    }
                }
            }
        }
    }

    // 2. Place trees and lights along roads/grass
    for (int r = 1; r < WORLD_SIZE - 1; ++r) {
        for (int c = 1; c < WORLD_SIZE - 1; ++c) {
            // Check if the tile is grass and next to a road or building (for urban density)
            bool adjacent_to_structure =
                (map[r + 1][c] == ROAD_TILE || map[r - 1][c] == ROAD_TILE ||
                    map[r][c + 1] == BUILDING_TILE || map[r][c - 1] == BUILDING_TILE);

            if (map[r][c] == GRASS_TILE && adjacent_to_structure) {
                if (std::rand() % 100 < 2) { // 2% chance for a streetlight
                    map[r][c] = LIGHT_TILE;
                }
                else if (std::rand() % 100 < 5) { // 5% chance for a tree
                    map[r][c] = TREE_TILE;
                }
            }
        }
    }
}


// --- MAIN GENERATION FUNCTION ---

void GenerateMap(char (* const map)[WORLD_SIZE])
{
    // Use the current time to ensure a unique map is generated each time
    std::srand(time(NULL));

    // 1. Macro-Level Generation: Land and Infrastructure
    initialize_terrain(map);
    generate_roads(map);
    // 

    // 2. Meso-Level Generation: Structures and Features
    place_buildings_and_features(map);

    // 3. Final Touches
    // Create a solid border of grass/wall (important for game boundaries)
    for (int i = 0; i < WORLD_SIZE; ++i) {
        map[0][i] = GRASS_TILE;
        map[WORLD_SIZE - 1][i] = GRASS_TILE;
        map[i][0] = GRASS_TILE;
        map[i][WORLD_SIZE - 1] = GRASS_TILE;
    }

    // Place the main entry/exit door in the world boundary
    map[WORLD_SIZE - 1][WORLD_SIZE / 2] = TILE_DOOR;

    // NOTE: The interior maps (Micro-Level) are generated on demand 
    // by the game engine using the 'generate_building_interior' function.
}

void DrawMapMenu(int screenW, int screenH, char (* const map)[31], Vector3 cameraPos, float zoom)
{
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

    float cellW = (float)mapAreaW / (float)MAP_SIZE;
    float cellH = (float)mapAreaH / (float)MAP_SIZE;

    for (int r = 0; r < MAP_SIZE; ++r)
    {
        for (int c = 0; c < MAP_SIZE; ++c)
        {
            Color col = PIPBOY_DIM;
            if (map[r][c] == TILE_WALL) col = Color{ 90, 90, 90, 255 };
            else if (map[r][c] == TILE_DOOR) col = Color{ 200, 170, 60, 255 };
            else col = Color{ 30, 120, 30, 200 };

            int x = mapAreaX + (int)floorf(c * cellW);
            int y = mapAreaY + (int)floorf(r * cellH);
            int w = (int)ceilf(cellW);
            int h = (int)ceilf(cellH);

            DrawRectangle(x, y, w, h, col);
        }
    }

    float pxf = cameraPos.x;
    float pzf = cameraPos.z;
    if (pxf >= 0.0f && pxf < (float)MAP_SIZE && pzf >= 0.0f && pzf < (float)MAP_SIZE)
    {
        float fx = mapAreaX + pxf * cellW;
        float fy = mapAreaY + pzf * cellH;
        float markerW = fmaxf(2.0f, cellW * 0.6f);
        float markerH = fmaxf(2.0f, cellH * 0.6f);
        DrawRectangle((int)(fx - markerW / 2.0f), (int)(fy - markerH / 2.0f), (int)markerW, (int)markerH, Color{ 255, 50, 50, 220 });
    }
}

void DrawMinimap(char (* const map)[31], Vector3 cameraPos, float yawDegrees, int x, int y, int w, int h, bool showBorder, int highlight)
{
    (void)highlight;

    int mmW = w <= 0 ? 160 : w;
    int mmH = h <= 0 ? 160 : h;

    DrawRectangle(x, y, mmW, mmH, Color{ 0, 0, 0, 200 });
    if (showBorder) DrawRectangleLines(x, y, mmW, mmH, PIPBOY_GREEN);

    float cellW = (float)mmW / (float)MAP_SIZE;
    float cellH = (float)mmH / (float)MAP_SIZE;

    for (int r = 0; r < MAP_SIZE; ++r)
    {
        for (int c = 0; c < MAP_SIZE; ++c)
        {
            Color col;
            if (map[r][c] == TILE_WALL)
                col = Color{ 90, 90, 90, 255 };
            else if (map[r][c] == TILE_DOOR)
                col = Color{ 200, 170, 60, 255 };
            else
                col = Color{ 20, 90, 20, 180 };

            int rx = x + (int)floorf(c * cellW);
            int ry = y + (int)floorf(r * cellH);
            int rw = (int)ceilf(cellW);
            int rh = (int)ceilf(cellH);

            DrawRectangle(rx, ry, rw, rh, col);
        }
    }

    float px = cameraPos.x;
    float pz = cameraPos.z;

    if (px >= 0.0f && px < (float)MAP_SIZE && pz >= 0.0f && pz < (float)MAP_SIZE)
    {
        float fx = x + px * cellW;
        float fy = y + pz * cellH;

        float markerSize = fmaxf(3.0f, fminf(cellW, cellH) * 0.8f);
        float rad = yawDegrees * DEG2RAD;

        float dirX = cosf(rad);
        float dirZ = sinf(rad);

        float arrowLength = markerSize * 1.5f;

        Vector2 tip = {
            fx + dirX * arrowLength,
            fy + dirZ * arrowLength
        };

        float perpX = -dirZ;
        float perpY = dirX;

        float baseWidth = markerSize * 0.7f;
        float baseBack = markerSize * 0.5f;

        Vector2 base1 = {
            fx - dirX * baseBack + perpX * baseWidth,
            fy - dirZ * baseBack + perpY * baseWidth
        };

        Vector2 base2 = {
            fx - dirX * baseBack - perpX * baseWidth,
            fy - dirZ * baseBack - perpY * baseWidth
        };

        DrawTriangle(tip, base1, base2, Color{ 255, 80, 80, 240 });
        DrawTriangleLines(tip, base1, base2, Color{ 200, 40, 40, 255 });
        DrawCircle((int)fx, (int)fy, markerSize * 0.3f, Color{ 255, 200, 200, 200 });
    }
}

// Enhanced realistic 3D geometry with better lighting and textures
void DrawMapGeometry(char (* const map)[31])
{
    for (int r = 0; r < MAP_SIZE; ++r)
    {
        for (int c = 0; c < MAP_SIZE; ++c)
        {
            Vector3 pos = { (float)c, 0.5f, (float)r };
            Vector3 size = { 1.0f, 1.0f, 1.0f };

            if (map[r][c] == TILE_WALL)
            {
                // Main wall with subtle color variation
                int variation = ((r * 7 + c * 13) % 20) - 10;
                Color wallColor = Color{
                    (unsigned char)(100 + variation),
                    (unsigned char)(100 + variation),
                    (unsigned char)(105 + variation),
                    255
                };

                // Draw main wall cube (using DrawCubeV)
                DrawCubeV(pos, size, wallColor);

                // Add darker edges for depth (using DrawCubeWiresV)
                DrawCubeWiresV(pos, size, Color{ 40, 40, 45, 255 });

                // Add subtle horizontal lines for concrete texture
                for (float yOffset = 0.2f; yOffset < 0.9f; yOffset += 0.3f)
                {
                    Vector3 linePos = { (float)c, yOffset, (float)r };
                    Vector3 lineSize = { 1.02f, 0.02f, 1.02f };
                    // FIXED: Use DrawCubeV
                    DrawCubeV(linePos, lineSize,
                        Color{ 80, 80, 85, 100 });
                }

                // Add shadowing to bottom
                Vector3 shadowPos = { (float)c, 0.05f, (float)r };
                Vector3 shadowSize = { 1.01f, 0.1f, 1.01f };
                // FIXED: Use DrawCubeV
                DrawCubeV(shadowPos, shadowSize,
                    Color{ 20, 20, 25, 150 });
            }
            else if (map[r][c] == TILE_FLOOR)
            {
                // Enhanced floor with tile pattern
                Vector3 floorPos = { (float)c, 0.0f, (float)r };
                Vector3 floorSize = { 1.0f, 0.05f, 1.0f };

                // Base floor color - dirty concrete
                Color floorColor = Color{ 60, 65, 60, 255 };

                // Add slight checkerboard pattern
                if ((r + c) % 2 == 0)
                {
                    floorColor = Color{ 55, 60, 55, 255 };
                }

                DrawCubeV(floorPos, floorSize, floorColor);

                // Add grout lines between tiles
                DrawCubeWiresV(floorPos, floorSize, Color{ 30, 35, 30, 180 });

                // Add random dirt/stains
                if ((r * 17 + c * 23) % 7 == 0)
                {
                    float stainSize = 0.2f + ((r * 13 + c * 19) % 10) * 0.03f;
                    float offsetX = ((r * 11 + c * 7) % 10) * 0.08f - 0.4f;
                    float offsetZ = ((r * 19 + c * 11) % 10) * 0.08f - 0.4f;

                    Vector3 stainPos = { (float)c + offsetX, 0.051f, (float)r + offsetZ };
                    Vector3 stainDim = { stainSize, 0.001f, stainSize };
                    // FIXED: Use DrawCubeV
                    DrawCubeV(stainPos, stainDim,
                        Color{ 30, 35, 30, 180 });
                }
            }
            else if (map[r][c] == TILE_DOOR)
            {
                // Enhanced door with frame
                Vector3 doorPos = { (float)c, 0.5f, (float)r };

                // Door frame (metallic)
                Color frameColor = Color{ 80, 85, 90, 255 };
                Vector3 frameSize = { 1.0f, 1.0f, 0.1f };
                // FIXED: Use DrawCubeV
                DrawCubeV(doorPos, frameSize, frameColor);

                // FIXED: Use DrawCubeWiresV
                DrawCubeWiresV(doorPos, frameSize,
                    Color{ 50, 55, 60, 255 });

                // Door itself (wood texture approximation)
                Color doorColor = Color{ 120, 80, 50, 255 };
                Vector3 doorSize = { 0.9f, 0.9f, 0.08f };
                // FIXED: Use DrawCubeV
                DrawCubeV(doorPos, doorSize, doorColor);

                // Door panels (raised sections)
                for (int panel = 0; panel < 2; panel++)
                {
                    float panelY = 0.3f + panel * 0.4f;
                    Vector3 panelPos = { (float)c, panelY, (float)r };
                    Vector3 panelSize = { 0.7f, 0.3f, 0.09f };
                    // FIXED: Use DrawCubeV
                    DrawCubeV(panelPos, panelSize,
                        Color{ 130, 85, 55, 255 });
                }

                // Door handle
                Vector3 handlePos = { (float)c + 0.35f, 0.5f, (float)r };
                DrawSphere(handlePos, 0.05f, Color{ 200, 180, 140, 255 });

                // Shadow under door
                Vector3 shadowPos = { (float)c, 0.02f, (float)r };
                Vector3 shadowSize = { 1.0f, 0.04f, 0.2f };
                // FIXED: Use DrawCubeV
                DrawCubeV(shadowPos, shadowSize,
                    Color{ 10, 10, 15, 150 });
            }
        }
    }

    // Add ceiling tiles for enclosed feeling
    for (int r = 0; r < MAP_SIZE; ++r)
    {
        for (int c = 0; c < MAP_SIZE; ++c)
        {
            if (map[r][c] != TILE_WALL)
            {
                Vector3 ceilingPos = { (float)c, 2.5f, (float)r };
                Color ceilingColor = Color{ 70, 70, 75, 255 };

                // Alternating ceiling tiles
                if ((r + c) % 2 == 0)
                {
                    ceilingColor = Color{ 65, 65, 70, 255 };
                }
                Vector3 ceilingSize = { 0.95f, 0.1f, 0.95f };

                DrawCubeV(ceilingPos, ceilingSize, ceilingColor);
                DrawCubeWiresV(ceilingPos, ceilingSize,
                    Color{ 40, 40, 45, 100 });
            }
        }
    }
}