#include "globals.h"
#include <cstdlib>
#include <cmath>

// Generate a simple map: border walls, interior floor with some random walls.
void GenerateMap(char (* const map)[31])
{
    for (int r = 0; r < MAP_SIZE; ++r)
    {
        for (int c = 0; c < MAP_SIZE; ++c)
        {
            if (r == 0 || r == MAP_SIZE - 1 || c == 0 || c == MAP_SIZE - 1)
                map[r][c] = TILE_WALL;
            else
                map[r][c] = TILE_FLOOR;
        }
    }

    std::srand(12345);
    for (int i = 0; i < (MAP_SIZE * MAP_SIZE) / 12; ++i)
    {
        int rx = 1 + std::rand() % (MAP_SIZE - 2);
        int ry = 1 + std::rand() % (MAP_SIZE - 2);
        map[ry][rx] = TILE_WALL;
    }

    map[MAP_SIZE / 2][MAP_SIZE - 2] = TILE_DOOR;
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

                // Draw main wall cube
                DrawCubeV(pos, Vector3{ 1.0f, 1.0f, 1.0f }, wallColor);

                // Add darker edges for depth
                DrawCubeWiresV(pos, Vector3{ 1.0f, 1.0f, 1.0f }, Color{ 40, 40, 45, 255 });

                // Add subtle horizontal lines for concrete texture
                for (float yOffset = 0.2f; yOffset < 0.9f; yOffset += 0.3f)
                {
                    Vector3 linePos = { (float)c, yOffset, (float)r };
                    DrawCube(linePos.x, linePos.y, linePos.z, 1.02f, 0.02f, 1.02f,
                        Color{ 80, 80, 85, 100 });
                }

                // Add shadowing to bottom
                Vector3 shadowPos = { (float)c, 0.05f, (float)r };
                DrawCube(shadowPos.x, shadowPos.y, shadowPos.z, 1.01f, 0.1f, 1.01f,
                    Color{ 20, 20, 25, 150 });
            }
            else if (map[r][c] == TILE_FLOOR)
            {
                // Enhanced floor with tile pattern
                Vector3 floorPos = { (float)c, 0.0f, (float)r };

                // Base floor color - dirty concrete
                Color floorColor = Color{ 60, 65, 60, 255 };

                // Add slight checkerboard pattern
                if ((r + c) % 2 == 0)
                {
                    floorColor = Color{ 55, 60, 55, 255 };
                }

                DrawCubeV(floorPos, Vector3{ 1.0f, 0.05f, 1.0f }, floorColor);

                // Add grout lines between tiles
                DrawCubeWiresV(floorPos, Vector3{ 1.0f, 0.05f, 1.0f }, Color{ 30, 35, 30, 180 });

                // Add random dirt/stains
                if ((r * 17 + c * 23) % 7 == 0)
                {
                    float stainSize = 0.2f + ((r * 13 + c * 19) % 10) * 0.03f;
                    float offsetX = ((r * 11 + c * 7) % 10) * 0.08f - 0.4f;
                    float offsetZ = ((r * 19 + c * 11) % 10) * 0.08f - 0.4f;

                    Vector3 stainPos = {
                        (float)c + offsetX,
                        0.051f,
                        (float)r + offsetZ
                    };
                    DrawCube(stainPos.x, stainPos.y, stainPos.z,
                        stainSize, 0.001f, stainSize,
                        Color{ 30, 35, 30, 180 });
                }
            }
            else if (map[r][c] == TILE_DOOR)
            {
                // Enhanced door with frame
                Vector3 doorPos = { (float)c, 0.5f, (float)r };

                // Door frame (metallic)
                Color frameColor = Color{ 80, 85, 90, 255 };
                DrawCube(doorPos.x, doorPos.y, doorPos.z, 1.0f, 1.0f, 0.1f, frameColor);
                DrawCubeWires(doorPos.x, doorPos.y, doorPos.z, 1.0f, 1.0f, 0.1f,
                    Color{ 50, 55, 60, 255 });

                // Door itself (wood texture approximation)
                Color doorColor = Color{ 120, 80, 50, 255 };
                DrawCube(doorPos.x, doorPos.y, doorPos.z, 0.9f, 0.9f, 0.08f, doorColor);

                // Door panels (raised sections)
                for (int panel = 0; panel < 2; panel++)
                {
                    float panelY = 0.3f + panel * 0.4f;
                    Vector3 panelPos = { (float)c, panelY, (float)r };
                    DrawCube(panelPos.x, panelPos.y, panelPos.z, 0.7f, 0.3f, 0.09f,
                        Color{ 130, 85, 55, 255 });
                }

                // Door handle
                Vector3 handlePos = { (float)c + 0.35f, 0.5f, (float)r };
                DrawSphere(handlePos, 0.05f, Color{ 200, 180, 140, 255 });

                // Shadow under door
                Vector3 shadowPos = { (float)c, 0.02f, (float)r };
                DrawCube(shadowPos.x, shadowPos.y, shadowPos.z, 1.0f, 0.04f, 0.2f,
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

                DrawCubeV(ceilingPos, Vector3{ 0.95f, 0.1f, 0.95f }, ceilingColor);
                DrawCubeWiresV(ceilingPos, Vector3{ 0.95f, 0.1f, 0.95f },
                    Color{ 40, 40, 45, 100 });
            }
        }
    }
}