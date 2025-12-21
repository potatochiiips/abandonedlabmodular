#pragma once
#include "globals.h"

// Weapon item IDs
#define ITEM_M16 9
#define ITEM_M16_MAG 10
#define ITEM_KNIFE 11

// Weapon animation states
enum WeaponAnimState {
    ANIM_IDLE,
    ANIM_RELOAD,
    ANIM_SHOOT,
    ANIM_ADS,
    ANIM_LOWERED,
    ANIM_SLASH // For melee weapons
};

// Weapon definition
struct WeaponStats {
    int weaponId;
    int magazineId;
    int magCapacity;
    float fireRate;
    float reloadTime;
    float damage;
    float recoilPitch;
    float recoilYaw;
    float accuracy;
    float adsAccuracyBonus;
    bool isAutomatic;
    bool isMelee;
};

// Weapon state
struct WeaponState {
    WeaponAnimState animState;
    float animTimer;
    bool isADS;
    float adsProgress;
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
        pistol.isMelee = false;
        weapons[ITEM_PISTOL] = pistol;

        // M16
        WeaponStats m16;
        m16.weaponId = ITEM_M16;
        m16.magazineId = ITEM_M16_MAG;
        m16.magCapacity = 30;
        m16.fireRate = 0.08f;
        m16.reloadTime = 2.2f;
        m16.damage = 35.0f;
        m16.recoilPitch = 2.0f;
        m16.recoilYaw = 0.3f;
        m16.accuracy = 0.85f;
        m16.adsAccuracyBonus = 0.15f;
        m16.isAutomatic = true;
        m16.isMelee = false;
        weapons[ITEM_M16] = m16;

        // Knife
        WeaponStats knife;
        knife.weaponId = ITEM_KNIFE;
        knife.magazineId = 0;
        knife.magCapacity = 0;
        knife.fireRate = 0.5f; // Swing speed
        knife.reloadTime = 0.0f;
        knife.damage = 50.0f;
        knife.recoilPitch = 0.0f;
        knife.recoilYaw = 0.0f;
        knife.accuracy = 1.0f; // Melee always hits at close range
        knife.adsAccuracyBonus = 0.0f;
        knife.isAutomatic = false;
        knife.isMelee = true;
        weapons[ITEM_KNIFE] = knife;
    }

    WeaponStats* GetWeaponStats(int weaponId) {
        if (weapons.find(weaponId) != weapons.end()) {
            return &weapons[weaponId];
        }
        return nullptr;
    }

    void UpdateWeapon(WeaponState& state, float deltaTime) {
        if (state.animTimer > 0.0f) {
            state.animTimer -= deltaTime;
            if (state.animTimer <= 0.0f) {
                state.animState = state.isADS ? ANIM_ADS : ANIM_IDLE;
            }
        }

        float adsTarget = state.isADS ? 1.0f : 0.0f;
        float adsSpeed = 5.0f;
        if (state.adsProgress < adsTarget) {
            state.adsProgress = fminf(state.adsProgress + deltaTime * adsSpeed, adsTarget);
        }
        else if (state.adsProgress > adsTarget) {
            state.adsProgress = fmaxf(state.adsProgress - deltaTime * adsSpeed, adsTarget);
        }

        state.recoilOffset.x *= 0.9f;
        state.recoilOffset.y *= 0.9f;
        state.recoilOffset.z *= 0.9f;
    }

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

        pos = Vector3Add(pos, state.recoilOffset);

        if (state.animState == ANIM_IDLE && state.adsProgress < 0.5f) {
            float time = (float)GetTime();
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

// Draw knife model
void DrawKnife(Vector3 basePos, Vector3 forward, Vector3 right, Vector3 up, const WeaponState& state);

// Draw left hand on weapon
void DrawLeftHandOnWeapon(Vector3 weaponPos, Vector3 forward, Vector3 right, Vector3 up, bool isRifle, float adsProgress);

// Draw idle hands animation
void DrawIdleHands(const Camera3D& camera, float time);

// Global weapon system
extern WeaponSystem g_WeaponSystem;
extern WeaponState g_CurrentWeaponState;