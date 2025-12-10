#pragma once
#include "globals.h"

// Player structure for map system compatibility
struct Player {
    Vector3 position;
    float yaw;
    float pitch;
    bool insideInterior;
    int worldX;
    int worldY;
    int interiorX;
    int interiorY;
    int currentBuildingId;
    std::string currentInteriorId;

    Player() : yaw(0), pitch(0), insideInterior(false),
        worldX(0), worldY(0), interiorX(0), interiorY(0),
        currentBuildingId(-1) {
        position = Vector3{ 0, 0, 0 };
    }
};

// Core player and controller logic prototypes
void DrawPlayerHands(Camera3D camera, InventorySlot* inventory, float pistolRecoilPitch, float pistolRecoilYaw);
const char* GetGamepadButtonName(int button);
bool IsActionPressed(int actionIndex, const ControllerBinding* currentBindings);
bool IsActionDown(int actionIndex, const ControllerBinding* currentBindings);

// Prototype for initialization (implementation in main.cpp)
void InitNewGame(Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* health, float* stamina, float* hunger, float* thirst, float* yaw, float* pitch, bool* onGround, InventorySlot* inventory, float* flashlightBattery, bool* isFlashlightOn, char map[MAP_SIZE][MAP_SIZE], float* fov);

// Player movement update (used in main loop)
void UpdatePlayer(float deltaTime, Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* yaw, float* pitch, bool* onGround, float playerSpeed, float playerHeight, float gravity, float jumpForce, float* stamina, bool isNoclip, bool useController);