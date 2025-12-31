#pragma once
#include "globals.h"
#include "map.h"
#include <vector>
#include <string>
#include <map>

// Enhanced world zones for the new map layout
enum WorldZone {
    ZONE_CENTRAL_CITY,
    ZONE_LABORATORY,
    ZONE_HOSPITAL,
    ZONE_BUSINESS_DISTRICT,
    ZONE_WESTERN_FARMLANDS,
    ZONE_EASTERN_COAST,
    ZONE_NORTHERN_MOUNTAINS,
    ZONE_SOUTHERN_INDUSTRIAL,
    ZONE_HARBOR,
    ZONE_COUNT
};

// Building types for procedural generation
enum EnhancedBuildingType {
    BLDG_SKYSCRAPER,
    BLDG_OFFICE_BUILDING,
    BLDG_LABORATORY,
    BLDG_HOSPITAL,
    BLDG_BANK,
    BLDG_RETAIL_STORE,
    BLDG_FARMHOUSE,
    BLDG_BARN,
    BLDG_BEACH_HOUSE,
    BLDG_WAREHOUSE,
    BLDG_FACTORY,
    BLDG_MILITARY_HANGAR,
    BLDG_MILITARY_BARRACKS,
    BLDG_PIER_BUILDING,
    BLDG_COUNT
};

// Enhanced building structure
struct EnhancedBuilding {
    int id;
    EnhancedBuildingType type;
    WorldZone zone;
    Vector3 position;
    Vector3 size;
    float rotation;
    bool hasInterior;
    std::string name;
    std::vector<int> interiorRooms;
    bool isAccessible;
};

// Zone definition with boundaries
struct ZoneDefinition {
    WorldZone type;
    Rectangle bounds;
    std::string name;
    Color mapColor;
    float buildingDensity;
    std::vector<EnhancedBuildingType> allowedBuildings;
};

// Enhanced map data for new layout
class EnhancedMapSystem {
public:
    EnhancedMapSystem();
    ~EnhancedMapSystem();

    void Initialize();
    void GenerateWorldMap();

    // Zone management
    ZoneDefinition* GetZone(WorldZone type);
    WorldZone GetZoneAt(float worldX, float worldZ);

    // Building management
    EnhancedBuilding* GetBuilding(int id);
    std::vector<EnhancedBuilding*> GetBuildingsInZone(WorldZone zone);

    // Player spawn
    Vector3 GetPlayerSpawnPosition();

    // Interior access
    const Interior* GetLabInterior();

    // Minimap rendering
    void DrawMinimap(int x, int y, int radius, Vector3 playerPos, float playerYaw);

    // Full map rendering
    void DrawFullMap(int screenW, int screenH, Vector3 playerPos);

private:
    std::vector<ZoneDefinition> zones;
    std::vector<EnhancedBuilding> buildings;
    std::vector<Vector3> riverPath;
    std::vector<Vector3> roadNetwork;
    std::map<std::string, Interior> interiors; // Store interiors by ID

    int nextBuildingId;

    void InitializeZones();
    void GenerateZoneBuildings(WorldZone zone);
    void GenerateRiverSystem();
    void GenerateRoadNetwork();
    void GenerateLaboratory();
    void GenerateLabInterior(EnhancedBuilding& building);
    void GenerateHospital();
    void GenerateMilitaryBase();
    void GenerateHarbor();

    Color GetZoneColor(WorldZone zone);
    void DrawMinimapZone(int centerX, int centerY, float scale, const ZoneDefinition& zone, Vector3 playerPos);
};

extern EnhancedMapSystem* g_EnhancedMapSystem;

void InitializeEnhancedMapSystem();
void CleanupEnhancedMapSystem();