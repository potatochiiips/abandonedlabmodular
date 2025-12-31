#include "hud.h"
#include "items.h"

// Global instance
HUDManager* g_HUDManager = nullptr;

HUDManager::HUDManager() {
    lowHealthWarning = false;
    lowHealthPulse = 0.0f;
    notificationTimer = 0.0f;
    vignetteTexture = { 0 };
}

HUDManager::~HUDManager() {
    if (vignetteTexture.id > 0) {
        UnloadRenderTexture(vignetteTexture);
    }
}

void HUDManager::Initialize() {
    TraceLog(LOG_INFO, "HUD Manager initialized");
}

void HUDManager::Update(float deltaTime) {
    // Update animations
    healthBarAnim.Update(deltaTime);
    staminaBarAnim.Update(deltaTime);
    hungerBarAnim.Update(deltaTime);
    thirstBarAnim.Update(deltaTime);
    damageVignetteAnim.Update(deltaTime);
    hitMarkerAnim.Update(deltaTime);

    // Update low health pulse
    if (lowHealthWarning) {
        lowHealthPulse += deltaTime * 5.0f;
    }

    // Update notification timer
    if (notificationTimer > 0.0f) {
        notificationTimer -= deltaTime;
    }
}

void HUDManager::Draw(int screenW, int screenH) {
    // This is called by individual HUD element functions
}

void HUDManager::DrawPlayerStats(int screenW, int screenH, float health, float stamina, float hunger, float thirst) {
    int barX = 20;
    int barY = screenH - 120;
    int barWidth = 200;
    int barHeight = 20;
    int barSpacing = 25;

    // Health bar (red)
    DrawModernBar(barX, barY, barWidth, barHeight, health / 100.0f,
        Color{ 220, 50, 50, 255 }, Color{ 40, 10, 10, 200 }, "HP");

    // Stamina bar (green)
    DrawModernBar(barX, barY + barSpacing, barWidth, barHeight, stamina / 100.0f,
        Color{ 50, 220, 50, 255 }, Color{ 10, 40, 10, 200 }, "STAM");

    // Hunger bar (orange)
    DrawModernBar(barX, barY + barSpacing * 2, barWidth, barHeight, hunger / 100.0f,
        Color{ 220, 150, 50, 255 }, Color{ 40, 30, 10, 200 }, "FOOD");

    // Thirst bar (blue)
    DrawModernBar(barX, barY + barSpacing * 3, barWidth, barHeight, thirst / 100.0f,
        Color{ 50, 150, 220, 255 }, Color{ 10, 30, 40, 200 }, "H2O");
}

void HUDManager::DrawWeaponInfo(int screenW, int screenH, const InventorySlot& weapon) {
    if (weapon.itemId == ITEM_NONE) return;

    int infoX = screenW - 220;
    int infoY = screenH - 100;

    // Draw weapon name
    const char* weaponName = GetItemName(weapon.itemId);
    DrawText(weaponName, infoX, infoY, 24, PIPBOY_GREEN);

    // Draw ammo counter
    if (weapon.itemId == ITEM_PISTOL || weapon.itemId == ITEM_M16) {
        int maxAmmo = (weapon.itemId == ITEM_PISTOL) ? 15 : 30;
        DrawText(TextFormat("%d / %d", weapon.ammo, maxAmmo),
            infoX, infoY + 30, 32, WHITE);
    }
}

void HUDManager::DrawFlashlightStatus(int screenW, int screenH, float battery, bool isOn) {
    if (!isOn) return;

    int iconX = screenW - 100;
    int iconY = screenH - 150;

    // Draw flashlight icon
    DrawCircle(iconX, iconY, 15, Color{ 255, 255, 200, 200 });

    // Draw battery bar
    int barW = 60;
    int barH = 8;
    int barX = iconX - barW / 2;
    int barY = iconY + 20;

    Color batteryColor = battery > 50.0f ? PIPBOY_GREEN :
        battery > 25.0f ? YELLOW : RED;

    DrawRectangle(barX, barY, barW, barH, Color{ 40, 40, 40, 200 });
    DrawRectangle(barX, barY, (int)(barW * (battery / 100.0f)), barH, batteryColor);
    DrawRectangleLines(barX, barY, barW, barH, PIPBOY_GREEN);
}

void HUDManager::DrawCrosshair(int screenW, int screenH, bool isAiming) {
    int centerX = screenW / 2;
    int centerY = screenH / 2;

    float spread = isAiming ? 5.0f : 10.0f;
    Color crosshairColor = PIPBOY_GREEN;

    DrawModernCrosshair(centerX, centerY, spread, crosshairColor);
}

void HUDManager::DrawHitMarker(int screenW, int screenH) {
    if (hitMarkerAnim.current <= 0.0f) return;

    int centerX = screenW / 2;
    int centerY = screenH / 2;
    int size = 20;

    unsigned char alpha = (unsigned char)(hitMarkerAnim.current * 255);
    Color markerColor = Color{ 255, 255, 255, alpha };

    // Draw X shape
    DrawLine(centerX - size, centerY - size, centerX + size, centerY + size, markerColor);
    DrawLine(centerX + size, centerY - size, centerX - size, centerY + size, markerColor);
}

void HUDManager::DrawDamageVignette(int screenW, int screenH, float damageIntensity) {
    if (damageVignetteAnim.current <= 0.0f) return;

    unsigned char alpha = (unsigned char)(damageVignetteAnim.current * 150);
    Color vignetteColor = Color{ 255, 0, 0, alpha };

    // Draw red vignette effect
    int vignetteSize = 100;
    DrawRectangleGradientEx(
        Rectangle{ 0, 0, (float)vignetteSize, (float)screenH },
        vignetteColor, Color{ 255, 0, 0, 0 }, Color{ 255, 0, 0, 0 }, vignetteColor
    );
    DrawRectangleGradientEx(
        Rectangle{ (float)(screenW - vignetteSize), 0, (float)vignetteSize, (float)screenH },
        Color{ 255, 0, 0, 0 }, vignetteColor, vignetteColor, Color{ 255, 0, 0, 0 }
    );
    DrawRectangleGradientEx(
        Rectangle{ 0, 0, (float)screenW, (float)vignetteSize },
        vignetteColor, vignetteColor, Color{ 255, 0, 0, 0 }, Color{ 255, 0, 0, 0 }
    );
    DrawRectangleGradientEx(
        Rectangle{ 0, (float)(screenH - vignetteSize), (float)screenW, (float)vignetteSize },
        Color{ 255, 0, 0, 0 }, Color{ 255, 0, 0, 0 }, vignetteColor, vignetteColor
    );
}

void HUDManager::DrawCompass(int screenW, int screenH, float yaw) {
    int compassX = screenW / 2;
    int compassY = 30;
    int compassRadius = 50;

    DrawCircle(compassX, compassY, compassRadius, Color{ 0, 0, 0, 150 });
    DrawCircleLines(compassX, compassY, compassRadius, PIPBOY_GREEN);

    // Draw cardinal directions
    const char* directions[] = { "N", "E", "S", "W" };
    float angles[] = { 0, 90, 180, 270 };

    for (int i = 0; i < 4; i++) {
        float angle = (angles[i] - yaw) * DEG2RAD;
        float dx = sinf(angle) * (compassRadius - 10);
        float dy = -cosf(angle) * (compassRadius - 10);

        DrawText(directions[i],
            compassX + (int)dx - 5,
            compassY + (int)dy - 5,
            20, PIPBOY_GREEN);
    }
}

void HUDManager::DrawNotification(const std::string& message, float duration) {
    currentNotification = message;
    notificationTimer = duration;
}

void HUDManager::ShowDamageIndicator(float damage) {
    damageVignetteAnim.SetTarget(1.0f, 3.0f);
    damageVignetteAnim.current = 1.0f;
}

void HUDManager::ShowHitMarker() {
    hitMarkerAnim.SetTarget(1.0f, 2.0f);
    hitMarkerAnim.current = 1.0f;
}

void HUDManager::SetLowHealthWarning(bool enabled) {
    lowHealthWarning = enabled;
}

void HUDManager::DrawModernBar(int x, int y, int width, int height, float value,
    Color fillColor, Color bgColor, const char* label) {
    // Background
    DrawRectangle(x, y, width, height, bgColor);

    // Fill
    int fillWidth = (int)(width * value);
    DrawRectangle(x, y, fillWidth, height, fillColor);

    // Border
    DrawRectangleLines(x, y, width, height, PIPBOY_GREEN);

    // Label
    if (label) {
        DrawText(label, x + 5, y + 3, 14, WHITE);
    }

    // Value text
    DrawText(TextFormat("%.0f%%", value * 100.0f),
        x + width - 45, y + 3, 14, WHITE);
}

void HUDManager::DrawRadialBar(int centerX, int centerY, float radius, float value,
    Color color, float thickness) {
    float angle = value * 360.0f;

    for (float a = 0; a < angle; a += 1.0f) {
        float rad = a * DEG2RAD;
        float x1 = centerX + cosf(rad) * (radius - thickness);
        float y1 = centerY + sinf(rad) * (radius - thickness);
        float x2 = centerX + cosf(rad) * radius;
        float y2 = centerY + sinf(rad) * radius;

        DrawLine((int)x1, (int)y1, (int)x2, (int)y2, color);
    }
}

void HUDManager::DrawGlowingText(const char* text, int x, int y, int fontSize,
    Color color, Color glowColor) {
    // Draw glow
    DrawText(text, x - 1, y - 1, fontSize, glowColor);
    DrawText(text, x + 1, y - 1, fontSize, glowColor);
    DrawText(text, x - 1, y + 1, fontSize, glowColor);
    DrawText(text, x + 1, y + 1, fontSize, glowColor);

    // Draw main text
    DrawText(text, x, y, fontSize, color);
}

void HUDManager::DrawModernCrosshair(int centerX, int centerY, float spread, Color color) {
    int lineLength = 8;
    int gap = (int)spread;

    // Top
    DrawLine(centerX, centerY - gap - lineLength, centerX, centerY - gap, color);
    // Bottom
    DrawLine(centerX, centerY + gap, centerX, centerY + gap + lineLength, color);
    // Left
    DrawLine(centerX - gap - lineLength, centerY, centerX - gap, centerY, color);
    // Right
    DrawLine(centerX + gap, centerY, centerX + gap + lineLength, centerY, color);

    // Center dot
    DrawCircle(centerX, centerY, 2, color);
}