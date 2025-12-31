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

    // Generate world geometry from enhanced map system
    if (g_EnhancedMapSystem && !g_MapPlayer.insideInterior) {
        TraceLog(LOG_INFO, "Generating world geometry from Enhanced Map System");
        mapRenderer->GenerateEnhancedWorld();
    }
    else if (g_MapPlayer.insideInterior) {
        const Interior* interior = GetInterior(g_MapData, g_MapPlayer.currentInteriorId);
        if (interior) {
            mapRenderer->GenerateInteriorGeometry(*interior);
        }
    }
    else {
        mapRenderer->GenerateWorldGeometry(g_MapData);
    }

    // Set equipped weapon
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    weaponRenderer->SetEquippedWeapon(inventory[BACKPACK_SLOTS].itemId);
}

void UpgradedGameManager::SetupLighting() {
    // Clear existing lights
    for (auto* light : sceneLights) {
        delete light;
    }
    sceneLights.clear();

    // Main directional light (sun/moon)
    UpgradedLight* mainLight = new UpgradedLight();
    mainLight->type = LIGHT_DIRECTIONAL;
    mainLight->direction = Vector3{ 0.5f, -1.0f, 0.3f };
    mainLight->color = Color{ 255, 250, 220, 255 };
    mainLight->intensity = 1.0f;
    mainLight->castShadows = false; // Disable shadows for performance
    mainLight->shadowResolution = 2048;
    sceneLights.push_back(mainLight);

    // Flashlight (spot light)
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

    // Set ambient lighting
    Color ambientColor = Color{ 100, 110, 120, 255 };
    if (g_DayNightCycle) {
        ambientColor = g_DayNightCycle->GetAmbientColor();
    }

    if (g_UpgradedPipeline) {
        g_UpgradedPipeline->SetAmbientLight(ambientColor, 0.5f);
    }

    // Set fog
    bool fogEnabled = false;
    Color fogColor = Color{ 128, 128, 128, 255 };
    float fogDensity = 0.05f;

    if (g_WeatherSystem && g_WeatherSystem->GetCurrentWeather() != WEATHER_CLEAR) {
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

    // Update subsystems
    mapRenderer->Update(deltaTime, mainCamera.camera);
    hudManager->Update(deltaTime);
    weaponRenderer->Update(deltaTime, mainCamera.camera);

    UpdateWeaponRendering();

    // Update lighting based on day/night cycle
    if (g_DayNightCycle && !sceneLights.empty()) {
        UpgradedLight* sun = sceneLights[0];
        sun->direction = g_DayNightCycle->GetSunDirection();
        sun->color = g_DayNightCycle->GetSunColor();
        sun->intensity = g_DayNightCycle->GetLightIntensity();
    }

    // Update flashlight position
    extern bool isFlashlightOn;
    for (auto* light : sceneLights) {
        if (light->type == LIGHT_SPOT) {
            light->position = mainCamera.camera.position;
            light->direction = Vector3Normalize(Vector3Subtract(
                mainCamera.camera.target, mainCamera.camera.position));
        }
    }

    // Check for low health warning
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
    // Collect map geometry
    std::vector<MeshRenderer*> mapRenderers = mapRenderer->GetActiveRenderers();
    renderers.insert(renderers.end(), mapRenderers.begin(), mapRenderers.end());

    // Add weapon renderer
    MeshRenderer* weapon = weaponRenderer->GetWeaponRenderer();
    if (weapon) {
        renderers.push_back(weapon);
    }
}

void UpgradedGameManager::Render() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // FIXED: Use traditional rendering instead of Upgraded pipeline
    // The Upgraded pipeline is too complex and causing white screen issues

    BeginMode3D(mainCamera.camera);

    // Draw skybox first
    if (g_SkyboxManager) {
        g_SkyboxManager->Draw(camera);
    }

    // FIXED: Draw world using traditional method
    // Draw map geometry directly
    std::vector<MeshRenderer*> renderers = mapRenderer->GetActiveRenderers();
    for (auto* renderer : renderers) {
        if (!renderer->enabled) continue;

        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(renderer->transform));

        // Draw mesh with material colors
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

    // Draw weather effects after 3D rendering
    if (g_WeatherSystem) {
        g_WeatherSystem->Draw(camera);
    }

    // FIXED: Draw HUD elements
    extern float health, stamina, hunger, thirst;
    extern float flashlightBattery;
    extern bool isFlashlightOn;
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    extern float yaw;

    hudManager->DrawPlayerStats(screenW, screenH, health, stamina, hunger, thirst);
    hudManager->DrawWeaponInfo(screenW, screenH, inventory[BACKPACK_SLOTS]);
    hudManager->DrawFlashlightStatus(screenW, screenH, flashlightBattery, isFlashlightOn);
    hudManager->DrawCrosshair(screenW, screenH, isAimingDownSights);

    // Damage vignette
    static float lastHealth = 100.0f;
    if (health < lastHealth) {
        hudManager->ShowDamageIndicator(lastHealth - health);
    }
    lastHealth = health;

    hudManager->DrawDamageVignette(screenW, screenH, 1.0f);
}

void UpgradedGameManager::Cleanup() {
    // Cleanup scene lights
    for (auto* light : sceneLights) {
        delete light;
    }
    sceneLights.clear();

    TraceLog(LOG_INFO, "Upgraded Game Manager cleaned up");
}
