#include "player.h"
#include "items.h" // For item name lookup

const char* GetGamepadButtonName(int button) {
    switch(button) {
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
    // [Implementation of IsActionPressed]
    if (!IsGamepadAvailable(0)) return false;
    if (actionIndex < 0 || actionIndex >= ACTION_COUNT) return false;
    const ControllerBinding& binding = currentBindings[actionIndex];
    
    if (binding.isAxis) {
        // Axis press detection is more complex and typically not done with IsGamepadButtonPressed
        return false; 
    } else {
        return IsGamepadButtonPressed(0, binding.inputId);
    }
}

bool IsActionDown(int actionIndex, const ControllerBinding* currentBindings) {
    // [Implementation of IsActionDown]
    if (!IsGamepadAvailable(0)) return false;
    if (actionIndex < 0 || actionIndex >= ACTION_COUNT) return false;
    const ControllerBinding& binding = currentBindings[actionIndex];
    if (binding.isAxis) {
        float axisValue = GetGamepadAxisMovement(0, binding.inputId);
        if (binding.threshold > 0.0f) {
            return axisValue >= binding.threshold;
        } else if (binding.threshold < 0.0f) {
            return axisValue <= binding.threshold;
        }
    } else {
        return IsGamepadButtonDown(0, binding.inputId);
    }
    return false;
}

void DrawPlayerHands(Camera3D camera, InventorySlot* inventory, float pistolRecoilPitch, float pistolRecoilYaw) {
    // [Implementation of DrawPlayerHands]
    int itemId = inventory[BACKPACK_SLOTS].itemId;
    float handDistance = 0.5f;
    float handHeight = -0.3f;
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = camera.up;
    
    // Base position for the weapon/item
    Vector3 basePos = Vector3Add(camera.position, Vector3Scale(forward, handDistance));
    basePos = Vector3Add(basePos, Vector3Scale(up, handHeight));
    basePos = Vector3Add(basePos, Vector3Scale(right, 0.2f)); 
    
    // Apply recoil offset
    Vector3 recoilOffset = Vector3Scale(forward, -0.01f);
    Vector3 pitchOffset = Vector3Scale(up, pistolRecoilPitch * 0.001f);
    Vector3 yawOffset = Vector3Scale(right, pistolRecoilYaw * 0.001f);
    basePos = Vector3Add(basePos, recoilOffset);
    basePos = Vector3Add(basePos, pitchOffset);
    basePos = Vector3Add(basePos, yawOffset);

    if (itemId == ITEM_PISTOL) {
        // Simplified rendering for a pistol
        DrawCube(basePos, 0.15f, 0.1f, 0.3f, GRAY);
        DrawCube(Vector3Add(basePos, Vector3Scale(forward, 0.1f)), 0.08f, 0.08f, 0.1f, DARKGRAY); // Barrel
    } else if (itemId == ITEM_FLASHLIGHT) {
        DrawCylinder(basePos, 0.02f, 0.02f, 0.4f, 16, GRAY);
        DrawCube(Vector3Add(basePos, Vector3Scale(forward, 0.2f)), 0.05f, 0.05f, 0.01f, WHITE); // Lens
    }
}

void UpdatePlayer(float deltaTime, Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* yaw, float* pitch, bool* onGround, float playerSpeed, float playerHeight, float gravity, float jumpForce, float* stamina, bool isNoclip, bool useController) {
    // Mouse/Controller Look Input
    Vector2 mouseDelta = GetMouseDelta();
    if (useController) {
        mouseDelta.x = GetGamepadAxisMovement(0, GAMEPAD_CAMERA_MOVE_AXIS_X) * 5.0f;
        mouseDelta.y = GetGamepadAxisMovement(0, GAMEPAD_CAMERA_MOVE_AXIS_Y) * 5.0f;
    }
    
    float mouseSensitivity = 0.2f;
    *yaw += mouseDelta.x * mouseSensitivity;
    *pitch -= mouseDelta.y * mouseSensitivity;
    *pitch = Clamp(*pitch, -89.0f, 89.0f);

    // Update camera target
    Vector3 target = { 
        cosf(DEG2RAD * *yaw) * cosf(DEG2RAD * *pitch),
        sinf(DEG2RAD * *pitch),
        sinf(DEG2RAD * *yaw) * cosf(DEG2RAD * *pitch) 
    };
    camera->target = Vector3Add(*playerPosition, target);
    camera->fovy = camera->fovy; // Already set via save/load

    // Player Movement
    Vector3 flatForward = Vector3Normalize({target.x, 0.0f, target.z});
    Vector3 right = Vector3Normalize(Vector3CrossProduct(flatForward, camera->up));
    Vector3 movement = {0};
    
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
        if (isSprinting) *stamina -= 0.3f * deltaTime * 60.0f; // Simplified cost
    }

    // Gravity/Jumping/Noclip
    if (!isNoclip) {
        const float JUMP_STAMINA_COST = 5.0f; // Define constant from main.cpp
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
    } else {
        if (IsKeyDown(KEY_SPACE)) playerPosition->y += currentSpeed;
        if (IsKeyDown(KEY_LEFT_CONTROL)) playerPosition->y -= currentSpeed;
    }
    camera->position = *playerPosition;
}