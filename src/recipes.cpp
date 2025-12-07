#include "recipes.h"

// Global definition and initialization of recipes
std::vector<CraftingRecipe> recipes = {
    { { {ITEM_WATER_BOTTLE, 1}, {ITEM_FLASHLIGHT, 1} }, ITEM_LAB_KEY, 1, "Lab Key" },
    { { {ITEM_WOOD, 1}, {ITEM_STONE, 1} }, ITEM_FLASHLIGHT, 1, "Flashlight" },
    { { {ITEM_STONE, 2} }, ITEM_WOOD, 1, "Fake Wood (x2 Stone)" }
};