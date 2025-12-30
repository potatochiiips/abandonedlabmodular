#include "enhanced_map_system.h"
#include <cmath>
#include <cstdlib>

EnhancedMapSystem* g_EnhancedMapSystem = nullptr;

EnhancedMapSystem::EnhancedMapSystem() {
    nextBuildingId = 1;
}

EnhancedMapSystem::~EnhancedMapSystem() {
}

void EnhancedMapSystem::Initialize() {
    TraceLog(LOG_INFO, "Initializing Enhanced Map System...");
    InitializeZones();
    GenerateWorldMap();
    TraceLog(LOG_INFO, "Enhanced Map System initialized");
}

void EnhancedMapSystem::InitializeZones() {
    // Central City (middle of map)
    ZoneDefinition centralCity;
    centralCity.type = ZONE_CENTRAL_CITY;
    centralCity.bounds = Rectangle{100, 100, 56, 56}; // Center area
    centralCity.name = "Central City";
    centralCity.mapColor = Color{120, 120, 140, 255};
    centralCity.buildingDensity = 0.9f;
    centralCity.allowedBuildings = {BLDG_SKYSCRAPER, BLDG_OFFICE_BUILDING};
    zones.push_back(centralCity);
    
    // Laboratory (near city edge)
    ZoneDefinition laboratory;
    laboratory.type = ZONE_LABORATORY;
    laboratory.bounds = Rectangle{80, 80, 15, 15};
    laboratory.name = "Research Laboratory";
    laboratory.mapColor = Color{100, 150, 180, 255};
    laboratory.buildingDensity = 1.0f;
    laboratory.allowedBuildings = {BLDG_LABORATORY};
    zones.push_back(laboratory);
    
    // Hospital (northeast)
    ZoneDefinition hospital;
    hospital.type = ZONE_HOSPITAL;
    hospital.bounds = Rectangle{160, 80, 20, 20};
    hospital.name = "Medical Center";
    hospital.mapColor = Color{200, 80, 80, 255};
    hospital.buildingDensity = 1.0f;
    hospital.allowedBuildings = {BLDG_HOSPITAL};
    zones.push_back(hospital);
    
    // Business District
    ZoneDefinition business;
    business.type = ZONE_BUSINESS_DISTRICT;
    business.bounds = Rectangle{90, 120, 76, 40};
    business.name = "Business District";
    business.mapColor = Color{140, 140, 160, 255};
    business.buildingDensity = 0.8f;
    business.allowedBuildings = {BLDG_BANK, BLDG_OFFICE_BUILDING, BLDG_RETAIL_STORE};
    zones.push_back(business);
    
    // Western Farmlands
    ZoneDefinition farmlands;
    farmlands.type = ZONE_WESTERN_FARMLANDS;
    farmlands.bounds = Rectangle{10, 50, 80, 150};
    farmlands.name = "Western Farmlands";
    farmlands.mapColor = Color{100, 180, 80, 255};
    farmlands.buildingDensity = 0.2f;
    farmlands.allowedBuildings = {BLDG_FARMHOUSE, BLDG_BARN};
    zones.push_back(farmlands);
    
    // Eastern Coast
    ZoneDefinition coast;
    coast.type = ZONE_EASTERN_COAST;
    coast.bounds = Rectangle{180, 50, 66, 150};
    coast.name = "Eastern Coast";
    coast.mapColor = Color{220, 200, 150, 255};
    coast.buildingDensity = 0.3f;
    coast.allowedBuildings = {BLDG_BEACH_HOUSE, BLDG_PIER_BUILDING};
    zones.push_back(coast);
    
    // Northern Mountains
    ZoneDefinition mountains;
    mountains.type = ZONE_NORTHERN_MOUNTAINS;
    mountains.bounds = Rectangle{50, 10, 156, 40};
    mountains.name = "Northern Mountains";
    mountains.mapColor = Color{160, 160, 170, 255};
    mountains.buildingDensity = 0.1f;
    mountains.allowedBuildings = {BLDG_MILITARY_HANGAR, BLDG_MILITARY_BARRACKS};
    zones.push_back(mountains);
    
    // Southern Industrial
    ZoneDefinition industrial;
    industrial.type = ZONE_SOUTHERN_INDUSTRIAL;
    industrial.bounds = Rectangle{80, 200, 86, 46};
    industrial.name = "Industrial District";
    industrial.mapColor = Color{100, 90, 80, 255};
    industrial.buildingDensity = 0.7f;
    industrial.allowedBuildings = {BLDG_FACTORY, BLDG_WAREHOUSE};
    zones.push_back(industrial);
    
    // Harbor
    ZoneDefinition harbor;
    harbor.type = ZONE_HARBOR;
    harbor.bounds = Rectangle{30, 230, 60, 16};
    harbor.name = "Harbor";
    harbor.mapColor = Color{60, 80, 120, 255};
    harbor.buildingDensity = 0.6f;
    harbor.allowedBuildings = {BLDG_WAREHOUSE};
    zones.push_back(harbor);
}

void EnhancedMapSystem::GenerateWorldMap() {
    TraceLog(LOG_INFO, "Generating world map...");
    
    // Generate special buildings first
    GenerateLaboratory();
    GenerateHospital();
    GenerateMilitaryBase();
    GenerateHarbor();
    
    // Generate buildings for each zone
    for (auto& zone : zones) {
        if (zone.type != ZONE_LABORATORY && zone.type != ZONE_HOSPITAL &&
            zone.type != ZONE_NORTHERN_MOUNTAINS && zone.type != ZONE_HARBOR) {
            GenerateZoneBuildings(zone.type);
        }
    }
    
    // Generate infrastructure
    GenerateRiverSystem();
    GenerateRoadNetwork();
    
    TraceLog(LOG_INFO, "World map generated with %d buildings", (int)buildings.size());
}

void EnhancedMapSystem::GenerateLaboratory() {
    EnhancedBuilding lab;
    lab.id = nextBuildingId++;
    lab.type = BLDG_LABORATORY;
    lab.zone = ZONE_LABORATORY;
    lab.position = Vector3{87, 0, 87}; // Near city edge
    lab.size = Vector3{12, 15, 12};
    lab.rotation = 0;
    lab.hasInterior = true;
    lab.name = "Cryogenic Research Laboratory";
    lab.isAccessible = true;
    buildings.push_back(lab);
    
    TraceLog(LOG_INFO, "Generated Laboratory at (%.1f, %.1f)", lab.position.x, lab.position.z);
}

void EnhancedMapSystem::GenerateHospital() {
    EnhancedBuilding hospital;
    hospital.id = nextBuildingId++;
    hospital.type = BLDG_HOSPITAL;
    hospital.zone = ZONE_HOSPITAL;
    hospital.position = Vector3{170, 0, 90};
    hospital.size = Vector3{18, 20, 18};
    hospital.rotation = 0;
    hospital.hasInterior = true;
    hospital.name = "Central Medical Center";
    hospital.isAccessible = true;
    buildings.push_back(hospital);
    
    TraceLog(LOG_INFO, "Generated Hospital at (%.1f, %.1f)", hospital.position.x, hospital.position.z);
}

void EnhancedMapSystem::GenerateMilitaryBase() {
    // Military base in northern mountains
    Vector3 baseCenter = {128, 0, 25};
    
    // Hangars
    for (int i = 0; i < 3; i++) {
        EnhancedBuilding hangar;
        hangar.id = nextBuildingId++;
        hangar.type = BLDG_MILITARY_HANGAR;
        hangar.zone = ZONE_NORTHERN_MOUNTAINS;
        hangar.position = Vector3{baseCenter.x + i * 15, 0, baseCenter.z};
        hangar.size = Vector3{12, 10, 20};
        hangar.rotation = 0;
        hangar.hasInterior = true;
        hangar.name = TextFormat("Hangar %d", i + 1);
        hangar.isAccessible = true;
        buildings.push_back(hangar);
    }
    
    // Barracks
    for (int i = 0; i < 4; i++) {
        EnhancedBuilding barracks;
        barracks.id = nextBuildingId++;
        barracks.type = BLDG_MILITARY_BARRACKS;
        barracks.zone = ZONE_NORTHERN_MOUNTAINS;
        barracks.position = Vector3{baseCenter.x + (i % 2) * 10, 0, baseCenter.z + 25 + (i / 2) * 12};
        barracks.size = Vector3{8, 6, 15};
        barracks.rotation = 0;
        barracks.hasInterior = true;
        barracks.name = TextFormat("Barracks %d", i + 1);
        barracks.isAccessible = true;
        buildings.push_back(barracks);
    }
    
    TraceLog(LOG_INFO, "Generated Military Base");
}

void EnhancedMapSystem::GenerateHarbor() {
    // Generate warehouses along harbor
    for (int i = 0; i < 8; i++) {
        EnhancedBuilding warehouse;
        warehouse.id = nextBuildingId++;
        warehouse.type = BLDG_WAREHOUSE;
        warehouse.zone = ZONE_HARBOR;
        warehouse.position = Vector3{35 + i * 9, 0, 235};
        warehouse.size = Vector3{8, 8, 12};
        warehouse.rotation = 0;
        warehouse.hasInterior = false;
        warehouse.name = TextFormat("Warehouse %d", i + 1);
        warehouse.isAccessible = false;
        buildings.push_back(warehouse);
    }
    
    TraceLog(LOG_INFO, "Generated Harbor");
}

void EnhancedMapSystem::GenerateZoneBuildings(WorldZone zoneType) {
    ZoneDefinition* zone = GetZone(zoneType);
    if (!zone) return;
    
    int buildingCount = (int)(zone->bounds.width * zone->bounds.height * zone->buildingDensity / 100.0f);
    
    for (int i = 0; i < buildingCount; i++) {
        if (zone->allowedBuildings.empty()) continue;
        
        EnhancedBuilding building;
        building.id = nextBuildingId++;
        building.type = zone->allowedBuildings[rand() % zone->allowedBuildings.size()];
        building.zone = zoneType;
        
        // Random position within zone
        building.position.x = zone->bounds.x + (rand() % (int)zone->bounds.width);
        building.position.y = 0;
        building.position.z = zone->bounds.y + (rand() % (int)zone->bounds.height);
        
        // Size based on type
        switch (building.type) {
            case BLDG_SKYSCRAPER:
                building.size = Vector3{8, 40 + rand() % 40, 8};
                break;
            case BLDG_OFFICE_BUILDING:
                building.size = Vector3{10, 15 + rand() % 20, 10};
                break;
            case BLDG_BANK:
                building.size = Vector3{12, 12, 12};
                break;
            case BLDG_RETAIL_STORE:
                building.size = Vector3{8, 8, 6};
                break;
            case BLDG_FARMHOUSE:
                building.size = Vector3{6, 6, 8};
                break;
            case BLDG_BARN:
                building.size = Vector3{10, 12, 12};
                break;
            case BLDG_BEACH_HOUSE:
                building.size = Vector3{6, 5, 8};
                break;
            case BLDG_FACTORY:
                building.size = Vector3{15, 10, 20};
                break;
            default:
                building.size = Vector3{8, 8, 8};
                break;
        }
        
        building.rotation = (float)(rand() % 4) * 90.0f;
        building.hasInterior = (rand() % 3 == 0);
        building.isAccessible = building.hasInterior;
        building.name = TextFormat("%s %d", zone->name.c_str(), i);
        
        buildings.push_back(building);
    }
}

void EnhancedMapSystem::GenerateRiverSystem() {
    // River flows from north to southwest
    riverPath.clear();
    
    for (int i = 0; i < 50; i++) {
        float t = i / 49.0f;
        Vector3 point;
        point.x = 90 - t * 60; // West direction
        point.y = 0;
        point.z = 20 + t * 220; // North to south
        riverPath.push_back(point);
    }
}

void EnhancedMapSystem::GenerateRoadNetwork() {
    // Major highways connecting zones
    roadNetwork.clear();
    // Implementation would add road waypoints
}

Vector3 EnhancedMapSystem::GetPlayerSpawnPosition() {
    // Spawn in laboratory's cryogenic room
    return Vector3{87, 1.8f, 87};
}

WorldZone EnhancedMapSystem::GetZoneAt(float worldX, float worldZ) {
    for (auto& zone : zones) {
        if (worldX >= zone.bounds.x && worldX < zone.bounds.x + zone.bounds.width &&
            worldZ >= zone.bounds.y && worldZ < zone.bounds.y + zone.bounds.height) {
            return zone.type;
        }
    }
    return ZONE_CENTRAL_CITY;
}

ZoneDefinition* EnhancedMapSystem::GetZone(WorldZone type) {
    for (auto& zone : zones) {
        if (zone.type == type) return &zone;
    }
    return nullptr;
}

EnhancedBuilding* EnhancedMapSystem::GetBuilding(int id) {
    for (auto& building : buildings) {
        if (building.id == id) return &building;
    }
    return nullptr;
}

void EnhancedMapSystem::DrawMinimap(int centerX, int centerY, int radius, Vector3 playerPos, float playerYaw) {
    // Draw circular minimap background
    DrawCircle(centerX, centerY, radius, Color{0, 0, 0, 180});
    DrawCircle(centerX, centerY, radius, Color{50, 50, 60, 150});
    DrawCircleLines(centerX, centerY, radius, PIPBOY_GREEN);
    
    float scale = radius / 50.0f; // Show 100 units diameter
    
    // Draw zones
    for (auto& zone : zones) {
        DrawMinimapZone(centerX, centerY, scale, zone, playerPos);
    }
    
    // Draw river
    for (size_t i = 1; i < riverPath.size(); i++) {
        Vector3 p1 = riverPath[i - 1];
        Vector3 p2 = riverPath[i];
        
        int x1 = centerX + (int)((p1.x - playerPos.x) * scale);
        int y1 = centerY + (int)((p1.z - playerPos.z) * scale);
        int x2 = centerX + (int)((p2.x - playerPos.x) * scale);
        int y2 = centerY + (int)((p2.z - playerPos.z) * scale);
        
        if (CheckCollisionPointCircle(Vector2{(float)x1, (float)y1}, Vector2{(float)centerX, (float)centerY}, radius)) {
            DrawLine(x1, y1, x2, y2, Color{100, 150, 200, 200});
        }
    }
    
    // Draw buildings as small rectangles
    for (auto& building : buildings) {
        float dx = building.position.x - playerPos.x;
        float dz = building.position.z - playerPos.z;
        float dist = sqrtf(dx * dx + dz * dz);
        
        if (dist < 50.0f) {
            int x = centerX + (int)(dx * scale);
            int y = centerY + (int)(dz * scale);
            
            if (CheckCollisionPointCircle(Vector2{(float)x, (float)y}, Vector2{(float)centerX, (float)centerY}, radius)) {
                Color buildingColor = GetZoneColor(building.zone);
                DrawRectangle(x - 1, y - 1, 3, 3, buildingColor);
            }
        }
    }
    
    // Draw player icon (always at center)
    DrawCircle(centerX, centerY, 4, PIPBOY_GREEN);
    
    // Draw player direction indicator
    float dirX = sinf(playerYaw * DEG2RAD) * 8;
    float dirY = cosf(playerYaw * DEG2RAD) * 8;
    DrawLine(centerX, centerY, centerX + (int)dirX, centerY + (int)dirY, WHITE);
}

void EnhancedMapSystem::DrawMinimapZone(int centerX, int centerY, float scale, const ZoneDefinition& zone, Vector3 playerPos) {
    // Calculate zone position relative to player
    float zoneX = zone.bounds.x + zone.bounds.width / 2 - playerPos.x;
    float zoneZ = zone.bounds.y + zone.bounds.height / 2 - playerPos.z;
    
    int x = centerX + (int)(zoneX * scale);
    int y = centerY + (int)(zoneZ * scale);
    int w = (int)(zone.bounds.width * scale);
    int h = (int)(zone.bounds.height * scale);
    
    // Only draw if visible in minimap
    if (abs(x - centerX) < 100 && abs(y - centerY) < 100) {
        Color zoneColor = zone.mapColor;
        zoneColor.a = 100;
        DrawRectangle(x - w/2, y - h/2, w, h, zoneColor);
    }
}

Color EnhancedMapSystem::GetZoneColor(WorldZone zone) {
    ZoneDefinition* zoneDef = GetZone(zone);
    if (zoneDef) return zoneDef->mapColor;
    return GRAY;
}

void InitializeEnhancedMapSystem() {
    g_EnhancedMapSystem = new EnhancedMapSystem();
    g_EnhancedMapSystem->Initialize();
}

void CleanupEnhancedMapSystem() {
    if (g_EnhancedMapSystem) {
        delete g_EnhancedMapSystem;
        g_EnhancedMapSystem = nullptr;
    }
}
