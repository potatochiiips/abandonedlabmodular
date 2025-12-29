#pragma once
#include "globals.h"
#include "rendering.h"
#include "map.h"
#include "hud.h"
#include "weapon_renderer.h"
#include "weapons.h"
#include "rendering_impl.h"
#include <vector>
#include <memory>

class WeaponRenderer;
struct WeaponState;
enum WeaponAnimState;
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