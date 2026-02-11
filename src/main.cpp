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

static float frameTimeAccumulator = 0.0f;
static int frameCount = 0;
static float avgFrameTime = 0.0f;

// World generation menu state
enum class WorldGenState {
    Menu,
    Generating,
    Done
};

WorldGenState worldGenState = WorldGenState::Menu;
int worldGenMenuSelection = 0;
bool worldGenConfirmed = false;

void CloseInGameMenus() {
    inventoryOpen = false;
    isCraftingOpen = false;
    isMapOpen = false;
}

void DrawMinimapHUD(int screenW, int screenH, Vector3 playerPos, float playerYaw) {
    if (!g_EnhancedMapSystem) return;
    int minimapSize = 200;
    int minimapX = screenW - minimapSize - 20;
    int minimapY = 20;
    g_EnhancedMapSystem->DrawMinimap(minimapX + minimapSize / 2, minimapY + minimapSize / 2,
        minimapSize / 2, playerPos, playerYaw);
}

void DrawCompass(int screenW, int screenH, float yaw) {
    int compassX = screenW / 2;
    int compassY = 30;
    int compassRadius = 50;
    DrawCircle((float)compassX, (float)compassY, (float)compassRadius, Color{ 0, 0, 0, 150 });
    DrawCircleLines((float)compassX, (float)compassY, (float)compassRadius, PIPBOY_GREEN);
    const char* directions[] = { "N", "E", "S", "W" };
    float angles[] = { 0, 90, 180, 270 };
    for (int i = 0; i < 4; i++) {
        float angle = (angles[i] - yaw) * DEG2RAD;
        float dx = sinf(angle) * (compassRadius - 10);
        float dy = -cosf(angle) * (compassRadius - 10);
        DrawText(directions[i], compassX + (int)dx - 5, compassY + (int)dy - 5, 20, PIPBOY_GREEN);
    }
    DrawTriangle(
        Vector2{ (float)compassX, (float)(compassY - 15) },
        Vector2{ (float)(compassX - 8), (float)(compassY - 5) },
        Vector2{ (float)(compassX + 8), (float)(compassY - 5) },
        RED
    );
}

void InitNewGame(Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* health, float* stamina, float* hunger, float* thirst, float* yaw, float* pitch, bool* onGround, InventorySlot* inventory, float* flashlightBattery, bool* isFlashlightOn, char map[MAP_SIZE][MAP_SIZE], float* fov) {
    if (g_EnhancedMapSystem) {
        *playerPosition = g_EnhancedMapSystem->GetPlayerSpawnPosition();
        camera->position = *playerPosition;
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
    g_QuestManager.AddObjective(quest1, QUEST_OBJ_REACH_LOCATION, 0, 1, "Find the exit door");

    TraceLog(LOG_INFO, "New game initialized with seed: %u", g_WorldSettings.seed);
}

Door* GetNearestDoor(Vector3 playerPos, float maxDistance);

// Forward declaration for splash screen function
void DrawSplashScreen(Texture2D splashTexture, float progress);

// Forward declaration for world gen menu
extern void DrawWorldGenMenu(int screenW, int screenH, WorldSettings* settings, int* menuSelection, bool* confirmed);
extern void UpdateWorldGenMenu(WorldSettings* settings, int* menuSelection, bool* confirmed);

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
    SetTargetFPS(60);

    // Load splash texture IMMEDIATELY
    Texture2D splashTexture = LoadTexture("assets/splash.png");
    
    float splashStartTime = (float)GetTime();
    float splashMinDuration = 2.0f;
    
    // ==========================
    // INITIALIZATION WITH SPLASH
    // ==========================
    
    // Define all initialization stages with their progress targets
    struct InitStage {
        const char* name;
        float progressTarget;
        std::function<void()> initFunc;
    };
    
    std::vector<InitStage> stages = {
        { "Upscaling System", 0.11f, [&]() { InitializeUpscalingSystem(initialRes.width, initialRes.height); } },
        { "Graphics Settings", 0.22f, [&]() { ApplyGraphicsSettings(graphicsSettings); } },
        { "Rendering Pipeline", 0.33f, []() { InitializeRenderingPipeline(); } },
        { "Model System", 0.44f, []() { InitializeModelSystem(); } },
        { "Skybox System", 0.55f, []() { InitializeSkyboxSystem(); } },
        { "Vehicle System", 0.66f, []() { InitializeVehicleSystem(); } },
        { "Animation System", 0.77f, []() { InitializeAnimationSystem(); } },
        { "Day/Night & Weather", 0.88f, [&]() { 
            InitializeDayNightSystem();
            InitializeWeatherSystem();
            InitializeZombieSystem();
        } },
        { "Map & Game Systems", 1.0f, []() { 
            InitializeEnhancedMapSystem();
            InitializeRenderingSystems();
        } }
    };
    
    float currentProgress = 0.0f;
    
    // Execute each initialization stage
    for (size_t i = 0; i < stages.size(); i++) {
        TraceLog(LOG_INFO, "Initializing: %s", stages[i].name);
        
        // Show splash while initializing
        float stageProgress = stages[i].progressTarget;
        stages[i].initFunc();
        
        // Update progress
        currentProgress = stageProgress;
        
        // Render splash for this stage
        BeginDrawing();
        DrawSplashScreen(splashTexture, currentProgress);
        EndDrawing();
        
        if (WindowShouldClose()) {
            if (splashTexture.id > 0) UnloadTexture(splashTexture);
            CloseWindow();
            return 0;
        }
        
        TraceLog(LOG_INFO, "%s initialized (%.0f%%)", stages[i].name, currentProgress * 100.0f);
    }
    
    // Initialize game manager
    UpgradedGameManager gameManager;
    gameManager.Initialize();
    
    // FIXED: Set the global game manager pointer for console commands
    extern UpgradedGameManager* g_GameManagerInstance;
    g_GameManagerInstance = &gameManager;
    
    // Final splash - show for minimum duration
    float initEndTime = (float)GetTime();
    float elapsedSplash = initEndTime - splashStartTime;
    float remainingSplashTime = fmaxf(0.0f, splashMinDuration - elapsedSplash);
    
    float splashEndTime = initEndTime + remainingSplashTime;
    while ((float)GetTime() < splashEndTime && !WindowShouldClose()) {
        BeginDrawing();
        DrawSplashScreen(splashTexture, 1.0f);
        EndDrawing();
    }
    
    if (splashTexture.id > 0) UnloadTexture(splashTexture);
    
    if (WindowShouldClose()) {
        CloseWindow();
        return 0;
    }

    // Main game loop starts here
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

        // Update sound system
        if (g_SoundManager) {
            g_SoundManager->Update(deltaTime);
        }

        if (g_DayNightCycle) g_DayNightCycle->Update(deltaTime);
        if (g_WeatherSystem) g_WeatherSystem->Update(deltaTime, playerPosition);
        if (g_ZombieManager) g_ZombieManager->Update(deltaTime, playerPosition);

        // Handle all game states
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

        // Main menu music
        if (g_SoundManager) {
            g_SoundManager->PlayMusic(MUS_MENU, true, 2.0f);
        }

        // Handle world generation menu ONLY when starting new game
        if (worldGenState == WorldGenState::Generating) {
            // Generate the world with current settings
            TraceLog(LOG_INFO, "Starting world generation with settings...");
            if (g_UpgradedMapRenderer) {
                g_UpgradedMapRenderer->GenerateEnhancedWorld();
            }
            worldGenState = WorldGenState::Done;
            TraceLog(LOG_INFO, "World generation complete!");
        }

        // Input handling for all states
        if (gameState == GameState::Console) {
            UpdateConsoleInput(&health, &stamina, &hunger, &thirst, &isNoclip, &fov);

            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_GRAVE)) {
                gameState = GameState::Gameplay;
                TraceLog(LOG_INFO, "Console closed");
            }
        }
        else if (IsKeyPressed(KEY_GRAVE)) {
            gameState = GameState::Console;
            TraceLog(LOG_INFO, "Console opened");
        }
        else if (IsKeyPressed(KEY_ESCAPE) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_START))) {
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
        else if (gameState == GameState::Gameplay) {
            // Gameplay-specific input
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
				UpdatePlayer(deltaTime, &camera, &playerPosition, &playerVelocity, &yaw, &pitch, &onGround, playerSpeed, playerHeight, gravity, jumpForce, &stamina, isNoclip, useController);

                if (g_VehicleManager && g_VehicleManager->IsPlayerInVehicle()) {
                    g_VehicleManager->HandleVehicleInput(deltaTime, useController);
                    Vehicle* v = g_VehicleManager->GetPlayerVehicle();
                    if (v) {
                        float radians = v->rotation * DEG2RAD;
                        Vector3 behindOffset = { -sinf(radians) * 8.0f, 0.0f, -cosf(radians) * 8.0f };

                        camera.position = Vector3Add(v->position, behindOffset);
                        camera.position.y = v->position.y + 4.0f;
                        camera.target = Vector3Add(v->position, Vector3{ 0, 1.0f, 0 });

                        playerPosition = v->position;
                    }
                    if (IsKeyPressed(KEY_F) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))) {
                        g_VehicleManager->ExitVehicle();
                        playerPosition = v->position;
                        playerPosition.y = playerHeight;
                    }
                }

                if (g_ZombieManager && !isAnyMenuOpen) {
                    const auto& zombies = g_ZombieManager->GetZombies();
                    for (const auto& zombie : zombies) {
                        if (!zombie.isAlive) continue;

                        float dist = Vector3Distance(playerPosition, zombie.position);
                        if (dist < zombie.attackRange && zombie.attackCooldown <= 0.0f) {
                            health -= zombie.attackDamage;
                            if (health < 0) health = 0;
                            TraceLog(LOG_INFO, "Zombie attacked! Health: %.1f", health);

                            if (g_HUDManager) {
                                g_HUDManager->ShowDamageIndicator(zombie.attackDamage);
                            }
                        }
                    }
                }

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
                HandleWeaponShooting();

                hunger = fmaxf(0, hunger - 0.5f * deltaTime);
                thirst = fmaxf(0, thirst - 0.7f * deltaTime);
                if (health <= 0) gameState = GameState::GameOver;
                shotTimer = fmaxf(0, shotTimer - deltaTime);
            }

            gameManager.Update(deltaTime);
            if (g_VehicleManager) g_VehicleManager->Update(deltaTime);
            if (g_AnimationManager) g_AnimationManager->UpdateAll(deltaTime);
        }
        else if (gameState == GameState::MainMenu) {
            // FIXED: Handle main menu input properly
            if (IsKeyPressed(KEY_UP) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP)) {
                mainMenuSelection = (mainMenuSelection - 1 + 4) % 4;
            }
            if (IsKeyPressed(KEY_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN)) {
                mainMenuSelection = (mainMenuSelection + 1) % 4;
            }

            if (IsKeyPressed(KEY_ENTER) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
                if (mainMenuSelection == 0) {
                    // New Game - show world gen menu
                    worldGenState = WorldGenState::Menu;
                    worldGenMenuSelection = 0;
                    worldGenConfirmed = false;
                    gameState = GameState::WorldGenMenu;
                }
                else if (mainMenuSelection == 1) {
                    // Load Game
                    gameState = GameState::LoadMenu;
                    stateBeforeSettings = GameState::MainMenu;
                    saveSlotSelection = 0;
                }
                else if (mainMenuSelection == 2) {
                    // Settings
                    gameState = GameState::Settings;
                    stateBeforeSettings = GameState::MainMenu;
                    settingsSelection = 0;
                }
                else if (mainMenuSelection == 3) {
                    // Exit
                    break;
                }
            }
        }
        else if (gameState == GameState::WorldGenMenu) {
            // FIXED: Handle world gen menu input
            UpdateWorldGenMenu(&g_WorldSettings, &worldGenMenuSelection, &worldGenConfirmed);
            
            if (worldGenConfirmed || IsKeyPressed(KEY_ENTER)) {
                worldGenConfirmed = false;
                worldGenState = WorldGenState::Generating;
                gameState = GameState::WorldGenGenerating;
            }
            
            // Allow escape to go back to main menu
            if (IsKeyPressed(KEY_ESCAPE)) {
                gameState = GameState::MainMenu;
                mainMenuSelection = 0;
                worldGenState = WorldGenState::Menu;
            }
        }
        else if (gameState == GameState::WorldGenGenerating) {
            // FIXED: Generate world then start game
            if (worldGenState == WorldGenState::Generating) {
                TraceLog(LOG_INFO, "Starting world generation with settings...");
                if (g_UpgradedMapRenderer) {
                    g_UpgradedMapRenderer->GenerateEnhancedWorld();
                }
                worldGenState = WorldGenState::Done;
                TraceLog(LOG_INFO, "World generation complete!");
                
                // Initialize the game
                InitNewGame(&camera, &playerPosition, &playerVelocity, &health, &stamina, &hunger, &thirst,
                    &yaw, &pitch, &onGround, inventory, &flashlightBattery, &isFlashlightOn, map, &fov);
                gameState = GameState::Gameplay;
                worldGenState = WorldGenState::Menu; // Reset for next game
            }
        }

        if (inventoryOpen) DrawInventory(screenW, screenH, inventory, &selectedHandSlot, &selectedInvSlot, useController);
        if (isCraftingOpen) DrawCraftingMenu(screenW, screenH, inventory, &selectedRecipeIndex, useController);
        if (isMapOpen) DrawMapMenu(screenW, screenH, map, playerPosition, yaw);

        if (gameState == GameState::Paused) {
            DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 180 });
            std::vector<std::string> opts = { "Continue", "Save Game", "Settings", "Main Menu" };
            DrawMenu(screenW, screenH, opts, &pauseMenuSelection, useController, "PAUSED");
            
            // FIXED: Handle pause menu input
            if (IsKeyPressed(KEY_UP) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP)) {
                pauseMenuSelection = (pauseMenuSelection - 1 + 4) % 4;
            }
            if (IsKeyPressed(KEY_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN)) {
                pauseMenuSelection = (pauseMenuSelection + 1) % 4;
            }
            if (IsKeyPressed(KEY_ENTER) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
                if (pauseMenuSelection == 0) {
                    gameState = GameState::Gameplay;
                }
                else if (pauseMenuSelection == 1) {
                    gameState = GameState::LoadMenu;
                    stateBeforeSettings = GameState::Paused;
                }
                else if (pauseMenuSelection == 2) {
                    gameState = GameState::Settings;
                    stateBeforeSettings = GameState::Paused;
                }
                else if (pauseMenuSelection == 3) {
                    gameState = GameState::MainMenu;
                    mainMenuSelection = 0;
                    worldGenState = WorldGenState::Menu;
                }
            }
        }
        else if (gameState == GameState::Console) {
            DrawConsole(screenW, screenH, consoleHistory, consoleInput, consoleInputLength);
        }
        else if (gameState == GameState::LoadMenu) {
            DrawLoadMenu(screenW, screenH, &saveSlotSelection, stateBeforeSettings);
            
            // FIXED: Handle load menu input
            if (IsKeyPressed(KEY_UP) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP)) {
                saveSlotSelection = (saveSlotSelection - 1 + 3) % 3;
            }
            if (IsKeyPressed(KEY_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN)) {
                saveSlotSelection = (saveSlotSelection + 1) % 3;
            }
            if (IsKeyPressed(KEY_ENTER) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
                if (stateBeforeSettings == GameState::Paused) {
                    SaveGame(saveSlotSelection + 1, playerPosition, yaw, pitch, health, stamina, hunger, thirst,
                        inventory, flashlightBattery, isFlashlightOn, map, fov);
                    gameState = GameState::Paused;
                    TraceLog(LOG_INFO, "Game saved to slot %d", saveSlotSelection + 1);
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
                        TraceLog(LOG_INFO, "Game loaded from slot %d", saveSlotSelection + 1);
                    }
                    else {
                        TraceLog(LOG_WARNING, "Failed to load game from slot %d", saveSlotSelection + 1);
                    }
                }
            }
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
        else if (gameState == GameState::WorldGenMenu) {
            // FIXED: Draw world gen menu
            DrawWorldGenMenu(screenW, screenH, &g_WorldSettings, &worldGenMenuSelection, &worldGenConfirmed);
        }
        else if (gameState == GameState::MainMenu && worldGenState == WorldGenState::Menu) {
            // Draw normal main menu (this is the fallback)
            std::vector<std::string> opts = { "New Game", "Load Game", "Settings", "Exit" };
            DrawMenu(screenW, screenH, opts, &mainMenuSelection, useController, "ECHOES OF TIME");
        }

        if (graphicsSettings.showFPS) DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
        EndDrawing();
    }

    // Cleanup
    CleanupRenderingSystems();
    CleanupZombieSystem();
    CleanupSoundSystem();
    CleanupWeatherSystem();
    CleanupDayNightSystem();
    CleanupModelSystem();
    CleanupRenderingPipeline();
    CleanupAnimationSystem();
    CleanupVehicleSystem();
    CleanupSkyboxSystem();
    CleanupEnhancedMapSystem();
    gameManager.Cleanup();
    CloseWindow();
    return 0;
}

// Helper function to draw splash screen with progress bar
void DrawSplashScreen(Texture2D splashTexture, float progress) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    ClearBackground(BLACK);
    
    // Draw splash texture
    if (splashTexture.id > 0) {
        DrawTexturePro(splashTexture,
            Rectangle{ 0, 0, (float)splashTexture.width, (float)splashTexture.height },
            Rectangle{ 0, 0, (float)screenW, (float)screenH },
            Vector2{ 0, 0 }, 0.0f, WHITE);
    }
    
    // Draw progress bar
    int barWidth = 400;
    int barHeight = 30;
    int barX = screenW / 2 - barWidth / 2;
    int barY = screenH - 120;
    
    // Background
    DrawRectangle(barX - 5, barY - 5, barWidth + 10, barHeight + 10, Color{ 40, 40, 40, 200 });
    DrawRectangle(barX, barY, barWidth, barHeight, Color{ 20, 20, 20, 200 });
    
    // Progress fill
    int fillWidth = (int)(barWidth * Clamp(progress, 0.0f, 1.0f));
    DrawRectangle(barX, barY, fillWidth, barHeight, Color{ 100, 200, 100, 255 });
    
    // Border
    DrawRectangleLines(barX, barY, barWidth, barHeight, Color{ 200, 255, 200, 255 });
    
    // Loading text above bar
    const char* loadingText = "LOADING...";
    int textWidth = MeasureText(loadingText, 32);
    DrawText(loadingText, screenW / 2 - textWidth / 2, barY - 60, 32, WHITE);
    
    // Percentage text
    DrawText(TextFormat("%.0f%%", progress * 100.0f), barX + barWidth + 20, barY + 5, 20, PIPBOY_GREEN);
}