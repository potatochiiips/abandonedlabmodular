#include "player.h"
#include "items.h"
#include "model_manager.h"
#include <math.h>

const char* GetGamepadButtonName(int button) {
    switch (button) {
    case GAMEPAD_BUTTON_RIGHT_FACE_DOWN: return "A";
    case GAMEPAD_BUTTON_RIGHT_FACE_RIGHT: return "B";
    case GAMEPAD_BUTTON_RIGHT_FACE_UP: return "Y";
    case GAMEPAD_BUTTON_RIGHT_FACE_LEFT: return "X";
    case GAMEPAD_BUTTON_LEFT_TRIGGER_1: return "LB";
    case GAMEPAD_BUTTON_RIGHT_TRIGGER_1: return "RB";
    case GAMEPAD_BUTTON_LEFT_THUMB: return "L3";
    case GAMEPAD_BUTTON_RIGHT_THUMB: return "R3";
    case GAMEPAD_BUTTON_DPAD_UP: return "D-Up";
    case GAMEPAD_BUTTON_DPAD_RIGHT: return "D-Right";
    case GAMEPAD_BUTTON_DPAD_DOWN: return "D-Down";
    case GAMEPAD_BUTTON_DPAD_LEFT: return "D-Left";
    default: return TextFormat("Btn %02d", button);
    }
}

bool IsActionPressed(int actionIndex, const ControllerBinding* currentBindings) {
    if (!IsGamepadAvailable(0)) return false;
    if (actionIndex < 0 || actionIndex >= ACTION_COUNT) return false;
    const ControllerBinding& binding = currentBindings[actionIndex];

    if (binding.isAxis) {
        return false;
    }
    else {
        return IsGamepadButtonPressed(0, binding.inputId);
    }
}

bool IsActionDown(int actionIndex, const ControllerBinding* currentBindings) {
    if (!IsGamepadAvailable(0)) return false;
    if (actionIndex < 0 || actionIndex >= ACTION_COUNT) return false;
    const ControllerBinding& binding = currentBindings[actionIndex];
    if (binding.isAxis) {
        float axisValue = GetGamepadAxisMovement(0, binding.inputId);
        if (binding.threshold > 0.0f) {
            return axisValue >= binding.threshold;
        }
        else if (binding.threshold < 0.0f) {
            return axisValue <= binding.threshold;
        }
    }
    else {
        return IsGamepadButtonDown(0, binding.inputId);
    }
    return false;
}

// Get ModelID from item ID
ModelID GetModelIDFromItem(int itemId) {
    switch (itemId) {
        case ITEM_PISTOL: return MODEL_PISTOL;
        case ITEM_M16: return MODEL_M16;
        case ITEM_FLASHLIGHT: return MODEL_FLASHLIGHT;
        case ITEM_WATER_BOTTLE: return MODEL_WATER_BOTTLE;
        case ITEM_LAB_KEY: return MODEL_LAB_KEY;
        case ITEM_WOOD: return MODEL_WOOD;
        case ITEM_STONE: return MODEL_STONE;
        case ITEM_POTATO_CHIPS: return MODEL_POTATO_CHIPS;
        case ITEM_MAG: return MODEL_MAGAZINE;
        case ITEM_M16_MAG: return MODEL_M16_MAGAZINE;
        default: return MODEL_PISTOL; // Fallback
    }
}

// Enhanced player hands with glTF model rendering
void DrawPlayerHands(Camera3D camera, InventorySlot* inventory, float pistolRecoilPitch, float pistolRecoilYaw) {
    int itemId = inventory[BACKPACK_SLOTS].itemId;
    if (itemId == ITEM_NONE) return;

    // External variable
    extern bool isFlashlightOn;

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = Vector3Normalize(camera.up);

    float handDistance = 0.45f;
    float handRightOffset = 0.22f;
    float handDownOffset = -0.25f;

    Vector3 basePos = camera.position;
    basePos = Vector3Add(basePos, Vector3Scale(forward, handDistance));
    basePos = Vector3Add(basePos, Vector3Scale(right, handRightOffset));
    basePos = Vector3Add(basePos, Vector3Scale(up, handDownOffset));

    // Apply recoil with smooth decay
    if (pistolRecoilPitch > 0.01f || pistolRecoilYaw > 0.01f) {
        Vector3 recoilOffset = Vector3Scale(forward, -pistolRecoilPitch * 0.002f);
        Vector3 recoilUp = Vector3Scale(up, pistolRecoilPitch * 0.003f);
        Vector3 recoilRight = Vector3Scale(right, pistolRecoilYaw * 0.002f);

        basePos = Vector3Add(basePos, recoilOffset);
        basePos = Vector3Add(basePos, recoilUp);
        basePos = Vector3Add(basePos, recoilRight);
    }

    // Draw item using model manager
    if (g_ModelManager) {
        ModelID modelId = GetModelIDFromItem(itemId);
        g_ModelManager->DrawModel(modelId, basePos, forward, right, up, WHITE);
        
        // Add glow effect for flashlight when on
        if (itemId == ITEM_FLASHLIGHT && isFlashlightOn) {
            Vector3 glowPos = Vector3Add(basePos, Vector3Scale(forward, 0.08f));
            DrawSphere(glowPos, 0.04f, Color{ 255, 255, 220, 100 });
            DrawSphere(glowPos, 0.03f, Color{ 255, 255, 200, 150 });
            DrawSphere(glowPos, 0.02f, Color{ 255, 255, 180, 200 });
        }
    } else {
        // Fallback to simple cube if model manager not available
        DrawCube(basePos, 0.05f, 0.05f, 0.05f, GRAY);
    }
}

void UpdatePlayer(float deltaTime, Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* yaw, float* pitch, bool* onGround, float playerSpeed, float playerHeight, float gravity, float jumpForce, float* stamina, bool isNoclip, bool useController) {
    extern ControllerBinding* bindings;

    Vector2 mouseDelta = GetMouseDelta();
    if (useController) {
        float moveAxisX = GetGamepadAxisMovement(0, GAMEPAD_CAMERA_MOVE_AXIS_X);
        float moveAxisY = GetGamepadAxisMovement(0, GAMEPAD_CAMERA_MOVE_AXIS_Y);
        mouseDelta.x = moveAxisX * 5.0f;
        mouseDelta.y = moveAxisY * 5.0f;
    }

    float mouseSensitivity = 0.2f;
    *yaw += mouseDelta.x * mouseSensitivity;
    *pitch -= mouseDelta.y * mouseSensitivity;
    *pitch = Clamp(*pitch, -89.0f, 89.0f);

    Vector3 target = {
        cosf(DEG2RAD * *yaw) * cosf(DEG2RAD * *pitch),
        sinf(DEG2RAD * *pitch),
        sinf(DEG2RAD * *yaw) * cosf(DEG2RAD * *pitch)
    };
    camera->target = Vector3Add(*playerPosition, target);

    Vector3 tempFlatForward = { target.x, 0.0f, target.z };
    Vector3 flatForward = Vector3Normalize(tempFlatForward);

    Vector3 right = Vector3Normalize(Vector3CrossProduct(flatForward, camera->up));
    Vector3 movement = { 0 };

    bool isSprinting = (IsKeyDown(KEY_LEFT_SHIFT) || (useController && IsActionDown(ACTION_SPRINT, bindings))) && *stamina > 0.0f;
    float currentSpeed = playerSpeed * (isSprinting ? 2.0f : 1.0f);

    if (useController) {
        float moveX = GetGamepadAxisMovement(0, GAMEPAD_PLAYER_MOVE_AXIS_X);
        float moveY = GetGamepadAxisMovement(0, GAMEPAD_PLAYER_MOVE_AXIS_Y);
        if (fabs(moveY) > 0.1f) movement = Vector3Add(movement, Vector3Scale(flatForward, -moveY * currentSpeed));
        if (fabs(moveX) > 0.1f) movement = Vector3Add(movement, Vector3Scale(right, moveX * currentSpeed));
    }

    if (Vector3LengthSqr(movement) == 0.0f || !useController) {
        if (IsKeyDown(KEY_W)) movement = Vector3Add(movement, Vector3Scale(flatForward, currentSpeed));
        if (IsKeyDown(KEY_S)) movement = Vector3Add(movement, Vector3Scale(flatForward, -currentSpeed));
        if (IsKeyDown(KEY_A)) movement = Vector3Add(movement, Vector3Scale(right, -currentSpeed));
        if (IsKeyDown(KEY_D)) movement = Vector3Add(movement, Vector3Scale(right, currentSpeed));
    }

    if (Vector3LengthSqr(movement) > 0.0f) {
        movement = Vector3Scale(Vector3Normalize(movement), currentSpeed);
        *playerPosition = Vector3Add(*playerPosition, movement);
        if (isSprinting) *stamina -= 0.3f * deltaTime * 60.0f;
    }

    if (!isNoclip) {
        const float JUMP_STAMINA_COST = 5.0f;
        bool jumpPressed = IsKeyPressed(KEY_SPACE) || (useController && IsActionPressed(ACTION_JUMP, bindings));
        if (jumpPressed && *onGround && *stamina >= JUMP_STAMINA_COST) {
            playerVelocity->y = jumpForce;
            *stamina -= JUMP_STAMINA_COST;
            *onGround = false;
        }
        playerVelocity->y -= gravity;
        playerPosition->y += playerVelocity->y;
        if (playerPosition->y <= playerHeight) {
            playerPosition->y = playerHeight;
            playerVelocity->y = 0.0f;
            *onGround = true;
        }
    }
    else {
        if (IsKeyDown(KEY_SPACE)) playerPosition->y += currentSpeed;
        if (IsKeyDown(KEY_LEFT_CONTROL)) playerPosition->y -= currentSpeed;
    }
    camera->position = *playerPosition;
}