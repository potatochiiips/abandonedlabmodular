#ifndef ENHANCED_MAP_SYSTEM_H
#define ENHANCED_MAP_SYSTEM_H

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

// NPC Types
enum NPCType {
    NPC_FRIENDLY_SURVIVOR,
    NPC_TRADER,
    NPC_FACTION_MEMBER,
    NPC_HOSTILE_RAIDER,
    NPC_TYPE_COUNT
};

// Damage states for buildings
enum BuildingDamageState {
    DAMAGE_STATE_INTACT,
    DAMAGE_STATE_MODERATE,
    DAMAGE_STATE_SEVERE,
    DAMAGE_STATE_COLLAPSED
};

// Light source types
enum LightSourceType {
    LIGHT_STREET_LAMP,
    LIGHT_STORE_SIGN,
    LIGHT_EMERGENCY_BEACON,
    LIGHT_FIRE,
    LIGHT_NEON_SIGN,
    LIGHT_TYPE_COUNT
};

// Light source structure
struct LightSource {
    int id;
    LightSourceType type;
    Vector3 position;
    float radius;
    Color color;
    float intensity;
    bool active;
    bool broken;
    float flicker; // For flickering effect
};

// Point of Interest types
enum POIType {
    POI_SAFE_HOUSE,
    POI_SUPPLY_CACHE,
    POI_QUEST_LOCATION,
    POI_LANDMARK,
    POI_DANGER_ZONE,
    POI_COUNT
};

// Point of Interest structure
struct PointOfInterest {
    int id;
    POIType type;
    Vector3 position;
    std::string name;
    WorldZone zone;
    bool discovered;
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
    BuildingDamageState damageState;
    std::vector<int> lootTables;    // Which items spawn inside
    bool isLooted;
    bool hasActiveEnemies;
    int estimatedLootValue;
    bool hasSecretRooms;
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

// Weather impact structure
struct WeatherImpact {
    WorldZone zone;
    float visibilityModifier;
    float movementSpeedModifier;
    bool isFlooded;
    bool isAccessible;
};

// NPC structure
struct NPC {
    int id;
    NPCType type;
    Vector3 position;
    std::string name;
    bool alive;
    float health;
    WorldZone zone;
    std::vector<int> inventory;
};

// Road segment structure for enhanced road network
struct RoadSegment {
    Vector3 start;
    Vector3 end;
    float width;
    int lanes;
    bool isHighway;
};

// Fast travel point structure
struct FastTravelPoint {
    int id;
    Vector3 position;
    WorldZone zone;
    std::string name;
    bool discovered;
    bool canTravelFrom;
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

    // NPC management
    void GenerateNPCs();
    std::vector<NPC*> GetNPCsInZone(WorldZone zone);
    NPC* GetNearestNPC(Vector3 position, float maxDistance);

    // Player spawn
    Vector3 GetPlayerSpawnPosition();

    // Interior access
    const Interior* GetLabInterior();

    // Minimap rendering
    void DrawMinimap(int x, int y, int radius, Vector3 playerPos, float playerYaw);

    // Full map rendering
    void DrawFullMap(int screenW, int screenH, Vector3 playerPos);

    // POI management
    std::vector<PointOfInterest> GetAllPOIs() const;
    PointOfInterest* GetNearestPOI(Vector3 position, float maxDistance);
    void DiscoverPOI(int poiId);
    void DrawPOIsOnMinimap(int centerX, int centerY, int radius, Vector3 playerPos);

private:
    std::vector<ZoneDefinition> zones;
    std::vector<EnhancedBuilding> buildings;
    std::vector<Vector3> riverPath;
    std::vector<RoadSegment> roadNetwork;
    std::map<std::string, Interior> interiors; // Store interiors by ID
    std::vector<FastTravelPoint> travelPoints; // Fast travel points

    int nextBuildingId;

    void InitializeZones();
    void GenerateZoneBuildings(WorldZone zone);
    void GenerateRiverSystem();
    void GenerateRoadNetwork();        // Enhanced version
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

// Interactive map UI class
class InteractiveMapUI {
public:
    void Initialize();
    void Update(float deltaTime);
    void Draw(int screenW, int screenH);

    void SetZoomLevel(float zoom);
    void SetMapCenter(Vector3 pos);
    void PlaceMarker(Vector3 pos, const std::string& name);
    void RemoveMarker(int id);
    void ShowQuestRoute(int questId);

    bool IsMapOpen() const;
    Vector3 GetScreenToWorldPos(Vector2 screenPos);

private:
    float zoomLevel;
    Vector3 mapCenter;
    std::vector<std::pair<Vector3, std::string>> markers;
    int selectedMarker;
};

// Fast travel system class
class FastTravelSystem {
public:
    void RegisterTravelPoint(Vector3 pos, const std::string& name);
    void DiscoverPoint(int id);
    bool TravelTo(int targetId, Vector3& outPos);
    std::vector<FastTravelPoint> GetAvailableDestinations() const;
    void DrawFastTravelUI(int screenW, int screenH);
};

#endif // ENHANCED_MAP_SYSTEM_H