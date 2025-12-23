#pragma once
#include "globals.h"
#include <vector>

// Weather types
enum WeatherType {
    WEATHER_CLEAR,
    WEATHER_RAIN,
    WEATHER_FOG,
    WEATHER_STORM,
    WEATHER_SNOW,
    WEATHER_COUNT
};

// Rain particle
struct RainDrop {
    Vector3 position;
    Vector3 velocity;
    float lifetime;
};

// Weather system
class WeatherSystem {
public:
    WeatherSystem();
    ~WeatherSystem();
    
    // Initialize system
    void Initialize();
    
    // Update weather
    void Update(float deltaTime, Vector3 playerPos);
    
    // Draw weather effects
    void Draw(const Camera3D& camera);
    
    // Set weather type
    void SetWeather(WeatherType type);
    
    // Get current weather
    WeatherType GetCurrentWeather() const { return currentWeather; }
    
    // Get weather intensity (0.0 - 1.0)
    float GetIntensity() const { return intensity; }
    
    // Get fog density
    float GetFogDensity() const;
    
    // Get fog color
    Color GetFogColor() const;
    
private:
    WeatherType currentWeather;
    float intensity;
    float transitionProgress;
    
    // Rain system
    std::vector<RainDrop> rainDrops;
    int maxRainDrops;
    
    // Fog system
    float fogDensity;
    
    // Create rain particles
    void SpawnRain(Vector3 playerPos);
    
    // Update rain particles
    void UpdateRain(float deltaTime);
    
    // Draw rain
    void DrawRain(const Camera3D& camera);
    
    // Draw fog overlay
    void DrawFog();
};

// Global weather system
extern WeatherSystem* g_WeatherSystem;

// Initialize weather system
void InitializeWeatherSystem();

// Cleanup weather system
void CleanupWeatherSystem();
