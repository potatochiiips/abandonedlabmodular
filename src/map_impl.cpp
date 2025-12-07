#include "globals.h"

// Prototypes for all menu drawing functions
void DrawMenu(int screenW, int screenH, const std::vector<std::string>& options, int* selectedIndex, bool useController, const char* title);
void DrawLoadMenu(int screenW, int screenH, int* selectedSlot, GameState currentState);
// Signature must match src/menus.cpp exactly (no showHUD parameter)
void DrawSettingsMenu(int screenW, int screenH, bool* showMinimap, bool* isControllerEnabled, int* settingsSelection, GameState* nextState);
void DrawControllerBindings(int screenW, int screenH, int* activeBindingIndex, bool* isBindingMode, int* controllerSettingsSelection, ControllerBinding* currentBindings);