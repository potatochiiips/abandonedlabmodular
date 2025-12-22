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
#include "weapons.h"
#include "quest_system.h"
#include "ui_tabs.h"
#include "rlgl.h"
#include "upscaling_manager.h"
#include "model_manager.h"
#include "sound_manager.h"
#include "skybox.h"
#include "vehicle_system.h"
#include "animation_system.h"

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
bool isAimingDownSights = false;
bool isReloading = false;
float reloadTimer = 0.0f;
float adsTransitionProgress = 0.0f;
bool showMinimap = true;
bool isControllerEnabled = true;
bool isFullscreen = false;
int settingsSelection = 0;
int controllerSettingsSelection = 0;
int graphicsSettingsSelection = 0;
int audioSettingsSelection = 0;
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

GraphicsSettings graphicsSettings = {
    2, true, false, 4, 60, 1.0f, false, true, true, 1000,
    UPSCALING_NONE, UPSCALE_QUALITY_QUALITY, 1.0f, 150.0f
};

AnimationState playerAnimState = {
    ANIM_TYPE_IDLE, 0.0f, 0.0f, false, true
};

std::vector<Vehicle> vehicles;
Vehicle* playerVehicle = nullptr;

static float frameTimeAccumulator = 0.0f;
static int frameCount = 0;
static float avgFrameTime = 0.0f;

void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color) {
    float x = position.x;
    float y = position.y;
    float z = position.z;

    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);
    rlSetTexture(texture.id);

    rlNormal3f(0.0f, 0.0f, 1.0f);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width / 2, y - height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z + length / 2);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width / 2, y + height / 2, z + length / 2);

    rlNormal3f(0.0f, 0.0f, -1.0f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width / 2, y - height / 2, z - length / 2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width / 2, y + height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z - length / 2);

    rlNormal3f(0.0f, 1.0f, 0.0f);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width / 2, y + height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width / 2, y + height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width / 2, y + height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z - length / 2);

    rlNormal3f(-1.0f, -1.0f, 0.0f);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width / 2, y - height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width / 2, y - height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z + length / 2);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width / 2, y - height / 2, z + length / 2);

    rlNormal3f(1.0f, 0.0f, 0.0f);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z - length / 2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z - length / 2);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width / 2, y + height / 2, z + length / 2);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width / 2, y - height / 2, z + length / 2);

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
    GenerateMapData(g_MapData);
    InitializePlayerFromMapStart(g_MapData, g_MapPlayer);

    *playerPosition = Vector3{ (float)g_MapPlayer.interiorX, playerHeight, (float)g_MapPlayer.interiorY };
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
    inventory[4] = { ITEM_M16, 1, 25 };
    inventory[5] = { ITEM_M16_MAG, 3, 0 };
    inventory[6] = { ITEM_KNIFE, 1, 0 };

    GenerateMap(map);

    g_CurrentWeaponState.animState = ANIM_IDLE;
    g_CurrentWeaponState.animTimer = 0.0f;
    g_CurrentWeaponState.isADS = false;
    g_CurrentWeaponState.adsProgress = 0.0f;
    g_CurrentWeaponState.recoilOffset = Vector3{ 0, 0, 0 };

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

    int quest1 = g_QuestManager.AddQuest("Welcome to the Lab", "Get familiar with your surroundings", 100);
    g_QuestManager.AddObjective(quest1, QUEST_OBJ_COLLECT, ITEM_WATER_BOTTLE, 2, "Collect 2 water bottles");

    TraceLog(LOG_INFO, "Player spawned inside lab at position: %.1f, %.1f, %.1f", playerPosition->x, playerPosition->y, playerPosition->z);
}

int main() {
    LoadGraphicsSettings(&graphicsSettings);
    const Resolution& initialRes = AVAILABLE_RESOLUTIONS[graphicsSettings.resolutionIndex];

    int monitorWidth = GetMonitorWidth(0);
    int monitorHeight = GetMonitorHeight(0);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_FULLSCREEN_MODE);
    if (graphicsSettings.msaa) {
        if (graphicsSettings.msaaSamples == 2) SetConfigFlags(FLAG_MSAA_4X_HINT);
        else if (graphicsSettings.msaaSamples == 4) SetConfigFlags(FLAG_MSAA_4X_HINT);
    }

    InitWindow(monitorWidth, monitorHeight, "Echoes of Time");
    SetExitKey(KEY_NULL);

    Texture2D splashTexture = LoadTexture("assets/splash.png");

    float splashTime = 2.5f;
    while (splashTime > 0 && !WindowShouldClose()) {
        float dt = GetFrameTime();
        splashTime -= dt;

        BeginDrawing();
        ClearBackground(BLACK);

        if (splashTexture.id > 0) {
            DrawTexturePro(
                splashTexture,
                Rectangle{ 0, 0, (float)splashTexture.width, (float)splashTexture.height },
                Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
                Vector2{ 0, 0 },
                0.0f,
                WHITE
            );
        }

        const char* loadingText = "LOADING...";
        int textWidth = MeasureText(loadingText, 40);
        DrawText(loadingText, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() - 80, 40, WHITE);
        EndDrawing();
    }

    if (splashTexture.id > 0) UnloadTexture(splashTexture);

    InitializeUpscalingSystem(initialRes.width, initialRes.height);
    ApplyGraphicsSettings(graphicsSettings);
    InitializeRenderingSystems();
    InitializeModelSystem();
    InitializeSkyboxSystem();
    InitializeVehicleSystem();
    InitializeAnimationSystem();

    InitNewGame(&camera, &playerPosition, &playerVelocity, &health, &stamina, &hunger, &thirst, &yaw, &pitch, &onGround, inventory, &flashlightBattery, &isFlashlightOn, map, &fov);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        frameTimeAccumulator += deltaTime;
        frameCount++;
        if (frameTimeAccumulator >= 1.0f) {
            avgFrameTime = frameTimeAccumulator / frameCount;
            frameTimeAccumulator = 0.0f;
            frameCount = 0;
        }

        bool isAnyMenuOpen = (inventoryOpen || isCraftingOpen || isMapOpen);
        bool useController = isControllerEnabled && IsGamepadAvailable(0);

        // Cursor handling
        bool shouldCaptureCursor = (gameState == GameState::Gameplay && !isAnyMenuOpen) ||
            (gameState == GameState::Console);
        static bool prevCursorCaptured = false;
        if (shouldCaptureCursor != prevCursorCaptured) {
            if (shouldCaptureCursor) {
                SetMousePosition(screenW / 2, screenH / 2);
                DisableCursor();
            }
            else {
                EnableCursor();
            }
            prevCursorCaptured = shouldCaptureCursor;
        }

        // ESC handling
        if (IsKeyPressed(KEY_ESCAPE) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_START))) {
            if (gameState == GameState::Gameplay && !isAnyMenuOpen) {
                gameState = GameState::Paused;
                pauseMenuSelection = 0;
                stateBeforeSettings = GameState::Paused;
            }
            else if (gameState == GameState::Paused) {
                gameState = GameState::Gameplay;
            }
            else if (isAnyMenuOpen) {
                CloseInGameMenus();
            }
            else if (gameState == GameState::Console) {
                gameState = GameState::Gameplay;
            }
            else if (gameState == GameState::LoadMenu) {
                gameState = stateBeforeSettings;
            }
            else if (gameState == GameState::Settings ||
                gameState == GameState::GraphicsSettings ||
                gameState == GameState::AudioSettings ||
                gameState == GameState::ControllerBindings) {
                gameState = stateBeforeSettings;
            }
        }

        if (IsKeyPressed(KEY_GRAVE)) {
            if (gameState == GameState::Gameplay) {
                gameState = GameState::Console;
            }
            else if (gameState == GameState::Console) {
                gameState = GameState::Gameplay;
            }
        }

        // Gameplay Logic
        if (gameState == GameState::Gameplay) {
            if (IsKeyPressed(KEY_I) || (useController && IsActionPressed(ACTION_INVENTORY, bindings))) {
                CloseInGameMenus();
                inventoryOpen = !inventoryOpen;
            }
            if (IsKeyPressed(KEY_C) || (useController && IsActionPressed(ACTION_CRAFTING, bindings))) {
                CloseInGameMenus();
                isCraftingOpen = !isCraftingOpen;
                selectedRecipeIndex = 0;
            }
            if (IsKeyPressed(KEY_M) || (useController && IsActionPressed(ACTION_MAP, bindings))) {
                CloseInGameMenus();
                isMapOpen = !isMapOpen;
            }

            if (!isAnyMenuOpen) {
                if (g_VehicleManager && g_VehicleManager->IsPlayerInVehicle()) {
                    g_VehicleManager->HandleVehicleInput(deltaTime, useController);
                    Vehicle* v = g_VehicleManager->GetPlayerVehicle();
                    if (v) {
                        camera.position = Vector3{ v->position.x, v->position.y + 3.0f, v->position.z - 5.0f };
                        camera.target = v->position;
                    }
                    if (IsKeyPressed(KEY_F) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))) {
                        g_VehicleManager->ExitVehicle();
                        playerPosition = v->position;
                    }
                }
                else {
                    UpdatePlayer(deltaTime, &camera, &playerPosition, &playerVelocity, &yaw, &pitch, &onGround, playerSpeed, playerHeight, gravity, jumpForce, &stamina, isNoclip, useController);
                }

                if (IsKeyPressed(KEY_E)) {
                    if (g_VehicleManager && !g_VehicleManager->TryEnterVehicle(playerPosition)) {
                        Door* nearDoor = GetNearestDoor(playerPosition, 2.5f);
                        if (nearDoor) {
                            if (g_MapPlayer.insideInterior) ExitInterior(g_MapData, g_MapPlayer);
                            else EnterInterior(g_MapData, g_MapPlayer, nearDoor->buildingId);
                            playerPosition = g_MapPlayer.insideInterior ?
                                Vector3{ (float)g_MapPlayer.interiorX, playerHeight, (float)g_MapPlayer.interiorY } :
                                Vector3{ (float)g_MapPlayer.worldX, playerHeight, (float)g_MapPlayer.worldY };
                            camera.position = playerPosition;
                        }
                    }
                }

                UpdateDoors(deltaTime);
                if (IsKeyPressed(KEY_F) && !g_VehicleManager->IsPlayerInVehicle())
                    isFlashlightOn = !isFlashlightOn;

                int eq = inventory[BACKPACK_SLOTS].itemId;
                if (eq == ITEM_PISTOL || eq == ITEM_M16) {
                    g_CurrentWeaponState.isADS = IsMouseButtonDown(MOUSE_RIGHT_BUTTON) ||
                        (useController && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2));
                }

                if (IsKeyPressed(KEY_R) && !isReloading && ReloadWeapon(inventory)) {
                    isReloading = true;
                    WeaponStats* s = g_WeaponSystem.GetWeaponStats(inventory[BACKPACK_SLOTS].itemId);
                    reloadTimer = s ? s->reloadTime : 1.5f;
                }

                if (isReloading) {
                    reloadTimer -= deltaTime;
                    if (reloadTimer <= 0) isReloading = false;
                }

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && shotTimer <= 0 && !isReloading) {
                    WeaponStats* s = g_WeaponSystem.GetWeaponStats(eq);
                    if (s && inventory[BACKPACK_SLOTS].ammo > 0) {
                        shotTimer = s->fireRate;
                        inventory[BACKPACK_SLOTS].ammo--;
                        g_CurrentWeaponState.animState = ANIM_SHOOT;
                        g_CurrentWeaponState.animTimer = 0.2f;
                    }
                }

                hunger = fmaxf(0, hunger - 0.5f * deltaTime);
                thirst = fmaxf(0, thirst - 0.7f * deltaTime);
                if (health <= 0) gameState = GameState::GameOver;
                shotTimer = fmaxf(0, shotTimer - deltaTime);
            }
            if (g_VehicleManager) g_VehicleManager->Update(deltaTime);
            if (g_AnimationManager) g_AnimationManager->UpdateAll(deltaTime);
        }
        else if (gameState == GameState::Console) {
            UpdateConsoleInput(&health, &stamina, &hunger, &thirst, &isNoclip, &fov);
        }
        else if (gameState == GameState::LoadMenu) {
            if (IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) {
                if (stateBeforeSettings == GameState::Paused) {
                    SaveGame(saveSlotSelection + 1, playerPosition, yaw, pitch, health, stamina, hunger, thirst,
                        inventory, flashlightBattery, isFlashlightOn, map, fov);
                    gameState = GameState::Paused;
                }
                else if (stateBeforeSettings == GameState::MainMenu) {
                    if (LoadGame(saveSlotSelection + 1, &playerPosition, &yaw, &pitch, &health, &stamina,
                        &hunger, &thirst, inventory, &flashlightBattery, &isFlashlightOn, map, &fov)) {
                        camera.position = playerPosition;
                        Vector3 forward = {
                            cosf(DEG2RAD * yaw) * cosf(DEG2RAD * pitch),
                            sinf(DEG2RAD * pitch),
                            sinf(DEG2RAD * yaw) * cosf(DEG2RAD * pitch)
                        };
                        camera.target = Vector3Add(camera.position, forward);
                        gameState = GameState::Gameplay;
                    }
                }
            }
        }

        // RENDERING
        BeginDrawing();
        ClearBackground(Color{ 5, 10, 15, 255 });

        if (gameState == GameState::Gameplay || gameState == GameState::Paused) {
            if (g_UpscalingManager && graphicsSettings.upscalingMode != UPSCALING_NONE)
                g_UpscalingManager->BeginUpscaledRender();

            if (g_ShaderManager) {
                g_ShaderManager->UpdateLighting(camera, { MAP_SIZE / 2.0f, 100, MAP_SIZE / 2.0f },
                    isFlashlightOn, camera.position,
                    Vector3Normalize(Vector3Subtract(camera.target, camera.position)),
                    (flashlightBattery / 100.0f) * 5.0f);
            }

            BeginMode3D(camera);
            if (!g_MapPlayer.insideInterior) DrawGrid(MAP_SIZE, GRID_SIZE);
            DrawMapGeometry(map);
            g_WaypointManager.DrawIn3D(playerPosition, 100.0f);

            int eq = inventory[BACKPACK_SLOTS].itemId;
            Vector3 wPos = g_WeaponSystem.CalculateWeaponPosition(camera, g_CurrentWeaponState, eq == ITEM_M16);
            Vector3 fwd = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
            Vector3 rgt = Vector3Normalize(Vector3CrossProduct(fwd, camera.up));
            if (eq == ITEM_PISTOL) DrawEnhancedPistol(wPos, fwd, rgt, camera.up, g_CurrentWeaponState);
            else if (eq == ITEM_M16) DrawM16Rifle(wPos, fwd, rgt, camera.up, g_CurrentWeaponState);
            else DrawPlayerHands(camera, inventory, 0, 0);

            EndMode3D();
            if (g_UpscalingManager && graphicsSettings.upscalingMode != UPSCALING_NONE)
                g_UpscalingManager->EndUpscaledRender(screenW, screenH);

            if (showMinimap) {
                int minimapRadius = 95; // INCREASED from 75
                int minimapX = screenW - minimapRadius - 20;
                int minimapY = minimapRadius + 20;
                int viewRange = 18; // INCREASED from 15 for better visibility
                DrawRoundMinimap(map, playerPosition, yaw, minimapX, minimapY, minimapRadius, viewRange);
            }
        
            if (inventoryOpen) DrawInventory(screenW, screenH, inventory, &selectedHandSlot, &selectedInvSlot, useController);
            if (isCraftingOpen) DrawCraftingMenu(screenW, screenH, inventory, &selectedRecipeIndex, useController);
            if (isMapOpen) DrawMapMenu(screenW, screenH, map, playerPosition, yaw);

            if (gameState == GameState::Paused) {
                DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 180 });
                std::vector<std::string> opts = { "Continue", "Save Game", "Settings", "Main Menu" };
                DrawMenu(screenW, screenH, opts, &pauseMenuSelection, useController, "PAUSED");
            }
        }
        else if (gameState == GameState::MainMenu) {
            std::vector<std::string> opts = { "New Game", "Load Game", "Settings", "Exit" };
            DrawMenu(screenW, screenH, opts, &mainMenuSelection, useController, "ECHOES OF TIME");
        }
        else if (gameState == GameState::Console) {
            DrawConsole(screenW, screenH, consoleHistory, consoleInput, consoleInputLength);
        }
        else if (gameState == GameState::LoadMenu) {
            DrawLoadMenu(screenW, screenH, &saveSlotSelection, stateBeforeSettings);
        }
        else if (gameState == GameState::Settings) {
            DrawSettingsMenu(screenW, screenH, &showMinimap, &isControllerEnabled, &isFullscreen,
                &settingsSelection, &stateBeforeSettings);
        }
        else if (gameState == GameState::GraphicsSettings) {
            DrawGraphicsSettingsMenu(screenW, screenH, &graphicsSettings, &graphicsSettingsSelection, &stateBeforeSettings);
        }
        else if (gameState == GameState::AudioSettings) {
            DrawAudioSettingsMenu(screenW, screenH, &audioSettingsSelection, &stateBeforeSettings);
        }
        else if (gameState == GameState::ControllerBindings) {
            DrawControllerBindings(screenW, screenH, &activeBindingIndex, &isBindingMode,
                &controllerSettingsSelection, bindings);
        }

        if (graphicsSettings.showFPS) DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
        EndDrawing();
    }

    CleanupModelSystem();
    CleanupRenderingSystems();
    CleanupAnimationSystem();
    CleanupVehicleSystem();
    CleanupSkyboxSystem();
    CloseWindow();
    return 0;
}