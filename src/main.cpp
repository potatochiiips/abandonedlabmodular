#include "globals.h"
#include "hud.h"
#include "menus.h"
#include "crafting.h"
#include "inventory.h"
#include "items.h"
#include "map.h"
#include "player.h"
#include "console.h"
#include "fileio.h" // Includes save/load logic

// --- GLOBAL VARIABLE DEFINITIONS ---
Camera3D camera = { 0 };
Vector3 playerPosition = { 0 };
Vector3 playerVelocity = { 0 };
float playerSpeed = 0.1f;
float jumpForce = 0.3f;
bool onGround = true;
float gravity = 0.015f;
float playerHeight = 1.8f;
float health = 100;
float stamina = 100;
float hunger = 100;
float thirst = 100;
float fov = 75.0f;
InventorySlot inventory[TOTAL_INVENTORY_SLOTS] = {};
bool inventoryOpen = false;
bool isCraftingOpen = false;
bool isMapOpen = false;
bool isNoclip = false;
float flashlightBattery = 100.0f;
bool isFlashlightOn = false;
char map[MAP_SIZE][MAP_SIZE];
int selectedHandSlot = 0;
int selectedInvSlot = 0;
int selectedRecipeIndex = 0;
float pistolRecoilPitch = 0.0f;
float pistolRecoilYaw = 0.0f;
const float RECOIL_DECAY_RATE = 1.0f;
float shotTimer = 0.0f;
const float SHOT_COOLDOWN = 0.5f;
bool showMinimap = true;
bool isControllerEnabled = true;
int settingsSelection = 0;
int controllerSettingsSelection = 0;
bool isBindingMode = false;
int activeBindingIndex = -1;
int saveSlotSelection = 0;
int mainMenuSelection = 0;
int pauseMenuSelection = 0;
float yaw = -90.0f;
float pitch = 0;
bool cursorHidden = true;
GameState gameState = GameState::MainMenu;
GameState stateBeforeSettings = GameState::MainMenu;

// ControllerBinding bindings[ACTION_COUNT]; // Defined here, declared extern in globals.h

// Helper function (internal to main)
void CloseInGameMenus() {
    inventoryOpen = false;
    isCraftingOpen = false;
    isMapOpen = false;
}

// Implementation of InitNewGame (declared in player.h)
void InitNewGame(Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* health, float* stamina, float* hunger, float* thirst, float* yaw, float* pitch, bool* onGround, InventorySlot* inventory, float* flashlightBattery, bool* isFlashlightOn, char map[MAP_SIZE][MAP_SIZE], float* fov) {
    *playerPosition = Vector3{ MAP_SIZE/2.0f, playerHeight, MAP_SIZE/2.0f }; 
    *playerVelocity = Vector3{ 0.0f, 0.0f, 0.0f };
    camera->position = *playerPosition;

    *yaw = -90.0f;
    *pitch = 0.0f;
    // Compute forward target relative to position using yaw/pitch so camera faces correctly
    Vector3 forward;
    forward.x = cosf(DEG2RAD * (*yaw)) * cosf(DEG2RAD * (*pitch));
    forward.y = sinf(DEG2RAD * (*pitch));
    forward.z = sinf(DEG2RAD * (*yaw)) * cosf(DEG2RAD * (*pitch));
    camera->target = Vector3Add(camera->position, forward);

    camera->up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera->fovy = *fov;

    *onGround = true;
    *health = 100.0f;
    *stamina = 100.0f;
    *hunger = 100.0f;
    *thirst = 100.0f;
    *flashlightBattery = 100.0f;
    *isFlashlightOn = false;
    *fov = 75.0f; 

    for (int i = 0; i < TOTAL_INVENTORY_SLOTS; i++) inventory[i] = { ITEM_NONE, 0, 0 };
    // Initial Items (Hand slot 0)
    inventory[BACKPACK_SLOTS] = { ITEM_PISTOL, 1, 7 }; // Pistol in hand slot 0
    inventory[BACKPACK_SLOTS + 1] = { ITEM_FLASHLIGHT, 1, 0 }; // Flashlight in hand slot 1
    
    // Backpack
    inventory[0] = { ITEM_WATER_BOTTLE, 2, 0 };
    inventory[1] = { ITEM_WOOD, 1, 0 };
    inventory[2] = { ITEM_STONE, 2, 0 };
    inventory[3] = { ITEM_MAG, 2, 0 };

    GenerateMap(map);
    
    // Initialize controller bindings
    ControllerBinding defaultBindings[ACTION_COUNT] = {
        { false, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0.0f, "A" },
        { false, GAMEPAD_BUTTON_LEFT_THUMB, 0.0f, "L3" },
        { false, GAMEPAD_BUTTON_RIGHT_FACE_UP, 0.0f, "Y" },
        { false, GAMEPAD_BUTTON_RIGHT_FACE_LEFT, 0.0f, "X" },
        { false, GAMEPAD_BUTTON_DPAD_RIGHT, 0.0f, "D-Right" },
        { false, GAMEPAD_BUTTON_LEFT_TRIGGER_1, 0.0f, "LB" },
        { false, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, 0.0f, "B" },
        { false, GAMEPAD_BUTTON_RIGHT_TRIGGER_1, 0.0f, "RB" }
    };
    for(int i=0; i<ACTION_COUNT; i++) bindings[i] = defaultBindings[i];
}


int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Echoes of Time");
    // Disable raylib's default behavior of closing the window when ESC is pressed
    SetExitKey(KEY_NULL);

    // Ensure cursor starts enabled in menus (avoid initial jump)
    EnableCursor();
    cursorHidden = false;

    InitNewGame(&camera, &playerPosition, &playerVelocity, &health, &stamina, &hunger, &thirst, &yaw, &pitch, &onGround, inventory, &flashlightBattery, &isFlashlightOn, map, &fov);

    SetTargetFPS(60);

    int screenW = screenWidth, screenH = screenHeight; // Track current screen size
    // Track previous binding/cursor state so we only call Enable/Disable once on transitions.
    bool prevBindingMode = isBindingMode;
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Decide whether the cursor should be captured for FPS look or free for UI
        bool isAnyMenuOpen = (inventoryOpen || isCraftingOpen || isMapOpen);
        bool useController = isControllerEnabled && IsGamepadAvailable(0);

        // Cursor capture policy:
        // - In gameplay (no menus open) we capture/hide cursor so mouse look works.
        // - In menus or console we show/release cursor so the user can click UI.
        // - In binding mode we capture/hide cursor (we want raw input).
        bool shouldCaptureCursor = (gameState == GameState::Gameplay && !isAnyMenuOpen && !isBindingMode) || isBindingMode;

        static bool prevCursorCaptured = false; // preserved across frames
        if (shouldCaptureCursor != prevCursorCaptured)
        {
            if (shouldCaptureCursor)
            {
                // Enter capture: hide/capture cursor for FPS-style look
                // Center mouse once so the first delta is stable (optional)
                Vector2 center = { (float)(screenW / 2), (float)(screenH / 2) };
                SetMousePosition((int)center.x, (int)center.y);
                DisableCursor();
            }
            else
            {
                // Release cursor for UI interaction
                EnableCursor();
            }
            prevCursorCaptured = shouldCaptureCursor;
        }

        // Global ESC handling
        if (IsKeyPressed(KEY_ESCAPE) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_START))) {
            if (gameState == GameState::Gameplay && !isAnyMenuOpen) {
                gameState = GameState::Paused;
                pauseMenuSelection = 0;
                stateBeforeSettings = GameState::Paused;
            } else if (gameState == GameState::Paused) {
                gameState = GameState::Gameplay;
            } else if (gameState == GameState::LoadMenu) {
                gameState = stateBeforeSettings;
            } else if (gameState == GameState::Settings) {
                gameState = GameState::Paused; // Only accessible from Paused/MainMenu
            } else if (gameState == GameState::ControllerBindings) {
                gameState = GameState::Settings; 
            } else if (gameState == GameState::Console) {
                gameState = GameState::Gameplay;
            } else if (gameState == GameState::MainMenu) {
                // Intentionally do NOT call `break` here to avoid immediately quitting the game
                // when the user presses ESC in the main menu. Use the "Exit" menu option to quit.
            } else if (isAnyMenuOpen) {
                CloseInGameMenus();
            }
        }
        
        // Console toggle
        if (IsKeyPressed(KEY_GRAVE)) {
            if (gameState == GameState::Gameplay) gameState = GameState::Console;
            else if (gameState == GameState::Console) gameState = GameState::Gameplay;
        }

        // --- Gameplay Input & Logic ---
        if (gameState == GameState::Gameplay) {
            
            // Toggle menus
            bool inventoryTogglePressed = IsKeyPressed(KEY_I) || (useController && IsActionPressed(ACTION_INVENTORY, bindings));
            if (inventoryTogglePressed) { CloseInGameMenus(); inventoryOpen = !inventoryOpen; }

            bool craftingTogglePressed = IsKeyPressed(KEY_C) || (useController && IsActionPressed(ACTION_CRAFTING, bindings));
            if (craftingTogglePressed) { CloseInGameMenus(); isCraftingOpen = !isCraftingOpen; if (isCraftingOpen) selectedRecipeIndex = 0; }
            
            bool mapTogglePressed = IsKeyPressed(KEY_M) || (useController && IsActionPressed(ACTION_MAP, bindings));
            if (mapTogglePressed) { CloseInGameMenus(); isMapOpen = !isMapOpen; }

            if (!isAnyMenuOpen) {
                // Player movement and camera logic (from original single file)
                UpdatePlayer(deltaTime, &camera, &playerPosition, &playerVelocity, &yaw, &pitch, &onGround, playerSpeed, playerHeight, gravity, jumpForce, &stamina, isNoclip, useController);
                
                // Flashlight logic
                bool flashlightPressed = useController ? IsActionPressed(ACTION_FLASHLIGHT, bindings) : IsKeyPressed(KEY_F);
                if (flashlightPressed) isFlashlightOn = !isFlashlightOn;

                if (isFlashlightOn && flashlightBattery > 0.0f) {
                    flashlightBattery -= 5.0f * deltaTime; // FLASHLIGHT_DRAIN_RATE
                } else if (flashlightBattery <= 0.0f) {
                    isFlashlightOn = false;
                    flashlightBattery = 0.0f;
                }
                bool useItemPressed = useController ? IsActionPressed(ACTION_USE_ITEM, bindings) : IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);
                if (useItemPressed) {
                    UseEquippedItem(inventory, &health, &stamina, &hunger, &thirst);
                }
                // Reload weapon with R key or controller X button
                bool reloadPressed = IsKeyPressed(KEY_R) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT));
                if (reloadPressed) {
                    ReloadWeapon(inventory);
                }
                // Survival stat drain (simplified)
                float drainRate = 1.0f * deltaTime;
                hunger = fmaxf(0.0f, hunger - drainRate);
                thirst = fmaxf(0.0f, thirst - drainRate * 1.5f);
                stamina = fminf(100.0f, stamina + drainRate * 2.0f); // Stamina regens faster

                if (health <= 0.0f) gameState = GameState::GameOver;

                // Shooting logic
                shotTimer = fmaxf(0.0f, shotTimer - deltaTime);
                bool shootPressed = useController ? IsActionPressed(ACTION_SHOOT, bindings) : IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                if (shootPressed && shotTimer <= 0.0f && inventory[BACKPACK_SLOTS].itemId == ITEM_PISTOL && inventory[BACKPACK_SLOTS].ammo > 0) {
                    shotTimer = SHOT_COOLDOWN;
                    inventory[BACKPACK_SLOTS].ammo--;
                    pistolRecoilPitch = 5.0f;
                    pistolRecoilYaw = 1.0f;
                    TraceLog(LOG_INFO, "Pistol fired!");
                }
            }
        }

        // --- Game State/Menu Logic ---
        if (gameState == GameState::MainMenu) {
            if (IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) {
                if (mainMenuSelection == 0) { InitNewGame(&camera, &playerPosition, &playerVelocity, &health, &stamina, &hunger, &thirst, &yaw, &pitch, &onGround, inventory, &flashlightBattery, &isFlashlightOn, map, &fov); gameState = GameState::Gameplay; }
                if (mainMenuSelection == 1) { stateBeforeSettings = GameState::MainMenu; saveSlotSelection = 0; gameState = GameState::LoadMenu; }
                if (mainMenuSelection == 2) { stateBeforeSettings = GameState::MainMenu; settingsSelection = 0; gameState = GameState::Settings; }
                if (mainMenuSelection == 3) break; // Exit
            }
        } else if (gameState == GameState::Paused) {
            if (IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) {
                if (pauseMenuSelection == 0) gameState = GameState::Gameplay;
                if (pauseMenuSelection == 1) { stateBeforeSettings = GameState::Paused; saveSlotSelection = 0; gameState = GameState::LoadMenu; }
                if (pauseMenuSelection == 2) { stateBeforeSettings = GameState::Paused; settingsSelection = 0; gameState = GameState::Settings; }
                if (pauseMenuSelection == 3) gameState = GameState::MainMenu; 
            }
        } else if (gameState == GameState::LoadMenu) {
            if ((IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)))) {
                bool fileExists = SaveFileExists(saveSlotSelection + 1);
                if (stateBeforeSettings == GameState::Paused) {
                    SaveGame(saveSlotSelection + 1, playerPosition, yaw, pitch, health, stamina, hunger, thirst, inventory, flashlightBattery, isFlashlightOn, map, fov);
                    gameState = GameState::Paused;
                } else if (stateBeforeSettings == GameState::MainMenu && fileExists) {
                    if (LoadGame(saveSlotSelection + 1, &playerPosition, &yaw, &pitch, &health, &stamina, &hunger, &thirst, inventory, &flashlightBattery, &isFlashlightOn, map, &fov)) {
                        camera.position = playerPosition;
                        Vector3 target = { cosf(DEG2RAD * yaw), sinf(DEG2RAD * pitch), sinf(DEG2RAD * yaw) * cosf(DEG2RAD * pitch) };
                        camera.target = Vector3Add(camera.position, target);
                        camera.fovy = fov;
                        gameState = GameState::Gameplay;
                    }
                }
            }
        } else if (gameState == GameState::Settings) {
             if (settingsSelection == 3 && (IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)))) {
                gameState = stateBeforeSettings;
            }
        } else if (gameState == GameState::ControllerBindings) {
            if (!isBindingMode && (IsKeyPressed(KEY_ESCAPE) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)))) {
                gameState = GameState::Settings;
            }
        }

        // After potential fullscreen change (run each frame or right after ToggleFullscreen)
        int newScreenW = GetScreenWidth();
        int newScreenH = GetScreenHeight();
        if (newScreenW != screenW || newScreenH != screenH) {
            screenW = newScreenW;
            screenH = newScreenH;
            // Recompute HUD/minimap positions or any cached UI scale values here if you cache them.
            // Using `screenW`/`screenH` everywhere below ensures UI follows window size automatically.
        }
        // --- Draw Frame ---
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw 3D world and hands during gameplay (world geometry must be rendered in gameplay too)
        BeginMode3D(camera);
        DrawGrid(MAP_SIZE, GRID_SIZE);
        DrawMapGeometry(map);
        // draw player hands (first-person objects) here
        DrawPlayerHands(camera, inventory, pistolRecoilPitch, pistolRecoilYaw);
        EndMode3D();

        // draw HUD/minimap after EndMode3D so they overlay the 3D scene
        if (showMinimap && gameState == GameState::Gameplay && !isMapOpen) {
    DrawMinimap(map, playerPosition, yaw, screenW - 160, 10, 150, 150, true, 0);
}
        // Draw HUD (health/stamina/etc.)
        if (gameState == GameState::Gameplay) DrawHUD(screenW, screenH, health, stamina, hunger, thirst, fov, flashlightBattery, isFlashlightOn, inventory);
        if (isMapOpen) DrawMapMenu(screenW, screenH, map, playerPosition, yaw);
        if (isCraftingOpen) DrawCraftingMenu(screenW, screenH, inventory, &selectedRecipeIndex, useController);
        if (inventoryOpen) DrawInventory(screenW, screenH, inventory, &selectedHandSlot, &selectedInvSlot, useController);
        
        if (gameState == GameState::MainMenu) {
            ClearBackground(PIPBOY_DARK);
            std::vector<std::string> options = {"New Game", "Load Game", "Settings", "Exit"};
            DrawMenu(screenW, screenH, options, &mainMenuSelection, useController, "ECHOES OF TIME");
        } else if (gameState == GameState::Paused) {
            ClearBackground(Color{10, 20, 10, 255}); 
            BeginMode3D(camera);
            DrawGrid(MAP_SIZE, GRID_SIZE);
            DrawMapGeometry(map);
            EndMode3D();
            DrawRectangle(0, 0, screenW, screenH, Color{0, 0, 0, 180}); 
            std::vector<std::string> options = {"Continue", "Save Game", "Settings", "Main Menu"};
            DrawMenu(screenW, screenH, options, &pauseMenuSelection, useController, "PAUSED");
        } else if (gameState == GameState::GameOver) {
             DrawRectangle(0, 0, screenW, screenH, Color{10, 10, 10, 200});
             DrawText("GAME OVER", screenW/2 - MeasureText("GAME OVER", 80)/2, screenH/2 - 40, 80, PIPBOY_GREEN);
             DrawText("You perished. Press ESC to return to main menu.", screenW/2 - MeasureText("You perished. Press ESC to return to main menu.", 20)/2, screenH/2 + 40, 20, PIPBOY_GREEN);
        } else if (gameState == GameState::LoadMenu) {
            DrawLoadMenu(screenW, screenH, &saveSlotSelection, stateBeforeSettings);
        } else if (gameState == GameState::Settings) {
            // Note: DrawSettingsMenu handles navigation/toggling, not the state change itself
            GameState tempState = stateBeforeSettings; // Pass temporary state for 'Back' logic
            DrawSettingsMenu(screenW, screenH, &showMinimap, &isControllerEnabled, &settingsSelection, &tempState);
        } else if (gameState == GameState::ControllerBindings) {
            DrawControllerBindings(screenW, screenH, &activeBindingIndex, &isBindingMode, &controllerSettingsSelection, bindings);
        } else if (gameState == GameState::Console) {
            DrawConsole(screenW, screenH, consoleHistory, consoleInput, consoleInputLength); // Assuming DrawConsole exists
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}