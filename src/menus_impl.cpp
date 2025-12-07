#include "menus.h"
#include "fileio.h"

// Draw the load/save menu
void DrawLoadMenu(int screenW, int screenH, int* selectedSlot, GameState currentState) {
    int menuW = 600;
    int menuH = 400;
    int menuX = (screenW - menuW) / 2;
    int menuY = (screenH - menuH) / 2;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);

    const char* title = (currentState == GameState::Paused) ? "SAVE GAME" : "LOAD GAME";
    DrawText(title, menuX + 20, menuY + 10, 28, PIPBOY_GREEN);

    // Navigation
    bool useController = isControllerEnabled && IsGamepadAvailable(0);
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        *selectedSlot = (*selectedSlot - 1 + MAX_SAVE_SLOTS) % MAX_SAVE_SLOTS;
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        *selectedSlot = (*selectedSlot + 1) % MAX_SAVE_SLOTS;
    }

    // Draw slots
    int slotY = menuY + 60;
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        bool fileExists = SaveFileExists(i + 1);
        Color bgColor = (*selectedSlot == i) ? PIPBOY_SELECTED : PIPBOY_DARK;
        Color fgColor = fileExists ? PIPBOY_GREEN : PIPBOY_DIM;

        DrawRectangle(menuX + 20, slotY, menuW - 40, 60, bgColor);
        DrawRectangleLines(menuX + 20, slotY, menuW - 40, 60, (*selectedSlot == i) ? PIPBOY_GREEN : PIPBOY_DIM);

        DrawText(TextFormat("Slot %d", i + 1), menuX + 30, slotY + 10, 20, fgColor);
        if (fileExists) {
            DrawText("Save file exists", menuX + 30, slotY + 35, 16, PIPBOY_DIM);
        }
        else {
            DrawText("Empty", menuX + 30, slotY + 35, 16, PIPBOY_DIM);
        }

        // Mouse selection
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuW - 20 &&
            mousePos.y >= slotY && mousePos.y <= slotY + 60) {
            *selectedSlot = i;
        }

        slotY += 70;
    }

    // Instructions
    if (currentState == GameState::Paused) {
        DrawText("Press ENTER to save", menuX + 20, menuY + menuH - 60, 18, PIPBOY_GREEN);
    }
    else {
        DrawText("Press ENTER to load", menuX + 20, menuY + menuH - 60, 18, PIPBOY_GREEN);
    }
    DrawText("Press ESC to cancel", menuX + 20, menuY + menuH - 35, 18, PIPBOY_DIM);
}

// Draw the settings menu with fullscreen toggle
void DrawSettingsMenu(int screenW, int screenH, bool* showMinimap, bool* isControllerEnabled, bool* isFullscreen, int* settingsSelection, GameState* nextState) {
    int menuW = 600;
    int menuH = 450;
    int menuX = (screenW - menuW) / 2;
    int menuY = (screenH - menuH) / 2;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);
    DrawText("SETTINGS", menuX + 20, menuY + 10, 28, PIPBOY_GREEN);

    bool useController = *isControllerEnabled && IsGamepadAvailable(0);

    // Navigation (now 5 options instead of 4)
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        *settingsSelection = (*settingsSelection - 1 + 5) % 5;
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        *settingsSelection = (*settingsSelection + 1) % 5;
    }

    // Draw options
    int optY = menuY + 60;
    const char* options[] = {
        TextFormat("Show Minimap: %s", *showMinimap ? "ON" : "OFF"),
        TextFormat("Controller Enabled: %s", *isControllerEnabled ? "ON" : "OFF"),
        TextFormat("Fullscreen: %s", *isFullscreen ? "ON" : "OFF"),
        "Controller Bindings",
        "Back"
    };

    for (int i = 0; i < 5; i++) {
        Color bgColor = (*settingsSelection == i) ? PIPBOY_SELECTED : PIPBOY_DARK;
        Color fgColor = (*settingsSelection == i) ? PIPBOY_GREEN : PIPBOY_DIM;

        DrawRectangle(menuX + 20, optY, menuW - 40, 50, bgColor);
        DrawRectangleLines(menuX + 20, optY, menuW - 40, 50, (*settingsSelection == i) ? PIPBOY_GREEN : PIPBOY_DIM);
        DrawText(options[i], menuX + 30, optY + 15, 20, fgColor);

        // Mouse selection
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuW - 20 &&
            mousePos.y >= optY && mousePos.y <= optY + 50) {
            *settingsSelection = i;
        }

        optY += 60;
    }

    // Handle selection
    if (IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) {
        if (*settingsSelection == 0) {
            *showMinimap = !(*showMinimap);
        }
        else if (*settingsSelection == 1) {
            *isControllerEnabled = !(*isControllerEnabled);
        }
        else if (*settingsSelection == 2) {
            *isFullscreen = !(*isFullscreen);
            ToggleFullscreen();
        }
        else if (*settingsSelection == 3) {
            gameState = GameState::ControllerBindings;
            controllerSettingsSelection = 0;
        }
        else if (*settingsSelection == 4) {
            *nextState = stateBeforeSettings;
            gameState = stateBeforeSettings;
        }
    }

    DrawText("Press ENTER to toggle/select", menuX + 20, menuY + menuH - 60, 16, PIPBOY_DIM);
    DrawText("Press ESC to go back", menuX + 20, menuY + menuH - 35, 16, PIPBOY_DIM);
}

// Draw the controller bindings menu
void DrawControllerBindings(int screenW, int screenH, int* activeBindingIndex, bool* isBindingMode, int* controllerSettingsSelection, ControllerBinding* currentBindings) {
    int menuW = 700;
    int menuH = 500;
    int menuX = (screenW - menuW) / 2;
    int menuY = (screenH - menuH) / 2;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);
    DrawText("CONTROLLER BINDINGS", menuX + 20, menuY + 10, 26, PIPBOY_GREEN);

    if (*isBindingMode) {
        DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 180 });
        DrawRectangle(screenW / 2 - 200, screenH / 2 - 50, 400, 100, PIPBOY_DARK);
        DrawRectangleLines(screenW / 2 - 200, screenH / 2 - 50, 400, 100, PIPBOY_GREEN);
        DrawText("Press any button to bind...", screenW / 2 - 140, screenH / 2 - 20, 20, PIPBOY_GREEN);
        DrawText("Press ESC to cancel", screenW / 2 - 100, screenH / 2 + 10, 16, PIPBOY_DIM);

        // Check for button press
        if (IsGamepadAvailable(0)) {
            for (int btn = 0; btn < GAMEPAD_BUTTON_COUNT; btn++) {
                if (IsGamepadButtonPressed(0, btn)) {
                    currentBindings[*activeBindingIndex].isAxis = false;
                    currentBindings[*activeBindingIndex].inputId = btn;
                    currentBindings[*activeBindingIndex].threshold = 0.0f;
                    *isBindingMode = false;
                    *activeBindingIndex = -1;
                    break;
                }
            }
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            *isBindingMode = false;
            *activeBindingIndex = -1;
        }

        return;
    }

    bool useController = isControllerEnabled && IsGamepadAvailable(0);

    // Navigation
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        *controllerSettingsSelection = (*controllerSettingsSelection - 1 + ACTION_COUNT) % ACTION_COUNT;
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        *controllerSettingsSelection = (*controllerSettingsSelection + 1) % ACTION_COUNT;
    }

    // Draw bindings
    int bindY = menuY + 50;
    const char* actionNames[] = { "Jump", "Sprint", "Inventory", "Crafting", "Map", "Flashlight", "Use Item", "Shoot" };

    for (int i = 0; i < ACTION_COUNT; i++) {
        Color bgColor = (*controllerSettingsSelection == i) ? PIPBOY_SELECTED : PIPBOY_DARK;
        Color fgColor = (*controllerSettingsSelection == i) ? PIPBOY_GREEN : PIPBOY_DIM;

        DrawRectangle(menuX + 20, bindY, menuW - 40, 45, bgColor);
        DrawRectangleLines(menuX + 20, bindY, menuW - 40, 45, (*controllerSettingsSelection == i) ? PIPBOY_GREEN : PIPBOY_DIM);

        DrawText(actionNames[i], menuX + 30, bindY + 13, 18, fgColor);

        const char* bindingText;
        if (currentBindings[i].isAxis) {
            bindingText = TextFormat("Axis %d (%.1f)", currentBindings[i].inputId, currentBindings[i].threshold);
        }
        else {
            bindingText = currentBindings[i].actionName;
        }
        DrawText(bindingText, menuX + 300, bindY + 13, 18, fgColor);

        // Mouse selection
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuW - 20 &&
            mousePos.y >= bindY && mousePos.y <= bindY + 45) {
            *controllerSettingsSelection = i;
        }

        bindY += 50;
    }

    // Handle rebinding
    if (IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) {
        *isBindingMode = true;
        *activeBindingIndex = *controllerSettingsSelection;
    }

    DrawText("Press ENTER to rebind", menuX + 20, menuY + menuH - 60, 16, PIPBOY_DIM);
    DrawText("Press ESC to go back", menuX + 20, menuY + menuH - 35, 16, PIPBOY_DIM);
}