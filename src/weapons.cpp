#include "weapons.h"
#include "model_manager.h"
#include "globals.h"
#include "rlgl.h"
#include "hud.h"
#include "zombie_system.h"
#include "rendering.h"
#include "weapon_renderer.h"
#include "sound_manager.h"

WeaponSystem g_WeaponSystem;
WeaponState g_CurrentWeaponState = {
    ANIM_IDLE, 0.0f, false, 0.0f, {0,0,0}, {0,0,0}, {0,0,0}
};

// ============================================================================
// WEAPON SHOOTING INTEGRATION EXAMPLE
// ============================================================================

void HandleWeaponShooting() {
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    extern WeaponRenderer* g_WeaponRenderer;
    extern HUDManager* g_HUDManager; // Changed from g_UpgradedHUD

    bool shootPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (shootPressed && inventory[BACKPACK_SLOTS].ammo > 0) {
        // Play weapon animation
        if (g_WeaponRenderer) {
            g_WeaponRenderer->PlayShootAnimation();
        }

        // Apply recoil
        WeaponStats* stats = g_WeaponSystem.GetWeaponStats(inventory[BACKPACK_SLOTS].itemId);
        if (stats && g_WeaponRenderer) {
            g_WeaponRenderer->ApplyRecoil(stats->recoilPitch, stats->recoilYaw);
        }

        // Decrease ammo
        inventory[BACKPACK_SLOTS].ammo--;

        // Raycast for hits
        extern Camera3D camera;
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

        // Check zombie hits
        if (g_ZombieManager && stats) {
            Zombie* hitZombie = g_ZombieManager->GetZombieAt(camera.position, 2.0f);
            if (hitZombie) {
                g_ZombieManager->DamageZombie(hitZombie->id, stats->damage);
                if (g_HUDManager) {
                    g_HUDManager->ShowHitMarker();
                }
            }
        }

        // Play sound
        if (g_SoundManager) {
            if (inventory[BACKPACK_SLOTS].itemId == ITEM_PISTOL) {
                g_SoundManager->PlaySound(SND_PISTOL_SHOT, 0.7f);
            }
            else if (inventory[BACKPACK_SLOTS].itemId == ITEM_M16) {
                g_SoundManager->PlaySound(SND_RIFLE_SHOT, 0.7f);
            }
        }
    }
}
// Add to weapon_renderer.cpp or hands_renderer.cpp

void HandsRenderer::DrawHands(const Camera3D& camera, bool holdingWeapon) {
    if (holdingWeapon) return; // Don't draw hands if holding weapon

    // Calculate hand positions
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = Vector3Normalize(camera.up);

    // Left hand
    Vector3 leftHandPos = camera.position;
    leftHandPos = Vector3Add(leftHandPos, Vector3Scale(forward, 0.4f));
    leftHandPos = Vector3Add(leftHandPos, Vector3Scale(right, -0.15f));
    leftHandPos = Vector3Add(leftHandPos, Vector3Scale(up, -0.25f));

    DrawHandMesh(leftHandPos, forward, right, up, true);

    // Right hand  
    Vector3 rightHandPos = camera.position;
    rightHandPos = Vector3Add(rightHandPos, Vector3Scale(forward, 0.4f));
    rightHandPos = Vector3Add(rightHandPos, Vector3Scale(right, 0.15f));
    rightHandPos = Vector3Add(rightHandPos, Vector3Scale(up, -0.25f));

    DrawHandMesh(rightHandPos, forward, right, up, false);
}

void HandsRenderer::DrawHandMesh(Vector3 position, Vector3 forward, Vector3 right,
    Vector3 up, bool leftHand) {
    // Simple hand representation using cubes
    Color skinColor = Color{ 255, 220, 177, 255 };

    // Palm
    rlPushMatrix();
    rlTranslatef(position.x, position.y, position.z);

    Matrix orientation = {
        right.x, right.y, right.z, 0.0f,
        up.x, up.y, up.z, 0.0f,
        forward.x, forward.y, forward.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    rlMultMatrixf(MatrixToFloat(orientation));

    // Draw palm
    DrawCube(Vector3{ 0, 0, 0 }, 0.08f, 0.12f, 0.05f, skinColor);

    // Fingers (simplified)
    for (int i = 0; i < 4; i++) {
        float xOffset = (i - 1.5f) * 0.02f;
        DrawCube(Vector3{ xOffset, 0, 0.04f }, 0.015f, 0.015f, 0.03f, skinColor);
    }

    rlPopMatrix();
}