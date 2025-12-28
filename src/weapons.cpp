#include "weapons.h"
#include "model_manager.h"
#include "rlgl.h"

WeaponSystem g_WeaponSystem;
WeaponState g_CurrentWeaponState = {
    ANIM_IDLE,
    0.0f,
    false,
    0.0f,
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f}
};

// FIXED: Add weapon system re-initialization flag
static bool g_WeaponSystemInitialized = false;

void DrawEnhancedPistol(Vector3 basePos, Vector3 forward, Vector3 right, Vector3 up, const WeaponState& state) {
    // FIXED: Re-initialize weapon system if needed
    if (!g_WeaponSystemInitialized) {
        g_WeaponSystem.InitializeWeapons();
        g_WeaponSystemInitialized = true;
    }

    if (g_ModelManager) {
        Vector3 adjustedPos = Vector3Add(basePos, state.recoilOffset);
        g_ModelManager->DrawModel(MODEL_PISTOL, adjustedPos, forward, right, up, WHITE);
    }
    else {
        DrawCube(basePos, 0.05f, 0.05f, 0.15f, Color{ 60, 60, 65, 255 });
    }
}

void DrawM16Rifle(Vector3 basePos, Vector3 forward, Vector3 right, Vector3 up, const WeaponState& state) {
    // FIXED: Re-initialize weapon system if needed
    if (!g_WeaponSystemInitialized) {
        g_WeaponSystem.InitializeWeapons();
        g_WeaponSystemInitialized = true;
    }

    if (g_ModelManager) {
        Vector3 adjustedPos = Vector3Add(basePos, state.recoilOffset);
        g_ModelManager->DrawModel(MODEL_M16, adjustedPos, forward, right, up, WHITE);
    }
    else {
        DrawCube(basePos, 0.05f, 0.08f, 0.25f, Color{ 40, 40, 45, 255 });
    }
}

void DrawKnife(Vector3 basePos, Vector3 forward, Vector3 right, Vector3 up, const WeaponState& state) {
    if (g_ModelManager) {
        Vector3 adjustedPos = basePos;

        // Add slash animation offset
        if (state.animState == ANIM_SLASH && state.animTimer > 0.0f) {
            float slashProgress = 1.0f - (state.animTimer / 0.5f);
            float slashAngle = slashProgress * 90.0f;
            adjustedPos = Vector3Add(adjustedPos, Vector3Scale(right, sinf(slashAngle * DEG2RAD) * 0.2f));
            adjustedPos = Vector3Add(adjustedPos, Vector3Scale(forward, slashProgress * 0.1f));
        }

        g_ModelManager->DrawModel(MODEL_KNIFE, adjustedPos, forward, right, up, WHITE);
    }
    else {
        // Fallback simple knife
        DrawCube(basePos, 0.02f, 0.01f, 0.12f, Color{ 180, 180, 190, 255 });
    }
}

void DrawLeftHandOnWeapon(Vector3 weaponPos, Vector3 forward, Vector3 right, Vector3 up, bool isRifle, float adsProgress) {
    Vector3 handPos = weaponPos;

    if (isRifle) {
        handPos = Vector3Add(handPos, Vector3Scale(forward, 0.15f));
        handPos = Vector3Add(handPos, Vector3Scale(up, -0.03f));
        handPos = Vector3Add(handPos, Vector3Scale(right, -0.05f + adsProgress * 0.02f));
    }
    else {
        handPos = Vector3Add(handPos, Vector3Scale(forward, 0.03f));
        handPos = Vector3Add(handPos, Vector3Scale(up, -0.04f));
        handPos = Vector3Add(handPos, Vector3Scale(right, -0.06f + adsProgress * 0.02f));
    }

    DrawCubeV(handPos, Vector3{ 0.03f, 0.04f, 0.06f }, Color{ 210, 180, 140, 255 });

    for (int i = 0; i < 4; i++) {
        Vector3 fingerPos = Vector3Add(handPos, Vector3Scale(forward, 0.04f + i * 0.008f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(right, -0.015f + i * 0.008f));
        DrawCubeV(fingerPos, Vector3{ 0.008f, 0.008f, 0.025f }, Color{ 200, 170, 130, 255 });
    }

    Vector3 thumbPos = Vector3Add(handPos, Vector3Scale(right, 0.02f));
    thumbPos = Vector3Add(thumbPos, Vector3Scale(forward, 0.02f));
    DrawCubeV(thumbPos, Vector3{ 0.01f, 0.01f, 0.02f }, Color{ 200, 170, 130, 255 });
}

void DrawIdleHands(const Camera3D& camera, float time) {
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = Vector3Normalize(camera.up);

    float bobAmount = sinf(time * 1.5f) * 0.01f;
    float swayAmount = cosf(time * 1.2f) * 0.008f;

    // Right hand
    Vector3 rightHandPos = camera.position;
    rightHandPos = Vector3Add(rightHandPos, Vector3Scale(forward, 0.35f));
    rightHandPos = Vector3Add(rightHandPos, Vector3Scale(right, 0.15f + swayAmount));
    rightHandPos = Vector3Add(rightHandPos, Vector3Scale(up, -0.2f + bobAmount));

    DrawCubeV(rightHandPos, Vector3{ 0.035f, 0.045f, 0.07f }, Color{ 210, 180, 140, 255 });

    for (int i = 0; i < 4; i++) {
        Vector3 fingerPos = Vector3Add(rightHandPos, Vector3Scale(forward, 0.04f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(right, -0.02f + i * 0.01f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(up, -0.01f * i));
        DrawCubeV(fingerPos, Vector3{ 0.009f, 0.009f, 0.028f }, Color{ 200, 170, 130, 255 });
    }

    // Left hand
    Vector3 leftHandPos = camera.position;
    leftHandPos = Vector3Add(leftHandPos, Vector3Scale(forward, 0.38f));
    leftHandPos = Vector3Add(leftHandPos, Vector3Scale(right, -0.18f - swayAmount));
    leftHandPos = Vector3Add(leftHandPos, Vector3Scale(up, -0.22f + bobAmount * 0.8f));

    DrawCubeV(leftHandPos, Vector3{ 0.035f, 0.045f, 0.07f }, Color{ 210, 180, 140, 255 });

    for (int i = 0; i < 4; i++) {
        Vector3 fingerPos = Vector3Add(leftHandPos, Vector3Scale(forward, 0.04f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(right, 0.02f - i * 0.01f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(up, -0.01f * i));
        DrawCubeV(fingerPos, Vector3{ 0.009f, 0.009f, 0.028f }, Color{ 200, 170, 130, 255 });
    }
}