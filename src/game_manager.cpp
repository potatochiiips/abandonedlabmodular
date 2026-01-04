#include "globals.h"
#include "game_manager.h"
#include "weapon_renderer.h"
#include "hands_renderer.h"
#include "daynight_system.h"
#include "weather_system.h"
#include "zombie_system.h"
#include "rendering.h"
#include "enhanced_map_system.h"
#include "rlgl.h"
#include "vehicle_system.h"
#include "skybox.h"
#include "hud.h"
#include "player.h"

UpgradedGameManager::UpgradedGameManager() {
}

UpgradedGameManager::~UpgradedGameManager() {
    Cleanup();
}

void UpgradedGameManager::Initialize() {
    TraceLog(LOG_INFO, "Initializing Game Manager...");

    // Create subsystems
    mapRenderer = std::make_unique<UpgradedMapRenderer>();
    hudManager = std::make_unique<HUDManager>();
    weaponRenderer = std::make_unique<WeaponRenderer>();
    handsRenderer = std::make_unique<HandsRenderer>();

    // Initialize subsystems
    mapRenderer->Initialize();
    hudManager->Initialize();
    weaponRenderer->Initialize();
    handsRenderer->Initialize();

    // Setup scene
    SetupScene();
    SetupLighting();

    TraceLog(LOG_INFO, "Game Manager initialized");
}

void UpgradedGameManager::SetupScene() {
    // Setup main camera
    mainCamera.camera = camera;
    mainCamera.clearColor = Color{ 135, 206, 235, 255 };
    mainCamera.nearClip = 0.1f;
    mainCamera.farClip = 1000.0f;
    mainCamera.depth = 0;
    mainCamera.cullingMask = -1;

    // Generate world geometry from enhanced map system OR interior
    if (g_MapPlayer.insideInterior && g_EnhancedMapSystem) {
        const Interior* interior = g_EnhancedMapSystem->GetLabInterior();
        if (interior) {
            TraceLog(LOG_INFO, "Loading interior geometry: %s", interior->id.c_str());
            mapRenderer->GenerateInteriorGeometry(*interior);
        }
        else {
            TraceLog(LOG_WARNING, "Interior not found, falling back to world geometry");
            mapRenderer->GenerateEnhancedWorld();
        }
    }
    else if (g_EnhancedMapSystem) {
        TraceLog(LOG_INFO, "Generating world geometry from Enhanced Map System");
        mapRenderer->GenerateEnhancedWorld();
    }
    else {
        mapRenderer->GenerateWorldGeometry(g_MapData);
    }

    // Set equipped weapon
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    weaponRenderer->SetEquippedWeapon(inventory[BACKPACK_SLOTS].itemId);
}

void UpgradedGameManager::SetupLighting() {
    for (auto* light : sceneLights) {
        delete light;
    }
    sceneLights.clear();

    // FIXED: Brighter directional light
    UpgradedLight* mainLight = new UpgradedLight();
    mainLight->type = LIGHT_DIRECTIONAL;
    mainLight->direction = Vector3{ 0.5f, -1.0f, 0.3f };
    mainLight->color = Color{ 255, 250, 220, 255 };
    mainLight->intensity = 1.5f; // INCREASED from 1.0f
    mainLight->castShadows = false;
    mainLight->shadowResolution = 2048;
    sceneLights.push_back(mainLight);

    extern bool isFlashlightOn;
    if (isFlashlightOn) {
        UpgradedLight* flashlight = new UpgradedLight();
        flashlight->type = LIGHT_SPOT;
        flashlight->position = camera.position;
        flashlight->direction = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        flashlight->color = Color{ 255, 250, 220, 255 };
        flashlight->intensity = 2.0f;
        flashlight->range = 25.0f;
        flashlight->spotAngle = 25.0f;
        flashlight->castShadows = false;
        sceneLights.push_back(flashlight);
    }

    // FIXED: Brighter ambient for interiors
    Color ambientColor = Color{ 100, 110, 120, 255 };
    float ambientIntensity = 0.5f; // Default

    if (g_MapPlayer.insideInterior) {
        // MUCH brighter ambient lighting inside
        ambientColor = Color{ 180, 190, 200, 255 };
        ambientIntensity = 0.8f; // Much brighter
    }
    else if (g_DayNightCycle) {
        ambientColor = g_DayNightCycle->GetAmbientColor();
        ambientIntensity = 0.5f;
    }

    if (g_UpgradedPipeline) {
        g_UpgradedPipeline->SetAmbientLight(ambientColor, ambientIntensity);
    }

    // Fog only outside
    bool fogEnabled = false;
    Color fogColor = Color{ 128, 128, 128, 255 };
    float fogDensity = 0.05f;

    if (!g_MapPlayer.insideInterior && g_WeatherSystem && g_WeatherSystem->GetCurrentWeather() != WEATHER_CLEAR) {
        fogEnabled = true;
        fogColor = g_WeatherSystem->GetFogColor();
        fogDensity = g_WeatherSystem->GetFogDensity();
    }

    if (g_UpgradedPipeline) {
        g_UpgradedPipeline->SetFog(fogEnabled, fogColor, fogDensity);
    }
}

void UpgradedGameManager::Update(float deltaTime) {
    UpdateCamera();

    mapRenderer->Update(deltaTime, mainCamera.camera);
    hudManager->Update(deltaTime);
    weaponRenderer->Update(deltaTime, mainCamera.camera);

    UpdateWeaponRendering();

    if (g_DayNightCycle && !sceneLights.empty()) {
        UpgradedLight* sun = sceneLights[0];
        sun->direction = g_DayNightCycle->GetSunDirection();
        sun->color = g_DayNightCycle->GetSunColor();
        sun->intensity = g_DayNightCycle->GetLightIntensity();
    }

    extern bool isFlashlightOn;
    for (auto* light : sceneLights) {
        if (light->type == LIGHT_SPOT) {
            light->position = mainCamera.camera.position;
            light->direction = Vector3Normalize(Vector3Subtract(
                mainCamera.camera.target, mainCamera.camera.position));
        }
    }

    extern float health;
    hudManager->SetLowHealthWarning(health < 30.0f);
}

void UpgradedGameManager::UpdateCamera() {
    mainCamera.camera = camera;
}

void UpgradedGameManager::UpdateWeaponRendering() {
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    extern bool isReloading;
    extern bool isAimingDownSights;

    int equippedItem = inventory[BACKPACK_SLOTS].itemId;

    static int lastEquipped = ITEM_NONE;
    if (equippedItem != lastEquipped) {
        weaponRenderer->SetEquippedWeapon(equippedItem);
        lastEquipped = equippedItem;
    }

    weaponRenderer->SetAimingDownSights(isAimingDownSights);

    if (isReloading) {
        static bool wasReloading = false;
        if (!wasReloading) {
            weaponRenderer->PlayReloadAnimation();
        }
        wasReloading = true;
    }
    else {
        static bool wasReloading = false;
        wasReloading = false;
    }
}

void UpgradedGameManager::CollectRenderables(std::vector<MeshRenderer*>& renderers) {
    std::vector<MeshRenderer*> mapRenderers = mapRenderer->GetActiveRenderers();
    renderers.insert(renderers.end(), mapRenderers.begin(), mapRenderers.end());

    MeshRenderer* weapon = weaponRenderer->GetWeaponRenderer();
    if (weapon) {
        renderers.push_back(weapon);
    }
}

void UpgradedGameManager::Render() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    BeginMode3D(mainCamera.camera);

    // Draw skybox first - ONLY if outside
    if (!g_MapPlayer.insideInterior && g_SkyboxManager) {
        g_SkyboxManager->Draw(camera);
    }

    // Draw map geometry
    std::vector<MeshRenderer*> renderers = mapRenderer->GetActiveRenderers();
    for (auto* renderer : renderers) {
        if (!renderer->enabled) continue;

        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(renderer->transform));

        if (renderer->material) {
            BeginShaderMode(renderer->material->GetShader());
            DrawMesh(renderer->mesh, LoadMaterialDefault(), MatrixIdentity());
            EndShaderMode();
        }
        else {
            DrawMesh(renderer->mesh, LoadMaterialDefault(), MatrixIdentity());
        }

        rlPopMatrix();
    }

    // Draw zombies
    if (g_ZombieManager) {
        g_ZombieManager->Draw(camera);
    }

    // Draw vehicles
    if (g_VehicleManager) {
        g_VehicleManager->Draw();
    }

    // Draw waypoints
    g_WaypointManager.DrawIn3D(playerPosition, 100.0f);

    EndMode3D();

    // Draw weather effects AFTER 3D
    if (g_WeatherSystem) {
        g_WeatherSystem->Draw(camera);
    }

    // FIXED: Draw weapon in first person AFTER 3D scene
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    extern float pistolRecoilPitch, pistolRecoilYaw;
    extern bool inventoryOpen, isCraftingOpen, isMapOpen;

    bool isAnyMenuOpen = (inventoryOpen || isCraftingOpen || isMapOpen);

    if (!isAnyMenuOpen && inventory[BACKPACK_SLOTS].itemId != ITEM_NONE) {
        // Draw weapon using the weapon renderer directly
        BeginMode3D(mainCamera.camera);

        // Get weapon model and draw it manually
        if (g_ModelManager) {
            ModelID modelId = GetModelIDFromItem(inventory[BACKPACK_SLOTS].itemId);

            Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
            Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
            Vector3 up = Vector3Normalize(camera.up);

            // Weapon position in first person
            float handDistance = 0.5f;
            float handRightOffset = 0.2f;
            float handDownOffset = -0.25f;

            Vector3 weaponPos = camera.position;
            weaponPos = Vector3Add(weaponPos, Vector3Scale(forward, handDistance));
            weaponPos = Vector3Add(weaponPos, Vector3Scale(right, handRightOffset));
            weaponPos = Vector3Add(weaponPos, Vector3Scale(up, handDownOffset));

            // Apply recoil
            if (pistolRecoilPitch > 0.01f || pistolRecoilYaw > 0.01f) {
                weaponPos = Vector3Add(weaponPos, Vector3Scale(forward, -pistolRecoilPitch * 0.002f));
                weaponPos = Vector3Add(weaponPos, Vector3Scale(up, pistolRecoilPitch * 0.003f));
            }

            // Draw weapon
            g_ModelManager->DrawModel(modelId, weaponPos, forward, right, up, WHITE);
        }

        EndMode3D();
    }

    // Draw HUD
    extern float health, stamina, hunger, thirst;
    extern float flashlightBattery;
    extern bool isFlashlightOn;
    extern float yaw;

    hudManager->DrawPlayerStats(screenW, screenH, health, stamina, hunger, thirst);
    hudManager->DrawWeaponInfo(screenW, screenH, inventory[BACKPACK_SLOTS]);
    hudManager->DrawFlashlightStatus(screenW, screenH, flashlightBattery, isFlashlightOn);
    hudManager->DrawCrosshair(screenW, screenH, isAimingDownSights);

    // FIXED: Draw hit marker
    hudManager->DrawHitMarker(screenW, screenH);

    // Damage vignette
    static float lastHealth = 100.0f;
    if (health < lastHealth) {
        hudManager->ShowDamageIndicator(lastHealth - health);
    }
    lastHealth = health;

    hudManager->DrawDamageVignette(screenW, screenH, 1.0f);
}

void UpgradedGameManager::Cleanup() {
    for (auto* light : sceneLights) {
        delete light;
    }
    sceneLights.clear();

    TraceLog(LOG_INFO, "Upgraded Game Manager cleaned up");
}
