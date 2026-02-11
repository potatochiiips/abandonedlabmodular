#include "map.h"
#include "menus.h"
#include <sstream>
#include <iomanip>

void DrawWorldGenMenu(int screenW, int screenH, WorldSettings* settings, int* menuSelection, bool* confirmed) {
    if (!settings || !menuSelection) return;

    int menuWidth = 500;
    int menuHeight = 600;
    int menuX = (screenW - menuWidth) / 2;
    int menuY = (screenH - menuHeight) / 2;
    int itemHeight = 50;
    int itemY = menuY + 80;

    // Semi-transparent background
    DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 150 });

    // Menu panel
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, Color{ 40, 45, 50, 255 });
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, PIPBOY_GREEN);

    // Title
    DrawText("CREATE WORLD", menuX + 150, menuY + 20, 28, PIPBOY_GREEN);

    // Menu items
    const char* items[] = {
        "Seed",
        "Scale",
        "Octaves",
        "Persistence",
        "Lacunarity",
        "Structures",
        "Water",
        "Vegetation",
        "Generate"
    };
    const int itemCount = sizeof(items) / sizeof(items[0]);

    for (int i = 0; i < itemCount; i++) {
        Color textColor = (*menuSelection == i) ? Color{ 255, 255, 0, 255 } : PIPBOY_GREEN;
        Color bgColor = (*menuSelection == i) ? Color{ 50, 100, 50, 200 } : Color{ 0, 0, 0, 0 };

        if (*menuSelection == i) {
            DrawRectangle(menuX + 10, itemY + i * itemHeight, menuWidth - 20, itemHeight - 5, bgColor);
        }

        DrawText(items[i], menuX + 30, itemY + i * itemHeight + 10, 20, textColor);

        // Draw values
        switch (i) {
            case 0: // Seed
                DrawText(TextFormat("%u", settings->seed), menuX + 300, itemY + i * itemHeight + 10, 20, PIPBOY_GREEN);
                break;
            case 1: // Scale
                DrawText(TextFormat("%.2f", settings->scale), menuX + 300, itemY + i * itemHeight + 10, 20, PIPBOY_GREEN);
                break;
            case 2: // Octaves
                DrawText(TextFormat("%d", settings->octaves), menuX + 300, itemY + i * itemHeight + 10, 20, PIPBOY_GREEN);
                break;
            case 3: // Persistence
                DrawText(TextFormat("%.2f", settings->persistence), menuX + 300, itemY + i * itemHeight + 10, 20, PIPBOY_GREEN);
                break;
            case 4: // Lacunarity
                DrawText(TextFormat("%.2f", settings->lacunarity), menuX + 300, itemY + i * itemHeight + 10, 20, PIPBOY_GREEN);
                break;
            case 5: // Structures
                DrawText(settings->generateStructures ? "ON" : "OFF", menuX + 300, itemY + i * itemHeight + 10, 20, PIPBOY_GREEN);
                break;
            case 6: // Water
                DrawText(settings->generateWater ? "ON" : "OFF", menuX + 300, itemY + i * itemHeight + 10, 20, PIPBOY_GREEN);
                break;
            case 7: // Vegetation
                DrawText(settings->generateVegetation ? "ON" : "OFF", menuX + 300, itemY + i * itemHeight + 10, 20, PIPBOY_GREEN);
                break;
            case 8: // Generate
                DrawText("PRESS ENTER", menuX + 250, itemY + i * itemHeight + 10, 20, Color{ 255, 100, 100, 255 });
                break;
        }
    }

    // Instructions
    DrawText("ARROW KEYS to navigate | LEFT/RIGHT to adjust | ENTER to select", 
             menuX + 10, menuY + menuHeight - 40, 14, PIPBOY_DIM);
    DrawText("'R' for Random Seed", menuX + 10, menuY + menuHeight - 20, 14, PIPBOY_DIM);
}

void UpdateWorldGenMenu(WorldSettings* settings, int* menuSelection, bool* confirmed) {
    if (!settings || !menuSelection) return;

    if (IsKeyPressed(KEY_UP)) {
        *menuSelection = (*menuSelection - 1 + 9) % 9;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        *menuSelection = (*menuSelection + 1) % 9;
    }

    // Random seed
    if (IsKeyPressed(KEY_R)) {
        settings->seed = (uint32_t)GetRandomValue(0, INT_MAX);
    }

    switch (*menuSelection) {
        case 0: // Seed
            if (IsKeyPressed(KEY_LEFT)) {
                settings->seed = (settings->seed > 100) ? settings->seed - 100 : 0;
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                settings->seed += 100;
            }
            break;

        case 1: // Scale
            if (IsKeyPressed(KEY_LEFT)) {
                settings->scale = fmaxf(1.0f, settings->scale - 5.0f);
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                settings->scale = fminf(200.0f, settings->scale + 5.0f);
            }
            break;

        case 2: // Octaves
            if (IsKeyPressed(KEY_LEFT)) {
                settings->octaves = fmax(1, settings->octaves - 1);
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                settings->octaves = fmin(12, settings->octaves + 1);
            }
            break;

        case 3: // Persistence
            if (IsKeyPressed(KEY_LEFT)) {
                settings->persistence = fmaxf(0.0f, settings->persistence - 0.05f);
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                settings->persistence = fminf(1.0f, settings->persistence + 0.05f);
            }
            break;

        case 4: // Lacunarity
            if (IsKeyPressed(KEY_LEFT)) {
                settings->lacunarity = fmaxf(1.0f, settings->lacunarity - 0.1f);
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                settings->lacunarity = fminf(4.0f, settings->lacunarity + 0.1f);
            }
            break;

        case 5: // Structures
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                settings->generateStructures = !settings->generateStructures;
            }
            break;

        case 6: // Water
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                settings->generateWater = !settings->generateWater;
            }
            break;

        case 7: // Vegetation
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                settings->generateVegetation = !settings->generateVegetation;
            }
            break;

        case 8: // Generate
            if (IsKeyPressed(KEY_ENTER)) {
                *confirmed = true;
            }
            break;
    }
}