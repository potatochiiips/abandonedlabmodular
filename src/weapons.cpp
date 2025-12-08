#include "weapons.h"
#include "rlgl.h"

// Global instances
WeaponSystem g_WeaponSystem;
WeaponState g_CurrentWeaponState = {ANIM_IDLE, 0.0f, false, 0.0f, {0}, {0}, {0}};

// Helper to draw textured cylinder (for barrels)
void DrawCylinderDetailed(Vector3 start, Vector3 end, float radius, Color color) {
    DrawCylinderEx(start, end, radius, radius, 16, color);
}

// Enhanced pistol with better proportions and details
void DrawEnhancedPistol(Vector3 basePos, Vector3 forward, Vector3 right, Vector3 up, const WeaponState& state) {
    // Scale factors for better visual
    float scale = 1.2f;
    
    // Grip (handle) - improved shape
    Vector3 gripPos = basePos;
    DrawCubeV(gripPos, Vector3{0.045f * scale, 0.14f * scale, 0.09f * scale}, Color{40, 35, 30, 255});
    
    // Grip texture (checkering)
    for (int i = 0; i < 4; i++) {
        Vector3 linePos = Vector3Add(gripPos, Vector3Scale(up, -0.05f * scale + i * 0.025f * scale));
        DrawCubeV(linePos, Vector3{0.046f * scale, 0.006f * scale, 0.091f * scale}, Color{30, 25, 20, 255});
    }
    
    // Frame - more detailed
    Vector3 framePos = Vector3Add(gripPos, Vector3Scale(up, 0.05f * scale));
    framePos = Vector3Add(framePos, Vector3Scale(forward, 0.025f * scale));
    DrawCubeV(framePos, Vector3{0.042f * scale, 0.06f * scale, 0.14f * scale}, Color{60, 60, 65, 255});
    
    // Slide with serrations
    Vector3 slidePos = Vector3Add(framePos, Vector3Scale(up, 0.03f * scale));
    DrawCubeV(slidePos, Vector3{0.04f * scale, 0.05f * scale, 0.17f * scale}, Color{70, 70, 75, 255});
    
    // Slide serrations (better detail)
    for (int i = 0; i < 7; i++) {
        Vector3 serPos = Vector3Add(slidePos, Vector3Scale(forward, -0.05f * scale + i * 0.012f * scale));
        DrawCubeV(serPos, Vector3{0.041f * scale, 0.004f * scale, 0.009f * scale}, Color{40, 40, 45, 255});
    }
    
    // Barrel (extended and detailed)
    Vector3 barrelPos = Vector3Add(slidePos, Vector3Scale(forward, 0.12f * scale));
    DrawCylinderDetailed(
        Vector3Subtract(barrelPos, Vector3Scale(forward, 0.05f * scale)),
        Vector3Add(barrelPos, Vector3Scale(forward, 0.05f * scale)),
        0.014f * scale,
        Color{45, 45, 50, 255}
    );
    
    // Muzzle
    Vector3 muzzlePos = Vector3Add(barrelPos, Vector3Scale(forward, 0.05f * scale));
    DrawCylinderDetailed(
        Vector3Subtract(muzzlePos, Vector3Scale(forward, 0.003f * scale)),
        Vector3Add(muzzlePos, Vector3Scale(forward, 0.003f * scale)),
        0.012f * scale,
        Color{20, 20, 25, 255}
    );
    
    // Front sight (more pronounced)
    Vector3 frontSightPos = Vector3Add(slidePos, Vector3Scale(forward, 0.08f * scale));
    frontSightPos = Vector3Add(frontSightPos, Vector3Scale(up, 0.027f * scale));
    DrawCubeV(frontSightPos, Vector3{0.018f * scale, 0.012f * scale, 0.012f * scale}, Color{255, 200, 50, 255});
    
    // Rear sight (better shaped)
    Vector3 rearSightPos = Vector3Add(slidePos, Vector3Scale(forward, -0.06f * scale));
    rearSightPos = Vector3Add(rearSightPos, Vector3Scale(up, 0.027f * scale));
    DrawCubeV(rearSightPos, Vector3{0.028f * scale, 0.012f * scale, 0.018f * scale}, Color{30, 30, 35, 255});
    
    // Trigger and trigger guard
    Vector3 triggerGuardPos = Vector3Add(gripPos, Vector3Scale(forward, 0.035f * scale));
    DrawCubeV(triggerGuardPos, Vector3{0.018f * scale, 0.045f * scale, 0.028f * scale}, Color{60, 60, 65, 255});
    
    Vector3 triggerPos = Vector3Add(triggerGuardPos, Vector3Scale(up, -0.007f * scale));
    DrawCubeV(triggerPos, Vector3{0.01f * scale, 0.024f * scale, 0.018f * scale}, Color{180, 160, 140, 255});
    
    // Magazine with better detail
    Vector3 magPos = Vector3Add(gripPos, Vector3Scale(up, -0.08f * scale));
    DrawCubeV(magPos, Vector3{0.035f * scale, 0.055f * scale, 0.07f * scale}, Color{40, 40, 45, 255});
    
    // Magazine baseplate
    Vector3 baseplatePos = Vector3Add(magPos, Vector3Scale(up, -0.03f * scale));
    DrawCubeV(baseplatePos, Vector3{0.037f * scale, 0.007f * scale, 0.075f * scale}, Color{30, 30, 35, 255});
    
    // Ejection port
    Vector3 ejectionPos = Vector3Add(slidePos, Vector3Scale(right, 0.021f * scale));
    DrawCubeV(ejectionPos, Vector3{0.003f * scale, 0.03f * scale, 0.045f * scale}, Color{10, 10, 15, 200});
    
    // Add shine/reflection on slide
    Vector3 shinePos = Vector3Add(slidePos, Vector3Scale(right, 0.017f * scale));
    DrawCubeV(shinePos, Vector3{0.006f * scale, 0.045f * scale, 0.14f * scale}, Color{100, 100, 110, 150});
}

// M16 rifle model
void DrawM16Rifle(Vector3 basePos, Vector3 forward, Vector3 right, Vector3 up, const WeaponState& state) {
    float scale = 1.0f;
    
    // Lower receiver
    Vector3 lowerPos = basePos;
    DrawCubeV(lowerPos, Vector3{0.05f, 0.08f, 0.25f}, Color{40, 40, 45, 255});
    
    // Pistol grip
    Vector3 gripPos = Vector3Add(lowerPos, Vector3Scale(up, -0.06f));
    gripPos = Vector3Add(gripPos, Vector3Scale(forward, -0.05f));
    DrawCubeV(gripPos, Vector3{0.045f, 0.12f, 0.08f}, Color{35, 35, 40, 255});
    
    // Upper receiver
    Vector3 upperPos = Vector3Add(lowerPos, Vector3Scale(up, 0.045f));
    DrawCubeV(upperPos, Vector3{0.048f, 0.05f, 0.3f}, Color{50, 50, 55, 255});
    
    // Barrel assembly
    Vector3 barrelPos = Vector3Add(upperPos, Vector3Scale(forward, 0.3f));
    DrawCylinderDetailed(
        Vector3Add(upperPos, Vector3Scale(forward, 0.1f)),
        barrelPos,
        0.015f,
        Color{45, 45, 50, 255}
    );
    
    // Flash hider
    Vector3 flashHiderPos = Vector3Add(barrelPos, Vector3Scale(forward, 0.03f));
    DrawCylinderDetailed(
        Vector3Subtract(flashHiderPos, Vector3Scale(forward, 0.02f)),
        Vector3Add(flashHiderPos, Vector3Scale(forward, 0.02f)),
        0.02f,
        Color{30, 30, 35, 255}
    );
    
    // Handguard
    Vector3 handguardPos = Vector3Add(upperPos, Vector3Scale(forward, 0.18f));
    handguardPos = Vector3Add(handguardPos, Vector3Scale(up, -0.025f));
    DrawCubeV(handguardPos, Vector3{0.055f, 0.06f, 0.2f}, Color{35, 35, 40, 255});
    
    // Handguard vent slots
    for (int i = 0; i < 8; i++) {
        Vector3 ventPos = Vector3Add(handguardPos, Vector3Scale(forward, -0.08f + i * 0.02f));
        DrawCubeV(Vector3Add(ventPos, Vector3Scale(right, 0.028f)), 
                  Vector3{0.002f, 0.05f, 0.008f}, Color{20, 20, 25, 255});
    }
    
    // Magazine
    Vector3 magPos = Vector3Add(lowerPos, Vector3Scale(up, -0.08f));
    DrawCubeV(magPos, Vector3{0.04f, 0.12f, 0.08f}, Color{40, 40, 45, 255});
    
    // Carry handle & rear sight
    Vector3 handlePos = Vector3Add(upperPos, Vector3Scale(up, 0.05f));
    handlePos = Vector3Add(handlePos, Vector3Scale(forward, -0.08f));
    DrawCubeV(handlePos, Vector3{0.045f, 0.04f, 0.15f}, Color{45, 45, 50, 255});
    
    // Rear sight aperture
    Vector3 rearSightPos = Vector3Add(handlePos, Vector3Scale(forward, 0.05f));
    rearSightPos = Vector3Add(rearSightPos, Vector3Scale(up, 0.025f));
    DrawCubeV(rearSightPos, Vector3{0.02f, 0.015f, 0.01f}, Color{30, 30, 35, 255});
    
    // Front sight post
    Vector3 frontSightPos = Vector3Add(upperPos, Vector3Scale(forward, 0.25f));
    frontSightPos = Vector3Add(frontSightPos, Vector3Scale(up, 0.03f));
    DrawCubeV(frontSightPos, Vector3{0.015f, 0.02f, 0.01f}, Color{255, 200, 50, 255});
    
    // Stock
    Vector3 stockPos = Vector3Add(lowerPos, Vector3Scale(forward, -0.15f));
    DrawCubeV(stockPos, Vector3{0.045f, 0.05f, 0.12f}, Color{35, 35, 40, 255});
    
    // Buttstock
    Vector3 buttPos = Vector3Add(stockPos, Vector3Scale(forward, -0.08f));
    DrawCubeV(buttPos, Vector3{0.06f, 0.12f, 0.02f}, Color{40, 40, 45, 255});
    
    // Charging handle
    Vector3 chargePos = Vector3Add(upperPos, Vector3Scale(forward, -0.05f));
    chargePos = Vector3Add(chargePos, Vector3Scale(up, 0.028f));
    DrawCubeV(chargePos, Vector3{0.015f, 0.008f, 0.03f}, Color{45, 45, 50, 255});
    
    // Ejection port cover
    Vector3 ejectionCoverPos = Vector3Add(upperPos, Vector3Scale(right, 0.025f));
    ejectionCoverPos = Vector3Add(ejectionCoverPos, Vector3Scale(forward, 0.02f));
    DrawCubeV(ejectionCoverPos, Vector3{0.002f, 0.04f, 0.08f}, Color{45, 45, 50, 255});
}

// Draw left hand on weapon
void DrawLeftHandOnWeapon(Vector3 weaponPos, Vector3 forward, Vector3 right, Vector3 up, bool isRifle, float adsProgress) {
    // Hand position varies based on weapon and ADS state
    Vector3 handPos = weaponPos;
    
    if (isRifle) {
        // On handguard
        handPos = Vector3Add(handPos, Vector3Scale(forward, 0.15f));
        handPos = Vector3Add(handPos, Vector3Scale(up, -0.03f));
        handPos = Vector3Add(handPos, Vector3Scale(right, -0.05f + adsProgress * 0.02f));
    } else {
        // Supporting pistol
        handPos = Vector3Add(handPos, Vector3Scale(forward, 0.03f));
        handPos = Vector3Add(handPos, Vector3Scale(up, -0.04f));
        handPos = Vector3Add(handPos, Vector3Scale(right, -0.06f + adsProgress * 0.02f));
    }
    
    // Palm
    DrawCubeV(handPos, Vector3{0.03f, 0.04f, 0.06f}, Color{210, 180, 140, 255});
    
    // Fingers
    for (int i = 0; i < 4; i++) {
        Vector3 fingerPos = Vector3Add(handPos, Vector3Scale(forward, 0.04f + i * 0.008f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(right, -0.015f + i * 0.008f));
        DrawCubeV(fingerPos, Vector3{0.008f, 0.008f, 0.025f}, Color{200, 170, 130, 255});
    }
    
    // Thumb
    Vector3 thumbPos = Vector3Add(handPos, Vector3Scale(right, 0.02f));
    thumbPos = Vector3Add(thumbPos, Vector3Scale(forward, 0.02f));
    DrawCubeV(thumbPos, Vector3{0.01f, 0.01f, 0.02f}, Color{200, 170, 130, 255});
}

// Draw idle hands when no weapon equipped
void DrawIdleHands(const Camera3D& camera, float time) {
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = Vector3Normalize(camera.up);
    
    // Animated idle position
    float bobAmount = sinf(time * 1.5f) * 0.01f;
    float swayAmount = cosf(time * 1.2f) * 0.008f;
    
    // Right hand
    Vector3 rightHandPos = camera.position;
    rightHandPos = Vector3Add(rightHandPos, Vector3Scale(forward, 0.35f));
    rightHandPos = Vector3Add(rightHandPos, Vector3Scale(right, 0.15f + swayAmount));
    rightHandPos = Vector3Add(rightHandPos, Vector3Scale(up, -0.2f + bobAmount));
    
    // Right palm
    DrawCubeV(rightHandPos, Vector3{0.035f, 0.045f, 0.07f}, Color{210, 180, 140, 255});
    
    // Right fingers (relaxed)
    for (int i = 0; i < 4; i++) {
        Vector3 fingerPos = Vector3Add(rightHandPos, Vector3Scale(forward, 0.04f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(right, -0.02f + i * 0.01f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(up, -0.01f * i));
        DrawCubeV(fingerPos, Vector3{0.009f, 0.009f, 0.028f}, Color{200, 170, 130, 255});
    }
    
    // Left hand (mirrored and slightly different)
    Vector3 leftHandPos = camera.position;
    leftHandPos = Vector3Add(leftHandPos, Vector3Scale(forward, 0.38f));
    leftHandPos = Vector3Add(leftHandPos, Vector3Scale(right, -0.18f - swayAmount));
    leftHandPos = Vector3Add(leftHandPos, Vector3Scale(up, -0.22f + bobAmount * 0.8f));
    
    // Left palm
    DrawCubeV(leftHandPos, Vector3{0.035f, 0.045f, 0.07f}, Color{210, 180, 140, 255});
    
    // Left fingers
    for (int i = 0; i < 4; i++) {
        Vector3 fingerPos = Vector3Add(leftHandPos, Vector3Scale(forward, 0.04f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(right, 0.02f - i * 0.01f));
        fingerPos = Vector3Add(fingerPos, Vector3Scale(up, -0.01f * i));
        DrawCubeV(fingerPos, Vector3{0.009f, 0.009f, 0.028f}, Color{200, 170, 130, 255});
    }
}