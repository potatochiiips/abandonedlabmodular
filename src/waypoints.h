#pragma once
#include "globals.h"
#include <vector>
#include <string>

// Waypoint structure
struct Waypoint {
    Vector3 position;
    std::string name;
    Color color;
    bool isActive;
    int id;
};

// Waypoint manager
class WaypointManager {
public:
    WaypointManager() : nextId(0) {}
    
    // Add a waypoint at the player's current position
    int AddWaypoint(Vector3 position, const std::string& name = "Waypoint", Color color = YELLOW) {
        Waypoint wp;
        wp.position = position;
        wp.name = name;
        wp.color = color;
        wp.isActive = true;
        wp.id = nextId++;
        waypoints.push_back(wp);
        return wp.id;
    }
    
    // Remove waypoint by ID
    bool RemoveWaypoint(int id) {
        for (auto it = waypoints.begin(); it != waypoints.end(); ++it) {
            if (it->id == id) {
                waypoints.erase(it);
                return true;
            }
        }
        return false;
    }
    
    // Get all waypoints
    const std::vector<Waypoint>& GetWaypoints() const {
        return waypoints;
    }
    
    // Draw waypoints on minimap
    void DrawOnMinimap(int minimapX, int minimapY, int minimapW, int minimapH, Vector3 playerPos, int viewRange, float cellSize) {
        int playerX = (int)playerPos.x;
        int playerZ = (int)playerPos.z;
        
        for (const auto& wp : waypoints) {
            if (!wp.isActive) continue;
            
            int wpX = (int)wp.position.x;
            int wpZ = (int)wp.position.z;
            
            int relX = wpX - playerX;
            int relZ = wpZ - playerZ;
            
            if (abs(relX) < viewRange && abs(relZ) < viewRange) {
                int drawX = minimapX + (int)((relX + viewRange) * cellSize);
                int drawY = minimapY + (int)((relZ + viewRange) * cellSize);
                
                DrawCircle(drawX, drawY, 4, wp.color);
                DrawCircle(drawX, drawY, 5, BLACK);
            }
        }
    }
    
    // Draw waypoints in 3D world
    void DrawIn3D(Vector3 playerPos, float maxDistance = 100.0f) {
        for (const auto& wp : waypoints) {
            if (!wp.isActive) continue;
            
            float distance = Vector3Distance(playerPos, wp.position);
            if (distance > maxDistance) continue;
            
            // Draw vertical beam
            Vector3 beamTop = wp.position;
            beamTop.y += 50.0f;
            DrawLine3D(wp.position, beamTop, wp.color);
            
            // Draw marker at waypoint location
            DrawSphere(wp.position, 0.5f, wp.color);
        }
    }
    
    // Clear all waypoints
    void ClearAll() {
        waypoints.clear();
    }
    
    // Save/Load waypoints
    void SaveToFile(const char* filename) {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << waypoints.size() << "\n";
            for (const auto& wp : waypoints) {
                file << wp.id << " " << wp.position.x << " " << wp.position.y << " " 
                     << wp.position.z << " " << wp.name << " " 
                     << (int)wp.color.r << " " << (int)wp.color.g << " " 
                     << (int)wp.color.b << " " << wp.isActive << "\n";
            }
            file.close();
        }
    }
    
    void LoadFromFile(const char* filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            waypoints.clear();
            size_t count;
            file >> count;
            
            for (size_t i = 0; i < count; i++) {
                Waypoint wp;
                int r, g, b;
                file >> wp.id >> wp.position.x >> wp.position.y >> wp.position.z;
                std::getline(file, wp.name); // Read rest of line for name
                file >> r >> g >> b >> wp.isActive;
                wp.color = Color{(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
                waypoints.push_back(wp);
                if (wp.id >= nextId) nextId = wp.id + 1;
            }
            file.close();
        }
    }
    
private:
    std::vector<Waypoint> waypoints;
    int nextId;
};

// Global waypoint manager
extern WaypointManager g_WaypointManager;