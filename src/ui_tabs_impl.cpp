#include "ui_tabs.h"
#include "globals.h"


// Global TabManager instance definition
TabManager g_TabManager;

// TabManager::DrawTabBar implementation
void TabManager::DrawTabBar(int screenW, int screenH, int menuX, int menuY, int menuW) {
    const int tabHeight = 45;
    const int tabCount = 5;
    const int tabWidth = menuW / tabCount;
    
    const char* tabNames[] = {"INVENTORY", "CRAFTING", "MAP", "SKILLS", "QUESTS"};
    const char* tabKeys[] = {"(I)", "(C)", "(M)", "(L)", "(Q)"};
    
    for (int i = 0; i < tabCount; i++) {
        int tabX = menuX + i * tabWidth;
        
        bool isSelected = (currentTab == (UITab)i);
        Color bgColor = isSelected ? PIPBOY_SELECTED : PIPBOY_DARK;
        Color borderColor = isSelected ? PIPBOY_GREEN : PIPBOY_DIM;
        Color textColor = isSelected ? PIPBOY_GREEN : PIPBOY_DIM;
        
        DrawRectangle(tabX, menuY, tabWidth - 2, tabHeight, bgColor);
        DrawRectangleLines(tabX, menuY, tabWidth - 2, tabHeight, borderColor);
        
        int textW = MeasureText(tabNames[i], 18);
        int keyW = MeasureText(tabKeys[i], 12);
        
        DrawText(tabNames[i], tabX + (tabWidth - textW) / 2, menuY + 8, 18, textColor);
        DrawText(tabKeys[i], tabX + (tabWidth - keyW) / 2, menuY + 28, 12, 
                 isSelected ? PIPBOY_GREEN : Color{100, 150, 100, 255});
        
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= tabX && mousePos.x <= tabX + tabWidth - 2 &&
            mousePos.y >= menuY && mousePos.y <= menuY + tabHeight) {
            currentTab = (UITab)i;
        }
        
        if (mousePos.x >= tabX && mousePos.x <= tabX + tabWidth - 2 &&
            mousePos.y >= menuY && mousePos.y <= menuY + tabHeight && !isSelected) {
            DrawRectangle(tabX, menuY, tabWidth - 2, tabHeight, Color{40, 70, 40, 100});
        }
    }
}

// TabManager::HandleTabInput implementation
void TabManager::HandleTabInput(bool useController) {
    if (IsKeyPressed(KEY_I)) currentTab = TAB_INVENTORY;
    if (IsKeyPressed(KEY_C)) currentTab = TAB_CRAFTING;
    if (IsKeyPressed(KEY_M)) currentTab = TAB_MAP;
    if (IsKeyPressed(KEY_L)) currentTab = TAB_SKILLS;
    if (IsKeyPressed(KEY_Q)) currentTab = TAB_QUESTS;
    
    if (useController && IsGamepadAvailable(0)) {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1)) {
            currentTab = (UITab)(((int)currentTab - 1 + TAB_COUNT) % TAB_COUNT);
        }
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) {
            currentTab = (UITab)(((int)currentTab + 1) % TAB_COUNT);
        }
    }
}