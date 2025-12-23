#include "daynight_system.h"
#include <cmath>

// Global instance
DayNightCycle* g_DayNightCycle = nullptr;

DayNightCycle::DayNightCycle() {
    currentTime = 12.0f; // Start at noon
    timeSpeed = 0.1f;    // 0.1 hours per second (1 day = ~4 minutes)
}

DayNightCycle::~DayNightCycle() {
}

void DayNightCycle::Initialize() {
    TraceLog(LOG_INFO, "Day/Night Cycle initialized - Starting at %.1f:00", currentTime);
}

void DayNightCycle::Update(float deltaTime) {
    currentTime += timeSpeed * deltaTime;
    
    // Wrap around 24 hours
    if (currentTime >= 24.0f) {
        currentTime -= 24.0f;
        TraceLog(LOG_INFO, "New day started");
    }
}

void DayNightCycle::SetTime(float time) {
    currentTime = fmodf(time, 24.0f);
    if (currentTime < 0.0f) currentTime += 24.0f;
    TraceLog(LOG_INFO, "Time set to %.1f:00", currentTime);
}

Vector3 DayNightCycle::GetSunDirection() const {
    // Sun moves in an arc across the sky
    // At 6:00 it rises in the east, at 12:00 it's overhead, at 18:00 it sets in the west
    
    float angle = (currentTime - 6.0f) * (PI / 12.0f); // -PI/2 to PI/2
    
    Vector3 direction;
    direction.x = cosf(angle);
    direction.y = sinf(angle);
    direction.z = 0.0f;
    
    return Vector3Normalize(direction);
}

Color DayNightCycle::GetSunColor() const {
    return CalculateSunColor();
}

Color DayNightCycle::GetAmbientColor() const {
    return CalculateAmbientColor();
}

Color DayNightCycle::GetSkyColor() const {
    return CalculateSkyColor();
}

Color DayNightCycle::GetFogColor() const {
    // Fog color matches sky but darker
    Color skyColor = GetSkyColor();
    return Color{
        (unsigned char)(skyColor.r * 0.6f),
        (unsigned char)(skyColor.g * 0.6f),
        (unsigned char)(skyColor.b * 0.6f),
        255
    };
}

float DayNightCycle::GetLightIntensity() const {
    if (currentTime >= 6.0f && currentTime <= 18.0f) {
        // Daytime: 6:00 to 18:00
        if (currentTime < 8.0f) {
            // Dawn: 6:00 to 8:00
            return (currentTime - 6.0f) / 2.0f * 0.8f + 0.2f;
        } else if (currentTime > 16.0f) {
            // Dusk: 16:00 to 18:00
            return (18.0f - currentTime) / 2.0f * 0.8f + 0.2f;
        } else {
            // Full day
            return 1.0f;
        }
    } else {
        // Nighttime
        return 0.15f; // Moonlight
    }
}

Color DayNightCycle::CalculateSunColor() const {
    if (currentTime >= 5.0f && currentTime <= 7.0f) {
        // Sunrise - orange/red
        float t = (currentTime - 5.0f) / 2.0f;
        return Color{
            255,
            (unsigned char)(180 + t * 75),
            (unsigned char)(100 + t * 100),
            255
        };
    } else if (currentTime > 7.0f && currentTime < 17.0f) {
        // Daytime - bright yellow
        return Color{255, 250, 220, 255};
    } else if (currentTime >= 17.0f && currentTime <= 19.0f) {
        // Sunset - orange/red
        float t = (19.0f - currentTime) / 2.0f;
        return Color{
            255,
            (unsigned char)(180 + t * 75),
            (unsigned char)(100 + t * 100),
            255
        };
    } else {
        // Night - moon light (blue-ish)
        return Color{150, 170, 200, 255};
    }
}

Color DayNightCycle::CalculateAmbientColor() const {
    float intensity = GetLightIntensity();
    
    if (IsNight()) {
        // Night ambient - dark blue
        return Color{
            (unsigned char)(20 * intensity),
            (unsigned char)(30 * intensity),
            (unsigned char)(60 * intensity),
            255
        };
    } else {
        // Day ambient - neutral
        return Color{
            (unsigned char)(180 * intensity),
            (unsigned char)(190 * intensity),
            (unsigned char)(200 * intensity),
            255
        };
    }
}

Color DayNightCycle::CalculateSkyColor() const {
    if (currentTime >= 5.0f && currentTime <= 7.0f) {
        // Sunrise
        float t = (currentTime - 5.0f) / 2.0f;
        return Color{
            (unsigned char)(255 * (1.0f - t) + 135 * t),
            (unsigned char)(180 * (1.0f - t) + 206 * t),
            (unsigned char)(150 * (1.0f - t) + 235 * t),
            255
        };
    } else if (currentTime > 7.0f && currentTime < 17.0f) {
        // Day - bright blue
        return Color{135, 206, 235, 255};
    } else if (currentTime >= 17.0f && currentTime <= 19.0f) {
        // Sunset
        float t = (19.0f - currentTime) / 2.0f;
        return Color{
            (unsigned char)(255 * (1.0f - t) + 135 * t),
            (unsigned char)(150 * (1.0f - t) + 206 * t),
            (unsigned char)(100 * (1.0f - t) + 235 * t),
            255
        };
    } else {
        // Night - dark blue/black
        return Color{15, 20, 40, 255};
    }
}

// Global initialization
void InitializeDayNightSystem() {
    g_DayNightCycle = new DayNightCycle();
    g_DayNightCycle->Initialize();
    TraceLog(LOG_INFO, "Day/Night system initialized");
}

void CleanupDayNightSystem() {
    if (g_DayNightCycle) {
        delete g_DayNightCycle;
        g_DayNightCycle = nullptr;
    }
    TraceLog(LOG_INFO, "Day/Night system cleaned up");
}
