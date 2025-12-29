#pragma once
#include "globals.h"
#include "rendering.h"
#include "weapons.h"
#include "texture_manager.h"
#include "model_manager.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include <rlgl.h>
#include <memory>

// ============================================================================
// Upgraded-STYLE WEAPON RENDERING SYSTEM
// ============================================================================

class WeaponRenderer {
public:
    WeaponRenderer();
    ~WeaponRenderer();
    
    void Initialize();
    void Update(float deltaTime, const Camera3D& camera);
    
    // Get weapon renderer for pipeline
    MeshRenderer* GetWeaponRenderer();
    
    // Weapon actions
    void SetEquippedWeapon(int itemId);
    void PlayShootAnimation();
    void PlayReloadAnimation();
    void SetAimingDownSights(bool aiming);
    void ApplyRecoil(float pitch, float yaw);
    
    // Item rendering
    void RenderItemInHand(const Camera3D& camera, int itemId);
    
private:
    std::unique_ptr<MeshRenderer> weaponRenderer;
    std::shared_ptr<UpgradedMaterial> weaponMaterial;
    
    // Animation state
    WeaponState currentState;
    Vector3 basePosition;
    Vector3 aimPosition;
    Vector3 currentRecoil;
    float viewmodelFOV;
    
    // Weapon sway
    Vector3 swayAmount;
    Vector3 swayVelocity;
    float swaySmooth;
    
    // Bob animation
    float bobTimer;
    Vector3 bobAmount;
    
    // Muzzle flash
    bool showMuzzleFlash;
    float muzzleFlashTimer;
    
    void UpdateWeaponPosition(const Camera3D& camera);
    void UpdateWeaponSway(const Camera3D& camera);
    void UpdateWeaponBob(float deltaTime);
    void UpdateAnimations(float deltaTime);
    
    Vector3 CalculateWeaponTransform(const Camera3D& camera);
    Matrix CreateViewmodelMatrix(const Camera3D& camera, Vector3 offset);
    
    void DrawMuzzleFlash(const Camera3D& camera);
};