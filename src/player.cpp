#include "player.h"
#include "items.h"
#include <math.h> // For sinf/cosf/fabs

// Note: Removed redundant struct and math function definitions. 
// Assuming all Vector3 math (Add, Scale, Normalize, etc.) and raylib types 
// (Vector3, Color, Camera3D, ControllerBinding) are correctly pulled from raylib.h/raymath.h 
// via player.h or other includes.

const char* GetGamepadButtonName(int button) {
    // Assuming gamepad button constants are correctly defined elsewhere (e.g., globals.h)

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
    // Assuming ACTION_COUNT is defined
    if (actionIndex < 0 || actionIndex >= 6) return false;
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
    // Assuming ACTION_COUNT is defined
    if (actionIndex < 0 || actionIndex >= 6) return false;
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
    // Assuming BACKPACK_SLOTS, ITEM_NONE, isFlashlightOn are defined elsewhere
    int itemId = inventory[BACKPACK_SLOTS].itemId;
    if (itemId == ITEM_NONE) return;

    // Placeholder for undefined variable 'isFlashlightOn'
    bool isFlashlightOn = true;

    // Use raymath.h functions (Vector3Normalize, Vector3Subtract, etc.)
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
        Vector3 gripSize = { 0.04f, 0.12f, 0.08f };
        Color gripColor = Color{ 40, 35, 30, 255 };
        DrawCubeV(gripPos, gripSize, gripColor);

        // Grip texture lines
        for (int i = 0; i < 3; i++) {
            Vector3 linePos = Vector3Add(gripPos, Vector3Scale(up, -0.04f + i * 0.02f));
            Vector3 lineSize = { 0.041f, 0.005f, 0.081f };
            DrawCubeV(linePos, lineSize, Color{ 30, 25, 20, 255 });
        }

        // Frame (main body)
        Vector3 framePos = Vector3Add(gripPos, Vector3Scale(up, 0.04f));
        framePos = Vector3Add(framePos, Vector3Scale(forward, 0.02f));
        Vector3 frameSize = { 0.038f, 0.05f, 0.12f };
        Color frameColor = Color{ 60, 60, 65, 255 };
        DrawCubeV(framePos, frameSize, frameColor);

        // Frame highlight (metallic shine)
        Vector3 shinePos = Vector3Add(framePos, Vector3Scale(right, 0.015f));
        Vector3 shineSize = { 0.005f, 0.045f, 0.11f };
        DrawCubeV(shinePos, shineSize, Color{ 100, 100, 110, 200 });

        // Slide (top part) with metallic finish
        Vector3 slidePos = Vector3Add(framePos, Vector3Scale(up, 0.025f));
        slidePos = Vector3Add(slidePos, Vector3Scale(forward, 0.01f));
        Vector3 slideSize = { 0.035f, 0.04f, 0.15f };
        Color slideColor = Color{ 70, 70, 75, 255 };
        DrawCubeV(slidePos, slideSize, slideColor);

        // Slide serrations (rear)
        for (int i = 0; i < 5; i++) {
            Vector3 serPos = Vector3Add(slidePos, Vector3Scale(forward, -0.04f + i * 0.01f));
            Vector3 serSize = { 0.036f, 0.003f, 0.008f };
            DrawCubeV(serPos, serSize, Color{ 40, 40, 45, 255 });
        }

        // Front sight
        Vector3 frontSightPos = Vector3Add(slidePos, Vector3Scale(forward, 0.07f));
        frontSightPos = Vector3Add(frontSightPos, Vector3Scale(up, 0.022f));
        Vector3 frontSightSize = { 0.015f, 0.008f, 0.01f };
        DrawCubeV(frontSightPos, frontSightSize, Color{ 255, 200, 50, 255 });

        // Rear sight
        Vector3 rearSightPos = Vector3Add(slidePos, Vector3Scale(forward, -0.05f));
        rearSightPos = Vector3Add(rearSightPos, Vector3Scale(up, 0.022f));
        Vector3 rearSightSize = { 0.025f, 0.01f, 0.015f };
        DrawCubeV(rearSightPos, rearSightSize, Color{ 30, 30, 35, 255 });

        // Barrel (protruding front)
        Vector3 barrelPos = Vector3Add(slidePos, Vector3Scale(forward, 0.1f));
        Color barrelColor = Color{ 45, 45, 50, 255 };

        float barrelRadius = 0.012f;
        // FIX: Using DrawCylinderEx(startPos, endPos, startRadius, endRadius, sides, color)
        Vector3 barrelStart = Vector3Subtract(barrelPos, Vector3Scale(forward, 0.08f / 2.0f));
        Vector3 barrelEnd = Vector3Add(barrelPos, Vector3Scale(forward, 0.08f / 2.0f));
        DrawCylinderEx(barrelStart, barrelEnd, barrelRadius, barrelRadius, 8, barrelColor);

        // Barrel opening (muzzle)
        Vector3 muzzlePos = Vector3Add(barrelPos, Vector3Scale(forward, 0.04f));
        float muzzleRadius = 0.01f;
        // FIX: Using DrawCylinderEx
        Vector3 muzzleStart = Vector3Subtract(muzzlePos, Vector3Scale(forward, 0.002f / 2.0f));
        Vector3 muzzleEnd = Vector3Add(muzzlePos, Vector3Scale(forward, 0.002f / 2.0f));
        DrawCylinderEx(muzzleStart, muzzleEnd, muzzleRadius, muzzleRadius, 8, Color{ 20, 20, 25, 255 });

        // Trigger guard
        Vector3 triggerGuardPos = Vector3Add(gripPos, Vector3Scale(forward, 0.03f));
        Vector3 triggerGuardSize = { 0.015f, 0.04f, 0.025f };
        DrawCubeV(triggerGuardPos, triggerGuardSize, frameColor);

        // Trigger
        Vector3 triggerPos = Vector3Add(triggerGuardPos, Vector3Scale(up, -0.005f));
        Vector3 triggerSize = { 0.008f, 0.02f, 0.015f };
        DrawCubeV(triggerPos, triggerSize, Color{ 180, 160, 140, 255 });

        // Magazine
        Vector3 magPos = Vector3Add(gripPos, Vector3Scale(up, -0.07f));
        Vector3 magSize = { 0.03f, 0.05f, 0.06f };
        DrawCubeV(magPos, magSize, Color{ 40, 40, 45, 255 });

        // Magazine baseplate
        Vector3 baseplatePos = Vector3Add(magPos, Vector3Scale(up, -0.028f));
        Vector3 baseplateSize = { 0.032f, 0.006f, 0.065f };
        DrawCubeV(baseplatePos, baseplateSize, Color{ 30, 30, 35, 255 });

        // Ejection port shadow
        Vector3 ejectionPos = Vector3Add(slidePos, Vector3Scale(right, 0.018f));
        Vector3 ejectionSize = { 0.002f, 0.025f, 0.04f };
        DrawCubeV(ejectionPos, ejectionSize, Color{ 10, 10, 15, 200 });

    }
    else if (itemId == ITEM_FLASHLIGHT) {
        // --- REALISTIC FLASHLIGHT MODEL ---

        Vector3 flashlightPos = basePos;

        // Tail cap (battery compartment)
        Vector3 tailPos = flashlightPos;
        Color tailColor = Color{ 50, 50, 55, 255 };
        float tailRadius = 0.018f;
        // FIX: Using DrawCylinderEx
        Vector3 tailStart = Vector3Subtract(tailPos, Vector3Scale(forward, 0.03f / 2.0f));
        Vector3 tailEnd = Vector3Add(tailPos, Vector3Scale(forward, 0.03f / 2.0f));
        DrawCylinderEx(tailStart, tailEnd, tailRadius, tailRadius, 12, tailColor);

        // Tail button
        Vector3 buttonPos = Vector3Add(tailPos, Vector3Scale(forward, -0.016f));
        float buttonRadius = 0.01f;
        // FIX: Using DrawCylinderEx
        Vector3 buttonStart = Vector3Subtract(buttonPos, Vector3Scale(forward, 0.005f / 2.0f));
        Vector3 buttonEnd = Vector3Add(buttonPos, Vector3Scale(forward, 0.005f / 2.0f));
        DrawCylinderEx(buttonStart, buttonEnd, buttonRadius, buttonRadius, 8, Color{ 180, 30, 30, 255 });

        // Body (main tube) with knurling texture
        Vector3 bodyPos = Vector3Add(tailPos, Vector3Scale(forward, 0.08f));
        Color bodyColor = Color{ 60, 60, 65, 255 };
        float bodyRadius = 0.02f;
        // FIX: Using DrawCylinderEx
        Vector3 bodyStart = Vector3Subtract(bodyPos, Vector3Scale(forward, 0.13f / 2.0f));
        Vector3 bodyEnd = Vector3Add(bodyPos, Vector3Scale(forward, 0.13f / 2.0f));
        DrawCylinderEx(bodyStart, bodyEnd, bodyRadius, bodyRadius, 16, bodyColor);

        // Knurling pattern (grip texture)
        for (int i = 0; i < 8; i++) {
            Vector3 knurlPos = Vector3Add(bodyPos, Vector3Scale(forward, -0.05f + i * 0.015f));
            float knurlRadius = 0.021f;
            // FIX: Using DrawCylinderEx
            Vector3 knurlStart = Vector3Subtract(knurlPos, Vector3Scale(forward, 0.008f / 2.0f));
            Vector3 knurlEnd = Vector3Add(knurlPos, Vector3Scale(forward, 0.008f / 2.0f));
            DrawCylinderEx(knurlStart, knurlEnd, knurlRadius, knurlRadius, 16, Color{ 50, 50, 55, 255 });
        }

        // Brand logo area (smooth section)
        Vector3 logoPos = Vector3Add(bodyPos, Vector3Scale(forward, 0.03f));
        float logoRadius = 0.0205f;
        // FIX: Using DrawCylinderEx
        Vector3 logoStart = Vector3Subtract(logoPos, Vector3Scale(forward, 0.04f / 2.0f));
        Vector3 logoEnd = Vector3Add(logoPos, Vector3Scale(forward, 0.04f / 2.0f));
        DrawCylinderEx(logoStart, logoEnd, logoRadius, logoRadius, 16, Color{ 65, 65, 70, 255 });

        // Head (lens housing)
        Vector3 headPos = Vector3Add(bodyPos, Vector3Scale(forward, 0.11f));
        Color headColor = Color{ 70, 70, 75, 255 };
        float headRadius = 0.0275f;
        // FIX: Using DrawCylinderEx
        Vector3 headStart = Vector3Subtract(headPos, Vector3Scale(forward, 0.04f / 2.0f));
        Vector3 headEnd = Vector3Add(headPos, Vector3Scale(forward, 0.04f / 2.0f));
        DrawCylinderEx(headStart, headEnd, headRadius, headRadius, 16, headColor);

        // Bezel (front ring)
        Vector3 bezelPos = Vector3Add(headPos, Vector3Scale(forward, 0.022f));
        float bezelRadius = 0.03f;
        // FIX: Using DrawCylinderEx
        Vector3 bezelStart = Vector3Subtract(bezelPos, Vector3Scale(forward, 0.008f / 2.0f));
        Vector3 bezelEnd = Vector3Add(bezelPos, Vector3Scale(forward, 0.008f / 2.0f));
        DrawCylinderEx(bezelStart, bezelEnd, bezelRadius, bezelRadius, 16, Color{ 50, 50, 55, 255 });

        // Lens/Reflector
        Vector3 lensPos = Vector3Add(headPos, Vector3Scale(forward, 0.025f));
        Color lensColor = Color{ 230, 240, 255, 230 };
        float lensRadius = 0.026f;
        // FIX: Using DrawCylinderEx
        Vector3 lensStart = Vector3Subtract(lensPos, Vector3Scale(forward, 0.005f / 2.0f));
        Vector3 lensEnd = Vector3Add(lensPos, Vector3Scale(forward, 0.005f / 2.0f));
        DrawCylinderEx(lensStart, lensEnd, lensRadius, lensRadius, 16, lensColor);

        // Reflector (inside lens)
        float reflectorRadius = 0.02f;
        // FIX: Using DrawCylinderEx
        Vector3 reflectorStart = Vector3Subtract(lensPos, Vector3Scale(forward, 0.004f / 2.0f));
        Vector3 reflectorEnd = Vector3Add(lensPos, Vector3Scale(forward, 0.004f / 2.0f));
        DrawCylinderEx(reflectorStart, reflectorEnd, reflectorRadius, reflectorRadius, 12, Color{ 245, 240, 200, 180 });

        // Light glow effect when on
        if (isFlashlightOn) {
            Vector3 glowPos = Vector3Add(lensPos, Vector3Scale(forward, 0.01f));

            // Multiple glow layers for realistic light (DrawSphere calls remain the same)
            DrawSphere(glowPos, 0.04f, Color{ 255, 255, 220, 100 });
            DrawSphere(glowPos, 0.03f, Color{ 255, 255, 200, 150 });
            DrawSphere(glowPos, 0.02f, Color{ 255, 255, 180, 200 });

            // Lens flare effect
            float glowCylRadius = 0.026f;
            // FIX: Using DrawCylinderEx
            Vector3 glowCylStart = Vector3Subtract(glowPos, Vector3Scale(forward, 0.001f / 2.0f));
            Vector3 glowCylEnd = Vector3Add(glowPos, Vector3Scale(forward, 0.001f / 2.0f));
            DrawCylinderEx(glowCylStart, glowCylEnd, glowCylRadius, glowCylRadius, 16, Color{ 255, 255, 240, 180 });
        }

        // Clip (for pocket attachment)
        Vector3 clipPos = Vector3Add(bodyPos, Vector3Scale(right, 0.021f));
        Vector3 clipSize = { 0.003f, 0.06f, 0.015f };
        DrawCubeV(clipPos, clipSize, Color{ 40, 40, 45, 255 });
    }
    else {
        // Generic item with basic shading
        Vector3 itemPos = basePos;
        Vector3 itemSize = { 0.05f, 0.05f, 0.05f };
        Color itemColor = Color{ 180, 160, 140, 255 };

        DrawCubeV(itemPos, itemSize, itemColor);

        Color shadowColor = Color{ 100, 90, 80, 255 };
        Vector3 shadowOffset = { 0.001f, 0.001f, 0.001f };
        Vector3 shadowPos = Vector3Subtract(itemPos, shadowOffset);
        Vector3 shadowSize = { 0.051f, 0.051f, 0.051f };
        DrawCubeV(shadowPos, shadowSize, shadowColor);

        Vector3 highlightPos = Vector3Add(itemPos, Vector3Scale(right, 0.02f));
        highlightPos = Vector3Add(highlightPos, Vector3Scale(up, 0.02f));
        Vector3 highlightSize = { 0.01f, 0.01f, 0.01f };
        DrawCubeV(highlightPos, highlightSize, Color{ 220, 200, 180, 200 });
    }
}

// --- Player Logic ---

void UpdatePlayer(float deltaTime, Camera3D* camera, Vector3* playerPosition, Vector3* playerVelocity, float* yaw, float* pitch, bool* onGround, float playerSpeed, float playerHeight, float gravity, float jumpForce, float* stamina, bool isNoclip, bool useController) {
    // Placeholder for undefined variable 'bindings'
    ControllerBinding* bindings = nullptr;

    Vector2 mouseDelta = GetMouseDelta();
    // Assuming GAMEPAD_CAMERA_MOVE_AXIS_X/Y are defined
    if (useController) {
        float moveAxisX = GetGamepadAxisMovement(0, 0); // Using 0 as placeholder for X
        float moveAxisY = GetGamepadAxisMovement(0, 1); // Using 1 as placeholder for Y
        mouseDelta.x = moveAxisX * 5.0f;
        mouseDelta.y = moveAxisY * 5.0f;
    }

    float mouseSensitivity = 0.2f;
    *yaw += mouseDelta.x * mouseSensitivity;
    *pitch -= mouseDelta.y * mouseSensitivity;
    *pitch = Clamp(*pitch, -89.0f, 89.0f);

    // Using DEG2RAD from raylib.h
    Vector3 target = {
        cosf(DEG2RAD * *yaw) * cosf(DEG2RAD * *pitch),
        sinf(DEG2RAD * *pitch),
        sinf(DEG2RAD * *yaw) * cosf(DEG2RAD * *pitch)
    };
    camera->target = Vector3Add(*playerPosition, target);

    // FIX C4576: Use a standard C++ aggregate initialization with a temporary variable
    // to avoid the non-standard compound literal syntax.
    Vector3 tempFlatForward = { target.x, 0.0f, target.z };
    Vector3 flatForward = Vector3Normalize(tempFlatForward);

    Vector3 right = Vector3Normalize(Vector3CrossProduct(flatForward, camera->up));
    Vector3 movement = { 0 }; // Initialize to zero vector

    // Assuming ACTION_SPRINT is defined elsewhere
    bool isSprinting = (IsKeyDown(KEY_LEFT_SHIFT) || (useController && IsActionDown(ACTION_SPRINT, bindings))) && *stamina > 0.0f;
    float currentSpeed = playerSpeed * (isSprinting ? 2.0f : 1.0f);

    if (useController) {
        // Assuming GAMEPAD_PLAYER_MOVE_AXIS_X/Y are defined
        float moveX = GetGamepadAxisMovement(0, 2); // Using 2 as placeholder for X
        float moveY = GetGamepadAxisMovement(0, 3); // Using 3 as placeholder for Y
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
        // Assuming ACTION_JUMP is defined elsewhere
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