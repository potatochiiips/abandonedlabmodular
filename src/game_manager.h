#pragma once
#include "globals.h"
#include "rendering.h"  // ADD THIS - must be before using UpgradedLight
#include <vector>
#include <memory>

// Forward declarations
class UpgradedMapRenderer;
class HUDManager;
class WeaponRenderer;
class HandsRenderer;
struct WeaponState;
enum WeaponAnimState;

#include "map.h"
#include "hud.h"
#include "weapon_renderer.h"
#include "weapons.h"
// ============================================================================
class UpgradedGameManager {
public:
    UpgradedGameManager();
    ~UpgradedGameManager();

    void Initialize();
    void Update(float deltaTime);
    void Render();
    void Cleanup();

private:
    // Core systems
    std::unique_ptr<UpgradedMapRenderer> mapRenderer;
    std::unique_ptr<HUDManager> hudManager;
    std::unique_ptr<WeaponRenderer> weaponRenderer;
    std::unique_ptr<HandsRenderer> handsRenderer;

    // Scene data
    std::vector<UpgradedLight*> sceneLights;
    UpgradedCamera mainCamera;

    void SetupScene();
    void SetupLighting();
    void UpdateCamera();
    void UpdateWeaponRendering();
    void CollectRenderables(std::vector<MeshRenderer*>& renderers);
};