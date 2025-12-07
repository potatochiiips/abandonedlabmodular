#include "inventory.h"
#include "items.h" // For item name lookup

int FindEmptySlot(InventorySlot* inventory) { 
    for(int i = 0; i < BACKPACK_SLOTS; i++) { if (inventory[i].itemId == ITEM_NONE) return i; } 
    return -1; 
}

bool AddItemToInventory(InventorySlot* inventory, int itemId, int quantity, int ammo) { 
    int slot = FindEmptySlot(inventory);
    if (slot != -1) { inventory[slot] = {itemId, quantity, ammo}; return true; }
    return false;
}

void DrawInventory(int screenW, int screenH, InventorySlot* inventory, int* selectedHandSlot, int* selectedInvSlot, bool useController) {
    // [Implementation of DrawInventory]
    int invWidth = 600;
    int invHeight = 400;
    int padding = 20;
    int slotSize = 60;
    int spacing = 10;
    int cols = 9; // BACKPACK_SLOTS / rows
    int rows = BACKPACK_SLOTS / cols;

    int invX = (screenW - invWidth) / 2;
    int invY = (screenH - invHeight) / 2;
    
    DrawRectangle(invX, invY, invWidth, invHeight, PIPBOY_DARK);
    DrawRectangleLines(invX, invY, invWidth, invHeight, PIPBOY_GREEN);
    DrawText("INVENTORY (I/Y)", invX + padding, invY + 10, 20, PIPBOY_GREEN);

    // --- Hand Slots ---
    int handY = invY + padding + 30;
    DrawText("EQUIPPED:", invX + padding, handY - 20, 15, PIPBOY_DIM);
    for (int i = 0; i < HAND_SLOTS; i++) {
        int index = BACKPACK_SLOTS + i;
        int x = invX + padding + i * (slotSize + spacing);
        Color borderColor = (*selectedHandSlot == i) ? PIPBOY_GREEN : PIPBOY_DIM;
        DrawRectangle(x, handY, slotSize, slotSize, PIPBOY_DARK);
        DrawRectangleLinesEx(Rectangle{(float)x, (float)handY, (float)slotSize, (float)slotSize}, 2, borderColor);
        if (inventory[index].itemId != ITEM_NONE) {
            DrawText(GetItemName(inventory[index].itemId), x + 5, handY + 5, 12, PIPBOY_GREEN);
            if (inventory[index].quantity > 1) DrawText(TextFormat("x%d", inventory[index].quantity), x + 5, handY + 40, 15, PIPBOY_GREEN);
            if (inventory[index].ammo > 0) DrawText(TextFormat("A: %d", inventory[index].ammo), x + 5, handY + 25, 12, PIPBOY_DIM);
        }
    }

    // --- Backpack Slots ---
    int backpackY = handY + slotSize + spacing + 10;
    DrawText("BACKPACK:", invX + padding, backpackY - 20, 15, PIPBOY_DIM);
    for (int i = 0; i < BACKPACK_SLOTS; i++) {
        int row = i / cols;
        int col = i % cols;
        int x = invX + padding + col * (slotSize + spacing);
        int y = backpackY + row * (slotSize + spacing);
        
        Color borderColor = (*selectedInvSlot == i) ? PIPBOY_GREEN : PIPBOY_DIM;
        DrawRectangle(x, y, slotSize, slotSize, PIPBOY_DARK);
        DrawRectangleLinesEx(Rectangle{(float)x, (float)y, (float)slotSize, (float)slotSize}, 2, borderColor);

        if (inventory[i].itemId != ITEM_NONE) {
            DrawText(GetItemName(inventory[i].itemId), x + 5, y + 5, 12, PIPBOY_GREEN);
            if (inventory[i].quantity > 1) DrawText(TextFormat("x%d", inventory[i].quantity), x + 5, y + 40, 15, PIPBOY_GREEN);
            if (inventory[i].ammo > 0) DrawText(TextFormat("A: %d", inventory[i].ammo), x + 5, y + 25, 12, PIPBOY_DIM);
        }

        // Mouse selection
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsCursorHidden() == false) { 
            Vector2 mousePos = GetMousePosition();
            if (mousePos.x >= x && mousePos.x <= x + slotSize && mousePos.y >= y && mousePos.y <= y + slotSize) {
                *selectedInvSlot = i;
                *selectedHandSlot = -1; 
            }
        }
    }
    
    // --- Item Details ---
    int detailPanelX = invX + invWidth + 10;
    DrawRectangle(detailPanelX, invY, 200, invHeight, PIPBOY_DARK);
    DrawRectangleLines(detailPanelX, invY, 200, invHeight, PIPBOY_GREEN);
    DrawText("ITEM DETAILS", detailPanelX + 10, invY + 10, 18, PIPBOY_GREEN);

    int selectedIndex = (*selectedHandSlot != -1) ? BACKPACK_SLOTS + *selectedHandSlot : *selectedInvSlot;
    if (selectedIndex >= 0 && selectedIndex < TOTAL_INVENTORY_SLOTS) {
        const InventorySlot& item = inventory[selectedIndex];
        int detailY = invY + 50;
        DrawText(TextFormat("Name: %s", GetItemName(item.itemId)), detailPanelX + 10, detailY, 15, PIPBOY_GREEN);
        DrawText(TextFormat("Qty: %d", item.quantity), detailPanelX + 10, detailY + 20, 15, PIPBOY_GREEN);
        if (item.ammo > 0) DrawText(TextFormat("Ammo: %d", item.ammo), detailPanelX + 10, detailY + 40, 15, PIPBOY_GREEN);
    }
}