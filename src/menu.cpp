#include "globals.h"
#include <vector>
#include <string>

// Draw main menu with a list of items and a title.
// options: list of menu entry strings
// selectedIndex: pointer to currently selected index (may be nullptr)
// useController: if true, controller input is considered, but keyboard and mouse still work
// title: optional title string (may be nullptr)
void DrawMenu(int screenW, int screenH, const std::vector<std::string> &options, int *selectedIndex, bool useController, const char *title)
{
    const int margin = 40;
    const int menuW = screenW - margin * 2;
    const int menuH = screenH - margin * 2;
    const int menuX = margin;
    const int menuY = margin;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);

    if (title && title[0] != '\0')
    {
        DrawText(title, menuX + 20, menuY + 12, 28, PIPBOY_GREEN);
    }

    const int lineHeight = 28;
    int startY = menuY + 60;
    int index = 0;

    // Ensure selected index is valid and available to be updated
    int localSel = 0;
    if (selectedIndex) localSel = *selectedIndex;
    if (localSel < 0) localSel = 0;
    if ((int)options.size() == 0) return;
    if (localSel >= (int)options.size()) localSel = (int)options.size() - 1;

    // Input: keyboard, mouse, controller
    Vector2 mouse = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // Allow keyboard navigation even if useController==true
    bool upPressed = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_KP_8);
    bool downPressed = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_KP_2);
    bool enterPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);

    // Controller navigation (dpad)
    bool gpDown = false, gpUp = false, gpConfirm = false;
    if (useController && IsGamepadAvailable(0))
    {
        gpDown    = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        gpUp      = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP)   || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
        gpConfirm = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
    }

    // Apply navigation (keyboard and controller both allowed)
    if (downPressed || gpDown)
    {
        localSel = (localSel + 1) % (int)options.size();
    }
    else if (upPressed || gpUp)
    {
        localSel = (localSel - 1 + (int)options.size()) % (int)options.size();
    }

    // Render items and handle mouse hover/click and activation
    for (const auto &it : options)
    {
        Color txtCol = PIPBOY_DIM;
        int itemX = menuX + 16;
        int itemY = startY - 6;
        int itemW = menuW - 32;
        int itemH = lineHeight + 8;

        bool hovered = false;
        if (mouse.x >= (float)itemX && mouse.x <= (float)(itemX + itemW) &&
            mouse.y >= (float)itemY && mouse.y <= (float)(itemY + itemH))
        {
            hovered = true;
            // moving mouse over an item updates selection
            localSel = index;
        }

        if (localSel == index)
        {
            DrawRectangle(itemX, itemY, itemW, itemH, Color{40, 80, 40, 200});
            txtCol = PIPBOY_GREEN;
        }
        else if (hovered)
        {
            DrawRectangle(itemX, itemY, itemW, itemH, Color{30, 60, 30, 150});
            txtCol = PIPBOY_GREEN;
        }

        DrawText(it.c_str(), menuX + 28, startY, 20, txtCol);

        // Activation via keyboard/controller
        if ((enterPressed || gpConfirm) && localSel == index)
        {
            if (gameState == GameState::MainMenu)
            {
                if (localSel == 0)
                {
                    InitNewGame(&camera, &playerPosition, &playerVelocity, &health, &stamina, &hunger, &thirst, &yaw, &pitch, &onGround, inventory, &flashlightBattery, &isFlashlightOn, map, &fov);
                    gameState = GameState::Gameplay;
                }
                else if (localSel == 1)
                {
                    stateBeforeSettings = GameState::MainMenu;
                    saveSlotSelection = 0;
                    gameState = GameState::LoadMenu;
                }
                else if (localSel == 2)
                {
                    stateBeforeSettings = GameState::MainMenu;
                    settingsSelection = 0;
                    gameState = GameState::Settings;
                }
                else if (localSel == 3)
                {
                    CloseWindow();
                    exit(0);
                }
            }
            else if (gameState == GameState::Paused)
            {
                if (localSel == 0) gameState = GameState::Gameplay;
                else if (localSel == 1) { stateBeforeSettings = GameState::Paused; saveSlotSelection = 0; gameState = GameState::LoadMenu; }
                else if (localSel == 2) { stateBeforeSettings = GameState::Paused; settingsSelection = 0; gameState = GameState::Settings; }
                else if (localSel == 3) { gameState = GameState::MainMenu; }
            }
        }

        // Activation via mouse click
        if (mouseClicked && hovered)
        {
            if (gameState == GameState::MainMenu)
            {
                if (index == 0)
                {
                    InitNewGame(&camera, &playerPosition, &playerVelocity, &health, &stamina, &hunger, &thirst, &yaw, &pitch, &onGround, inventory, &flashlightBattery, &isFlashlightOn, map, &fov);
                    gameState = GameState::Gameplay;
                }
                else if (index == 1)
                {
                    stateBeforeSettings = GameState::MainMenu;
                    saveSlotSelection = 0;
                    gameState = GameState::LoadMenu;
                }
                else if (index == 2)
                {
                    stateBeforeSettings = GameState::MainMenu;
                    settingsSelection = 0;
                    gameState = GameState::Settings;
                }
                else if (index == 3)
                {
                    CloseWindow();
                    exit(0);
                }
            }
            else if (gameState == GameState::Paused)
            {
                if (index == 0) gameState = GameState::Gameplay;
                else if (index == 1) { stateBeforeSettings = GameState::Paused; saveSlotSelection = 0; gameState = GameState::LoadMenu; }
                else if (index == 2) { stateBeforeSettings = GameState::Paused; settingsSelection = 0; gameState = GameState::Settings; }
                else if (index == 3) { gameState = GameState::MainMenu; }
            }
        }

        startY += lineHeight + 6;
        ++index;
    }

    // Persist selection back to caller
    if (selectedIndex) *selectedIndex = localSel;

    if (!useController)
    {
        const char *tip = "Click options or use arrows + Enter to select.";
        DrawText(tip, menuX + 20, menuY + menuH - 34, 16, PIPBOY_DIM);
    }
}