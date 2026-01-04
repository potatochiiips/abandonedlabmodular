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
        if (g_WeaponRenderer) {
            g_WeaponRenderer->ApplyRecoil(stats->recoilPitch, stats->recoilYaw);
        }

        // Decrease ammo
        inventory[BACKPACK_SLOTS].ammo--;

        // Calculate shoot direction
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 shootOrigin = camera.position;

        // Raycast for hits
        bool didHit = false;

        // Check zombie hits - FIXED: Proper raycast
        if (g_ZombieManager) {
            const auto& zombies = g_ZombieManager->GetZombies();
            float closestDist = 100.0f;
            Zombie* hitZombie = nullptr;

            for (const auto& zombie : zombies) {
                if (!zombie.isAlive) continue;

                // Check if ray hits zombie
                float dist = Vector3Distance(shootOrigin, zombie.position);
                if (dist > closestDist) continue;

                // Simple sphere collision check
                Vector3 toZombie = Vector3Subtract(zombie.position, shootOrigin);
                float dotProduct = Vector3DotProduct(Vector3Normalize(toZombie), forward);

                if (dotProduct > 0.99f && dist < 50.0f) { // Within 50 units and looking at zombie
                    closestDist = dist;
                    hitZombie = const_cast<Zombie*>(&zombie);
                    didHit = true;
                }
            }

            // Apply damage to closest zombie
            if (hitZombie) {
                g_ZombieManager->DamageZombie(hitZombie->id, stats->damage);
                TraceLog(LOG_INFO, "Hit zombie %d for %.1f damage!", hitZombie->id, stats->damage);
            }
        }

        // FIXED: Show hit marker when we hit something
        if (didHit && g_HUDManager) {
            g_HUDManager->ShowHitMarker();
            TraceLog(LOG_INFO, "Showing hit marker!");
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