#pragma once
#include "globals.h"

void DrawCraftingMenu(int screenW, int screenH, InventorySlot* inventory, int* selectedRecipeIndex, bool useController);
bool HasIngredients(const CraftingRecipe& recipe, InventorySlot* inventory);
void ConsumeIngredients(const CraftingRecipe& recipe, InventorySlot* inventory);