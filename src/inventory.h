#pragma once
#include "globals.h"

void DrawInventory(int screenW, int screenH, InventorySlot* inventory, int* selectedHandSlot, int* selectedInvSlot, bool useController);
int FindEmptySlot(InventorySlot* inventory);
bool AddItemToInventory(InventorySlot* inventory, int itemId, int quantity, int ammo);