#include "globals.h"
#include "map.h"
#include "texture_manager.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

// Tile definitions
#define ROAD_TILE '='
#define GRASS_TILE '"'
#define BUILDING_TILE 'B'
#define TREE_TILE 'T'
#define LIGHT_TILE 'L'
#define BUSH_TILE 'b'
#define FLOWER_TILE 'f'

// Building config
#define ROAD_SPACING 16
#define ROAD_DEVIATION 8
#define BUILDING_FOOTPRINT_W 8
#define BUILDING_FOOTPRINT_H 8
#define BUILDING_CHANCE 60

// Global variables for building system
std::vector<BuildingInterior> buildingInteriors;
std::vector<Door> doors;
int currentFloor = -1;
int currentBuildingIndex = -1;

// Helper function to draw textured cube with shader support
void DrawTexturedCube(Vector3 position, Vector3 size, Texture2D texture, Color tint = WHITE) {
    if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
        BeginShaderMode(g_ShaderManager->GetLightingShader());
    }

    // Use the DrawCubeTexture function from main.cpp
    extern void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color);
    DrawCubeTexture(texture, position, size.x, size.y, size.z, tint);

    if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
        EndShaderMode();
    }
}

// Helper to generate random building interior
void GenerateBuildingInterior(BuildingInterior& building, int floors) {
    building.floors = floors;

    for (int f = 0; f < floors; f++) {
        // Generate walls
        for (int r = 0; r < 20; r++) {
            for (int c = 0; c < 20; c++) {
                if (r == 0 || r == 19 || c == 0 || c == 19) {
                    building.layout[f][r][c] = TILE_WALL;
                }
                else {
                    building.layout[f][r][c] = TILE_FLOOR;
                }
            }
        }

        // Add some interior walls for rooms
        if (f == 0) {
            // Ground floor - rooms
            for (int r = 5; r < 15; r++) {
                building.layout[f][r][10] = TILE_WALL;
            }
            building.layout[f][10][10] = TILE_DOOR; // Interior door

            // Vertical wall
            for (int c = 5; c < 15; c++) {
                if (c != 10) building.layout[f][10][c] = TILE_WALL;
            }
        }

        // Entry door on ground floor
        if (f == 0) {
            building.layout[f][19][10] = TILE_DOOR;
        }

        // Stairs to next floor
        if (f < floors - 1) {
            building.layout[f][2][2] = 'S'; // Stairs up
        }
        if (f > 0) {
            building.layout[f][2][2] = 's'; // Stairs down
        }
    }
}

// Initialize terrain
void initialize_terrain(char map[WORLD_SIZE][WORLD_SIZE]) {
    for (int r = 0; r < WORLD_SIZE; ++r)
        for (int c = 0; c < WORLD_SIZE; ++c)
            map[r][c] = GRASS_TILE;
}

// Generate roads
void generate_roads(char map[WORLD_SIZE][WORLD_SIZE]) {
    for (int i = 0; i < WORLD_SIZE; i += ROAD_SPACING)
        for (int c = 0; c < WORLD_SIZE; ++c) {
            map[i][c] = ROAD_TILE;
            map[c][i] = ROAD_TILE;
        }
}

// Place buildings with interiors
void place_buildings_and_features(char map[WORLD_SIZE][WORLD_SIZE]) {
    int step_r = BUILDING_FOOTPRINT_H + 2;
    int step_c = BUILDING_FOOTPRINT_W + 2;

    buildingInteriors.clear();
    doors.clear();

    for (int r = 1; r < WORLD_SIZE - step_r; r += step_r) {
        for (int c = 1; c < WORLD_SIZE - step_c; c += step_c) {
            bool near_road = (r > 0 && map[r - 1][c] == ROAD_TILE) ||
                (c > 0 && map[r][c - 1] == ROAD_TILE);

            if (near_road && (std::rand() % 100 < BUILDING_CHANCE)) {
                // Mark building footprint
                for (int br = r; br < r + BUILDING_FOOTPRINT_H; ++br)
                    for (int bc = c; bc < c + BUILDING_FOOTPRINT_W; ++bc)
                        map[br][bc] = BUILDING_TILE;

                // Create building interior
                BuildingInterior building;
                building.worldPos = Vector3{ (float)c + BUILDING_FOOTPRINT_W / 2.0f, 0, (float)r + BUILDING_FOOTPRINT_H / 2.0f };
                building.width = BUILDING_FOOTPRINT_W;
                building.depth = BUILDING_FOOTPRINT_H;
                building.floors = 1 + (std::rand() % 3); // 1-3 floors

                GenerateBuildingInterior(building, building.floors);
                buildingInteriors.push_back(building);

                // Create entry door
                Door entryDoor;
                entryDoor.position = Vector3{ (float)c + BUILDING_FOOTPRINT_W / 2.0f, 0.5f, (float)(r + BUILDING_FOOTPRINT_H) };
                entryDoor.isOpen = false;
                entryDoor.openAmount = 0.0f;
                entryDoor.targetFloor = 0;
                entryDoor.targetPosition = Vector3{ 10.0f, 0.5f, 17.0f }; // Inside position
                entryDoor.isStairs = false;
                doors.push_back(entryDoor);
            }
        }
    }

    // Add vegetation near roads/buildings
    for (int r = 1; r < WORLD_SIZE - 1; ++r) {
        for (int c = 1; c < WORLD_SIZE - 1; ++c) {
            if (map[r][c] == GRASS_TILE) {
                int rand_val = std::rand() % 100;

                // Trees
                if (rand_val < 3) {
                    map[r][c] = TREE_TILE;
                }
                // Bushes
                else if (rand_val < 8) {
                    map[r][c] = BUSH_TILE;
                }
                // Flowers
                else if (rand_val < 12) {
                    map[r][c] = FLOWER_TILE;
                }
            }
        }
    }
}

// Main map generation
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
}

// Draw vegetation with textures
void DrawVegetation(int x, int z, char tileType) {
    Vector3 pos = Vector3{ (float)x, 0.0f, (float)z };

    if (tileType == TREE_TILE) {
        // Tree trunk with bark texture
        Texture2D barkTex = g_TextureManager->GetTexture(TEX_TREE_BARK);

        // Using shader if available
        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            BeginShaderMode(g_ShaderManager->GetLightingShader());
        }
        DrawCylinderEx(Vector3{ (float)x, 0.0f, (float)z },
            Vector3{ (float)x, 1.0f, (float)z },
            0.15f, 0.12f, 8, Color{ 101, 67, 33, 255 });
        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            EndShaderMode();
        }

        // Tree foliage with leaf texture
        Texture2D leafTex = g_TextureManager->GetTexture(TEX_TREE_LEAVES);
        Vector3 foliagePos1 = Vector3{ (float)x, 1.5f, (float)z };
        Vector3 foliagePos2 = Vector3{ (float)x + 0.2f, 1.7f, (float)z };
        Vector3 foliagePos3 = Vector3{ (float)x - 0.2f, 1.6f, (float)z + 0.1f };

        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            BeginShaderMode(g_ShaderManager->GetLightingShader());
        }
        DrawSphere(foliagePos1, 0.6f, Color{ 34, 139, 34, 255 });
        DrawSphere(foliagePos2, 0.5f, Color{ 50, 205, 50, 255 });
        DrawSphere(foliagePos3, 0.5f, Color{ 34, 139, 34, 255 });
        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            EndShaderMode();
        }
    }
    else if (tileType == BUSH_TILE) {
        // Bush with texture
        Texture2D bushTex = g_TextureManager->GetTexture(TEX_BUSH_LEAVES);
        Vector3 bushPos = Vector3{ (float)x, 0.2f, (float)z };

        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            BeginShaderMode(g_ShaderManager->GetLightingShader());
        }
        DrawSphere(bushPos, 0.3f, Color{ 46, 125, 50, 255 });
        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            EndShaderMode();
        }
    }
    else if (tileType == FLOWER_TILE) {
        // Flowers with texture
        Texture2D flowerTex = g_TextureManager->GetTexture(TEX_FLOWER_PETALS);
        Vector3 flowerPos = Vector3{ (float)x, 0.1f, (float)z };
        Color colors[] = {
            Color{255, 20, 147, 255}, // Pink
            Color{255, 255, 0, 255},  // Yellow
            Color{138, 43, 226, 255}, // Purple
            Color{255, 69, 0, 255}    // Red
        };
        int colorIdx = (x + z) % 4;

        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            BeginShaderMode(g_ShaderManager->GetLightingShader());
        }
        DrawSphere(flowerPos, 0.08f, colors[colorIdx]);
        DrawCylinder(Vector3{ (float)x, 0.05f, (float)z }, 0.02f, 0.02f, 0.1f, 4, Color{ 34, 139, 34, 255 });
        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            EndShaderMode();
        }
    }
}

// Draw building exterior with textures and windows
void DrawBuildingExterior(const BuildingInterior& building) {
    float height = (float)building.floors * 3.0f;
    Vector3 pos = building.worldPos;
    pos.y = height / 2.0f;

    // Get textures
    Texture2D exteriorTex = g_TextureManager->GetTexture(TEX_BUILDING_EXTERIOR);
    Texture2D windowTex = g_TextureManager->GetTexture(TEX_WINDOW_GLASS);
    Texture2D roofTex = g_TextureManager->GetTexture(TEX_ROOF_SHINGLES);

    // Main building body with texture
    Vector3 size = Vector3{ (float)building.width, height, (float)building.depth };
    DrawTexturedCube(pos, size, exteriorTex, Color{ 255, 255, 255, 255 });
    DrawCubeWiresV(pos, size, Color{ 80, 80, 90, 255 });

    // Draw windows for each floor
    for (int floor = 0; floor < building.floors; floor++) {
        float floorY = floor * 3.0f + 1.5f;

        // Front windows with glass texture
        for (int i = 1; i < building.width - 1; i += 2) {
            Vector3 windowPos = Vector3{
                building.worldPos.x - building.width / 2.0f + i + 0.5f,
                floorY,
                building.worldPos.z + building.depth / 2.0f + 0.01f
            };
            DrawTexturedCube(windowPos, Vector3{ 0.8f, 1.2f, 0.1f }, windowTex, Color{ 255, 255, 255, 180 });
        }

        // Side windows
        for (int i = 1; i < building.depth - 1; i += 2) {
            Vector3 windowPos = Vector3{
                building.worldPos.x + building.width / 2.0f + 0.01f,
                floorY,
                building.worldPos.z - building.depth / 2.0f + i + 0.5f
            };
            DrawTexturedCube(windowPos, Vector3{ 0.1f, 1.2f, 0.8f }, windowTex, Color{ 255, 255, 255, 180 });
        }
    }

    // Roof with shingles texture
    Vector3 roofPos = Vector3{ pos.x, height + 0.2f, pos.z };
    DrawTexturedCube(roofPos, Vector3{ (float)building.width + 0.5f, 0.3f, (float)building.depth + 0.5f }, roofTex);
}

// Draw building interior with textures
void DrawBuildingInterior(const BuildingInterior& building, int floor) {
    if (floor < 0 || floor >= building.floors) return;

    // Get interior textures
    Texture2D wallTex = g_TextureManager->GetTexture(TEX_WALL_CONCRETE);
    Texture2D floorTex = g_TextureManager->GetTexture(TEX_FLOOR_TILE);
    Texture2D ceilingTex = g_TextureManager->GetTexture(TEX_CEILING_TILE);
    Texture2D doorTex = g_TextureManager->GetTexture(TEX_DOOR_WOOD);

    for (int r = 0; r < 20; r++) {
        for (int c = 0; c < 20; c++) {
            Vector3 pos = Vector3{ (float)c, (float)floor * 3.0f + 0.5f, (float)r };
            char tile = building.layout[floor][r][c];

            if (tile == TILE_WALL) {
                DrawTexturedCube(pos, Vector3{ 1.0f, 3.0f, 1.0f }, wallTex);
                DrawCubeWiresV(pos, Vector3{ 1.0f, 3.0f, 1.0f }, Color{ 100, 100, 105, 255 });
            }
            else if (tile == TILE_FLOOR || tile == 'S' || tile == 's') {
                // Floor with texture
                DrawTexturedCube(Vector3{ (float)c, (float)floor * 3.0f, (float)r },
                    Vector3{ 1.0f, 0.1f, 1.0f }, floorTex);

                // Ceiling with texture
                DrawTexturedCube(Vector3{ (float)c, (float)floor * 3.0f + 2.9f, (float)r },
                    Vector3{ 1.0f, 0.1f, 1.0f }, ceilingTex);

                // Stairs marker
                if (tile == 'S' || tile == 's') {
                    if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
                        BeginShaderMode(g_ShaderManager->GetLightingShader());
                    }
                    DrawCube(pos, 0.8f, 0.3f, 0.8f, Color{ 139, 90, 43, 255 });
                    if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
                        EndShaderMode();
                    }
                }
            }
            else if (tile == TILE_DOOR) {
                // Door with texture
                DrawTexturedCube(pos, Vector3{ 1.0f, 2.0f, 0.1f }, doorTex);
            }
        }
    }
}

// Draw door with texture
void DrawDoor(const Door& door) {
    float openAngle = door.openAmount * 90.0f;

    Vector3 hingePos = door.position;
    hingePos.x -= 0.4f;

    Texture2D doorTex = g_TextureManager->GetTexture(TEX_DOOR_WOOD);

    // Draw door frame
    DrawTexturedCube(door.position, Vector3{ 1.0f, 2.0f, 0.15f }, doorTex, Color{ 101, 67, 33, 255 });

    // Draw door panel (rotates when opening)
    if (!door.isOpen || door.openAmount < 0.99f) {
        Vector3 doorPos = door.position;
        doorPos.x -= door.openAmount * 0.8f;
        DrawTexturedCube(doorPos, Vector3{ 0.8f, 1.8f, 0.1f }, doorTex, Color{ 139, 90, 43, 255 });

        // Door handle
        Vector3 handlePos = doorPos;
        handlePos.x += 0.3f;

        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            BeginShaderMode(g_ShaderManager->GetLightingShader());
        }
        DrawSphere(handlePos, 0.05f, Color{ 212, 175, 55, 255 });
        if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
            EndShaderMode();
        }
    }
}

// Update door animations
void UpdateDoors(float deltaTime) {
    for (auto& door : doors) {
        if (door.isOpen && door.openAmount < 1.0f) {
            door.openAmount += deltaTime * 2.0f;
            if (door.openAmount > 1.0f) door.openAmount = 1.0f;
        }
        else if (!door.isOpen && door.openAmount > 0.0f) {
            door.openAmount -= deltaTime * 2.0f;
            if (door.openAmount < 0.0f) door.openAmount = 0.0f;
        }
    }
}

// Get nearest door to player
Door* GetNearestDoor(Vector3 playerPos, float maxDistance) {
    Door* nearest = nullptr;
    float nearestDist = maxDistance;

    for (auto& door : doors) {
        float dist = Vector3Distance(playerPos, door.position);
        if (dist < nearestDist) {
            nearestDist = dist;
            nearest = &door;
        }
    }

    return nearest;
}

// Draw minimap with interior support
void DrawMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw, int minimapX, int minimapY, int minimapW, int minimapH, bool largeMap, int screenH) {
    DrawRectangle(minimapX, minimapY, minimapW, minimapH, Color{ 0, 0, 0, 180 });
    DrawRectangleLines(minimapX, minimapY, minimapW, minimapH, PIPBOY_GREEN);

    int viewRange = largeMap ? 20 : 15;
    float cellSize = (float)minimapW / (viewRange * 2);

    // Inside building - show interior layout
    if (currentFloor >= 0 && currentBuildingIndex >= 0) {
        const BuildingInterior& building = buildingInteriors[currentBuildingIndex];

        for (int r = 0; r < 20; r++) {
            for (int c = 0; c < 20; c++) {
                char tile = building.layout[currentFloor][r][c];
                Color col = PIPBOY_DIM;

                switch (tile) {
                case TILE_WALL: col = Color{ 90, 90, 90, 255 }; break;
                case TILE_DOOR: col = Color{ 200, 170, 60, 255 }; break;
                case TILE_FLOOR: col = Color{ 100, 100, 100, 255 }; break;
                case 'S': col = Color{ 0, 255, 0, 255 }; break; // Stairs up
                case 's': col = Color{ 255, 0, 0, 255 }; break; // Stairs down
                }

                int drawX = minimapX + (int)(c * cellSize);
                int drawY = minimapY + (int)(r * cellSize);
                DrawRectangle(drawX, drawY, (int)ceilf(cellSize), (int)ceilf(cellSize), col);
            }
        }

        // Draw player at their position in interior
        Vector2 playerMapPos = {
            minimapX + (int)(playerPos.x * cellSize),
            minimapY + (int)(playerPos.z * cellSize)
        };
        DrawCircleV(playerMapPos, fmaxf(3.0f, cellSize * 0.8f), Color{ 255, 50, 50, 255 });

        DrawText(TextFormat("Floor %d/%d", currentFloor + 1, building.floors),
            minimapX + 5, minimapY + 5, 12, PIPBOY_GREEN);
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
                case TILE_WALL: col = Color{ 90, 90, 90, 255 }; break;
                case TILE_DOOR: col = Color{ 200, 170, 60, 255 }; break;
                case BUILDING_TILE: col = Color{ 100, 100, 120, 255 }; break;
                case ROAD_TILE: col = Color{ 80, 80, 80, 255 }; break;
                case TREE_TILE: col = Color{ 10, 80, 10, 255 }; break;
                case BUSH_TILE: col = Color{ 46, 125, 50, 255 }; break;
                case FLOWER_TILE: col = Color{ 255, 105, 180, 255 }; break;
                case LIGHT_TILE: col = Color{ 255, 255, 180, 255 }; break;
                case GRASS_TILE: col = Color{ 30, 120, 30, 200 }; break;
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

// Draw 2D map menu
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

// Draw 3D map geometry with textures and shaders
void DrawMapGeometry(char map[MAP_SIZE][MAP_SIZE]) {
    // Update shader lighting before drawing (if available)
    if (g_ShaderManager) {
        // This is handled in main.cpp before calling this function
        // Just draw the geometry here
    }

    // Draw outside world
    if (currentFloor < 0) {
        Texture2D grassTex = g_TextureManager->GetTexture(TEX_GRASS);
        Texture2D roadTex = g_TextureManager->GetTexture(TEX_ROAD_ASPHALT);

        for (int r = 0; r < MAP_SIZE; ++r) {
            for (int c = 0; c < MAP_SIZE; ++c) {
                Vector3 pos = Vector3{ (float)c, 0.5f, (float)r };

                if (map[r][c] == BUILDING_TILE) {
                    // Buildings are drawn separately
                    continue;
                }
                else if (map[r][c] == TILE_FLOOR || map[r][c] == GRASS_TILE || map[r][c] == ROAD_TILE) {
                    Texture2D floorTex;
                    if (map[r][c] == ROAD_TILE) floorTex = roadTex;
                    else if (map[r][c] == GRASS_TILE) floorTex = grassTex;
                    else floorTex = g_TextureManager->GetTexture(TEX_FLOOR_CONCRETE);

                    DrawTexturedCube(Vector3{ (float)c, 0.0f, (float)r },
                        Vector3{ 1.0f, 0.05f, 1.0f }, floorTex);
                }
                else if (map[r][c] == TREE_TILE || map[r][c] == BUSH_TILE || map[r][c] == FLOWER_TILE) {
                    // Draw grass underneath vegetation
                    DrawTexturedCube(Vector3{ (float)c, 0.0f, (float)r },
                        Vector3{ 1.0f, 0.05f, 1.0f }, grassTex);
                    // Draw vegetation
                    DrawVegetation(c, r, map[r][c]);
                }
            }
        }

        // Draw building exteriors
        for (const auto& building : buildingInteriors) {
            DrawBuildingExterior(building);
        }

        // Draw doors
        for (const auto& door : doors) {
            DrawDoor(door);
        }
    }
    // Draw interior
    else if (currentBuildingIndex >= 0) {
        DrawBuildingInterior(buildingInteriors[currentBuildingIndex], currentFloor);
    }
}