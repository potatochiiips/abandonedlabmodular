#include "globals.h"
#include <cstdlib>
#include <cmath>

// Generate a simple map: border walls, interior floor with some random walls.
void GenerateMap(char (* const map)[31])
{
    // Ensure MAP_SIZE matches 31 as used across the project.
    for (int r = 0; r < MAP_SIZE; ++r)
    {
        for (int c = 0; c < MAP_SIZE; ++c)
        {
            // Border walls
            if (r == 0 || r == MAP_SIZE - 1 || c == 0 || c == MAP_SIZE - 1)
                map[r][c] = TILE_WALL;
            else
                map[r][c] = TILE_FLOOR;
        }
    }

    // Add a few random walls to make the map interesting (deterministic-ish)
    std::srand(12345);
    for (int i = 0; i < (MAP_SIZE * MAP_SIZE) / 12; ++i)
    {
        int rx = 1 + std::rand() % (MAP_SIZE - 2);
        int ry = 1 + std::rand() % (MAP_SIZE - 2);
        map[ry][rx] = TILE_WALL;
    }

    // Place a door somewhere
    map[MAP_SIZE/2][MAP_SIZE-2] = TILE_DOOR;
}

// Draw a simple, centered full-screen map menu
void DrawMapMenu(int screenW, int screenH, char (* const map)[31], Vector3 cameraPos, float zoom)
{
    (void)zoom; // keep signature compatible; not used in this simple stub

    const int menuW = screenW - 200;
    const int menuH = screenH - 120;
    const int menuX = 100;
    const int menuY = 60;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);
    DrawText("MAP", menuX + 20, menuY + 10, 30, PIPBOY_GREEN);

    // Draw the map grid scaled to the available area
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
            if (map[r][c] == TILE_WALL) col = Color{90, 90, 90, 255};
            else if (map[r][c] == TILE_DOOR) col = Color{200, 170, 60, 255};
            else col = Color{30, 120, 30, 200};

            int x = mapAreaX + (int)floorf(c * cellW);
            int y = mapAreaY + (int)floorf(r * cellH);
            int w = (int)ceilf(cellW);
            int h = (int)ceilf(cellH);

            DrawRectangle(x, y, w, h, col);
        }
    }

    // Draw player marker (fractional position)
    float pxf = cameraPos.x;
    float pzf = cameraPos.z;
    if (pxf >= 0.0f && pxf < (float)MAP_SIZE && pzf >= 0.0f && pzf < (float)MAP_SIZE)
    {
        float fx = mapAreaX + pxf * cellW;
        float fy = mapAreaY + pzf * cellH;
        float markerW = fmaxf(2.0f, cellW * 0.6f);
        float markerH = fmaxf(2.0f, cellH * 0.6f);
        DrawRectangle((int)(fx - markerW/2.0f), (int)(fy - markerH/2.0f), (int)markerW, (int)markerH, Color{255, 50, 50, 220});
    }
}

// Draw a compact minimap overlay (used during gameplay)
//
/** Note: the 3rd float parameter supplied by callers has historically been
    used for "zoom" but main.cpp passes `yaw` there. This function treats
    that float as the player's yaw in degrees for the direction indicator.
    Signature unchanged to remain ABI-compatible with existing calls.
*/
void DrawMinimap(char (* const map)[31], Vector3 cameraPos, float yawDegrees, int x, int y, int w, int h, bool showBorder, int highlight)
{
    (void)highlight; // unused in this implementation

    // Validate size and clamp
    int mmW = w <= 0 ? 160 : w;
    int mmH = h <= 0 ? 160 : h;

    // Draw background and optional border
    DrawRectangle(x, y, mmW, mmH, Color{0, 0, 0, 200});
    if (showBorder) DrawRectangleLines(x, y, mmW, mmH, PIPBOY_GREEN);

    // Compute cell size
    float cellW = (float)mmW / (float)MAP_SIZE;
    float cellH = (float)mmH / (float)MAP_SIZE;

    // Draw a simplified minimap: floor/wall/door
    for (int r = 0; r < MAP_SIZE; ++r)
    {
        for (int c = 0; c < MAP_SIZE; ++c)
        {
            Color col;
            if (map[r][c] == TILE_WALL) col = Color{90, 90, 90, 255};
            else if (map[r][c] == TILE_DOOR) col = Color{200, 170, 60, 255};
            else col = Color{20, 90, 20, 180};

            int rx = x + (int)floorf(c * cellW);
            int ry = y + (int)floorf(r * cellH);
            DrawRectangle(rx, ry, (int)ceilf(cellW), (int)ceilf(cellH), col);
        }
    }

    // Draw player marker with fractional precision and a direction indicator (triangle)
    float px = cameraPos.x;
    float pz = cameraPos.z;
    if (px >= 0.0f && px < (float)MAP_SIZE && pz >= 0.0f && pz < (float)MAP_SIZE)
    {
        float fx = x + px * cellW;
        float fy = y + pz * cellH;
        float radius = fmaxf(2.0f, fminf(cellW, cellH) * 0.5f);

        // Compute forward vector from yawDegrees (degrees -> radians)
        float rad = yawDegrees * DEG2RAD;
        float dirX = cosf(rad); // +X map direction
        float dirY = sinf(rad); // +Z map direction (mapped to screen Y)

        // Convert world-direction to minimap pixel-space direction (scale by cell size)
        float pdx = dirX * cellW;
        float pdy = dirY * cellH;

        // Normalize direction (avoid zero-length)
        float len = sqrtf(pdx * pdx + pdy * pdy);
        if (len < 1e-4f)
        {
            pdx = 0.0f;
            pdy = -1.0f;
            len = 1.0f;
        }
        pdx /= len;
        pdy /= len;

        // Triangle geometry: tip in front, base behind center, base width perpendicular to direction
        float tipDist = radius * 1.2f;
        float baseDist = radius * -0.6f; // behind the center
        float halfWidth = radius * 0.8f;

        // Perpendicular vector to (pdx, pdy)
        float perpX = -pdy;
        float perpY = pdx;

        // Points in pixel coordinates
        Vector2 tip = { fx + pdx * tipDist, fy + pdy * tipDist };
        Vector2 baseCenter = { fx + pdx * baseDist, fy + pdy * baseDist };
        Vector2 baseLeft = { baseCenter.x + perpX * halfWidth, baseCenter.y + perpY * halfWidth };
        Vector2 baseRight = { baseCenter.x - perpX * halfWidth, baseCenter.y - perpY * halfWidth };

        // Draw filled triangle for direction
        DrawTriangle(tip, baseLeft, baseRight, Color{255, 100, 100, 230});
        // Optional outline for clarity
        DrawTriangleLines(tip, baseLeft, baseRight, Color{200, 50, 50, 200});
    }
}

// Optional: draw simple 3D geometry for walls (called inside BeginMode3D)
void DrawMapGeometry(char (* const map)[31])
{
    // Simple cubes for walls at y = 0.5 (height 1.0)
    for (int r = 0; r < MAP_SIZE; ++r)
    {
        for (int c = 0; c < MAP_SIZE; ++c)
        {
            if (map[r][c] == TILE_WALL)
            {
                Vector3 pos = { (float)c, 0.5f, (float)r };
                DrawCubeV(pos, Vector3{ 1.0f, 1.0f, 1.0f }, Color{120, 120, 120, 255});
                DrawCubeWiresV(pos, Vector3{ 1.0f, 1.0f, 1.0f }, Color{40, 40, 40, 200});
            }
            else if (map[r][c] == TILE_DOOR)
            {
                Vector3 pos = { (float)c, 0.0f, (float)r };
                DrawCubeV(pos, Vector3{ 1.0f, 0.1f, 1.0f }, Color{200, 170, 60, 255});
            }
        }
    }
}