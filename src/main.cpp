// Complete updated main.cpp with fixes for build errors

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

// Graphics settings
GraphicsSettings graphicsSettings = {
    2,      // resolutionIndex (1280x720)
    true,   // vsync
    false,  // msaa
    4,      // msaaSamples
    60,     // targetFPS
    1.0f,   // renderScale
    false,  // showFPS
    true,   // enableLOD
    true,   // enableFrustumCulling
    1000,   // maxDrawCalls
    UPSCALING_NONE,        // upscalingMode
    UPSCALE_QUALITY_QUALITY // upscalingQuality
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
    inventory[4] = { ITEM_M16, 1, 25 };
    inventory[5] = { ITEM_M16_MAG, 3, 0 };

    GenerateMap(map);

    currentFloor = -1;
    currentBuildingIndex = -1;

    // Reset weapon state
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

    // Initialize quests
    int quest1 = g_QuestManager.AddQuest(
        "Welcome to the Lab",
        "Get familiar with your surroundings and find basic supplies",
        100
    );
    g_QuestManager.AddObjective(quest1, QUEST_OBJ_COLLECT, ITEM_WATER_BOTTLE, 2,
        "Collect 2 water bottles");
    g_QuestManager.AddObjective(quest1, QUEST_OBJ_COLLECT, ITEM_FLASHLIGHT, 1,
        "Find a flashlight");

    int quest2 = g_QuestManager.AddQuest(
        "Armed and Ready",
        "Equip yourself with weapons and ammunition",
        150
    );
    g_QuestManager.AddObjective(quest2, QUEST_OBJ_COLLECT, ITEM_PISTOL, 1,
        "Find a pistol");
    g_QuestManager.AddObjective(quest2, QUEST_OBJ_COLLECT, ITEM_MAG, 3,
        "Collect 3 magazines");
}


int main() {
    LoadGraphicsSettings(&graphicsSettings);
    const Resolution& initialRes = AVAILABLE_RESOLUTIONS[graphicsSettings.resolutionIndex];

    // Set fullscreen flag BEFORE InitWindow
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_FULLSCREEN_MODE);
    if (graphicsSettings.msaa) {
        if (graphicsSettings.msaaSamples == 2) SetConfigFlags(FLAG_MSAA_4X_HINT);
        else if (graphicsSettings.msaaSamples == 4) SetConfigFlags(FLAG_MSAA_4X_HINT);
    }
    
    // Initialize window in fullscreen
    int monitorWidth = GetMonitorWidth(0);
    int monitorHeight = GetMonitorHeight(0);
    InitWindow(monitorWidth, monitorHeight, "Echoes of Time");
    SetExitKey(KEY_NULL);
    BeginDrawing();

    // Load splash screen FIRST (before other assets)
    Texture2D splashTexture = LoadTexture("assets/splash.png");

    // Draw splash screen while loading
    BeginDrawing();
    ClearBackground(BLACK);
    if (splashTexture.id > 0) {
        // Center splash on screen
        int splashX = (monitorWidth - splashTexture.width) / 2;
        int splashY = (monitorHeight - splashTexture.height) / 2;
        DrawTexture(splashTexture, splashX, splashY, WHITE);
    }
    DrawText("LOADING...", monitorWidth / 2 - 60, monitorHeight - 50, 20, WHITE);
    EndDrawing();

    InitializeUpscalingSystem(initialRes.width, initialRes.height);
    ApplyGraphicsSettings(graphicsSettings);

    // Initialize all systems (this takes time - splash is visible during this)
    InitializeRenderingSystems();
    InitializeModelSystem();

    // Unload splash after everything loaded
    if (splashTexture.id > 0) {
        UnloadTexture(splashTexture);
    }

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
            else if (gameState == GameState::AudioSettings) {
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

                // Door interaction - FIXED: Check nearDoor before using it
                if (IsKeyPressed(KEY_E) && !inventoryOpen && !isCraftingOpen && !isMapOpen) {
                    Door* nearDoor = GetNearestDoor(playerPosition, 2.5f);

                    if (nearDoor) {
                        if (g_MapPlayer.insideInterior) {
                            // Inside a building - check if this is the exit door
                            if (nearDoor->isInteriorDoor && nearDoor->buildingId == g_MapPlayer.currentBuildingId) {
                                // Exit to exterior
                                if (ExitInterior(g_MapData, g_MapPlayer)) {
                                    playerPosition = Vector3{
                                        (float)g_MapPlayer.worldX,
                                        playerHeight,
                                        (float)g_MapPlayer.worldY
                                    };
                                    camera.position = playerPosition;
                                    TraceLog(LOG_INFO, "Exited to exterior");
                                }
                            }
                        }
                        else {
                            // Outside - check if this is an entrance door
                            if (!nearDoor->isInteriorDoor) {
                                // Try to enter the building
                                if (EnterInterior(g_MapData, g_MapPlayer, nearDoor->buildingId)) {
                                    // Teleport player to interior spawn position
                                    playerPosition = Vector3{
                                        (float)g_MapPlayer.interiorX,
                                        playerHeight,
                                        (float)g_MapPlayer.interiorY
                                    };
                                    camera.position = playerPosition;
                                    TraceLog(LOG_INFO, "Entered building interior");
                                }
                            }
                        }
                    }
                }

                UpdateDoors(deltaTime);

                // Flashlight toggle
                bool flashlightPressed = useController ? IsActionPressed(ACTION_FLASHLIGHT, bindings) : IsKeyPressed(KEY_F);
                if (flashlightPressed) isFlashlightOn = !isFlashlightOn;

                if (isFlashlightOn && flashlightBattery > 0.0f) {
                    flashlightBattery -= 5.0f * deltaTime;
                }
                else if (flashlightBattery <= 0.0f) {
                    isFlashlightOn = false;
                    flashlightBattery = 0.0f;
                }

                // Use item
                bool useItemPressed = useController ? IsActionPressed(ACTION_USE_ITEM, bindings) : IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);
                if (useItemPressed) {
                    UseEquippedItem(inventory, &health, &stamina, &hunger, &thirst);
                }

                // ADS toggle (right mouse hold for pistol/rifle)
                int equippedWeapon = inventory[BACKPACK_SLOTS].itemId;
                if (equippedWeapon == ITEM_PISTOL || equippedWeapon == ITEM_M16) {
                    bool adsPressed = IsMouseButtonDown(MOUSE_RIGHT_BUTTON) ||
                        (useController && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2));
                    g_CurrentWeaponState.isADS = adsPressed;
                }

                // Reload weapon
                bool reloadPressed = IsKeyPressed(KEY_R) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT));
                if (reloadPressed && !isReloading) {
                    if (ReloadWeapon(inventory)) {
                        isReloading = true;
                        WeaponStats* stats = g_WeaponSystem.GetWeaponStats(inventory[BACKPACK_SLOTS].itemId);
                        reloadTimer = stats ? stats->reloadTime : 1.5f;
                        g_CurrentWeaponState.animState = ANIM_RELOAD;
                        g_CurrentWeaponState.animTimer = reloadTimer;
                    }
                }

                // Update reload timer
                if (isReloading) {
                    reloadTimer -= deltaTime;
                    if (reloadTimer <= 0.0f) {
                        isReloading = false;
                        reloadTimer = 0.0f;
                    }
                }

                // Weapon shooting with weapon system
                bool shootPressed = useController ? IsActionPressed(ACTION_SHOOT, bindings) : IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                if (shootPressed && shotTimer <= 0.0f && !isReloading) {
                    int weaponId = inventory[BACKPACK_SLOTS].itemId;
                    WeaponStats* stats = g_WeaponSystem.GetWeaponStats(weaponId);

                    if (stats && inventory[BACKPACK_SLOTS].ammo > 0) {
                        shotTimer = stats->fireRate;
                        inventory[BACKPACK_SLOTS].ammo--;

                        // Apply recoil
                        pistolRecoilPitch = stats->recoilPitch;
                        pistolRecoilYaw = stats->recoilYaw;

                        // Visual recoil on weapon
                        g_CurrentWeaponState.recoilOffset.y = -0.02f;
                        g_CurrentWeaponState.recoilOffset.z = -0.05f;

                        g_CurrentWeaponState.animState = ANIM_SHOOT;
                        g_CurrentWeaponState.animTimer = 0.2f;

                        TraceLog(LOG_INFO, TextFormat("%s fired! Damage: %.0f",
                            GetItemName(weaponId), stats->damage));
                    }
                }

                // Stat draining
                float drainRate = 1.0f * deltaTime;
                hunger = fmaxf(0.0f, hunger - drainRate);
                thirst = fmaxf(0.0f, thirst - drainRate * 1.5f);
                stamina = fminf(100.0f, stamina + drainRate * 2.0f);

                if (health <= 0.0f) gameState = GameState::GameOver;

                // Update weapon system
                g_WeaponSystem.UpdateWeapon(g_CurrentWeaponState, deltaTime);

                // Recoil decay
                pistolRecoilPitch = fmaxf(0.0f, pistolRecoilPitch - RECOIL_DECAY_RATE * deltaTime * 60.0f);
                pistolRecoilYaw = fmaxf(0.0f, pistolRecoilYaw - RECOIL_DECAY_RATE * deltaTime * 60.0f);

                shotTimer = fmaxf(0.0f, shotTimer - deltaTime);
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
            // Begin upscaled rendering
            if (g_UpscalingManager && graphicsSettings.upscalingMode != UPSCALING_NONE) {
                g_UpscalingManager->BeginUpscaledRender();
            }
            // Update shader lighting uniforms
            if (g_ShaderManager) {
                Vector3 sunPos = { MAP_SIZE / 2.0f, 100.0f, MAP_SIZE / 2.0f };
                Vector3 flashDir = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
                float flashIntensity = (flashlightBattery / 100.0f) * 5.0f;

                g_ShaderManager->UpdateLighting(camera, sunPos, isFlashlightOn,
                    camera.position, flashDir, flashIntensity);
            }

            BeginMode3D(camera);

            // Draw grid ONLY when outside
            if (!g_MapPlayer.insideInterior) {
                if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
                    BeginShaderMode(g_ShaderManager->GetLightingShader());
                }
                DrawGrid(MAP_SIZE, GRID_SIZE);
                if (g_ShaderManager && g_ShaderManager->GetLightingShader().id > 0) {
                    EndShaderMode();
                }
            }
            DrawMapGeometry(map);

            // Draw waypoints in 3D
            g_WaypointManager.DrawIn3D(playerPosition, 100.0f);

            // Draw weapon using weapon system
            int equippedWeapon = inventory[BACKPACK_SLOTS].itemId;
            if (equippedWeapon != ITEM_NONE) {
                Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
                Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
                Vector3 up = Vector3Normalize(camera.up);

                Vector3 weaponPos = g_WeaponSystem.CalculateWeaponPosition(camera, g_CurrentWeaponState, equippedWeapon == ITEM_M16);

                if (equippedWeapon == ITEM_PISTOL) {
                    DrawEnhancedPistol(weaponPos, forward, right, up, g_CurrentWeaponState);
                    DrawLeftHandOnWeapon(weaponPos, forward, right, up, false, g_CurrentWeaponState.adsProgress);
                }
                else if (equippedWeapon == ITEM_M16) {
                    DrawM16Rifle(weaponPos, forward, right, up, g_CurrentWeaponState);
                    DrawLeftHandOnWeapon(weaponPos, forward, right, up, true, g_CurrentWeaponState.adsProgress);
                }
                else if (equippedWeapon == ITEM_FLASHLIGHT) {
                    // Still use the old DrawPlayerHands for flashlight
                    DrawPlayerHands(camera, inventory, pistolRecoilPitch, pistolRecoilYaw);
                }
                else {
                    // Generic items use old system
                    DrawPlayerHands(camera, inventory, pistolRecoilPitch, pistolRecoilYaw);
                }
            }
            else {
                // No weapon - draw idle hands
                DrawIdleHands(camera, (float)GetTime());
            }

            EndMode3D();
            // End upscaled rendering
            if (g_UpscalingManager && graphicsSettings.upscalingMode != UPSCALING_NONE) {
                g_UpscalingManager->EndUpscaledRender(screenW, screenH);
            }
            // Check for nearby door and show prompt
            Door* nearDoor = GetNearestDoor(playerPosition, 2.5f);
            if (nearDoor && !isAnyMenuOpen) {
                const char* doorText = g_MapPlayer.insideInterior ? "Press E to Exit" : "Press E to Enter";
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

                // Draw waypoints on minimap
                g_WaypointManager.DrawOnMinimap(screenW - 160, 10, 150, 150, playerPosition, 15, 10.0f);
            }

            if (gameState == GameState::Gameplay) {
                DrawHUD(screenW, screenH, health, stamina, hunger, thirst, fov, flashlightBattery, isFlashlightOn, inventory);

                // Draw quest tracker
                g_QuestManager.DrawQuestTrackerCompact(screenW, screenH);

                // Draw reload indicator
                if (isReloading) {
                    int barW = 200;
                    int barH = 20;
                    int barX = screenW / 2 - barW / 2;
                    int barY = screenH / 2 + 100;

                    WeaponStats* stats = g_WeaponSystem.GetWeaponStats(inventory[BACKPACK_SLOTS].itemId);
                    float reloadProgress = stats ? 1.0f - (reloadTimer / stats->reloadTime) : 0.0f;

                    DrawRectangle(barX, barY, barW, barH, PIPBOY_DARK);
                    DrawRectangle(barX, barY, (int)(barW * reloadProgress), barH, PIPBOY_GREEN);
                    DrawRectangleLines(barX, barY, barW, barH, PIPBOY_GREEN);
                    DrawText("RELOADING...", barX + 50, barY + 3, 16, PIPBOY_GREEN);
                }
            }

            if (isMapOpen) DrawMapMenu(screenW, screenH, map, playerPosition, yaw);
            if (isCraftingOpen) DrawCraftingMenu(screenW, screenH, inventory, &selectedRecipeIndex, useController);
            if (inventoryOpen) {
                // Use tabbed interface for inventory
                int menuW = (int)(screenW * 0.9f);
                int menuH = (int)(screenH * 0.9f);
                int menuX = (screenW - menuW) / 2;
                int menuY = (screenH - menuH) / 2;

                // Draw background
                DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
                DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);

                // Draw tab bar
                g_TabManager.DrawTabBar(screenW, screenH, menuX, menuY, menuW);

                // Handle tab input
                g_TabManager.HandleTabInput(useController);

                // Calculate content area (below tabs)
                int contentY = menuY + 50;
                int contentH = menuH - 50;

                // Draw content based on active tab
                switch (g_TabManager.GetCurrentTab()) {
                case TAB_INVENTORY:
                    DrawInventory(screenW, screenH, inventory, &selectedHandSlot, &selectedInvSlot, useController);
                    break;
                case TAB_CRAFTING:
                    DrawCraftingMenu(screenW, screenH, inventory, &selectedRecipeIndex, useController);
                    break;
                case TAB_MAP:
                    DrawMapMenu(screenW, screenH, map, playerPosition, yaw);
                    break;
                case TAB_SKILLS:
                    DrawSkillsScreen(screenW, screenH, menuX + 10, contentY, menuW - 20, contentH - 10, useController);
                    break;
                case TAB_QUESTS:
                    DrawQuestsScreen(screenW, screenH, menuX + 10, contentY, menuW - 20, contentH - 10, useController);
                    break;
                }
            }
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
        else if (gameState == GameState::AudioSettings) {
            GameState tempState = GameState::Settings;
            DrawAudioSettingsMenu(screenW, screenH, &audioSettingsSelection, &tempState);
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
    CleanupModelSystem();  
	//close sound system      
    CleanupRenderingSystems();

    CloseWindow();
    return 0;
}