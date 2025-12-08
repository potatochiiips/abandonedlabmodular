// Complete updated main.cpp with texture and shader system integration

#include "globals.h"
#include "hud.h"
#include "menus.h"
#include "crafting.h"
#include "inventory.h"
#include "items.h"
#include "map.h"
#include "player.h"
#include "console.h"
#include "fileio.h"
#include "texture_manager.h"

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
bool isFullscreen = false;
int settingsSelection = 0;
int controllerSettingsSelection = 0;
int graphicsSettingsSelection = 0;
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

// Graphics settings
GraphicsSettings graphicsSettings = {
    2,      // resolutionIndex (1280x720)
    true,   // vsync
    false,  // msaa
    4,      // msaaSamples
    60,     // targetFPS
    1.0f,   // renderScale
    false   // showFPS
};

// Performance optimization: Frame time tracking
static float frameTimeAccumulator = 0.0f;
static int frameCount = 0;
static float avgFrameTime = 0.0f;

// Helper function to draw textured cubes
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color) {
    float x = position.x;
    float y = position.y;
    float z = position.z;

    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);

    rlSetTexture(texture.id);

    // Front Face
    rlNormal3f(0.0f, 0.0f, 1.0f);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width / 2, y - height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z + length / 2);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width / 2, y + height / 2, z + length / 2);

    // Back Face
    rlNormal3f(0.0f, 0.0f, -1.0f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width / 2, y - height / 2, z - length / 2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width / 2, y + height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z - length / 2);

    // Top Face
    rlNormal3f(0.0f, 1.0f, 0.0f);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width / 2, y + height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width / 2, y + height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width / 2, y + height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z - length / 2);

    // Bottom Face
    rlNormal3f(0.0f, -1.0f, 0.0f);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width / 2, y - height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width / 2, y - height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width / 2, y - height / 2, z + length / 2);

    // Right face
    rlNormal3f(1.0f, 0.0f, 0.0f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z - length / 2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z + length / 2);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z + length / 2);

    // Left Face
    rlNormal3f(-1.0f, 0.0f, 0.0f);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width / 2, y - height / 2, z - length / 2);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width / 2, y - height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width / 2, y + height / 2, z + length / 2);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width / 2, y + height / 2, z - length / 2);

    rlEnd();
    rlSetTexture(0);
}

void CloseInGameMenus() {
    inventoryOpen = false;
    isCraftingOpen = false;
    isMapOpen = false;
}

void InitNewGame(Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* health, float* stamina, float* hunger, float* thirst, float* yaw, float* pitch, bool* onGround, InventorySlot* inventory, float* flashlightBattery, bool* isFlashlightOn, char map[MAP_SIZE][MAP_SIZE], float* fov) {
    *playerPosition = Vector3{ MAP_SIZE / 2.0f, playerHeight, MAP_SIZE / 2.0f };
    *playerVelocity = Vector3{ 0.0f, 0.0f, 0.0f };
    camera->position = *playerPosition;

    *yaw = -90.0f;
    *pitch = 0.0f;
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
    inventory[BACKPACK_SLOTS] = { ITEM_PISTOL, 1, 7 };
    inventory[BACKPACK_SLOTS + 1] = { ITEM_FLASHLIGHT, 1, 0 };

    inventory[0] = { ITEM_WATER_BOTTLE, 2, 0 };
    inventory[1] = { ITEM_WOOD, 1, 0 };
    inventory[2] = { ITEM_STONE, 2, 0 };
    inventory[3] = { ITEM_MAG, 2, 0 };

    GenerateMap(map);

    currentFloor = -1;
    currentBuildingIndex = -1;

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
    for (int i = 0; i < ACTION_COUNT; i++) bindings[i] = defaultBindings[i];
}

int main() {
    // Load graphics settings before window creation
    LoadGraphicsSettings(&graphicsSettings);

    const Resolution& initialRes = AVAILABLE_RESOLUTIONS[graphicsSettings.resolutionIndex];

    // Set config flags before InitWindow
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    if (graphicsSettings.msaa) {
        if (graphicsSettings.msaaSamples == 2) SetConfigFlags(FLAG_MSAA_4X_HINT);
        else if (graphicsSettings.msaaSamples == 4) SetConfigFlags(FLAG_MSAA_4X_HINT);
    }

    InitWindow(initialRes.width, initialRes.height, "Echoes of Time - Enhanced Edition");
    SetExitKey(KEY_NULL);

    EnableCursor();
    cursorHidden = false;

    // Apply initial graphics settings
    ApplyGraphicsSettings(graphicsSettings);

    // Initialize texture and shader systems
    InitializeRenderingSystems();

    InitNewGame(&camera, &playerPosition, &playerVelocity, &health, &stamina, &hunger, &thirst, &yaw, &pitch, &onGround, inventory, &flashlightBattery, &isFlashlightOn, map, &fov);

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    bool prevBindingMode = isBindingMode;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Performance monitoring
        frameTimeAccumulator += deltaTime;
        frameCount++;
        if (frameTimeAccumulator >= 1.0f) {
            avgFrameTime = frameTimeAccumulator / frameCount;
            frameTimeAccumulator = 0.0f;
            frameCount = 0;
        }

        bool isAnyMenuOpen = (inventoryOpen || isCraftingOpen || isMapOpen);
        bool useController = isControllerEnabled && IsGamepadAvailable(0);

        bool shouldCaptureCursor = (gameState == GameState::Gameplay && !isAnyMenuOpen && !isBindingMode) || isBindingMode;

        static bool prevCursorCaptured = false;
        if (shouldCaptureCursor != prevCursorCaptured) {
            if (shouldCaptureCursor) {
                Vector2 center = { (float)(screenW / 2), (float)(screenH / 2) };
                SetMousePosition((int)center.x, (int)center.y);
                DisableCursor();
            }
            else {
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
            }
            else if (gameState == GameState::Paused) {
                gameState = GameState::Gameplay;
            }
            else if (gameState == GameState::LoadMenu) {
                gameState = stateBeforeSettings;
            }
            else if (gameState == GameState::Settings) {
                gameState = GameState::Paused;
            }
            else if (gameState == GameState::GraphicsSettings) {
                gameState = GameState::Settings;
            }
            else if (gameState == GameState::ControllerBindings) {
                gameState = GameState::Settings;
            }
            else if (gameState == GameState::Console) {
                gameState = GameState::Gameplay;
            }
            else if (isAnyMenuOpen) {
                CloseInGameMenus();
            }
        }

        if (IsKeyPressed(KEY_GRAVE)) {
            if (gameState == GameState::Gameplay) gameState = GameState::Console;
            else if (gameState == GameState::Console) gameState = GameState::Gameplay;
        }

        // --- Gameplay Input & Logic ---
        if (gameState == GameState::Gameplay) {
            bool inventoryTogglePressed = IsKeyPressed(KEY_I) || (useController && IsActionPressed(ACTION_INVENTORY, bindings));
            if (inventoryTogglePressed) { CloseInGameMenus(); inventoryOpen = !inventoryOpen; }

            bool craftingTogglePressed = IsKeyPressed(KEY_C) || (useController && IsActionPressed(ACTION_CRAFTING, bindings));
            if (craftingTogglePressed) { CloseInGameMenus(); isCraftingOpen = !isCraftingOpen; if (isCraftingOpen) selectedRecipeIndex = 0; }

            bool mapTogglePressed = IsKeyPressed(KEY_M) || (useController && IsActionPressed(ACTION_MAP, bindings));
            if (mapTogglePressed) { CloseInGameMenus(); isMapOpen = !isMapOpen; }

            if (!isAnyMenuOpen) {
                UpdatePlayer(deltaTime, &camera, &playerPosition, &playerVelocity, &yaw, &pitch, &onGround, playerSpeed, playerHeight, gravity, jumpForce, &stamina, isNoclip, useController);

                if (IsKeyPressed(KEY_E)) {
                    Door* nearDoor = GetNearestDoor(playerPosition, 2.0f);
                    if (nearDoor) {
                        if (currentFloor < 0) {
                            nearDoor->isOpen = true;
                            for (size_t i = 0; i < buildingInteriors.size(); i++) {
                                if (Vector3Distance(nearDoor->position, buildingInteriors[i].worldPos) < 10.0f) {
                                    currentBuildingIndex = (int)i;
                                    currentFloor = 0;
                                    playerPosition = nearDoor->targetPosition;
                                    camera.position = playerPosition;
                                    break;
                                }
                            }
                        }
                        else {
                            currentFloor = -1;
                            currentBuildingIndex = -1;
                            playerPosition = Vector3Add(nearDoor->position, Vector3{ 0, 0, 2.0f });
                            camera.position = playerPosition;
                        }
                    }
                }

                UpdateDoors(deltaTime);

                bool flashlightPressed = useController ? IsActionPressed(ACTION_FLASHLIGHT, bindings) : IsKeyPressed(KEY_F);
                if (flashlightPressed) isFlashlightOn = !isFlashlightOn;

                if (isFlashlightOn && flashlightBattery > 0.0f) {
                    flashlightBattery -= 5.0f * deltaTime;
                }
                else if (flashlightBattery <= 0.0f) {
                    isFlashlightOn = false;
                    flashlightBattery = 0.0f;
                }

                bool useItemPressed = useController ? IsActionPressed(ACTION_USE_ITEM, bindings) : IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);
                if (useItemPressed) {
                    UseEquippedItem(inventory, &health, &stamina, &hunger, &thirst);
                }

                bool reloadPressed = IsKeyPressed(KEY_R) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT));
                if (reloadPressed) {
                    ReloadWeapon(inventory);
                }

                float drainRate = 1.0f * deltaTime;
                hunger = fmaxf(0.0f, hunger - drainRate);
                thirst = fmaxf(0.0f, thirst - drainRate * 1.5f);
                stamina = fminf(100.0f, stamina + drainRate * 2.0f);

                if (health <= 0.0f) gameState = GameState::GameOver;

                pistolRecoilPitch = fmaxf(0.0f, pistolRecoilPitch - RECOIL_DECAY_RATE * deltaTime * 60.0f);
                pistolRecoilYaw = fmaxf(0.0f, pistolRecoilYaw - RECOIL_DECAY_RATE * deltaTime * 60.0f);

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

        // Menu state handling
        if (gameState == GameState::MainMenu) {
            if (IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) {
                if (mainMenuSelection == 0) { InitNewGame(&camera, &playerPosition, &playerVelocity, &health, &stamina, &hunger, &thirst, &yaw, &pitch, &onGround, inventory, &flashlightBattery, &isFlashlightOn, map, &fov); gameState = GameState::Gameplay; }
                if (mainMenuSelection == 1) { stateBeforeSettings = GameState::MainMenu; saveSlotSelection = 0; gameState = GameState::LoadMenu; }
                if (mainMenuSelection == 2) { stateBeforeSettings = GameState::MainMenu; settingsSelection = 0; gameState = GameState::Settings; }
                if (mainMenuSelection == 3) break;
            }
        }
        else if (gameState == GameState::Paused) {
            if (IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) {
                if (pauseMenuSelection == 0) gameState = GameState::Gameplay;
                if (pauseMenuSelection == 1) { stateBeforeSettings = GameState::Paused; saveSlotSelection = 0; gameState = GameState::LoadMenu; }
                if (pauseMenuSelection == 2) { stateBeforeSettings = GameState::Paused; settingsSelection = 0; gameState = GameState::Settings; }
                if (pauseMenuSelection == 3) gameState = GameState::MainMenu;
            }
        }
        else if (gameState == GameState::LoadMenu) {
            if ((IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)))) {
                bool fileExists = SaveFileExists(saveSlotSelection + 1);
                if (stateBeforeSettings == GameState::Paused) {
                    SaveGame(saveSlotSelection + 1, playerPosition, yaw, pitch, health, stamina, hunger, thirst, inventory, flashlightBattery, isFlashlightOn, map, fov);
                    gameState = GameState::Paused;
                }
                else if (stateBeforeSettings == GameState::MainMenu && fileExists) {
                    if (LoadGame(saveSlotSelection + 1, &playerPosition, &yaw, &pitch, &health, &stamina, &hunger, &thirst, inventory, &flashlightBattery, &isFlashlightOn, map, &fov)) {
                        camera.position = playerPosition;
                        Vector3 target = { cosf(DEG2RAD * yaw), sinf(DEG2RAD * pitch), sinf(DEG2RAD * yaw) * cosf(DEG2RAD * pitch) };
                        camera.target = Vector3Add(camera.position, target);
                        camera.fovy = fov;
                        gameState = GameState::Gameplay;
                    }
                }
            }
        }

        int newScreenW = GetScreenWidth();
        int newScreenH = GetScreenHeight();
        if (newScreenW != screenW || newScreenH != screenH) {
            screenW = newScreenW;
            screenH = newScreenH;
        }

        // --- RENDERING ---
        BeginDrawing();

        ClearBackground(Color{ 5, 10, 15, 255 });

        // Only render 3D when necessary
        if (gameState == GameState::Gameplay || gameState == GameState::Paused) {

            // Update shader lighting uniforms
            if (g_ShaderManager) {
                Vector3 sunPos = { MAP_SIZE / 2.0f, 100.0f, MAP_SIZE / 2.0f };
                Vector3 flashDir = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
                float flashIntensity = (flashlightBattery / 100.0f) * 5.0f;

                g_ShaderManager->UpdateLighting(camera, sunPos, isFlashlightOn,
                    camera.position, flashDir, flashIntensity);
            }

            BeginMode3D(camera);

            // Optimized grid drawing (only when needed)
            if (currentFloor < 0) {
                if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
                    BeginShaderMode(g_ShaderManager->GetLightingShader());
                }
                DrawGrid(MAP_SIZE, GRID_SIZE);
                if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
                    EndShaderMode();
                }
            }

            DrawMapGeometry(map);
            DrawPlayerHands(camera, inventory, pistolRecoilPitch, pistolRecoilYaw);

            EndMode3D();

            Door* nearDoor = GetNearestDoor(playerPosition, 2.0f);
            if (nearDoor && !isAnyMenuOpen) {
                const char* doorText = currentFloor < 0 ? "Press E to Enter" : "Press E to Exit";
                int textWidth = MeasureText(doorText, 20);
                DrawText(doorText, screenW / 2 - textWidth / 2, screenH - 100, 20, PIPBOY_GREEN);
            }

            // Post-processing effects
            if (graphicsSettings.renderScale >= 0.9f) {
                DrawRectangleGradientV(0, 0, screenW, screenH / 5, Color{ 0, 0, 0, 120 }, Color{ 0, 0, 0, 0 });
                DrawRectangleGradientV(0, screenH * 4 / 5, screenW, screenH / 5, Color{ 0, 0, 0, 0 }, Color{ 0, 0, 0, 120 });
                DrawRectangleGradientH(0, 0, screenW / 6, screenH, Color{ 0, 0, 0, 100 }, Color{ 0, 0, 0, 0 });
                DrawRectangleGradientH(screenW * 5 / 6, 0, screenW / 6, screenH, Color{ 0, 0, 0, 0 }, Color{ 0, 0, 0, 100 });

                if (isFlashlightOn && flashlightBattery > 0.0f) {
                    float glowAlpha = (flashlightBattery / 100.0f) * 30.0f;
                    DrawRectangle(0, 0, screenW, screenH, Color{ 255, 245, 200, (unsigned char)glowAlpha });
                }

                if (health < 30.0f) {
                    float pulseIntensity = sinf(GetTime() * 2.0f) * 0.5f + 0.5f;
                    unsigned char redAlpha = (unsigned char)((30.0f - health) * 2.0f * pulseIntensity);
                    DrawRectangle(0, 0, screenW, screenH, Color{ 180, 0, 0, redAlpha });
                }
            }

            if (showMinimap && gameState == GameState::Gameplay && !isMapOpen) {
                DrawMinimap(map, playerPosition, yaw, screenW - 160, 10, 150, 150, true, 0);
            }

            if (gameState == GameState::Gameplay) {
                DrawHUD(screenW, screenH, health, stamina, hunger, thirst, fov, flashlightBattery, isFlashlightOn, inventory);
            }

            if (isMapOpen) DrawMapMenu(screenW, screenH, map, playerPosition, yaw);
            if (isCraftingOpen) DrawCraftingMenu(screenW, screenH, inventory, &selectedRecipeIndex, useController);
            if (inventoryOpen) DrawInventory(screenW, screenH, inventory, &selectedHandSlot, &selectedInvSlot, useController);
        }

        // Menu rendering
        if (gameState == GameState::MainMenu) {
            ClearBackground(PIPBOY_DARK);
            std::vector<std::string> options = { "New Game", "Load Game", "Settings", "Exit" };
            DrawMenu(screenW, screenH, options, &mainMenuSelection, useController, "ECHOES OF TIME");
        }
        else if (gameState == GameState::Paused) {
            DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 180 });
            std::vector<std::string> options = { "Continue", "Save Game", "Settings", "Main Menu" };
            DrawMenu(screenW, screenH, options, &pauseMenuSelection, useController, "PAUSED");
        }
        else if (gameState == GameState::GameOver) {
            DrawRectangle(0, 0, screenW, screenH, Color{ 10, 10, 10, 200 });
            DrawText("GAME OVER", screenW / 2 - MeasureText("GAME OVER", 80) / 2, screenH / 2 - 40, 80, PIPBOY_GREEN);
            DrawText("You perished. Press ESC to return to main menu.", screenW / 2 - MeasureText("You perished. Press ESC to return to main menu.", 20) / 2, screenH / 2 + 40, 20, PIPBOY_GREEN);
        }
        else if (gameState == GameState::LoadMenu) {
            DrawLoadMenu(screenW, screenH, &saveSlotSelection, stateBeforeSettings);
        }
        else if (gameState == GameState::Settings) {
            GameState tempState = stateBeforeSettings;
            DrawSettingsMenu(screenW, screenH, &showMinimap, &isControllerEnabled, &isFullscreen, &settingsSelection, &tempState);
        }
        else if (gameState == GameState::GraphicsSettings) {
            GameState tempState = GameState::Settings;
            DrawGraphicsSettingsMenu(screenW, screenH, &graphicsSettings, &graphicsSettingsSelection, &tempState);
        }
        else if (gameState == GameState::ControllerBindings) {
            DrawControllerBindings(screenW, screenH, &activeBindingIndex, &isBindingMode, &controllerSettingsSelection, bindings);
        }
        else if (gameState == GameState::Console) {
            DrawConsole(screenW, screenH, consoleHistory, consoleInput, consoleInputLength);
        }

        // FPS display
        if (graphicsSettings.showFPS) {
            DrawText(TextFormat("FPS: %d (%.2fms)", GetFPS(), avgFrameTime * 1000.0f),
                10, 10, 20, PIPBOY_GREEN);
        }

        EndDrawing();
    }

    // Cleanup rendering systems
    CleanupRenderingSystems();

    CloseWindow();
    return 0;
}