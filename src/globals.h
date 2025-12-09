#pragma once

#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <cstdio>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <cstring>
#include <map>
#include <sstream>
#include <cmath>
#include <fstream>

// ----------------------------------------------------------------------------------
// COMPATIBILITY FIXES FOR OLDER RAYLIB VERSIONS (Missing constants/functions)
// ----------------------------------------------------------------------------------
#ifndef GAMEPAD_BUTTON_DPAD_UP
#define GAMEPAD_BUTTON_DPAD_UP 12
#define GAMEPAD_BUTTON_DPAD_RIGHT 13
#define GAMEPAD_BUTTON_DPAD_DOWN 14
#define GAMEPAD_BUTTON_DPAD_LEFT 15
#define GAMEPAD_BUTTON_START 9
#endif 
#ifndef GAMEPAD_BUTTON_COUNT
#define GAMEPAD_BUTTON_COUNT 16
#endif
#ifndef GAMEPAD_AXIS_COUNT
#define GAMEPAD_AXIS_COUNT 6
#endif

// ----------------------------------------------------------------------------------
// DATA STRUCTURES & DEFINITIONS
// ----------------------------------------------------------------------------------

// --- MAP DEFINITIONS ---
#define WORLD_SIZE 128
#define MAP_SIZE WORLD_SIZE
#define TILE_WALL 0
#define TILE_FLOOR 1
#define TILE_DOOR 2
#define GRID_SIZE 1.0f
// -----------------------

enum class GameState {
    MainMenu,
    Gameplay,
    GameOver,
    Settings,
    Paused,
    LoadMenu,
    Console,
    ControllerBindings,
    GraphicsSettings
};

// --- ITEM DEFINITIONS ---
#define ITEM_NONE 0
#define ITEM_WATER_BOTTLE 1
#define ITEM_LAB_KEY 2
#define ITEM_FLASHLIGHT 3
#define ITEM_WOOD 4
#define ITEM_STONE 5
#define ITEM_POTATO_CHIPS 6
#define ITEM_PISTOL 7
#define ITEM_MAG 8
#define ITEM_M16 9
#define ITEM_M16_MAG 10
// ------------------------

// --- FALLOUT 4 (PIP-BOY) STYLE COLORS ---
#define PIPBOY_GREEN Color{65, 255, 65, 255}
#define PIPBOY_DARK Color{10, 30, 10, 240}
#define PIPBOY_SELECTED Color{40, 70, 40, 255}
#define PIPBOY_DIM Color{30, 150, 30, 255}
// ----------------------------------------

struct InventorySlot {
    int itemId;
    int quantity;
    int ammo;
};

// Constants
const int BACKPACK_SLOTS = 27;
const int HAND_SLOTS = 2;
const int TOTAL_INVENTORY_SLOTS = BACKPACK_SLOTS + HAND_SLOTS;
const int MAX_SAVE_SLOTS = 3;
#define SAVE_FILE_NAME_FORMAT "savegame_%d.sav"

// Console
#define MAX_COMMAND_LENGTH 100

// ==================================================================================
// UPSCALING ENUMS (Must be defined before GraphicsSettings)
// ==================================================================================

enum UpscalingMode {
    UPSCALING_NONE = 0,
    UPSCALING_FSR,
    UPSCALING_DLSS,
    UPSCALING_MODE_COUNT
};

enum UpscalingQuality {
    UPSCALE_QUALITY_PERFORMANCE = 0,
    UPSCALE_QUALITY_BALANCED,
    UPSCALE_QUALITY_QUALITY,
    UPSCALE_QUALITY_ULTRA,
    UPSCALE_QUALITY_COUNT
};

// ==================================================================================
// GRAPHICS SETTINGS STRUCTURES
// ==================================================================================

struct Resolution {
    int width;
    int height;
    const char* label;
};

struct GraphicsSettings {
    int resolutionIndex;
    bool vsync;
    bool msaa;
    int msaaSamples;
    int targetFPS;
    float renderScale;
    bool showFPS;
    bool enableLOD;
    bool enableFrustumCulling;
    int maxDrawCalls;
    UpscalingMode upscalingMode;      // Added
    UpscalingQuality upscalingQuality; // Added
};

const Resolution AVAILABLE_RESOLUTIONS[] = {
    {800, 600, "800x600"},
    {1024, 768, "1024x768"},
    {1280, 720, "1280x720 (HD)"},
    {1366, 768, "1366x768"},
    {1600, 900, "1600x900"},
    {1920, 1080, "1920x1080 (Full HD)"},
    {2560, 1440, "2560x1440 (2K)"},
    {3840, 2160, "3840x2160 (4K)"}
};
const int RESOLUTION_COUNT = 8;

// ==================================================================================
// CRAFTING STRUCTURES
// ==================================================================================

struct CraftingIngredient {
    int itemId;
    int quantity;
};

struct CraftingRecipe {
    std::vector<CraftingIngredient> ingredients;
    int resultId;
    int resultQuantity;
    const char* recipeName;
};

// ==================================================================================
// CONTROLLER BINDING STRUCTURES AND HELPERS 
// ==================================================================================

#define GAMEPAD_PLAYER_MOVE_AXIS_X GAMEPAD_AXIS_LEFT_X
#define GAMEPAD_PLAYER_MOVE_AXIS_Y GAMEPAD_AXIS_LEFT_Y
#define GAMEPAD_CAMERA_MOVE_AXIS_X GAMEPAD_AXIS_RIGHT_X
#define GAMEPAD_CAMERA_MOVE_AXIS_Y GAMEPAD_AXIS_RIGHT_Y

enum PlayerAction {
    ACTION_JUMP,
    ACTION_SPRINT,
    ACTION_INVENTORY,
    ACTION_CRAFTING,
    ACTION_MAP,
    ACTION_FLASHLIGHT,
    ACTION_USE_ITEM,
    ACTION_SHOOT,
    ACTION_COUNT
};

struct ControllerBinding {
    bool isAxis;
    int inputId;
    float threshold;
    const char* actionName;
};

extern ControllerBinding bindings[ACTION_COUNT];

// ----------------------------------------------------------------------------------
// Externs for globals used across translation units
// ----------------------------------------------------------------------------------
extern Camera3D camera;
extern Vector3 playerPosition;
extern Vector3 playerVelocity;
extern float playerSpeed;
extern float jumpForce;
extern bool onGround;
extern float gravity;
extern float playerHeight;

extern float health;
extern float stamina;
extern float hunger;
extern float thirst;
extern float fov;

extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
extern bool inventoryOpen;
extern bool isCraftingOpen;
extern bool isMapOpen;
extern bool isNoclip;

extern float flashlightBattery;
extern bool isFlashlightOn;

extern char map[MAP_SIZE][MAP_SIZE];

extern int selectedHandSlot;
extern int selectedInvSlot;
extern int selectedRecipeIndex;

extern float pistolRecoilPitch;
extern float pistolRecoilYaw;
extern const float RECOIL_DECAY_RATE;
extern float shotTimer;
extern const float SHOT_COOLDOWN;
extern bool isAimingDownSights;
extern bool isReloading;
extern float reloadTimer;
extern float adsTransitionProgress;

extern bool showMinimap;
extern bool isControllerEnabled;
extern bool isFullscreen;
extern int settingsSelection;
extern int controllerSettingsSelection;
extern int graphicsSettingsSelection;

extern bool isBindingMode;
extern int activeBindingIndex;
extern int saveSlotSelection;
extern int mainMenuSelection;
extern int pauseMenuSelection;

extern float yaw;
extern float pitch;
extern bool cursorHidden;

extern GameState gameState;
extern GameState stateBeforeSettings;

extern GraphicsSettings graphicsSettings;


// Prototype for InitNewGame
void InitNewGame(Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* health, float* stamina, float* hunger, float* thirst, float* yaw, float* pitch, bool* onGround, InventorySlot* inventory, float* flashlightBattery, bool* isFlashlightOn, char map[MAP_SIZE][MAP_SIZE], float* fov);

// Graphics functions
void ApplyGraphicsSettings(const GraphicsSettings& settings);
void SaveGraphicsSettings(const GraphicsSettings& settings);
void LoadGraphicsSettings(GraphicsSettings* settings);

// Include other headers after forward declarations
#include "waypoints.h"
#include "quest_system.h"
#include "weapons.h"
#include "ui_tabs.h"