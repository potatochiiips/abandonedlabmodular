#pragma once
#include "globals.h"

void DrawInventory(int screenW, int screenH, InventorySlot* inventory, int* selectedHandSlot, int* selectedInvSlot, bool useController);
void DrawInventoryTab(int screenW, int screenH, int invX, int invY, int invWidth, int invHeight,
    InventorySlot* inventory, int* selectedHandSlot, int* selectedInvSlot, bool useController);
int FindEmptySlot(InventorySlot* inventory);
bool AddItemToInventory(InventorySlot* inventory, int itemId, int quantity, int ammo);

// Use the equipped item
void UseEquippedItem(InventorySlot* inventory, float* health, float* stamina, float* hunger, float* thirst);

// Reload weapon from inventory magazines
bool ReloadWeapon(InventorySlot* inventory);