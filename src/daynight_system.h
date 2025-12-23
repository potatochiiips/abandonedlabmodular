#pragma once
#include "globals.h"

// Time of day system
class DayNightCycle {
public:
    DayNightCycle();
    ~DayNightCycle();
    
    // Initialize system
    void Initialize();
    
    // Update time progression
    void Update(float deltaTime);
    
    // Get current time (0.0 - 24.0)
    float GetCurrentTime() const { return currentTime; }
    
    // Set time directly
    void SetTime(float time);
    
    // Get sun direction
    Vector3 GetSunDirection() const;
    
    // Get sun color
    Color GetSunColor() const;
    
    // Get ambient color
    Color GetAmbientColor() const;
    
    // Get sky color
    Color GetSkyColor() const;
    
    // Get fog color
    Color GetFogColor() const;
    
    // Check if it's night
    bool IsNight() const { return currentTime < 6.0f || currentTime > 20.0f; }
    
    // Get light intensity (0.0 - 1.0)
    float GetLightIntensity() const;
    
    // Set time speed multiplier
    void SetTimeSpeed(float speed) { timeSpeed = speed; }
    
private:
    float currentTime;      // 0.0 to 24.0 (hours)
    float timeSpeed;        // Hours per real second
    
    // Calculate colors based on time
    Color CalculateSunColor() const;
    Color CalculateAmbientColor() const;
    Color CalculateSkyColor() const;
};

// Global day/night system
extern DayNightCycle* g_DayNightCycle;

// Initialize day/night system
void InitializeDayNightSystem();

// Cleanup day/night system
void CleanupDayNightSystem();
