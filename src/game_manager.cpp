#include "globals.h"
#include "game_manager.h"
#include "daynight_system.h"
#include "weather_system.h"
#include "zombie_system.h"

UpgradedGameManager::UpgradedGameManager() {
}

UpgradedGameManager::~UpgradedGameManager() {
    Cleanup();
}

void UpgradedGameManager::Initialize() {
    TraceLog(LOG_INFO, "Initializing Game Manager...");

    // Initialize rendering pipeline
    InitializeUpgradedPipeline();

    // Create subsystems
    mapRenderer = std::make_unique<UpgradedMapRenderer>();
    hudManager = std::make_unique<HUDManager>();
    weaponRenderer = std::make_unique<WeaponRenderer>();
    handsRenderer = std::make_unique<HandsRenderer>();

    // Initialize subsystems
    mapRenderer->Initialize();
    hudManager->Initialize();
    weaponRenderer->Initialize();

    // Setup scene
    SetupScene();
    SetupLighting();

    TraceLog(LOG_INFO, "Game Manager initialized");
}

void UpgradedGameManager::SetupScene() {
    // Setup main camera
    mainCamera.camera = camera; // Use existing global camera
    mainCamera.clearColor = Color{ 135, 206, 235, 255 }; // Sky blue
    mainCamera.nearClip = 0.1f;
    mainCamera.farClip = 1000.0f;
    mainCamera.depth = 0;
    mainCamera.cullingMask = -1;

    // Generate map geometry
    if (g_MapPlayer.insideInterior) {
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
    mainLight->castShadows = true;
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
    Color ambientColor = Color{ 54, 58, 66, 255 };
    if (g_DayNightCycle) {
        ambientColor = g_DayNightCycle->GetAmbientColor();
    }
    g_UpgradedPipeline->SetAmbientLight(ambientColor, 0.3f);

    // Set fog
    bool fogEnabled = false;
    Color fogColor = Color{ 128, 128, 128, 255 };
    float fogDensity = 0.05f;

    if (g_WeatherSystem && g_WeatherSystem->GetCurrentWeather() != WEATHER_CLEAR) {
        fogEnabled = true;
        fogColor = g_WeatherSystem->GetFogColor();
        fogDensity = g_WeatherSystem->GetFogDensity();
    }

    g_UpgradedPipeline->SetFog(fogEnabled, fogColor, fogDensity);
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
    // Sync main camera with global camera
    mainCamera.camera = camera;
}

void UpgradedGameManager::UpdateWeaponRendering() {
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    extern bool isReloading;
    extern bool isAimingDownSights;

    int equippedItem = inventory[BACKPACK_SLOTS].itemId;

    // Handle weapon changes
    static int lastEquipped = ITEM_NONE;
    if (equippedItem != lastEquipped) {
        weaponRenderer->SetEquippedWeapon(equippedItem);
        lastEquipped = equippedItem;
    }

    // Update weapon state
    weaponRenderer->SetAimingDownSights(isAimingDownSights);

    // Handle reload animation
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

    // Add zombie renderers (if zombie system is active)
    // This would require converting zombie drawing to Upgraded-style
}

void UpgradedGameManager::Render() {
    // Collect all renderables
    std::vector<MeshRenderer*> allRenderers;
    CollectRenderables(allRenderers);

    // Render using Upgraded pipeline
    g_UpgradedPipeline->Render(allRenderers, sceneLights, mainCamera);

    // Draw HUD on top (2D overlay)
    extern float health, stamina, hunger, thirst;
    extern float flashlightBattery;
    extern bool isFlashlightOn;
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    extern float yaw;
    extern bool isAimingDownSights;

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    hudManager->DrawPlayerStats(screenW, screenH, health, stamina, hunger, thirst);
    hudManager->DrawWeaponInfo(screenW, screenH, inventory[BACKPACK_SLOTS]);
    hudManager->DrawFlashlightStatus(screenW, screenH, flashlightBattery, isFlashlightOn);
    hudManager->DrawCrosshair(screenW, screenH, isAimingDownSights);
    hudManager->DrawCompass(screenW, screenH, yaw);
    hudManager->DrawHitMarker(screenW, screenH);

    // Damage vignette
    static float lastHealth = 100.0f;
    if (health < lastHealth) {
        hudManager->ShowDamageIndicator(lastHealth - health);
    }
    lastHealth = health;

    hudManager->DrawDamageVignette(screenW, screenH, 1.0f);

    // Draw hands for non-weapon items
    if (inventory[BACKPACK_SLOTS].itemId != ITEM_PISTOL &&
        inventory[BACKPACK_SLOTS].itemId != ITEM_M16 &&
        inventory[BACKPACK_SLOTS].itemId != ITEM_KNIFE) {
        handsRenderer->DrawHands(mainCamera.camera, false);
    }
}

void UpgradedGameManager::Cleanup() {
    // Cleanup scene lights
    for (auto* light : sceneLights) {
        delete light;
    }
    sceneLights.clear();

    // Subsystems cleanup automatically via unique_ptr

    // Cleanup rendering pipeline
    CleanupUpgradedPipeline();

    TraceLog(LOG_INFO, "Upgraded Game Manager cleaned up");
}
