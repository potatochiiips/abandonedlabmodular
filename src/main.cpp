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
#include "rendering.h"
#include "game_manager.h"
#include "enhanced_map_system.h"

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
bool showcompass = true;
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
//door management

static float frameTimeAccumulator = 0.0f;
static int frameCount = 0;
static float avgFrameTime = 0.0f;

void CloseInGameMenus() {
    inventoryOpen = false;
    isCraftingOpen = false;
    isMapOpen = false;
}

// FIXED: DrawMinimapHUD implementation
void DrawMinimapHUD(int screenW, int screenH, Vector3 playerPos, float playerYaw) {
    if (!g_EnhancedMapSystem) return;

    int minimapSize = 200;
    int minimapX = screenW - minimapSize - 20;
    int minimapY = 20;

    g_EnhancedMapSystem->DrawMinimap(minimapX + minimapSize / 2, minimapY + minimapSize / 2,
        minimapSize / 2, playerPos, playerYaw);
}

// FIXED: DrawCompass implementation
void DrawCompass(int screenW, int screenH, float yaw) {
    int compassX = screenW / 2;
    int compassY = 30;
    int compassRadius = 50;

    DrawCircle(compassX, compassY, compassRadius, Color{ 0, 0, 0, 150 });
    DrawCircleLines(compassX, compassY, compassRadius, PIPBOY_GREEN);

    // Draw cardinal directions
    const char* directions[] = { "N", "E", "S", "W" };
    float angles[] = { 0, 90, 180, 270 };

    for (int i = 0; i < 4; i++) {
        float angle = (angles[i] - yaw) * DEG2RAD;
        float dx = sinf(angle) * (compassRadius - 10);
        float dy = -cosf(angle) * (compassRadius - 10);

        DrawText(directions[i],
            compassX + (int)dx - 5,
            compassY + (int)dy - 5,
            20, PIPBOY_GREEN);
    }

    // Draw player arrow
    DrawTriangle(
        Vector2{ (float)compassX, (float)(compassY - 15) },
        Vector2{ (float)(compassX - 8), (float)(compassY - 5) },
        Vector2{ (float)(compassX + 8), (float)(compassY - 5) },
        RED
    );
}
void InitNewGame(Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* health, float* stamina, float* hunger, float* thirst, float* yaw, float* pitch, bool* onGround, InventorySlot* inventory, float* flashlightBattery, bool* isFlashlightOn, char map[MAP_SIZE][MAP_SIZE], float* fov) {
    if (g_EnhancedMapSystem) {
        // Spawn player inside the laboratory
        *playerPosition = g_EnhancedMapSystem->GetPlayerSpawnPosition();
        camera->position = *playerPosition;

        // Set player state to be inside the laboratory interior
        g_MapPlayer.insideInterior = true;
        g_MapPlayer.currentInteriorId = "lab_1";
        g_MapPlayer.currentBuildingId = 1;
        g_MapPlayer.interiorX = 10;
        g_MapPlayer.interiorY = 15;
        g_MapPlayer.worldX = 87;
        g_MapPlayer.worldY = 87;

        TraceLog(LOG_INFO, "Player spawned inside laboratory interior");
    }
    else {
        GenerateMapData(g_MapData);
        InitializePlayerFromMapStart(g_MapData, g_MapPlayer);
        *playerPosition = Vector3{ (float)g_MapPlayer.interiorX, playerHeight, (float)g_MapPlayer.interiorY };
        camera->position = *playerPosition;
    }

    *playerVelocity = Vector3{ 0.0f, 0.0f, 0.0f };
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

    int quest1 = g_QuestManager.AddQuest(
        "Escape the Laboratory",
        "Find a way out of the cryogenic facility",
        100
    );
    g_QuestManager.AddObjective(
        quest1,
        QUEST_OBJ_REACH_LOCATION,
        0,
        1,
        "Find the exit door"
    );

    TraceLog(LOG_INFO, "New game initialized - Player spawned at position: %.1f, %.1f, %.1f (inside laboratory)", playerPosition->x, playerPosition->y, playerPosition->z);
}
Door* GetNearestDoor(Vector3 playerPos, float maxDistance);
int main() {
    InitializeSoundSystem();
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
    InitializeDayNightSystem();
    InitializeWeatherSystem();
    InitializeZombieSystem();
    InitializeEnhancedMapSystem();

    // IMPORTANT: Initialize Upgraded pipeline BEFORE game manager
    InitializeUpgradedPipeline();

    UpgradedGameManager gameManager;
    gameManager.Initialize();

    InitNewGame(&camera, &playerPosition, &playerVelocity, &health, &stamina, &hunger, &thirst, &yaw, &pitch, &onGround, inventory, &flashlightBattery, &isFlashlightOn, map, &fov);

    if (g_SoundManager) {
        MusicID currentMusic = g_MapPlayer.insideInterior ? MUS_AMBIENT_INSIDE : MUS_AMBIENT_OUTSIDE;
        g_SoundManager->PlayMusic(currentMusic, true, 3.0f);
    }
    if (g_SoundManager) {
        g_SoundManager->PlayMusic(MUS_MENU, true, 2.0f);
    }

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

        if (g_DayNightCycle) g_DayNightCycle->Update(deltaTime);
        if (g_WeatherSystem) g_WeatherSystem->Update(deltaTime, playerPosition);
        if (g_ZombieManager) g_ZombieManager->Update(deltaTime, playerPosition);

        // Update game manager
        if (gameState == GameState::Gameplay) {
            gameManager.Update(deltaTime);
        }

        bool isAnyMenuOpen = (inventoryOpen || isCraftingOpen || isMapOpen);
        bool useController = isControllerEnabled && IsGamepadAvailable(0);

        bool shouldCaptureCursor = (gameState == GameState::Gameplay && !isAnyMenuOpen && gameState != GameState::Console);

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
                TraceLog(LOG_INFO, "Console opened");
            }
            else if (gameState == GameState::Console) {
                gameState = GameState::Gameplay;
                TraceLog(LOG_INFO, "Console closed");
            }
        }

        // FIXED: Escape key handling for console
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (gameState == GameState::Console) {
                // Close console with ESC
                gameState = GameState::Gameplay;
                TraceLog(LOG_INFO, "Console closed via ESC");
            }
            else if (gameState == GameState::Gameplay && !isAnyMenuOpen) {
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

        // Update sound manager
        if (g_SoundManager) {
            g_SoundManager->Update(deltaTime);
        }

        // FIXED: Console input handling - only when console is open
        if (gameState == GameState::Console) {
            UpdateConsoleInput(&health, &stamina, &hunger, &thirst, &isNoclip, &fov);
        }
        else if (gameState == GameState::Gameplay) {
            // Regular gameplay input handling
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

                // Door interaction (E key)
                if (IsKeyPressed(KEY_E)) {
                    if (g_VehicleManager && !g_VehicleManager->TryEnterVehicle(playerPosition)) {
                        Door* nearDoor = GetNearestDoor(playerPosition, 2.5f);

                        if (nearDoor) {
                            if (nearDoor->isInteriorDoor) {
                                TraceLog(LOG_INFO, "Exiting interior via door at (%.1f, %.1f, %.1f)",
                                    nearDoor->position.x, nearDoor->position.y, nearDoor->position.z);

                                g_MapPlayer.insideInterior = false;

                                playerPosition = Vector3{
                                    (float)g_MapPlayer.worldX,
                                    playerHeight,
                                    (float)g_MapPlayer.worldY
                                };
                                camera.position = playerPosition;

                                gameManager.RegenerateScene();

                                if (g_SoundManager) {
                                    g_SoundManager->PlayMusic(MUS_AMBIENT_OUTSIDE, true, 2.0f);
                                }

                                TraceLog(LOG_INFO, "Player exited to world position: (%.1f, %.1f, %.1f)",
                                    playerPosition.x, playerPosition.y, playerPosition.z);
                            }
                            else {
                                TraceLog(LOG_INFO, "Entering building %d via door at (%.1f, %.1f, %.1f)",
                                    nearDoor->buildingId, nearDoor->position.x, nearDoor->position.y, nearDoor->position.z);

                                g_MapPlayer.worldX = (int)playerPosition.x;
                                g_MapPlayer.worldY = (int)playerPosition.z;

                                if (g_EnhancedMapSystem) {
                                    const Interior* interior = g_EnhancedMapSystem->GetLabInterior();

                                    if (interior) {
                                        g_MapPlayer.insideInterior = true;
                                        g_MapPlayer.currentInteriorId = interior->id;
                                        g_MapPlayer.currentBuildingId = nearDoor->buildingId;

                                        if (!interior->floors.empty()) {
                                            g_MapPlayer.interiorX = interior->floors[0].playerSpawnX;
                                            g_MapPlayer.interiorY = interior->floors[0].playerSpawnY;

                                            playerPosition = Vector3{
                                                (float)g_MapPlayer.interiorX,
                                                playerHeight,
                                                (float)g_MapPlayer.interiorY
                                            };
                                            camera.position = playerPosition;

                                            gameManager.RegenerateScene();

                                            if (g_SoundManager) {
                                                g_SoundManager->PlayMusic(MUS_AMBIENT_INSIDE, true, 2.0f);
                                            }

                                            TraceLog(LOG_INFO, "Player entered interior at: (%.1f, %.1f, %.1f)",
                                                playerPosition.x, playerPosition.y, playerPosition.z);
                                        }
                                    }
                                    else {
                                        TraceLog(LOG_WARNING, "Interior not found for building %d", nearDoor->buildingId);
                                    }
                                }
                                else {
                                    TraceLog(LOG_ERROR, "Enhanced map system not initialized!");
                                }
                            }

                            if (g_SoundManager) {
                                g_SoundManager->PlaySound(SND_DOOR_OPEN, 0.5f);
                            }
                        }
                        else {
                            TraceLog(LOG_INFO, "No door nearby (closest is > 2.5 units away)");
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
                    if (g_SoundManager) {
                        g_SoundManager->PlaySound(SND_RELOAD, 0.5f);
                    }
                }

                if (isReloading) {
                    reloadTimer -= deltaTime;
                    if (reloadTimer <= 0) isReloading = false;
                }

                g_WeaponSystem.UpdateWeapon(g_CurrentWeaponState, deltaTime);

                // FIXED: Call the weapon shooting handler
                HandleWeaponShooting();

                hunger = fmaxf(0, hunger - 0.5f * deltaTime);
                thirst = fmaxf(0, thirst - 0.7f * deltaTime);
                if (health <= 0) gameState = GameState::GameOver;
                shotTimer = fmaxf(0, shotTimer - deltaTime);
            }
            }
            if (g_VehicleManager) g_VehicleManager->Update(deltaTime);
            if (g_AnimationManager) g_AnimationManager->UpdateAll(deltaTime);
        }
        else if (gameState == GameState::Console) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key <= 126 && consoleInputLength < MAX_COMMAND_LENGTH - 1) {
                    consoleInput[consoleInputLength] = (char)key;
                    consoleInputLength++;
                    consoleInput[consoleInputLength] = '\0';
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && consoleInputLength > 0) {
                consoleInputLength--;
                consoleInput[consoleInputLength] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER)) {
                ProcessConsoleCommand(consoleHistory, &health, &stamina, &hunger, &thirst, &isNoclip, &fov);
            }
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
        ClearBackground(Color{ 135, 206, 235, 255 }); // Sky blue

        if (gameState == GameState::Gameplay) {
            // FIXED: Render game world properly
            gameManager.Render();

            // FIXED: Draw HUD elements
            if (!isAnyMenuOpen) {
                // Draw player weapon/hands in first person
                DrawPlayerHands(camera, inventory, pistolRecoilPitch, pistolRecoilYaw);
            }
        }
        else if (gameState == GameState::MainMenu) {
            std::vector<std::string> opts = { "New Game", "Load Game", "Settings", "Exit" };
            DrawMenu(GetScreenWidth(), GetScreenHeight(), opts, &mainMenuSelection,
                false, "ECHOES OF TIME");
        }

        // FIXED: Draw minimap correctly
        if (gameState == GameState::Gameplay && showMinimap && !isAnyMenuOpen) {
            DrawMinimapHUD(screenW, screenH, playerPosition, yaw);
        }

        g_QuestManager.DrawQuestTrackerCompact(screenW, screenH);

        // FIXED: Draw compass correctly
        if (gameState == GameState::Gameplay && showcompass && !isAnyMenuOpen) {
            DrawCompass(screenW, screenH, yaw);
        }

        // Also add a visual indicator when near a door (in the HUD drawing section):
        if (gameState == GameState::Gameplay && !isAnyMenuOpen) {
            Door* nearbyDoor = GetNearestDoor(playerPosition, 2.5f);
            if (nearbyDoor) {
                const char* doorText = nearbyDoor->isInteriorDoor ?
                    "Press E to Exit Building" :
                    "Press E to Enter Building";

                int textWidth = MeasureText(doorText, 20);
                DrawText(doorText,
                    screenW / 2 - textWidth / 2,
                    screenH - 100,
                    20,
                    PIPBOY_GREEN);
            }
        }

        if (inventoryOpen) DrawInventory(screenW, screenH, inventory, &selectedHandSlot, &selectedInvSlot, useController);
        if (isCraftingOpen) DrawCraftingMenu(screenW, screenH, inventory, &selectedRecipeIndex, useController);
        if (isMapOpen) DrawMapMenu(screenW, screenH, map, playerPosition, yaw);

        if (gameState == GameState::Paused) {
            DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 180 });
            std::vector<std::string> opts = { "Continue", "Save Game", "Settings", "Main Menu" };
            DrawMenu(screenW, screenH, opts, &pauseMenuSelection, useController, "PAUSED");
        }
        else if (gameState == GameState::Console) {
            DrawConsole(screenW, screenH, consoleHistory, consoleInput, consoleInputLength);
        }
        else if (gameState == GameState::LoadMenu) {
            DrawLoadMenu(screenW, screenH, &saveSlotSelection, stateBeforeSettings);
        }
        else if (gameState == GameState::Settings) {
            DrawSettingsMenu(screenW, screenH, &showMinimap, &showcompass, &isControllerEnabled, &isFullscreen,
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

    CleanupZombieSystem();
    CleanupSoundSystem();
    CleanupWeatherSystem();
    CleanupDayNightSystem();
    CleanupModelSystem();
    CleanupRenderingSystems();
    CleanupAnimationSystem();
    CleanupVehicleSystem();
    CleanupSkyboxSystem();
    CleanupEnhancedMapSystem();
    CleanupUpgradedPipeline();
    gameManager.Cleanup();
    CloseWindow();
    return 0;
}