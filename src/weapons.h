#pragma once
#include "globals.h"

// New weapon item IDs (add to your items system)
#define ITEM_M16 9
#define ITEM_M16_MAG 10

// Weapon animation states
enum WeaponAnimState {
    ANIM_IDLE,
    ANIM_RELOAD,
    ANIM_SHOOT,
    ANIM_ADS, // Aim Down Sights
    ANIM_LOWERED
};

// Weapon definition
struct WeaponStats {
    int weaponId;
    int magazineId;
    int magCapacity;
    float fireRate; // Seconds between shots
    float reloadTime; // Seconds to reload
    float damage;
    float recoilPitch;
    float recoilYaw;
    float accuracy; // 0.0 to 1.0
    float adsAccuracyBonus; // Additional accuracy when ADS
    bool isAutomatic;
};

// Weapon state
struct WeaponState {
    WeaponAnimState animState;
    float animTimer;
    bool isADS; // Is aiming down sights
    float adsProgress; // 0.0 to 1.0 for smooth transition
    Vector3 position;
    Vector3 rotation;
    Vector3 recoilOffset;
};

// Weapon definitions
class WeaponSystem {
public:
    WeaponSystem() {
        InitializeWeapons();
    }

    void InitializeWeapons() {
        // Pistol
        WeaponStats pistol;
        pistol.weaponId = ITEM_PISTOL;
        pistol.magazineId = ITEM_MAG;
        pistol.magCapacity = 15;
        pistol.fireRate = 0.2f;
        pistol.reloadTime = 1.5f;
        pistol.damage = 25.0f;
        pistol.recoilPitch = 3.0f;
        pistol.recoilYaw = 0.5f;
        pistol.accuracy = 0.75f;
        pistol.adsAccuracyBonus = 0.2f;
        pistol.isAutomatic = false;
        weapons[ITEM_PISTOL] = pistol;

        // M16
        WeaponStats m16;
        m16.weaponId = ITEM_M16;
        m16.magazineId = ITEM_M16_MAG;
        m16.magCapacity = 30;
        m16.fireRate = 0.08f; // 3-round burst
        m16.reloadTime = 2.2f;
        m16.damage = 35.0f;
        m16.recoilPitch = 2.0f;
        m16.recoilYaw = 0.3f;
        m16.accuracy = 0.85f;
        m16.adsAccuracyBonus = 0.15f;
        m16.isAutomatic = true;
        weapons[ITEM_M16] = m16;
    }

    WeaponStats* GetWeaponStats(int weaponId) {
        if (weapons.find(weaponId) != weapons.end()) {
            return &weapons[weaponId];
        }
        return nullptr;
    }

    // Update weapon animations
    void UpdateWeapon(WeaponState& state, float deltaTime) {
        // Update animation timer
        if (state.animTimer > 0.0f) {
            state.animTimer -= deltaTime;
            if (state.animTimer <= 0.0f) {
                state.animState = state.isADS ? ANIM_ADS : ANIM_IDLE;
            }
        }

        // Smooth ADS transition
        float adsTarget = state.isADS ? 1.0f : 0.0f;
        float adsSpeed = 5.0f;
        if (state.adsProgress < adsTarget) {
            state.adsProgress = fminf(state.adsProgress + deltaTime * adsSpeed, adsTarget);
        }
        else if (state.adsProgress > adsTarget) {
            state.adsProgress = fmaxf(state.adsProgress - deltaTime * adsSpeed, adsTarget);
        }

        // Smooth recoil recovery
        state.recoilOffset.x *= 0.9f;
        state.recoilOffset.y *= 0.9f;
        state.recoilOffset.z *= 0.9f;
    }

    // Calculate weapon position based on animation state
    Vector3 CalculateWeaponPosition(const Camera3D& camera, const WeaponState& state, bool isRifle) {
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        Vector3 up = Vector3Normalize(camera.up);

        float handDistance = 0.45f - (state.adsProgress * 0.2f);
        float handRightOffset = 0.22f - (state.adsProgress * 0.15f);
        float handDownOffset = -0.25f + (state.adsProgress * 0.1f);

        if (isRifle) {
            handDistance = 0.5f - (state.adsProgress * 0.25f);
            handRightOffset = 0.15f - (state.adsProgress * 0.15f);
            handDownOffset = -0.2f + (state.adsProgress * 0.15f);
        }

        Vector3 pos = camera.position;
        pos = Vector3Add(pos, Vector3Scale(forward, handDistance));
        pos = Vector3Add(pos, Vector3Scale(right, handRightOffset));
        pos = Vector3Add(pos, Vector3Scale(up, handDownOffset));

        // Add recoil offset
        pos = Vector3Add(pos, state.recoilOffset);

        // Add animation bobbing
        if (state.animState == ANIM_IDLE && state.adsProgress < 0.5f) {
            float time = (float)GetTime();  // Cast to float to avoid warning
            float bob = sinf(time * 2.0f) * 0.005f;
            pos.y += bob;
        }

        return pos;
    }

private:
    std::map<int, WeaponStats> weapons;
};

// Draw enhanced pistol model
void DrawEnhancedPistol(Vector3 basePos, Vector3 forward, Vector3 right, Vector3 up, const WeaponState& state);

// Draw M16 model
void DrawM16Rifle(Vector3 basePos, Vector3 forward, Vector3 right, Vector3 up, const WeaponState& state);

// Draw left hand on weapon - FIXED SIGNATURE (6 parameters)
void DrawLeftHandOnWeapon(Vector3 weaponPos, Vector3 forward, Vector3 right, Vector3 up, bool isRifle, float adsProgress);

// Draw idle hands animation
void DrawIdleHands(const Camera3D& camera, float time);

// Global weapon system (DECLARED HERE, DEFINED IN weapons.cpp)
extern WeaponSystem g_WeaponSystem;
extern WeaponState g_CurrentWeaponState;