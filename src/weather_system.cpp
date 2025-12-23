#include "weather_system.h"
#include "daynight_system.h"
#include <cstdlib>

// Global instance
WeatherSystem* g_WeatherSystem = nullptr;

WeatherSystem::WeatherSystem() {
    currentWeather = WEATHER_CLEAR;
    intensity = 0.0f;
    transitionProgress = 1.0f;
    maxRainDrops = 1000;
    fogDensity = 0.0f;
}

WeatherSystem::~WeatherSystem() {
}

void WeatherSystem::Initialize() {
    rainDrops.reserve(maxRainDrops);
    TraceLog(LOG_INFO, "Weather system initialized");
}

void WeatherSystem::Update(float deltaTime, Vector3 playerPos) {
    // Transition to target intensity
    if (transitionProgress < 1.0f) {
        transitionProgress += deltaTime * 0.5f;
        if (transitionProgress > 1.0f) transitionProgress = 1.0f;
    }
    
    // Update weather effects based on type
    switch (currentWeather) {
        case WEATHER_RAIN:
        case WEATHER_STORM:
            SpawnRain(playerPos);
            UpdateRain(deltaTime);
            break;
            
        case WEATHER_FOG:
            fogDensity = intensity * 0.5f;
            break;
            
        case WEATHER_CLEAR:
            fogDensity *= 0.95f; // Fade out fog
            break;
            
        default:
            break;
    }
}

void WeatherSystem::Draw(const Camera3D& camera) {
    switch (currentWeather) {
        case WEATHER_RAIN:
        case WEATHER_STORM:
            DrawRain(camera);
            break;
            
        case WEATHER_FOG:
            DrawFog();
            break;
            
        default:
            break;
    }
}

void WeatherSystem::SetWeather(WeatherType type) {
    if (type != currentWeather) {
        currentWeather = type;
        transitionProgress = 0.0f;
        
        // Set target intensity based on weather type
        switch (type) {
            case WEATHER_CLEAR:
                intensity = 0.0f;
                break;
            case WEATHER_RAIN:
                intensity = 0.6f;
                break;
            case WEATHER_FOG:
                intensity = 0.8f;
                break;
            case WEATHER_STORM:
                intensity = 1.0f;
                break;
            default:
                intensity = 0.5f;
                break;
        }
        
        const char* weatherNames[] = {"Clear", "Rain", "Fog", "Storm", "Snow"};
        TraceLog(LOG_INFO, "Weather changed to: %s", weatherNames[type]);
    }
}

float WeatherSystem::GetFogDensity() const {
    float baseFog = 0.0f;
    
    switch (currentWeather) {
        case WEATHER_FOG:
            baseFog = 0.8f;
            break;
        case WEATHER_RAIN:
            baseFog = 0.3f;
            break;
        case WEATHER_STORM:
            baseFog = 0.5f;
            break;
        default:
            baseFog = 0.0f;
            break;
    }
    
    return baseFog * intensity * transitionProgress;
}

Color WeatherSystem::GetFogColor() const {
    if (g_DayNightCycle) {
        return g_DayNightCycle->GetFogColor();
    }
    return Color{100, 100, 110, 255};
}

void WeatherSystem::SpawnRain(Vector3 playerPos) {
    int targetDrops = (int)(maxRainDrops * intensity);
    
    while ((int)rainDrops.size() < targetDrops) {
        RainDrop drop;
        
        // Spawn above and around player
        drop.position.x = playerPos.x + (rand() % 100 - 50);
        drop.position.y = playerPos.y + 30.0f + (rand() % 20);
        drop.position.z = playerPos.z + (rand() % 100 - 50);
        
        // Rain falls down with slight wind
        drop.velocity.x = -2.0f + (rand() % 100) / 100.0f;
        drop.velocity.y = -20.0f - (rand() % 10);
        drop.velocity.z = 0.0f;
        
        drop.lifetime = 3.0f;
        
        rainDrops.push_back(drop);
    }
}

void WeatherSystem::UpdateRain(float deltaTime) {
    for (auto it = rainDrops.begin(); it != rainDrops.end();) {
        it->position = Vector3Add(it->position, Vector3Scale(it->velocity, deltaTime));
        it->lifetime -= deltaTime;
        
        // Remove drops that hit ground or expired
        if (it->position.y < 0.0f || it->lifetime <= 0.0f) {
            it = rainDrops.erase(it);
        } else {
            ++it;
        }
    }
}

void WeatherSystem::DrawRain(const Camera3D& camera) {
    // Draw rain as lines
    for (const auto& drop : rainDrops) {
        Vector3 endPos = Vector3Add(drop.position, Vector3Scale(drop.velocity, 0.05f));
        
        // Only draw rain close to camera
        float distance = Vector3Distance(camera.position, drop.position);
        if (distance < 50.0f) {
            float alpha = 1.0f - (distance / 50.0f);
            Color rainColor = Color{200, 200, 220, (unsigned char)(alpha * 150)};
            DrawLine3D(drop.position, endPos, rainColor);
        }
    }
}

void WeatherSystem::DrawFog() {
    if (fogDensity > 0.01f) {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        
        Color fogColor = GetFogColor();
        fogColor.a = (unsigned char)(fogDensity * 180);
        
        DrawRectangle(0, 0, screenW, screenH, fogColor);
    }
}

// Global initialization
void InitializeWeatherSystem() {
    g_WeatherSystem = new WeatherSystem();
    g_WeatherSystem->Initialize();
    TraceLog(LOG_INFO, "Weather system initialized");
}

void CleanupWeatherSystem() {
    if (g_WeatherSystem) {
        delete g_WeatherSystem;
        g_WeatherSystem = nullptr;
    }
    TraceLog(LOG_INFO, "Weather system cleaned up");
}
