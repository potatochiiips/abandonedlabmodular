#include "inventory.h"
#include "items.h"
#include "model_manager.h"
#include "player.h" // For GetModelIDFromItem - this is the correct header

// Static variables for drag and drop
static bool isDragging = false;
static int draggedSlotIndex = -1;
static bool draggedFromHand = false;
static Vector2 dragOffset = { 0, 0 };

int FindEmptySlot(InventorySlot* inventory) {
    for (int i = 0; i < BACKPACK_SLOTS; i++) {
        if (inventory[i].itemId == ITEM_NONE) return i;
    }
    return -1;
}

bool AddItemToInventory(InventorySlot* inventory, int itemId, int quantity, int ammo) {
    int slot = FindEmptySlot(inventory);
    if (slot != -1) {
        inventory[slot] = { itemId, quantity, ammo };
        return true;
    }
    return false;
}

void UseEquippedItem(InventorySlot* inventory, float* health, float* stamina, float* hunger, float* thirst) {
    InventorySlot* equippedSlot = &inventory[BACKPACK_SLOTS];
    if (equippedSlot->itemId == ITEM_NONE) return;

    bool itemUsed = false;
    switch (equippedSlot->itemId) {
    case ITEM_WATER_BOTTLE:
        *thirst = fminf(100.0f, *thirst + 50.0f);
        *health = fminf(100.0f, *health + 5.0f);
        itemUsed = true;
        TraceLog(LOG_INFO, "Drank water bottle. Thirst restored.");
        break;
    case ITEM_POTATO_CHIPS:
        *hunger = fminf(100.0f, *hunger + 40.0f);
        *thirst = fmaxf(0.0f, *thirst - 10.0f);
        itemUsed = true;
        TraceLog(LOG_INFO, "Ate potato chips. Hunger reduced.");
        break;
    case ITEM_FLASHLIGHT:
        TraceLog(LOG_INFO, "Use F key to toggle flashlight.");
        break;
    case ITEM_PISTOL:
    case ITEM_M16:
        TraceLog(LOG_INFO, "Use left mouse button or RT to shoot.");
        break;
    case ITEM_LAB_KEY:
        TraceLog(LOG_INFO, "Lab key will be used automatically at locked doors.");
        break;
    default:
        TraceLog(LOG_INFO, TextFormat("Cannot use %s", GetItemName(equippedSlot->itemId)));
        break;
    }

    if (itemUsed) {
        equippedSlot->quantity--;
        if (equippedSlot->quantity <= 0) {
            equippedSlot->itemId = ITEM_NONE;
            equippedSlot->quantity = 0;
            equippedSlot->ammo = 0;
        }
    }
}

bool ReloadWeapon(InventorySlot* inventory) {
    InventorySlot* equippedSlot = &inventory[BACKPACK_SLOTS];
    if (equippedSlot->itemId != ITEM_PISTOL && equippedSlot->itemId != ITEM_M16) {
        TraceLog(LOG_INFO, "No weapon equipped that can be reloaded.");
        return false;
    }

    int maxAmmo = 15;
    int magId = ITEM_MAG;
    int magAmmoCount = 15;

    if (equippedSlot->itemId == ITEM_M16) {
        maxAmmo = 30;
        magId = ITEM_M16_MAG;
        magAmmoCount = 30;
    }

    if (equippedSlot->ammo >= maxAmmo) {
        TraceLog(LOG_INFO, "Weapon already at maximum ammo.");
        return false;
    }

    int magSlotIndex = -1;
    for (int i = 0; i < BACKPACK_SLOTS; i++) {
        if (inventory[i].itemId == magId && inventory[i].quantity > 0) {
            magSlotIndex = i;
            break;
        }
    }

    if (magSlotIndex == -1) {
        TraceLog(LOG_INFO, "No magazine in inventory! Find more ammo.");
        return false;
    }

    int ammoNeeded = maxAmmo - equippedSlot->ammo;
    int ammoToAdd = (ammoNeeded < magAmmoCount) ? ammoNeeded : magAmmoCount;
    equippedSlot->ammo += ammoToAdd;

    inventory[magSlotIndex].quantity--;
    if (inventory[magSlotIndex].quantity <= 0) {
        inventory[magSlotIndex].itemId = ITEM_NONE;
        inventory[magSlotIndex].quantity = 0;
        inventory[magSlotIndex].ammo = 0;
    }

    TraceLog(LOG_INFO, TextFormat("Reloaded! Ammo: %d/%d", equippedSlot->ammo, maxAmmo));
    return true;
}

void DrawInventory(int screenW, int screenH, InventorySlot* inventory, int* selectedHandSlot, int* selectedInvSlot, bool useController) {
    int invWidth = (int)(screenW * 0.85f);
    int invHeight = (int)(screenH * 0.8f);
    int padding = 15;

    int cols = 9;
    int rows = BACKPACK_SLOTS / cols;
    int availableWidth = invWidth - (padding * 3);
    int slotSize = (int)fminf(50.0f, (float)availableWidth / (float)(cols + 0.5f));
    int spacing = (int)fmaxf(3.0f, slotSize * 0.1f);

    int invX = (screenW - invWidth) / 2;
    int invY = (screenH - invHeight) / 2 + 50;

    DrawText("Drag & Drop items to move them", invX + padding, invY + 10, 14, PIPBOY_DIM);

    Vector2 mousePos = GetMousePosition();
    bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    // --- Hand Slots ---
    int handY = invY + padding + 30;
    DrawText("EQUIPPED:", invX + padding, handY - 20, 15, PIPBOY_DIM);

    for (int i = 0; i < HAND_SLOTS; i++) {
        int index = BACKPACK_SLOTS + i;
        int x = invX + padding + i * (slotSize + spacing);

        bool isSelected = (*selectedHandSlot == i);
        bool isHovered = (mousePos.x >= x && mousePos.x <= x + slotSize && mousePos.y >= handY && mousePos.y <= handY + slotSize);
        bool isBeingDragged = (isDragging && draggedFromHand && draggedSlotIndex == i);
        Color borderColor = isSelected ? PIPBOY_GREEN : (isHovered ? PIPBOY_GREEN : PIPBOY_DIM);

        if (!isBeingDragged) {
            DrawRectangle(x, handY, slotSize, slotSize, PIPBOY_DARK);
            DrawRectangleLinesEx(Rectangle{ (float)x, (float)handY, (float)slotSize, (float)slotSize }, 2, borderColor);

            if (inventory[index].itemId != ITEM_NONE) {
                DrawText(GetItemName(inventory[index].itemId), x + 5, handY + 5, 12, PIPBOY_GREEN);
                if (inventory[index].quantity > 1) DrawText(TextFormat("x%d", inventory[index].quantity), x + 5, handY + 40, 15, PIPBOY_GREEN);
                if (inventory[index].ammo > 0) DrawText(TextFormat("A:%d", inventory[index].ammo), x + 5, handY + 25, 12, PIPBOY_DIM);
            }
        }
        else {
            DrawRectangle(x, handY, slotSize, slotSize, Color{ 20, 40, 20, 255 });
            DrawRectangleLinesEx(Rectangle{ (float)x, (float)handY, (float)slotSize, (float)slotSize }, 2, PIPBOY_DIM);
        }

        if (isHovered && mousePressed && inventory[index].itemId != ITEM_NONE && !isDragging) {
            isDragging = true;
            draggedSlotIndex = i;
            draggedFromHand = true;
            dragOffset.x = mousePos.x - x;
            dragOffset.y = mousePos.y - handY;
            *selectedHandSlot = i;
            *selectedInvSlot = -1;
        }

        if (isHovered && mouseReleased && isDragging) {
            if (draggedFromHand) {
                InventorySlot temp = inventory[BACKPACK_SLOTS + draggedSlotIndex];
                inventory[BACKPACK_SLOTS + draggedSlotIndex] = inventory[index];
                inventory[index] = temp;
            }
            else {
                InventorySlot temp = inventory[draggedSlotIndex];
                inventory[draggedSlotIndex] = inventory[index];
                inventory[index] = temp;
            }
            isDragging = false;
            draggedSlotIndex = -1;
        }

        if (isHovered && mousePressed && !isDragging) {
            *selectedHandSlot = i;
            *selectedInvSlot = -1;
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

        bool isSelected = (*selectedInvSlot == i);
        bool isHovered = (mousePos.x >= x && mousePos.x <= x + slotSize && mousePos.y >= y && mousePos.y <= y + slotSize);
        bool isBeingDragged = (isDragging && !draggedFromHand && draggedSlotIndex == i);
        Color borderColor = isSelected ? PIPBOY_GREEN : (isHovered ? PIPBOY_GREEN : PIPBOY_DIM);

        if (!isBeingDragged) {
            DrawRectangle(x, y, slotSize, slotSize, PIPBOY_DARK);
            DrawRectangleLinesEx(Rectangle{ (float)x, (float)y, (float)slotSize, (float)slotSize }, 2, borderColor);

            if (inventory[i].itemId != ITEM_NONE) {
                DrawText(GetItemName(inventory[i].itemId), x + 5, y + 5, 12, PIPBOY_GREEN);
                if (inventory[i].quantity > 1) DrawText(TextFormat("x%d", inventory[i].quantity), x + 5, y + 40, 15, PIPBOY_GREEN);
                if (inventory[i].ammo > 0) DrawText(TextFormat("A:%d", inventory[i].ammo), x + 5, y + 25, 12, PIPBOY_DIM);
            }
        }
        else {
            DrawRectangle(x, y, slotSize, slotSize, Color{ 20, 40, 20, 255 });
            DrawRectangleLinesEx(Rectangle{ (float)x, (float)y, (float)slotSize, (float)slotSize }, 2, PIPBOY_DIM);
        }

        if (isHovered && mousePressed && inventory[i].itemId != ITEM_NONE && !isDragging) {
            isDragging = true;
            draggedSlotIndex = i;
            draggedFromHand = false;
            dragOffset.x = mousePos.x - x;
            dragOffset.y = mousePos.y - y;
            *selectedInvSlot = i;
            *selectedHandSlot = -1;
        }

        if (isHovered && mouseReleased && isDragging) {
            if (draggedFromHand) {
                InventorySlot temp = inventory[BACKPACK_SLOTS + draggedSlotIndex];
                inventory[BACKPACK_SLOTS + draggedSlotIndex] = inventory[i];
                inventory[i] = temp;
            }
            else {
                InventorySlot temp = inventory[draggedSlotIndex];
                inventory[draggedSlotIndex] = inventory[i];
                inventory[i] = temp;
            }
            isDragging = false;
            draggedSlotIndex = -1;
        }

        if (isHovered && mousePressed && !isDragging) {
            *selectedInvSlot = i;
            *selectedHandSlot = -1;
        }
    }

    // Draw dragged item
    if (isDragging && mouseDown) {
        int drawX = (int)(mousePos.x - dragOffset.x);
        int drawY = (int)(mousePos.y - dragOffset.y);
        DrawRectangle(drawX, drawY, slotSize, slotSize, Color{ 40, 80, 40, 230 });
        DrawRectangleLinesEx(Rectangle{ (float)drawX, (float)drawY, (float)slotSize, (float)slotSize }, 3, PIPBOY_GREEN);

        int sourceIndex = draggedFromHand ? (BACKPACK_SLOTS + draggedSlotIndex) : draggedSlotIndex;
        InventorySlot draggedItem = inventory[sourceIndex];

        if (draggedItem.itemId != ITEM_NONE) {
            DrawText(GetItemName(draggedItem.itemId), drawX + 5, drawY + 5, 12, PIPBOY_GREEN);
            if (draggedItem.quantity > 1) DrawText(TextFormat("x%d", draggedItem.quantity), drawX + 5, drawY + 40, 15, PIPBOY_GREEN);
            if (draggedItem.ammo > 0) DrawText(TextFormat("A:%d", draggedItem.ammo), drawX + 5, drawY + 25, 12, PIPBOY_DIM);
        }
    }

    if (mouseReleased && isDragging) {
        isDragging = false;
        draggedSlotIndex = -1;
    }

    // --- Item Details Panel WITH 3D PREVIEW ---
    int detailPanelX = invX + invWidth - 210;
    int detailPanelW = 200;
    int detailPanelH = invHeight - 60;

    DrawRectangle(detailPanelX, invY + 40, detailPanelW, detailPanelH, PIPBOY_DARK);
    DrawRectangleLines(detailPanelX, invY + 40, detailPanelW, detailPanelH, PIPBOY_GREEN);
    DrawText("ITEM DETAILS", detailPanelX + 10, invY + 50, 18, PIPBOY_GREEN);

    int selectedIndex = (*selectedHandSlot != -1) ? BACKPACK_SLOTS + *selectedHandSlot : *selectedInvSlot;
    if (selectedIndex >= 0 && selectedIndex < TOTAL_INVENTORY_SLOTS) {
        const InventorySlot& item = inventory[selectedIndex];
        if (item.itemId != ITEM_NONE) {
            int detailY = invY + 90;

            // === 3D ITEM PREVIEW ===
            if (g_ModelManager) {
                // Create a small 3D viewport for item preview
                int previewSize = 120;
                int previewX = detailPanelX + (detailPanelW - previewSize) / 2;
                int previewY = detailY;

                // Draw preview background
                DrawRectangle(previewX, previewY, previewSize, previewSize, Color{ 10, 20, 10, 255 });
                DrawRectangleLines(previewX, previewY, previewSize, previewSize, PIPBOY_GREEN);

                // Set up a mini 3D camera for item preview
                Camera3D previewCam = { 0 };
                previewCam.position = Vector3{ 0.3f, 0.2f, 0.3f };
                previewCam.target = Vector3{ 0.0f, 0.0f, 0.0f };
                previewCam.up = Vector3{ 0.0f, 1.0f, 0.0f };
                previewCam.fovy = 45.0f;
                previewCam.projection = CAMERA_PERSPECTIVE;

                // Render item model in 3D
                BeginScissorMode(previewX, previewY, previewSize, previewSize);
                BeginMode3D(previewCam);

                // Rotate item slowly for better view
                float rotation = (float)GetTime() * 30.0f;
                Vector3 itemPos = Vector3{ 0.0f, 0.0f, 0.0f };
                Vector3 forward = Vector3{ cosf(rotation * DEG2RAD), 0.0f, sinf(rotation * DEG2RAD) };
                Vector3 right = Vector3{ -sinf(rotation * DEG2RAD), 0.0f, cosf(rotation * DEG2RAD) };
                Vector3 up = Vector3{ 0.0f, 1.0f, 0.0f };

                ModelID modelId = GetModelIDFromItem(item.itemId);
                g_ModelManager->DrawModel(modelId, itemPos, forward, right, up, WHITE);

                EndMode3D();
                EndScissorMode();

                detailY += previewSize + 10;
            }

            // Text details below preview
            DrawText(TextFormat("Name: %s", GetItemName(item.itemId)), detailPanelX + 10, detailY, 15, PIPBOY_GREEN);
            DrawText(TextFormat("Qty: %d", item.quantity), detailPanelX + 10, detailY + 20, 15, PIPBOY_GREEN);
            if (item.ammo > 0)
                DrawText(TextFormat("Ammo: %d", item.ammo), detailPanelX + 10, detailY + 40, 15, PIPBOY_GREEN);
        }
    }
}