#include "crafting.h"
#include "recipes.h" // Needed for the recipes global
#include "items.h"
#include "inventory.h" // For AddItemToInventory

bool HasIngredients(const CraftingRecipe& recipe, InventorySlot* inventory) { 
    // [Implementation of HasIngredients]
    for (const auto& ing : recipe.ingredients) {
        bool found = false;
        int needed = ing.quantity;
        for(int i=0; i<BACKPACK_SLOTS; i++) { 
            if (inventory[i].itemId == ing.itemId && inventory[i].quantity >= needed) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true; 
}

void ConsumeIngredients(const CraftingRecipe& recipe, InventorySlot* inventory) {
    // [Implementation of ConsumeIngredients]
    for (const auto& ing : recipe.ingredients) {
        int consumed = 0;
        for(int i=0; i<BACKPACK_SLOTS; i++) {
            if (consumed >= ing.quantity) break;
            if (inventory[i].itemId == ing.itemId) {
                int canTake = (int)fmin(ing.quantity - consumed, inventory[i].quantity);
                inventory[i].quantity -= canTake;
                consumed += canTake;
                if (inventory[i].quantity == 0) inventory[i] = {ITEM_NONE, 0, 0};
            }
        }
    }
}

void DrawCraftingMenu(int screenW, int screenH, InventorySlot* inventory, int* selectedRecipeIndex, bool useController) {
    // [Implementation of DrawCraftingMenu]
    int menuWidth = 800;
    int menuHeight = 500;
    int menuX = (screenW - menuWidth) / 2;
    int menuY = (screenH - menuHeight) / 2;
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, PIPBOY_GREEN);
    DrawText("CRAFTING (C/X)", menuX + 20, menuY + 10, 20, PIPBOY_GREEN);

    // ... (rest of implementation for navigation and drawing)
    int listW = 350;
    int listX = menuX + 20;
    int listY = menuY + 50;
    int recipeHeight = 40;
    
    // Input handling for menu navigation
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        *selectedRecipeIndex = (*selectedRecipeIndex - 1 + (int)recipes.size()) % (int)recipes.size();
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        *selectedRecipeIndex = (*selectedRecipeIndex + 1) % (int)recipes.size();
    }

    // Draw recipe list
    for (size_t i = 0; i < recipes.size(); ++i) {
        const CraftingRecipe& recipe = recipes[i];
        bool canCraft = HasIngredients(recipe, inventory);
        Color bgColor = (*selectedRecipeIndex == (int)i) ? PIPBOY_SELECTED : (canCraft ? PIPBOY_DARK : Color{10, 10, 10, 240});
        Color fgColor = canCraft ? PIPBOY_GREEN : PIPBOY_DIM;
        
        int recipeY = listY + (int)i * (recipeHeight + 5);
        DrawRectangle(listX, recipeY, listW, recipeHeight, bgColor);
        DrawRectangleLines(listX, recipeY, listW, recipeHeight, (*selectedRecipeIndex == (int)i) ? PIPBOY_GREEN : PIPBOY_DIM);
        
        DrawText(recipe.recipeName, listX + 10, recipeY + 10, 20, fgColor);
        DrawText(TextFormat("Result: %s x%d", GetItemName(recipe.resultId), recipe.resultQuantity), listX + 200, recipeY + 15, 12, PIPBOY_DIM);
        
        // Mouse selection check
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && GetMousePosition().x >= listX && GetMousePosition().x <= listX + listW && GetMousePosition().y >= recipeY && GetMousePosition().y <= recipeY + recipeHeight) {
             *selectedRecipeIndex = (int)i;
        }
    }
    
    // Draw ingredient details
    int detailX = menuX + 400;
    DrawText("Required Ingredients:", detailX, listY - 30, 18, PIPBOY_GREEN);
    if (*selectedRecipeIndex >= 0 && (size_t)*selectedRecipeIndex < recipes.size()) {
        const CraftingRecipe& recipe = recipes[*selectedRecipeIndex];
        int ingredientY = listY;
        for (const auto& ing : recipe.ingredients) {
            bool hasIng = false;
            int totalQty = 0;
            for(int i=0; i<BACKPACK_SLOTS; i++) { if(inventory[i].itemId == ing.itemId) { totalQty += inventory[i].quantity; } }
            hasIng = totalQty >= ing.quantity;
            Color ingColor = hasIng ? PIPBOY_GREEN : PIPBOY_DIM;
            DrawText(TextFormat("- %s x%d", GetItemName(ing.itemId), ing.quantity), detailX + 10, ingredientY, 16, ingColor);
            ingredientY += 20;
        }

        // Craft button
        int btnY = menuY + menuHeight - 50;
        bool canCraft = HasIngredients(recipe, inventory);
        Color btnColor = canCraft ? PIPBOY_GREEN : PIPBOY_DIM;
        DrawRectangle(detailX, btnY, 150, 30, btnColor);
        DrawText("CRAFT (E)", detailX + 10, btnY + 8, 18, canCraft ? BLACK : PIPBOY_DARK);
        
        bool craftAction = IsKeyPressed(KEY_E) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
        if (canCraft && craftAction) {
             ConsumeIngredients(recipe, inventory); 
             AddItemToInventory(inventory, recipe.resultId, recipe.resultQuantity, 0); 
             TraceLog(LOG_INFO, TextFormat("Crafted %s", recipe.recipeName));
        }
    }
}