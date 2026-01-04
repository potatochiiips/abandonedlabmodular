#include "weapons.h"
#include "model_manager.h"
#include "globals.h"
#include "rlgl.h"
#include "hud.h"
#include "zombie_system.h"
#include "rendering.h"
#include "weapon_renderer.h"
#include "sound_manager.h"
#include "hands_renderer.h"

WeaponSystem g_WeaponSystem;
WeaponState g_CurrentWeaponState = {
    ANIM_IDLE, 0.0f, false, 0.0f, {0,0,0}, {0,0,0}, {0,0,0}
};

// FIXED: Improved weapon shooting with proper hit marker triggering
void HandleWeaponShooting() {
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    extern WeaponRenderer* g_WeaponRenderer;
    extern HUDManager* g_HUDManager;
    extern Camera3D camera;
    extern bool isReloading;
    extern float pistolRecoilPitch, pistolRecoilYaw;

    // Check if we can shoot
    if (isReloading) return;
    if (inventory[BACKPACK_SLOTS].itemId == ITEM_NONE) return;

    bool shootPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // Get weapon stats
    WeaponStats* stats = g_WeaponSystem.GetWeaponStats(inventory[BACKPACK_SLOTS].itemId);
    if (!stats) return;

    if (shootPressed && inventory[BACKPACK_SLOTS].ammo > 0) {
        // Play weapon animation
        if (g_WeaponRenderer) {
            g_WeaponRenderer->PlayShootAnimation();
        }

        // Apply recoil
        pistolRecoilPitch += stats->recoilPitch;
        pistolRecoilYaw += (rand() % 100 - 50) / 100.0f * stats->recoilYaw;

        if (g_WeaponRenderer) {
            g_WeaponRenderer->ApplyRecoil(stats->recoilPitch, stats->recoilYaw);
        }

        // Decrease ammo
        inventory[BACKPACK_SLOTS].ammo--;

        // Calculate shoot direction
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 shootOrigin = camera.position;

        // FIXED: Proper raycast for zombie hits
        bool didHit = false;

        if (g_ZombieManager) {
            const auto& zombies = g_ZombieManager->GetZombies();
            float closestDist = 1000.0f;
            Zombie* hitZombie = nullptr;

            for (const auto& zombie : zombies) {
                if (!zombie.isAlive) continue;

                // Raycast to zombie
                Vector3 toZombie = Vector3Subtract(zombie.position, shootOrigin);
                float distance = Vector3Length(toZombie);

                if (distance > 100.0f) continue; // Max range

                Vector3 toZombieNorm = Vector3Normalize(toZombie);
                float alignment = Vector3DotProduct(forward, toZombieNorm);

                // Check if looking at zombie (more lenient check)
                if (alignment > 0.95f) { // Within ~18 degree cone
                    // Check if this is the closest zombie in our aim
                    if (distance < closestDist) {
                        closestDist = distance;
                        hitZombie = const_cast<Zombie*>(&zombie);
                        didHit = true;
                    }
                }
            }

            // Apply damage to closest zombie
            if (hitZombie) {
                g_ZombieManager->DamageZombie(hitZombie->id, stats->damage);
                TraceLog(LOG_INFO, "Hit zombie %d for %.1f damage!", hitZombie->id, stats->damage);
            }
        }

        // Show hit marker when we hit
        if (didHit && g_HUDManager) {
            g_HUDManager->ShowHitMarker();
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
    else if (shootPressed && inventory[BACKPACK_SLOTS].ammo <= 0) {
        // Empty click sound
        if (g_SoundManager) {
            g_SoundManager->PlaySound(SND_EMPTY_CLICK, 0.5f);
        }
    }
}