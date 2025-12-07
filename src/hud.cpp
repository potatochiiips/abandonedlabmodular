#include "hud.h"
#include "items.h" // For item name lookup

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
    
    // Equipped Item Info (Hand Slot 0)
    int itemX = screenW / 2 - 100;
    int itemY = screenH - 30;
    const InventorySlot& equipped = inventory[BACKPACK_SLOTS];
    if (equipped.itemId != ITEM_NONE) {
        DrawText(TextFormat("Equipped: %s", GetItemName(equipped.itemId)), itemX, itemY - 20, 15, PIPBOY_GREEN);
        if (equipped.quantity > 1) {
            DrawText(TextFormat("Qty: x%d", equipped.quantity), itemX, itemY, 15, PIPBOY_GREEN);
        }
        if (equipped.ammo > 0) {
            DrawText(TextFormat("Ammo: %d", equipped.ammo), itemX + 80, itemY, 15, PIPBOY_DIM);
        }
    }
}