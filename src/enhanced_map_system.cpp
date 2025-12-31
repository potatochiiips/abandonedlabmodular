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
    centralCity.bounds = Rectangle{ 100.0f, 100.0f, 56.0f, 56.0f };
    centralCity.name = "Central City";
    centralCity.mapColor = Color{ 120, 120, 140, 255 };
    centralCity.buildingDensity = 0.9f;
    centralCity.allowedBuildings = { BLDG_SKYSCRAPER, BLDG_OFFICE_BUILDING };
    zones.push_back(centralCity);

    // Laboratory (near city edge)
    ZoneDefinition laboratory;
    laboratory.type = ZONE_LABORATORY;
    laboratory.bounds = Rectangle{ 80.0f, 80.0f, 15.0f, 15.0f };
    laboratory.name = "Research Laboratory";
    laboratory.mapColor = Color{ 100, 150, 180, 255 };
    laboratory.buildingDensity = 1.0f;
    laboratory.allowedBuildings = { BLDG_LABORATORY };
    zones.push_back(laboratory);

    // Hospital (northeast)
    ZoneDefinition hospital;
    hospital.type = ZONE_HOSPITAL;
    hospital.bounds = Rectangle{ 160.0f, 80.0f, 20.0f, 20.0f };
    hospital.name = "Medical Center";
    hospital.mapColor = Color{ 200, 80, 80, 255 };
    hospital.buildingDensity = 1.0f;
    hospital.allowedBuildings = { BLDG_HOSPITAL };
    zones.push_back(hospital);

    // Business District
    ZoneDefinition business;
    business.type = ZONE_BUSINESS_DISTRICT;
    business.bounds = Rectangle{ 90.0f, 120.0f, 76.0f, 40.0f };
    business.name = "Business District";
    business.mapColor = Color{ 140, 140, 160, 255 };
    business.buildingDensity = 0.8f;
    business.allowedBuildings = { BLDG_BANK, BLDG_OFFICE_BUILDING, BLDG_RETAIL_STORE };
    zones.push_back(business);

    // Western Farmlands
    ZoneDefinition farmlands;
    farmlands.type = ZONE_WESTERN_FARMLANDS;
    farmlands.bounds = Rectangle{ 10.0f, 50.0f, 80.0f, 150.0f };
    farmlands.name = "Western Farmlands";
    farmlands.mapColor = Color{ 100, 180, 80, 255 };
    farmlands.buildingDensity = 0.2f;
    farmlands.allowedBuildings = { BLDG_FARMHOUSE, BLDG_BARN };
    zones.push_back(farmlands);

    // Eastern Coast
    ZoneDefinition coast;
    coast.type = ZONE_EASTERN_COAST;
    coast.bounds = Rectangle{ 180.0f, 50.0f, 66.0f, 150.0f };
    coast.name = "Eastern Coast";
    coast.mapColor = Color{ 220, 200, 150, 255 };
    coast.buildingDensity = 0.3f;
    coast.allowedBuildings = { BLDG_BEACH_HOUSE, BLDG_PIER_BUILDING };
    zones.push_back(coast);

    // Northern Mountains
    ZoneDefinition mountains;
    mountains.type = ZONE_NORTHERN_MOUNTAINS;
    mountains.bounds = Rectangle{ 50.0f, 10.0f, 156.0f, 40.0f };
    mountains.name = "Northern Mountains";
    mountains.mapColor = Color{ 160, 160, 170, 255 };
    mountains.buildingDensity = 0.1f;
    mountains.allowedBuildings = { BLDG_MILITARY_HANGAR, BLDG_MILITARY_BARRACKS };
    zones.push_back(mountains);

    // Southern Industrial
    ZoneDefinition industrial;
    industrial.type = ZONE_SOUTHERN_INDUSTRIAL;
    industrial.bounds = Rectangle{ 80.0f, 200.0f, 86.0f, 46.0f };
    industrial.name = "Industrial District";
    industrial.mapColor = Color{ 100, 90, 80, 255 };
    industrial.buildingDensity = 0.7f;
    industrial.allowedBuildings = { BLDG_FACTORY, BLDG_WAREHOUSE };
    zones.push_back(industrial);

    // Harbor
    ZoneDefinition harbor;
    harbor.type = ZONE_HARBOR;
    harbor.bounds = Rectangle{ 30.0f, 230.0f, 60.0f, 16.0f };
    harbor.name = "Harbor";
    harbor.mapColor = Color{ 60, 80, 120, 255 };
    harbor.buildingDensity = 0.6f;
    harbor.allowedBuildings = { BLDG_WAREHOUSE };
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
    lab.position = Vector3{ 87, 0, 87 };
    lab.size = Vector3{ 12, 15, 12 };
    lab.rotation = 0;
    lab.hasInterior = true;
    lab.name = "Cryogenic Research Laboratory";
    lab.isAccessible = true;

    // Generate interior for the lab
    GenerateLabInterior(lab);

    buildings.push_back(lab);

    // Create door for the lab
    Door labDoor;
    labDoor.position = Vector3{ 87, 0, 81 }; // Front entrance
    labDoor.isInteriorDoor = false;
    labDoor.isLocked = false;
    labDoor.buildingId = lab.id;
    doors.push_back(labDoor);

    TraceLog(LOG_INFO, "Generated Laboratory at (%.1f, %.1f) with interior", lab.position.x, lab.position.z);
}

void EnhancedMapSystem::GenerateLabInterior(EnhancedBuilding& building) {
    // Create a simple interior layout for the lab
    Interior interior;
    interior.id = TextFormat("lab_%d", building.id);
    interior.width = 20;
    interior.height = 20;

    // Initialize floor tiles
    interior.tiles.resize(interior.width * interior.height, IT_FLOOR);

    // Create walls around perimeter
    for (int y = 0; y < interior.height; y++) {
        for (int x = 0; x < interior.width; x++) {
            if (x == 0 || x == interior.width - 1 || y == 0 || y == interior.height - 1) {
                interior.tiles[y * interior.width + x] = IT_WALL;
            }
        }
    }

    // Create entrance door
    interior.tiles[0 * interior.width + 10] = IT_DOOR;

    // Add interior door to doors list
    Door interiorExitDoor;
    interiorExitDoor.position = Vector3{ 10, 0, 0 };
    interiorExitDoor.isInteriorDoor = true;
    interiorExitDoor.isLocked = false;
    interiorExitDoor.buildingId = building.id;
    doors.push_back(interiorExitDoor);

    // Add cryopods in main chamber (center area)
    for (int i = 0; i < 4; i++) {
        int x = 5 + i * 3;
        int y = 10;
        interior.tiles[y * interior.width + x] = IT_CRYOPOD_BROKEN;

        // Store prop position
        building.interiorRooms.push_back(y * interior.width + x);
    }

    // Add consoles along walls
    interior.tiles[5 * interior.width + 3] = IT_CONSOLE;
    interior.tiles[5 * interior.width + 16] = IT_CONSOLE;
    interior.tiles[15 * interior.width + 3] = IT_CONSOLE;
    interior.tiles[15 * interior.width + 16] = IT_CONSOLE;

    building.interiorRooms.push_back(5 * interior.width + 3);
    building.interiorRooms.push_back(5 * interior.width + 16);
    building.interiorRooms.push_back(15 * interior.width + 3);
    building.interiorRooms.push_back(15 * interior.width + 16);

    // Add desks and chairs
    interior.tiles[8 * interior.width + 8] = IT_DESK;
    interior.tiles[8 * interior.width + 12] = IT_DESK;
    interior.tiles[9 * interior.width + 8] = IT_CHAIR;
    interior.tiles[9 * interior.width + 12] = IT_CHAIR;

    building.interiorRooms.push_back(8 * interior.width + 8);
    building.interiorRooms.push_back(8 * interior.width + 12);
    building.interiorRooms.push_back(9 * interior.width + 8);
    building.interiorRooms.push_back(9 * interior.width + 12);

    // Add shelves and lockers
    interior.tiles[10 * interior.width + 18] = IT_SHELF;
    interior.tiles[11 * interior.width + 18] = IT_LOCKER;
    interior.tiles[12 * interior.width + 18] = IT_CABINET;

    building.interiorRooms.push_back(10 * interior.width + 18);
    building.interiorRooms.push_back(11 * interior.width + 18);
    building.interiorRooms.push_back(12 * interior.width + 18);

    // Create a floor for the interior
    Floor floor;
    floor.floorNumber = 0;
    floor.width = interior.width;
    floor.height = interior.height;
    floor.tiles = interior.tiles;
    floor.playerSpawnX = 10; // Spawn in center
    floor.playerSpawnY = 15;

    interior.floors.push_back(floor);

    // Store the interior
    interiors[interior.id] = interior;

    TraceLog(LOG_INFO, "Generated interior for laboratory: %s", interior.id.c_str());
}

void EnhancedMapSystem::GenerateHospital() {
    EnhancedBuilding hospital;
    hospital.id = nextBuildingId++;
    hospital.type = BLDG_HOSPITAL;
    hospital.zone = ZONE_HOSPITAL;
    hospital.position = Vector3{ 170, 0, 90 };
    hospital.size = Vector3{ 18, 20, 18 };
    hospital.rotation = 0;
    hospital.hasInterior = true;
    hospital.name = "Central Medical Center";
    hospital.isAccessible = true;
    buildings.push_back(hospital);

    TraceLog(LOG_INFO, "Generated Hospital at (%.1f, %.1f)", hospital.position.x, hospital.position.z);
}

void EnhancedMapSystem::GenerateMilitaryBase() {
    Vector3 baseCenter = { 128.0f, 0.0f, 25.0f };

    for (int i = 0; i < 3; i++) {
        EnhancedBuilding hangar;
        hangar.id = nextBuildingId++;
        hangar.type = BLDG_MILITARY_HANGAR;
        hangar.zone = ZONE_NORTHERN_MOUNTAINS;
        hangar.position = Vector3{ baseCenter.x + i * 15.0f, 0.0f, baseCenter.z };
        hangar.size = Vector3{ 12.0f, 10.0f, 20.0f };
        hangar.rotation = 0.0f;
        hangar.hasInterior = true;
        hangar.name = TextFormat("Hangar %d", i + 1);
        hangar.isAccessible = true;
        buildings.push_back(hangar);
    }

    for (int i = 0; i < 4; i++) {
        EnhancedBuilding barracks;
        barracks.id = nextBuildingId++;
        barracks.type = BLDG_MILITARY_BARRACKS;
        barracks.zone = ZONE_NORTHERN_MOUNTAINS;
        barracks.position = Vector3{ baseCenter.x + (i % 2) * 10.0f, 0.0f, baseCenter.z + 25.0f + (i / 2) * 12.0f };
        barracks.size = Vector3{ 8.0f, 6.0f, 15.0f };
        barracks.rotation = 0.0f;
        barracks.hasInterior = true;
        barracks.name = TextFormat("Barracks %d", i + 1);
        barracks.isAccessible = true;
        buildings.push_back(barracks);
    }

    TraceLog(LOG_INFO, "Generated Military Base");
}

void EnhancedMapSystem::GenerateHarbor() {
    for (int i = 0; i < 8; i++) {
        EnhancedBuilding warehouse;
        warehouse.id = nextBuildingId++;
        warehouse.type = BLDG_WAREHOUSE;
        warehouse.zone = ZONE_HARBOR;
        warehouse.position = Vector3{ 35.0f + i * 9, 0, 235 };
        warehouse.size = Vector3{ 8, 8, 12 };
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

        building.position.x = zone->bounds.x + (float)(rand() % (int)zone->bounds.width);
        building.position.y = 0.0f;
        building.position.z = zone->bounds.y + (float)(rand() % (int)zone->bounds.height);

        switch (building.type) {
        case BLDG_SKYSCRAPER:
            building.size = Vector3{ 8.0f, 40.0f + (float)(rand() % 40), 8.0f };
            break;
        case BLDG_OFFICE_BUILDING:
            building.size = Vector3{ 10.0f, 15.0f + (float)(rand() % 20), 10.0f };
            break;
        case BLDG_BANK:
            building.size = Vector3{ 12.0f, 12.0f, 12.0f };
            break;
        case BLDG_RETAIL_STORE:
            building.size = Vector3{ 8.0f, 8.0f, 6.0f };
            break;
        case BLDG_FARMHOUSE:
            building.size = Vector3{ 6.0f, 6.0f, 8.0f };
            break;
        case BLDG_BARN:
            building.size = Vector3{ 10.0f, 12.0f, 12.0f };
            break;
        case BLDG_BEACH_HOUSE:
            building.size = Vector3{ 6.0f, 5.0f, 8.0f };
            break;
        case BLDG_FACTORY:
            building.size = Vector3{ 15.0f, 10.0f, 20.0f };
            break;
        default:
            building.size = Vector3{ 8.0f, 8.0f, 8.0f };
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
    riverPath.clear();

    for (int i = 0; i < 50; i++) {
        float t = i / 49.0f;
        Vector3 point;
        point.x = 90 - t * 60;
        point.y = 0;
        point.z = 20 + t * 220;
        riverPath.push_back(point);
    }
}

void EnhancedMapSystem::GenerateRoadNetwork() {
    roadNetwork.clear();
}

Vector3 EnhancedMapSystem::GetPlayerSpawnPosition() {
    // Spawn inside the lab interior
    return Vector3{ 10.0f, 1.8f, 15.0f }; // Interior coordinates
}

const Interior* EnhancedMapSystem::GetLabInterior() {
    auto it = interiors.find("lab_1");
    if (it != interiors.end()) {
        return &it->second;
    }
    return nullptr;
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
    DrawCircle(centerX, centerY, radius, Color{ 0, 0, 0, 180 });
    DrawCircle(centerX, centerY, radius, Color{ 50, 50, 60, 150 });
    DrawCircleLines(centerX, centerY, radius, PIPBOY_GREEN);

    float scale = radius / 50.0f;

    for (auto& zone : zones) {
        DrawMinimapZone(centerX, centerY, scale, zone, playerPos);
    }

    for (size_t i = 1; i < riverPath.size(); i++) {
        Vector3 p1 = riverPath[i - 1];
        Vector3 p2 = riverPath[i];

        int x1 = centerX + (int)((p1.x - playerPos.x) * scale);
        int y1 = centerY + (int)((p1.z - playerPos.z) * scale);
        int x2 = centerX + (int)((p2.x - playerPos.x) * scale);
        int y2 = centerY + (int)((p2.z - playerPos.z) * scale);

        if (CheckCollisionPointCircle(Vector2{ (float)x1, (float)y1 }, Vector2{ (float)centerX, (float)centerY }, radius)) {
            DrawLine(x1, y1, x2, y2, Color{ 100, 150, 200, 200 });
        }
    }

    for (auto& building : buildings) {
        float dx = building.position.x - playerPos.x;
        float dz = building.position.z - playerPos.z;
        float dist = sqrtf(dx * dx + dz * dz);

        if (dist < 50.0f) {
            int x = centerX + (int)(dx * scale);
            int y = centerY + (int)(dz * scale);

            if (CheckCollisionPointCircle(Vector2{ (float)x, (float)y }, Vector2{ (float)centerX, (float)centerY }, radius)) {
                Color buildingColor = GetZoneColor(building.zone);
                DrawRectangle(x - 1, y - 1, 3, 3, buildingColor);
            }
        }
    }

    DrawCircle(centerX, centerY, 4, PIPBOY_GREEN);

    float dirX = sinf(playerYaw * DEG2RAD) * 8;
    float dirY = cosf(playerYaw * DEG2RAD) * 8;
    DrawLine(centerX, centerY, centerX + (int)dirX, centerY + (int)dirY, WHITE);
}

void EnhancedMapSystem::DrawMinimapZone(int centerX, int centerY, float scale, const ZoneDefinition& zone, Vector3 playerPos) {
    float zoneX = zone.bounds.x + zone.bounds.width / 2 - playerPos.x;
    float zoneZ = zone.bounds.y + zone.bounds.height / 2 - playerPos.z;

    int x = centerX + (int)(zoneX * scale);
    int y = centerY + (int)(zoneZ * scale);
    int w = (int)(zone.bounds.width * scale);
    int h = (int)(zone.bounds.height * scale);

    if (abs(x - centerX) < 100 && abs(y - centerY) < 100) {
        Color zoneColor = zone.mapColor;
        zoneColor.a = 100;
        DrawRectangle(x - w / 2, y - h / 2, w, h, zoneColor);
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