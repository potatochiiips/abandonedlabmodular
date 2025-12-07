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
#define MAP_SIZE 31
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
    ControllerBindings 
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
    int ammo; // for weapons
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

// Raylib Gamepad Axis mapping for movement/look
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
    bool isAxis; // true for axis, false for button
    int inputId; // GamepadButton or GamepadAxis ID
    float threshold; // for axis: positive or negative threshold (e.g., 0.5 or -0.5)
    const char* actionName; // Display name of the binding
};

extern ControllerBinding bindings[ACTION_COUNT];

// ----------------------------------------------------------------------------------
// Externs for globals used across translation units (menu, main, player, etc.)
// ----------------------------------------------------------------------------------
// These match the definitions placed in src/main.cpp. Declaring them here allows
// other modules (menu.cpp, player.cpp, ...) to reference the shared state.
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

extern bool showMinimap;
extern bool isControllerEnabled;
extern int settingsSelection;
extern int controllerSettingsSelection;

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

// Prototype for InitNewGame implemented in src/main.cpp
void InitNewGame(Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* health, float* stamina, float* hunger, float* thirst, float* yaw, float* pitch, bool* onGround, InventorySlot* inventory, float* flashlightBattery, bool* isFlashlightOn, char map[MAP_SIZE][MAP_SIZE], float* fov);