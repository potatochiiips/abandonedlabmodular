#include "hud.h"
#include "items.h"

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
        // Draw equipped item box
        DrawRectangle(itemX, itemY, 300, 40, Color{ 0, 0, 0, 180 });
        DrawRectangleLines(itemX, itemY, 300, 40, PIPBOY_GREEN);

        // Item name
        DrawText(GetItemName(equipped.itemId), itemX + 10, itemY + 5, 18, PIPBOY_GREEN);

        // Quantity (if more than 1)
        if (equipped.quantity > 1) {
            DrawText(TextFormat("x%d", equipped.quantity), itemX + 180, itemY + 5, 18, PIPBOY_GREEN);
        }

        // Ammo display for weapons (large and prominent)
        if (equipped.itemId == ITEM_PISTOL) {
            // Current ammo in magazine
            DrawText(TextFormat("%d", equipped.ammo), itemX + 10, itemY + 25, 16,
                equipped.ammo > 0 ? PIPBOY_GREEN : Color{ 255, 50, 50, 255 });

            // Count magazines in inventory
            int magCount = 0;
            for (int i = 0; i < BACKPACK_SLOTS; i++) {
                if (inventory[i].itemId == ITEM_MAG) {
                    magCount += inventory[i].quantity;
                }
            }

            // Reserve ammo (magazines)
            DrawText(TextFormat("/ %d mags", magCount), itemX + 40, itemY + 25, 14, PIPBOY_DIM);

            // Reload hint if empty
            if (equipped.ammo == 0 && magCount > 0) {
                DrawText("Press R to reload", itemX + 150, itemY + 25, 14, Color{ 255, 200, 50, 255 });
            }
        }

        // Instructions for consumables
        if (equipped.itemId == ITEM_WATER_BOTTLE || equipped.itemId == ITEM_POTATO_CHIPS) {
            DrawText("Right-click to use", itemX + 150, itemY + 25, 12, PIPBOY_DIM);
        }
    }
    else {
        // No item equipped
        DrawRectangle(itemX, itemY, 300, 40, Color{ 0, 0, 0, 100 });
        DrawRectangleLines(itemX, itemY, 300, 40, PIPBOY_DIM);
        DrawText("No item equipped", itemX + 80, itemY + 12, 16, PIPBOY_DIM);
    }
}