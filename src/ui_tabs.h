#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

// Forward declarations to avoid circular dependency
struct InventorySlot;
enum class GameState;

// Tab types
enum UITab {
    TAB_INVENTORY,
    TAB_CRAFTING,
    TAB_MAP,
    TAB_SKILLS,
    TAB_QUESTS,
    TAB_COUNT
};

// Tab system manager
class TabManager {
public:
    TabManager() : currentTab(TAB_INVENTORY) {}
    
    UITab GetCurrentTab() const { return currentTab; }
    void SetCurrentTab(UITab tab) { currentTab = tab; }
    
    // Draw tab bar at top of screen
    void DrawTabBar(int screenW, int screenH, int menuX, int menuY, int menuW);
    
    // Handle keyboard tab switching
    void HandleTabInput(bool useController);
    
private:
    UITab currentTab;
};

// Draw skills screen
void DrawSkillsScreen(int screenW, int screenH, int contentX, int contentY, int contentW, int contentH, bool useController);

// Draw quests screen
void DrawQuestsScreen(int screenW, int screenH, int contentX, int contentY, int contentW, int contentH, bool useController);

// Global tab manager
extern TabManager g_TabManager;