#include "weapons.h"
#include "model_manager.h"
#include "globals.h"
#include "rlgl.h"
#include "hud.h"
#include "zombie_system.h"
#include "rendering.h"
#include "weapon_renderer.h"

// ============================================================================
// WEAPON SHOOTING INTEGRATION EXAMPLE
// ============================================================================

void HandleWeaponShooting() {
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    extern UpgradedWeaponRenderer* g_UpgradedWeaponRenderer;
    extern UpgradedHUDManager* g_UpgradedHUD;

    bool shootPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (shootPressed && inventory[BACKPACK_SLOTS].ammo > 0) {
        // Play weapon animation
        g_UpgradedWeaponRenderer->PlayShootAnimation();

        // Apply recoil
        WeaponStats* stats = g_WeaponSystem.GetWeaponStats(inventory[BACKPACK_SLOTS].itemId);
        if (stats) {
            g_UpgradedWeaponRenderer->ApplyRecoil(stats->recoilPitch, stats->recoilYaw);
        }

        // Decrease ammo
        inventory[BACKPACK_SLOTS].ammo--;

        // Raycast for hits
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

        // Check zombie hits
        if (g_ZombieManager) {
            Zombie* hitZombie = g_ZombieManager->GetZombieAt(camera.position, 2.0f);
            if (hitZombie) {
                g_ZombieManager->DamageZombie(hitZombie->id, stats->damage);
                g_UpgradedHUD->ShowHitMarker();
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
