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
    int itemId = inventory[BACKPACK_SLOTS].itemId;
    if (itemId == ITEM_NONE) return; // Don't draw anything if no item equipped

    // Fixed positioning relative to camera view
    // These create a stable "view model" that doesn't move with player velocity
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = Vector3Normalize(camera.up);

    // Base hand position - fixed relative to camera
    // Positioned in front and slightly down and to the right
    float handDistance = 0.45f;    // Distance in front of camera
    float handRightOffset = 0.22f;  // Offset to the right
    float handDownOffset = -0.25f;  // Offset downward

    Vector3 basePos = camera.position;
    basePos = Vector3Add(basePos, Vector3Scale(forward, handDistance));
    basePos = Vector3Add(basePos, Vector3Scale(right, handRightOffset));
    basePos = Vector3Add(basePos, Vector3Scale(up, handDownOffset));

    // Apply recoil offset (only for shooting, not movement)
    if (pistolRecoilPitch > 0.01f || pistolRecoilYaw > 0.01f) {
        Vector3 recoilOffset = Vector3Scale(forward, -pistolRecoilPitch * 0.002f);
        Vector3 recoilUp = Vector3Scale(up, pistolRecoilPitch * 0.003f);
        Vector3 recoilRight = Vector3Scale(right, pistolRecoilYaw * 0.002f);

        basePos = Vector3Add(basePos, recoilOffset);
        basePos = Vector3Add(basePos, recoilUp);
        basePos = Vector3Add(basePos, recoilRight);
    }

    // Render the equipped item
    if (itemId == ITEM_PISTOL) {
        // Pistol model - simple rectangular shapes
        // Main body (grip + frame)
        Vector3 gripPos = basePos;
        DrawCube(gripPos, 0.04f, 0.12f, 0.08f, DARKGRAY);

        // Slide (top part)
        Vector3 slidePos = Vector3Add(gripPos, Vector3Scale(up, 0.04f));
        slidePos = Vector3Add(slidePos, Vector3Scale(forward, 0.02f));
        DrawCube(slidePos, 0.035f, 0.04f, 0.15f, GRAY);

        // Barrel
        Vector3 barrelPos = Vector3Add(slidePos, Vector3Scale(forward, 0.1f));
        DrawCube(barrelPos, 0.02f, 0.02f, 0.06f, DARKGRAY);

        // Trigger guard
        Vector3 triggerPos = Vector3Add(gripPos, Vector3Scale(forward, 0.03f));
        DrawCube(triggerPos, 0.01f, 0.03f, 0.02f, BLACK);

    }
    else if (itemId == ITEM_FLASHLIGHT) {
        // Flashlight model - cylindrical
        Vector3 flashlightPos = basePos;

        // Rotate the flashlight to point forward
        // Draw as a series of cubes to approximate a cylinder
        Vector3 handlePos = flashlightPos;
        Vector3 bodyPos = Vector3Add(handlePos, Vector3Scale(forward, 0.08f));
        Vector3 lensPos = Vector3Add(bodyPos, Vector3Scale(forward, 0.12f));

        // Handle (grip area)
        DrawCube(handlePos, 0.03f, 0.03f, 0.08f, DARKGRAY);

        // Body (main tube)
        DrawCube(bodyPos, 0.035f, 0.035f, 0.15f, GRAY);

        // Lens/head
        DrawCube(lensPos, 0.04f, 0.04f, 0.03f, Color{ 230, 230, 200, 255 });

        // Light glow effect when on
        if (isFlashlightOn) {
            Vector3 glowPos = Vector3Add(lensPos, Vector3Scale(forward, 0.02f));
            DrawSphere(glowPos, 0.03f, Color{ 255, 255, 200, 150 });
        }
    }
    else {
        // Generic item representation (small cube)
        DrawCube(basePos, 0.05f, 0.05f, 0.05f, BEIGE);
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