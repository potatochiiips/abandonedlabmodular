#include "player.h"
#include "items.h"

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

// Enhanced realistic weapon/item rendering
void DrawPlayerHands(Camera3D camera, InventorySlot* inventory, float pistolRecoilPitch, float pistolRecoilYaw) {
    int itemId = inventory[BACKPACK_SLOTS].itemId;
    if (itemId == ITEM_NONE) return;

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

    // Recoil with smooth decay
    if (pistolRecoilPitch > 0.01f || pistolRecoilYaw > 0.01f) {
        Vector3 recoilOffset = Vector3Scale(forward, -pistolRecoilPitch * 0.002f);
        Vector3 recoilUp = Vector3Scale(up, pistolRecoilPitch * 0.003f);
        Vector3 recoilRight = Vector3Scale(right, pistolRecoilYaw * 0.002f);

        basePos = Vector3Add(basePos, recoilOffset);
        basePos = Vector3Add(basePos, recoilUp);
        basePos = Vector3Add(basePos, recoilRight);
    }

    if (itemId == ITEM_PISTOL) {
        // --- REALISTIC PISTOL MODEL ---

        // Grip (handle)
        Vector3 gripPos = basePos;
        Color gripColor = Color{ 40, 35, 30, 255 };
        DrawCube(gripPos.x, gripPos.y, gripPos.z, 0.04f, 0.12f, 0.08f, gripColor);

        // Grip texture lines
        for (int i = 0; i < 3; i++) {
            Vector3 linePos = Vector3Add(gripPos, Vector3Scale(up, -0.04f + i * 0.02f));
            DrawCube(linePos.x, linePos.y, linePos.z, 0.041f, 0.005f, 0.081f,
                Color{ 30, 25, 20, 255 });
        }

        // Frame (main body)
        Vector3 framePos = Vector3Add(gripPos, Vector3Scale(up, 0.04f));
        framePos = Vector3Add(framePos, Vector3Scale(forward, 0.02f));
        Color frameColor = Color{ 60, 60, 65, 255 };
        DrawCube(framePos.x, framePos.y, framePos.z, 0.038f, 0.05f, 0.12f, frameColor);

        // Frame highlight (metallic shine)
        Vector3 shinePos = Vector3Add(framePos, Vector3Scale(right, 0.015f));
        DrawCube(shinePos.x, shinePos.y, shinePos.z, 0.005f, 0.045f, 0.11f,
            Color{ 100, 100, 110, 200 });

        // Slide (top part) with metallic finish
        Vector3 slidePos = Vector3Add(framePos, Vector3Scale(up, 0.025f));
        slidePos = Vector3Add(slidePos, Vector3Scale(forward, 0.01f));
        Color slideColor = Color{ 70, 70, 75, 255 };
        DrawCube(slidePos.x, slidePos.y, slidePos.z, 0.035f, 0.04f, 0.15f, slideColor);

        // Slide serrations (rear)
        for (int i = 0; i < 5; i++) {
            Vector3 serPos = Vector3Add(slidePos, Vector3Scale(forward, -0.04f + i * 0.01f));
            DrawCube(serPos.x, serPos.y + 0.02f, serPos.z, 0.036f, 0.003f, 0.008f,
                Color{ 40, 40, 45, 255 });
        }

        // Front sight
        Vector3 frontSightPos = Vector3Add(slidePos, Vector3Scale(forward, 0.07f));
        frontSightPos = Vector3Add(frontSightPos, Vector3Scale(up, 0.022f));
        DrawCube(frontSightPos.x, frontSightPos.y, frontSightPos.z, 0.015f, 0.008f, 0.01f,
            Color{ 255, 200, 50, 255 });

        // Rear sight
        Vector3 rearSightPos = Vector3Add(slidePos, Vector3Scale(forward, -0.05f));
        rearSightPos = Vector3Add(rearSightPos, Vector3Scale(up, 0.022f));
        DrawCube(rearSightPos.x, rearSightPos.y, rearSightPos.z, 0.025f, 0.01f, 0.015f,
            Color{ 30, 30, 35, 255 });

        // Barrel (protruding front)
        Vector3 barrelPos = Vector3Add(slidePos, Vector3Scale(forward, 0.1f));
        Color barrelColor = Color{ 45, 45, 50, 255 };
        DrawCylinder(barrelPos, 0.012f, 0.012f, 0.08f, 8, barrelColor);

        // Barrel opening (muzzle)
        Vector3 muzzlePos = Vector3Add(barrelPos, Vector3Scale(forward, 0.04f));
        DrawCylinder(muzzlePos, 0.01f, 0.01f, 0.002f, 8, Color{ 20, 20, 25, 255 });

        // Trigger guard
        Vector3 triggerGuardPos = Vector3Add(gripPos, Vector3Scale(forward, 0.03f));
        DrawCube(triggerGuardPos.x, triggerGuardPos.y, triggerGuardPos.z,
            0.015f, 0.04f, 0.025f, frameColor);

        // Trigger
        Vector3 triggerPos = Vector3Add(triggerGuardPos, Vector3Scale(up, -0.005f));
        DrawCube(triggerPos.x, triggerPos.y, triggerPos.z, 0.008f, 0.02f, 0.015f,
            Color{ 180, 160, 140, 255 });

        // Magazine
        Vector3 magPos = Vector3Add(gripPos, Vector3Scale(up, -0.07f));
        DrawCube(magPos.x, magPos.y, magPos.z, 0.03f, 0.05f, 0.06f,
            Color{ 40, 40, 45, 255 });

        // Magazine baseplate
        Vector3 baseplatePos = Vector3Add(magPos, Vector3Scale(up, -0.028f));
        DrawCube(baseplatePos.x, baseplatePos.y, baseplatePos.z, 0.032f, 0.006f, 0.065f,
            Color{ 30, 30, 35, 255 });

        // Ejection port shadow
        Vector3 ejectionPos = Vector3Add(slidePos, Vector3Scale(right, 0.018f));
        DrawCube(ejectionPos.x, ejectionPos.y, ejectionPos.z, 0.002f, 0.025f, 0.04f,
            Color{ 10, 10, 15, 200 });

    }
    else if (itemId == ITEM_FLASHLIGHT) {
        // --- REALISTIC FLASHLIGHT MODEL ---

        Vector3 flashlightPos = basePos;

        // Tail cap (battery compartment)
        Vector3 tailPos = flashlightPos;
        Color tailColor = Color{ 50, 50, 55, 255 };
        DrawCylinder(tailPos, 0.018f, 0.018f, 0.03f, 12, tailColor);

        // Tail button
        Vector3 buttonPos = Vector3Add(tailPos, Vector3Scale(forward, -0.016f));
        DrawCylinder(buttonPos, 0.01f, 0.01f, 0.005f, 8, Color{ 180, 30, 30, 255 });

        // Body (main tube) with knurling texture
        Vector3 bodyPos = Vector3Add(tailPos, Vector3Scale(forward, 0.08f));
        Color bodyColor = Color{ 60, 60, 65, 255 };
        DrawCylinder(bodyPos, 0.02f, 0.02f, 0.13f, 16, bodyColor);

        // Knurling pattern (grip texture)
        for (int i = 0; i < 8; i++) {
            Vector3 knurlPos = Vector3Add(bodyPos, Vector3Scale(forward, -0.05f + i * 0.015f));
            DrawCylinder(knurlPos.x, knurlPos.y, knurlPos.z, 0.021f, 0.021f, 0.008f, 16, Color{ 50, 50, 55, 255 });
        }

        // Brand logo area (smooth section)
        Vector3 logoPos = Vector3Add(bodyPos, Vector3Scale(forward, 0.03f));
        DrawCylinder(logoPos, 0.0205f, 0.0205f, 0.04f, 16, Color{ 65, 65, 70, 255 });

        // Head (lens housing)
        Vector3 headPos = Vector3Add(bodyPos, Vector3Scale(forward, 0.11f));
        Color headColor = Color{ 70, 70, 75, 255 };
        DrawCylinder(headPos, 0.025f, 0.03f, 0.04f, 16, headColor);

        // Bezel (front ring)
        Vector3 bezelPos = Vector3Add(headPos, Vector3Scale(forward, 0.022f));
        DrawCylinder(bezelPos, 0.032f, 0.028f, 0.008f, 16, Color{ 50, 50, 55, 255 });

        // Lens/Reflector
        Vector3 lensPos = Vector3Add(headPos, Vector3Scale(forward, 0.025f));
        Color lensColor = Color{ 230, 240, 255, 230 };
        DrawCylinder(lensPos, 0.026f, 0.026f, 0.005f, 16, lensColor);

        // Reflector (inside lens)
        DrawCylinder(lensPos, 0.02f, 0.02f, 0.004f, 12, Color{ 245, 240, 200, 180 });

        // Light glow effect when on
        if (isFlashlightOn) {
            Vector3 glowPos = Vector3Add(lensPos, Vector3Scale(forward, 0.01f));

            // Multiple glow layers for realistic light
            DrawSphere(glowPos, 0.04f, Color{ 255, 255, 220, 100 });
            DrawSphere(glowPos, 0.03f, Color{ 255, 255, 200, 150 });
            DrawSphere(glowPos, 0.02f, Color{ 255, 255, 180, 200 });

            // Lens flare effect
            DrawCylinder(glowPos, 0.026f, 0.026f, 0.001f, 16, Color{ 255, 255, 240, 180 });
        }

        // Clip (for pocket attachment)
        Vector3 clipPos = Vector3Add(bodyPos, Vector3Scale(right, 0.021f));
        DrawCube(clipPos.x, clipPos.y, clipPos.z, 0.003f, 0.06f, 0.015f,
            Color{ 40, 40, 45, 255 });
    }
    else {
        // Generic item with basic shading
        Vector3 itemPos = basePos;
        Color itemColor = Color{ 180, 160, 140, 255 };

        DrawCube(itemPos.x, itemPos.y, itemPos.z, 0.05f, 0.05f, 0.05f, itemColor);

        Color shadowColor = Color{ 100, 90, 80, 255 };
        DrawCube(itemPos.x - 0.001f, itemPos.y - 0.001f, itemPos.z - 0.001f,
            0.051f, 0.051f, 0.051f, shadowColor);

        Vector3 highlightPos = Vector3Add(itemPos, Vector3Scale(right, 0.02f));
        highlightPos = Vector3Add(highlightPos, Vector3Scale(up, 0.02f));
        DrawCube(highlightPos.x, highlightPos.y, highlightPos.z,
            0.01f, 0.01f, 0.01f, Color{ 220, 200, 180, 200 });
    }
}

void UpdatePlayer(float deltaTime, Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* yaw, float* pitch, bool* onGround, float playerSpeed, float playerHeight, float gravity, float jumpForce, float* stamina, bool isNoclip, bool useController) {
    Vector2 mouseDelta = GetMouseDelta();
    if (useController) {
        mouseDelta.x = GetGamepadAxisMovement(0, GAMEPAD_CAMERA_MOVE_AXIS_X) * 5.0f;
        mouseDelta.y = GetGamepadAxisMovement(0, GAMEPAD_CAMERA_MOVE_AXIS_Y) * 5.0f;
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

    Vector3 flatForward = Vector3Normalize({ target.x, 0.0f, target.z });
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