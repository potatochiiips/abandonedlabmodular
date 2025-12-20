#include "hud.h"
#include "items.h"
#include "rlgl.h"

void DrawHUD(int screenW, int screenH, float health, float stamina, float hunger, float thirst, float fov, float flashlightBattery, bool isFlashlightOn, InventorySlot* inventory) {
    int barWidth = 200;
    int barHeight = 20;
    int barX = 10;
    int barYStart = screenH - 100;

    // Health Bar
    DrawRectangle(barX, barYStart, barWidth, barHeight, PIPBOY_DARK);
    DrawRectangle(barX, barYStart, (int)(barWidth * (health / 100.0f)), barHeight, PIPBOY_GREEN);
    DrawText(TextFormat("HP: %.0f", health), barX + 5, barYStart + 3, 15, BLACK);

    // Stamina Bar
    DrawRectangle(barX, barYStart + barHeight + 5, barWidth, barHeight, PIPBOY_DARK);
    DrawRectangle(barX, barYStart + barHeight + 5, (int)(barWidth * (stamina / 100.0f)), barHeight, PIPBOY_GREEN);
    DrawText(TextFormat("STA: %.0f", stamina), barX + 5, barYStart + barHeight + 8, 15, BLACK);

    // Hunger Bar
    DrawRectangle(barX, barYStart + (barHeight + 5) * 2, barWidth, barHeight, PIPBOY_DARK);
    DrawRectangle(barX, barYStart + (barHeight + 5) * 2, (int)(barWidth * (hunger / 100.0f)), barHeight, PIPBOY_GREEN);
    DrawText(TextFormat("HNG: %.0f", hunger), barX + 5, barYStart + (barHeight + 5) * 2 + 3, 15, BLACK);

    // Thirst Bar
    DrawRectangle(barX, barYStart + (barHeight + 5) * 3, barWidth, barHeight, PIPBOY_DARK);
    DrawRectangle(barX, barYStart + (barHeight + 5) * 3, (int)(barWidth * (thirst / 100.0f)), barHeight, PIPBOY_GREEN);
    DrawText(TextFormat("THR: %.0f", thirst), barX + 5, barYStart + (barHeight + 5) * 3 + 3, 15, BLACK);

    // Flashlight Status
    if (isFlashlightOn) {
        int batteryX = screenW - 100;
        int batteryY = screenH - 30;
        DrawText("FLASHLIGHT ON", batteryX - 100, batteryY - 20, 15, PIPBOY_GREEN);
        DrawRectangle(batteryX, batteryY, 80, 20, PIPBOY_DARK);
        DrawRectangle(batteryX, batteryY, (int)(80 * (flashlightBattery / 100.0f)), 20, PIPBOY_GREEN);
        DrawText(TextFormat("BATT: %.0f%%", flashlightBattery), batteryX + 5, batteryY + 3, 15, BLACK);
    }

    // Equipped Item Info (Hand Slot 0) - Bottom Center
    int itemX = screenW / 2 - 150;
    int itemY = screenH - 50;
    const InventorySlot& equipped = inventory[BACKPACK_SLOTS];

    if (equipped.itemId != ITEM_NONE) {
        DrawRectangle(itemX, itemY, 300, 40, Color{ 0, 0, 0, 180 });
        DrawRectangleLines(itemX, itemY, 300, 40, PIPBOY_GREEN);
        DrawText(GetItemName(equipped.itemId), itemX + 10, itemY + 5, 18, PIPBOY_GREEN);

        if (equipped.quantity > 1) {
            DrawText(TextFormat("x%d", equipped.quantity), itemX + 180, itemY + 5, 18, PIPBOY_GREEN);
        }

        if (equipped.itemId == ITEM_PISTOL || equipped.itemId == ITEM_M16) {
            DrawText(TextFormat("%d", equipped.ammo), itemX + 10, itemY + 25, 16,
                equipped.ammo > 0 ? PIPBOY_GREEN : Color{ 255, 50, 50, 255 });

            int magId = (equipped.itemId == ITEM_PISTOL) ? ITEM_MAG : ITEM_M16_MAG;
            int magCount = 0;
            for (int i = 0; i < BACKPACK_SLOTS; i++) {
                if (inventory[i].itemId == magId) {
                    magCount += inventory[i].quantity;
                }
            }

            DrawText(TextFormat("/ %d mags", magCount), itemX + 40, itemY + 25, 14, PIPBOY_DIM);

            if (equipped.ammo == 0 && magCount > 0) {
                DrawText("Press R to reload", itemX + 150, itemY + 25, 14, Color{ 255, 200, 50, 255 });
            }
        }

        if (equipped.itemId == ITEM_WATER_BOTTLE || equipped.itemId == ITEM_POTATO_CHIPS) {
            DrawText("Right-click to use", itemX + 150, itemY + 25, 12, PIPBOY_DIM);
        }
    }
    else {
        DrawRectangle(itemX, itemY, 300, 40, Color{ 0, 0, 0, 100 });
        DrawRectangleLines(itemX, itemY, 300, 40, PIPBOY_DIM);
        DrawText("No item equipped", itemX + 80, itemY + 12, 16, PIPBOY_DIM);
    }
}

// NEW: Draw round minimap with circular mask
void DrawRoundMinimap(char map[MAP_SIZE][MAP_SIZE], Vector3 playerPos, float yaw,
    int centerX, int centerY, int radius, int viewRange) {
    // Create render texture for minimap
    RenderTexture2D minimapTarget = LoadRenderTexture(radius * 2, radius * 2);

    BeginTextureMode(minimapTarget);
    ClearBackground(BLANK);

    // Draw map tiles
    int playerX = (int)playerPos.x;
    int playerZ = (int)playerPos.z;
    float cellSize = (float)(radius * 2) / (float)(viewRange * 2);

    for (int r = -viewRange; r < viewRange; ++r) {
        for (int c = -viewRange; c < viewRange; ++c) {
            int worldX = playerX + c;
            int worldZ = playerZ + r;

            if (worldX < 0 || worldX >= MAP_SIZE || worldZ < 0 || worldZ >= MAP_SIZE) continue;

            // Check if within circle
            float dx = c * cellSize;
            float dy = r * cellSize;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > radius) continue;

            Color col = PIPBOY_DIM;
            switch (map[worldZ][worldX]) {
            case '~': col = Color{ 30, 60, 120, 255 }; break;
            case 'B': col = Color{ 100, 100, 120, 255 }; break;
            case '=': col = Color{ 80, 80, 80, 255 }; break;
            case '"': col = Color{ 30, 120, 30, 200 }; break;
            case '.': col = Color{ 90, 90, 95, 255 }; break;
            }

            int drawX = (int)((c + viewRange) * cellSize);
            int drawY = (int)((r + viewRange) * cellSize);
            DrawRectangle(drawX, drawY, (int)ceilf(cellSize), (int)ceilf(cellSize), col);
        }
    }

    // Draw player arrow in center
    Vector2 centerPos = { (float)radius, (float)radius };
    float arrowSize = fmaxf(5.0f, cellSize * 1.5f);

    // Calculate arrow points based on yaw
    Vector2 p1 = { centerPos.x + arrowSize * cosf((yaw - 90.0f) * DEG2RAD),
                   centerPos.y + arrowSize * sinf((yaw - 90.0f) * DEG2RAD) };
    Vector2 p2 = { centerPos.x + arrowSize * 0.5f * cosf((yaw + 150.0f) * DEG2RAD),
                   centerPos.y + arrowSize * 0.5f * sinf((yaw + 150.0f) * DEG2RAD) };
    Vector2 p3 = { centerPos.x + arrowSize * 0.5f * cosf((yaw - 150.0f) * DEG2RAD),
                   centerPos.y + arrowSize * 0.5f * sinf((yaw - 150.0f) * DEG2RAD) };

    DrawTriangle(p1, p2, p3, Color{ 255, 50, 50, 255 });
    DrawTriangle(p1, p3, p2, Color{ 255, 100, 100, 255 }); // Backface

    EndTextureMode();

    // Draw the minimap texture with circular mask
    BeginScissorMode(centerX - radius, centerY - radius, radius * 2, radius * 2);

    // Draw black background circle
    DrawCircle(centerX, centerY, (float)radius, Color{ 0, 0, 0, 200 });

    // Draw minimap texture
    DrawTexturePro(
        minimapTarget.texture,
        Rectangle{ 0, 0, (float)(radius * 2), (float)(-radius * 2) },
        Rectangle{ (float)(centerX - radius), (float)(centerY - radius), (float)(radius * 2), (float)(radius * 2) },
        Vector2{ 0, 0 },
        0.0f,
        WHITE
    );

    EndScissorMode();

    // Draw outer ring
    DrawRing(Vector2{ (float)centerX, (float)centerY }, (float)(radius - 2), (float)(radius + 2), 0, 360, 64, PIPBOY_GREEN);

    // Draw compass markers
    const char* directions[] = { "N", "E", "S", "W" };
    float angles[] = { 0.0f, 90.0f, 180.0f, 270.0f };

    for (int i = 0; i < 4; i++) {
        float angle = (angles[i] - yaw) * DEG2RAD;
        float markerDist = radius + 15.0f;
        int textX = (int)(centerX + markerDist * cosf(angle - PI / 2));
        int textY = (int)(centerY + markerDist * sinf(angle - PI / 2));

        int textW = MeasureText(directions[i], 16);
        DrawText(directions[i], textX - textW / 2, textY - 8, 16, PIPBOY_GREEN);
    }

    UnloadRenderTexture(minimapTarget);
}