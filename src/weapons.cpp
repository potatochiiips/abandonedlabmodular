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