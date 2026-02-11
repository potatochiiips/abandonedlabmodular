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
    // Setup main camera from global
    mainCamera.position = camera.position;
    mainCamera.target = camera.target;
    mainCamera.up = camera.up;
    mainCamera.fovy = camera.fovy;
    mainCamera.projection = CAMERA_PERSPECTIVE;

    // Check if we're in test zone
    extern Vector3 playerPosition;
    bool inTestZone = (playerPosition.x > 450 && playerPosition.x < 550 && 
                       playerPosition.z > 450 && playerPosition.z < 550);

    if (inTestZone) {
        TraceLog(LOG_INFO, "Setting up TEST ZONE with all models...");
        CreateTestZone();
        return;
    }

    // CRITICAL: Generate map data first if empty
    if (g_MapData.tiles.empty()) {
        TraceLog(LOG_INFO, "Map data empty, generating...");
        GenerateMapData(g_MapData);
        TraceLog(LOG_INFO, "Map data generated: %d x %d tiles", g_MapData.width, g_MapData.height);
    }

    // Generate world geometry from enhanced map system OR fallback
    if (g_EnhancedMapSystem) {
        TraceLog(LOG_INFO, "Checking Enhanced Map System...");
        const Interior* interior = nullptr;
        
        if (g_MapPlayer.insideInterior) {
            interior = g_EnhancedMapSystem->GetLabInterior();
            if (interior) {
                TraceLog(LOG_INFO, "Loading interior geometry: %s", interior->id.c_str());
                mapRenderer->GenerateInteriorGeometry(*interior);
                TraceLog(LOG_INFO, "Interior loaded successfully");
            }
            else {
                TraceLog(LOG_WARNING, "Interior not found");
                g_MapPlayer.insideInterior = false;
            }
        }
        
        // Always generate world if not in interior
        if (!g_MapPlayer.insideInterior) {
            TraceLog(LOG_INFO, "Generating world from Enhanced Map System");
            mapRenderer->GenerateEnhancedWorld();
        }
    }
    else {
        TraceLog(LOG_INFO, "Enhanced Map System not available, using standard world generation");
        mapRenderer->GenerateWorldGeometry(g_MapData);
    }

    // Verify renderers were created
    std::vector<MeshRenderer*> activeRenderers = mapRenderer->GetActiveRenderers();
    TraceLog(LOG_INFO, "SetupScene complete - %d renderers created", (int)activeRenderers.size());

    if (activeRenderers.empty()) {
        TraceLog(LOG_WARNING, "WARNING: No renderers created! Creating fallback geometry...");
        CreateFallbackGeometry();
    }

    // Set equipped weapon
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    weaponRenderer->SetEquippedWeapon(inventory[BACKPACK_SLOTS].itemId);
}

void UpgradedGameManager::CreateTestZone() {
    TraceLog(LOG_INFO, "Creating TEST ZONE...");
    
    // Create a large grass field
    auto groundRenderer = std::make_unique<MeshRenderer>();
    groundRenderer->mesh = GenMeshPlane(500.0f, 500.0f, 64, 64);
    groundRenderer->transform = MatrixTranslate(500.0f, 0.0f, 500.0f);
    groundRenderer->castShadows = false;
    groundRenderer->receiveShadows = true;
    groundRenderer->enabled = true;
    groundRenderer->tint = Color{ 100, 160, 80, 255 }; // Green grass color
    
    mapRenderer->GetWorldRenderers().push_back(std::move(groundRenderer));

    // All available model IDs from ModelID enum
    ModelID modelIds[] = {
        MODEL_PISTOL, MODEL_M16, MODEL_FLASHLIGHT, MODEL_WATER_BOTTLE,
        MODEL_LAB_KEY, MODEL_WOOD, MODEL_STONE, MODEL_POTATO_CHIPS,
        MODEL_MAGAZINE, MODEL_M16_MAGAZINE, MODEL_KNIFE, MODEL_CRYOPOD,
        MODEL_CONSOLE_TERMINAL, MODEL_BED, MODEL_DESK, MODEL_CHAIR,
        MODEL_TABLE, MODEL_SHELF, MODEL_LOCKER, MODEL_CABINET,
        MODEL_BENCH, MODEL_SERVER_RACK, MODEL_BROKEN_GLASS, MODEL_DEBRIS_CONCRETE,
        MODEL_DEBRIS_METAL, MODEL_BARREL, MODEL_CRATE
    };
    
    int modelCount = sizeof(modelIds) / sizeof(modelIds[0]);
    
    // Add all available models in a grid pattern (6x5 grid, center of zone)
    const int gridCols = 6;
    const int gridRows = 5;
    const float spacing = 40.0f;
    const float startX = 500.0f - (gridCols * spacing / 2.0f);
    const float startZ = 500.0f - (gridRows * spacing / 2.0f);

    int displayedCount = 0;

    for (int row = 0; row < gridRows && displayedCount < modelCount; row++) {
        for (int col = 0; col < gridCols && displayedCount < modelCount; col++) {
            float x = startX + (col * spacing);
            float z = startZ + (row * spacing);
            
            ModelID modelId = modelIds[displayedCount];
            
            if (g_ModelManager && g_ModelManager->IsLoaded(modelId)) {
                Model model = g_ModelManager->GetModel(modelId);
                
                if (model.meshCount > 0) {
                    for (int meshIdx = 0; meshIdx < model.meshCount; meshIdx++) {
                        auto modelRenderer = std::make_unique<MeshRenderer>();
                        modelRenderer->mesh = model.meshes[meshIdx];
                        modelRenderer->transform = MatrixTranslate(x, 1.0f, z);
                        modelRenderer->enabled = true;
                        modelRenderer->tint = WHITE;
                        
                        mapRenderer->GetWorldRenderers().push_back(std::move(modelRenderer));
                    }
                    
                    TraceLog(LOG_INFO, "Added model %d to test zone at (%.1f, %.1f)", 
                             displayedCount, x, z);
                }
            }
            else {
                TraceLog(LOG_WARNING, "Model %d not loaded, skipping", displayedCount);
            }
            
            displayedCount++;
        }
    }

    TraceLog(LOG_INFO, "TEST ZONE created with %d/%d models displayed", displayedCount, modelCount);
}

void UpgradedGameManager::CreateFallbackGeometry() {
    TraceLog(LOG_WARNING, "Creating fallback geometry - large ground plane");
    
    // Create a simple ground plane as fallback
    auto groundRenderer = std::make_unique<MeshRenderer>();
    groundRenderer->mesh = GenMeshPlane(256.0f, 256.0f, 32, 32);
    groundRenderer->transform = MatrixTranslate(128.0f, 0.0f, 128.0f);
    groundRenderer->castShadows = false;
    groundRenderer->receiveShadows = true;
    groundRenderer->enabled = true;
    
    mapRenderer->GetWorldRenderers().push_back(std::move(groundRenderer));
    
    TraceLog(LOG_WARNING, "Fallback geometry created");
}

void UpgradedGameManager::SetupLighting() {
    for (auto* light : sceneLights) {
        delete light;
    }
    sceneLights.clear();

    // FIXED: Brighter directional light for better visibility
    UpgradedLight* mainLight = new UpgradedLight();
    mainLight->type = LIGHT_DIRECTIONAL;
    mainLight->direction = Vector3Normalize(Vector3{ 0.5f, -1.0f, 0.3f });
    mainLight->color = Color{ 255, 250, 220, 255 };
    mainLight->intensity = 2.0f;
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
        flashlight->intensity = 2.5f;
        flashlight->range = 35.0f;
        flashlight->spotAngle = 30.0f;
        flashlight->castShadows = false;
        sceneLights.push_back(flashlight);
    }

    // MUCH brighter ambient lighting for visibility
    Color ambientColor = Color{ 200, 210, 220, 255 };
    float ambientIntensity = 0.9f;

    if (g_MapPlayer.insideInterior) {
        // MUCH brighter ambient for interiors
        ambientColor = Color{ 220, 225, 230, 255 };
        ambientIntensity = 1.0f;
    }
    else if (g_DayNightCycle) {
        ambientColor = g_DayNightCycle->GetAmbientColor();
        ambientIntensity = 0.7f;
    }

    if (g_UpgradedPipeline) {
        g_UpgradedPipeline->SetAmbientLight(ambientColor, ambientIntensity);
    }

    // Fog only outside
    bool fogEnabled = false;
    Color fogColor = Color{ 180, 180, 180, 255 };
    float fogDensity = 0.02f;

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

    mapRenderer->Update(deltaTime, mainCamera);
    hudManager->Update(deltaTime);
    weaponRenderer->Update(deltaTime, mainCamera);

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
            light->position = mainCamera.position;
            light->direction = Vector3Normalize(Vector3Subtract(mainCamera.target, mainCamera.position));
        }
    }

    extern float health;
    hudManager->SetLowHealthWarning(health < 30.0f);
}

void UpgradedGameManager::UpdateCamera() {
    mainCamera.position = camera.position;
    mainCamera.target = camera.target;
    mainCamera.up = camera.up;
    mainCamera.fovy = camera.fovy;
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

    BeginMode3D(mainCamera);

    // Draw skybox
    if (!g_MapPlayer.insideInterior && g_SkyboxManager) {
        g_SkyboxManager->Draw(camera);
    }

    // Draw map geometry
    std::vector<MeshRenderer*> renderers = mapRenderer->GetActiveRenderers();
    for (auto* renderer : renderers) {
        if (!renderer->enabled) continue;
        g_RenderingPipeline->RenderMesh(*renderer, renderer->tint);
    }

    // Draw weapon
    MeshRenderer* weaponMesh = weaponRenderer->GetWeaponRenderer();
    if (weaponMesh && weaponMesh->enabled) {
        g_RenderingPipeline->RenderMesh(*weaponMesh, WHITE);
    }

    // Draw zombies
    if (g_ZombieManager) {
        g_ZombieManager->Draw(camera);
    }

    // Draw vehicles
    if (g_VehicleManager) {
        g_VehicleManager->Draw();
    }

    EndMode3D();

    // Draw weather effects
    if (g_WeatherSystem) {
        g_WeatherSystem->Draw(camera);
    }

    // Draw HUD - SIMPLIFIED
    extern float health, stamina, hunger, thirst;
    extern float flashlightBattery;
    extern bool isFlashlightOn;
    extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
    extern bool inventoryOpen, isCraftingOpen, isMapOpen;

    if (g_HUDManager) {
        g_HUDManager->DrawPlayerStats(screenW, screenH, health, stamina, hunger, thirst);
        g_HUDManager->DrawWeaponInfo(screenW, screenH, inventory[BACKPACK_SLOTS]);
        g_HUDManager->DrawFlashlightStatus(screenW, screenH, flashlightBattery, isFlashlightOn);
        g_HUDManager->DrawCrosshair(screenW, screenH, isAimingDownSights);
        g_HUDManager->DrawHitMarker(screenW, screenH);
    }
}

void UpgradedGameManager::Cleanup() {
    for (auto* light : sceneLights) {
        delete light;
    }
    sceneLights.clear();

    TraceLog(LOG_INFO, "Upgraded Game Manager cleaned up");
}
