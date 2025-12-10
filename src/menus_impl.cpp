#include "menus.h"
#include "fileio.h"
#include <fstream>
#include "sound_manager.h"

// Graphics settings menu implementation
void DrawGraphicsSettingsMenu(int screenW, int screenH, GraphicsSettings* settings, int* selection, GameState* nextState) {
    int menuW = 800;
    int menuH = 700;
    int menuX = (screenW - menuW) / 2;
    int menuY = (screenH - menuH) / 2;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);
    DrawText("GRAPHICS SETTINGS", menuX + 20, menuY + 10, 26, PIPBOY_GREEN);

    bool useController = isControllerEnabled && IsGamepadAvailable(0);

    // Navigation (11 options now)
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        *selection = (*selection - 1 + 11) % 11;
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        *selection = (*selection + 1) % 11;
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }

    // Draw current settings info
    int infoY = menuY + 50;
    DrawText(TextFormat("Current: %s", AVAILABLE_RESOLUTIONS[settings->resolutionIndex].label),
        menuX + 20, infoY, 16, PIPBOY_DIM);
    DrawText(TextFormat("FPS: %d | Render Scale: %.0f%%",
        settings->targetFPS, settings->renderScale * 100.0f),
        menuX + 20, infoY + 20, 16, PIPBOY_DIM);

    // Draw options
    int optY = menuY + 90;
    const char* msaaLabels[] = { "OFF", "2x", "4x", "8x" };
    int msaaIndex = settings->msaa ? (settings->msaaSamples == 2 ? 1 : settings->msaaSamples == 4 ? 2 : 3) : 0;

    std::vector<std::string> optionLabels = {
        TextFormat("Resolution: %s", AVAILABLE_RESOLUTIONS[settings->resolutionIndex].label),
        TextFormat("V-Sync: %s", settings->vsync ? "ON" : "OFF"),
        TextFormat("MSAA: %s", msaaLabels[msaaIndex]),
        TextFormat("Target FPS: %d", settings->targetFPS),
        TextFormat("Render Scale: %.0f%%", settings->renderScale * 100.0f),
        TextFormat("Show FPS: %s", settings->showFPS ? "ON" : "OFF"),
        TextFormat("LOD System: %s", settings->enableLOD ? "ON" : "OFF"),
        TextFormat("Frustum Culling: %s", settings->enableFrustumCulling ? "ON" : "OFF"),
        TextFormat("Max Draw Calls: %d", settings->maxDrawCalls),
        "Apply Changes",
        "Back"
    };

    for (int i = 0; i < 11; i++) {
        Color bgColor = (*selection == i) ? PIPBOY_SELECTED : PIPBOY_DARK;
        Color fgColor = (*selection == i) ? PIPBOY_GREEN : PIPBOY_DIM;

        DrawRectangle(menuX + 20, optY, menuW - 40, 45, bgColor);
        DrawRectangleLines(menuX + 20, optY, menuW - 40, 45, (*selection == i) ? PIPBOY_GREEN : PIPBOY_DIM);
        DrawText(optionLabels[i].c_str(), menuX + 30, optY + 12, 18, fgColor);

        // Mouse selection
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuW - 20 &&
            mousePos.y >= optY && mousePos.y <= optY + 45) {
            *selection = i;
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }

        optY += 50;
    }

    // Handle input for adjusting values
    bool leftPressed = IsKeyPressed(KEY_LEFT) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_LEFT));
    bool rightPressed = IsKeyPressed(KEY_RIGHT) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_RIGHT));
    bool enterPressed = IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));

    if (leftPressed || rightPressed) {
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }

    switch (*selection) {
    case 0: // Resolution
        if (leftPressed) settings->resolutionIndex = (settings->resolutionIndex - 1 + RESOLUTION_COUNT) % RESOLUTION_COUNT;
        if (rightPressed) settings->resolutionIndex = (settings->resolutionIndex + 1) % RESOLUTION_COUNT;
        break;
    case 1: // V-Sync
        if (leftPressed || rightPressed || enterPressed) settings->vsync = !settings->vsync;
        break;
    case 2: // MSAA
        if (leftPressed) {
            if (!settings->msaa) settings->msaa = true, settings->msaaSamples = 8;
            else if (settings->msaaSamples == 2) settings->msaa = false;
            else if (settings->msaaSamples == 4) settings->msaaSamples = 2;
            else if (settings->msaaSamples == 8) settings->msaaSamples = 4;
        }
        if (rightPressed) {
            if (!settings->msaa) settings->msaa = true, settings->msaaSamples = 2;
            else if (settings->msaaSamples == 2) settings->msaaSamples = 4;
            else if (settings->msaaSamples == 4) settings->msaaSamples = 8;
            else if (settings->msaaSamples == 8) settings->msaa = false;
        }
        break;
    case 3: // Target FPS
        if (leftPressed) {
            if (settings->targetFPS > 30) settings->targetFPS -= 30;
            if (settings->targetFPS == 30) settings->targetFPS = 0; // Unlimited
        }
        if (rightPressed) {
            if (settings->targetFPS == 0) settings->targetFPS = 30;
            else if (settings->targetFPS < 240) settings->targetFPS += 30;
        }
        break;
    case 4: // Render Scale
        if (leftPressed && settings->renderScale > 0.5f) settings->renderScale -= 0.1f;
        if (rightPressed && settings->renderScale < 1.0f) settings->renderScale += 0.1f;
        settings->renderScale = Clamp(settings->renderScale, 0.5f, 1.0f);
        break;
    case 5: // Show FPS
        if (leftPressed || rightPressed || enterPressed) settings->showFPS = !settings->showFPS;
        break;
    case 6: // LOD System
        if (leftPressed || rightPressed || enterPressed) settings->enableLOD = !settings->enableLOD;
        break;
    case 7: // Frustum Culling
        if (leftPressed || rightPressed || enterPressed) settings->enableFrustumCulling = !settings->enableFrustumCulling;
        break;
    case 8: // Max Draw Calls
        if (leftPressed && settings->maxDrawCalls > 100) settings->maxDrawCalls -= 100;
        if (rightPressed && settings->maxDrawCalls < 5000) settings->maxDrawCalls += 100;
        break;
    case 9: // Apply
        if (enterPressed) {
            ApplyGraphicsSettings(*settings);
            SaveGraphicsSettings(*settings);
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.7f);
            TraceLog(LOG_INFO, "Graphics settings applied and saved");
        }
        break;
    case 10: // Back
        if (enterPressed) {
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_BACK, 0.5f);
            *nextState = GameState::Settings;
            gameState = GameState::Settings;
        }
        break;
    }

    // Instructions
    DrawText("Use Arrow Keys to adjust values", menuX + 20, menuY + menuH - 60, 16, PIPBOY_DIM);
    DrawText("Press ENTER to select | ESC to go back", menuX + 20, menuY + menuH - 35, 16, PIPBOY_DIM);
}

// Updated DrawSettingsMenu with sound support
void DrawSettingsMenu(int screenW, int screenH, bool* showMinimap, bool* isControllerEnabled, bool* isFullscreen, int* settingsSelection, GameState* nextState) {
    int menuW = 600;
    int menuH = 600;
    int menuX = (screenW - menuW) / 2;
    int menuY = (screenH - menuH) / 2;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);
    DrawText("SETTINGS", menuX + 20, menuY + 10, 28, PIPBOY_GREEN);

    bool useController = *isControllerEnabled && IsGamepadAvailable(0);

    // Navigation (8 options now - added audio settings)
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        *settingsSelection = (*settingsSelection - 1 + 8) % 8;
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        *settingsSelection = (*settingsSelection + 1) % 8;
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }

    // Draw options
    int optY = menuY + 60;

    float masterVol = g_SoundManager ? g_SoundManager->GetMasterVolume() : 1.0f;
    float sfxVol = g_SoundManager ? g_SoundManager->GetSFXVolume() : 1.0f;
    float musicVol = g_SoundManager ? g_SoundManager->GetMusicVolume() : 1.0f;

    const char* options[] = {
        TextFormat("Show Minimap: %s", *showMinimap ? "ON" : "OFF"),
        TextFormat("Controller Enabled: %s", *isControllerEnabled ? "ON" : "OFF"),
        TextFormat("Fullscreen: %s", *isFullscreen ? "ON" : "OFF"),
        TextFormat("Master Volume: %.0f%%", masterVol * 100.0f),
        TextFormat("SFX Volume: %.0f%%", sfxVol * 100.0f),
        TextFormat("Music Volume: %.0f%%", musicVol * 100.0f),
        "Graphics Settings",
        "Controller Bindings",
        "Back"
    };

    for (int i = 0; i < 9; i++) {
        Color bgColor = (*settingsSelection == i) ? PIPBOY_SELECTED : PIPBOY_DARK;
        Color fgColor = (*settingsSelection == i) ? PIPBOY_GREEN : PIPBOY_DIM;

        DrawRectangle(menuX + 20, optY, menuW - 40, 50, bgColor);
        DrawRectangleLines(menuX + 20, optY, menuW - 40, 50, (*settingsSelection == i) ? PIPBOY_GREEN : PIPBOY_DIM);
        DrawText(options[i], menuX + 30, optY + 15, 20, fgColor);

        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuW - 20 &&
            mousePos.y >= optY && mousePos.y <= optY + 50) {
            *settingsSelection = i;
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }

        optY += 55;
    }

    bool leftPressed = IsKeyPressed(KEY_LEFT) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_LEFT));
    bool rightPressed = IsKeyPressed(KEY_RIGHT) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_RIGHT));
    bool enterPressed = IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));

    if (leftPressed || rightPressed) {
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }

    switch (*settingsSelection) {
    case 0: // Minimap
        if (enterPressed || leftPressed || rightPressed) {
            *showMinimap = !(*showMinimap);
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }
        break;
    case 1: // Controller
        if (enterPressed || leftPressed || rightPressed) {
            *isControllerEnabled = !(*isControllerEnabled);
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }
        break;
    case 2: // Fullscreen
        if (enterPressed || leftPressed || rightPressed) {
            *isFullscreen = !(*isFullscreen);
            ToggleFullscreen();
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }
        break;
    case 3: // Master Volume
        if (leftPressed && g_SoundManager) {
            float vol = Clamp(masterVol - 0.1f, 0.0f, 1.0f);
            g_SoundManager->SetMasterVolume(vol);
        }
        if (rightPressed && g_SoundManager) {
            float vol = Clamp(masterVol + 0.1f, 0.0f, 1.0f);
            g_SoundManager->SetMasterVolume(vol);
        }
        break;
    case 4: // SFX Volume
        if (leftPressed && g_SoundManager) {
            float vol = Clamp(sfxVol - 0.1f, 0.0f, 1.0f);
            g_SoundManager->SetSFXVolume(vol);
            g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }
        if (rightPressed && g_SoundManager) {
            float vol = Clamp(sfxVol + 0.1f, 0.0f, 1.0f);
            g_SoundManager->SetSFXVolume(vol);
            g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }
        break;
    case 5: // Music Volume
        if (leftPressed && g_SoundManager) {
            float vol = Clamp(musicVol - 0.1f, 0.0f, 1.0f);
            g_SoundManager->SetMusicVolume(vol);
        }
        if (rightPressed && g_SoundManager) {
            float vol = Clamp(musicVol + 0.1f, 0.0f, 1.0f);
            g_SoundManager->SetMusicVolume(vol);
        }
        break;
    case 6: // Graphics
        if (enterPressed) {
            gameState = GameState::GraphicsSettings;
            graphicsSettingsSelection = 0;
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }
        break;
    case 7: // Controller Bindings
        if (enterPressed) {
            gameState = GameState::ControllerBindings;
            controllerSettingsSelection = 0;
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }
        break;
    case 8: // Back
        if (enterPressed) {
            *nextState = stateBeforeSettings;
            gameState = stateBeforeSettings;
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_BACK, 0.5f);
        }
        break;
    }

    DrawText("Press ENTER to toggle/select", menuX + 20, menuY + menuH - 60, 16, PIPBOY_DIM);
    DrawText("Press ESC to go back", menuX + 20, menuY + menuH - 35, 16, PIPBOY_DIM);
}

// Graphics settings persistence - UPDATED with all settings
void SaveGraphicsSettings(const GraphicsSettings& settings) {
    std::ofstream file("graphics_settings.cfg");
    if (file.is_open()) {
        file << "resolution_index " << settings.resolutionIndex << "\n";
        file << "vsync " << (settings.vsync ? 1 : 0) << "\n";
        file << "msaa " << (settings.msaa ? 1 : 0) << "\n";
        file << "msaa_samples " << settings.msaaSamples << "\n";
        file << "target_fps " << settings.targetFPS << "\n";
        file << "render_scale " << settings.renderScale << "\n";
        file << "show_fps " << (settings.showFPS ? 1 : 0) << "\n";
        file << "enable_lod " << (settings.enableLOD ? 1 : 0) << "\n";
        file << "enable_frustum_culling " << (settings.enableFrustumCulling ? 1 : 0) << "\n";
        file << "max_draw_calls " << settings.maxDrawCalls << "\n";
        file << "upscaling_mode " << settings.upscalingMode << "\n";
        file << "upscaling_quality " << settings.upscalingQuality << "\n";
        file.close();
        TraceLog(LOG_INFO, "Graphics settings saved");
    }
}
void LoadGraphicsSettings(GraphicsSettings* settings) {
    std::ifstream file("graphics_settings.cfg");
    if (file.is_open()) {
        std::string key;
        while (file >> key) {
            if (key == "resolution_index") file >> settings->resolutionIndex;
            else if (key == "vsync") { int v; file >> v; settings->vsync = (v != 0); }
            else if (key == "msaa") { int v; file >> v; settings->msaa = (v != 0); }
            else if (key == "msaa_samples") file >> settings->msaaSamples;
            else if (key == "target_fps") file >> settings->targetFPS;
            else if (key == "render_scale") file >> settings->renderScale;
            else if (key == "show_fps") { int v; file >> v; settings->showFPS = (v != 0); }
            else if (key == "enable_lod") { int v; file >> v; settings->enableLOD = (v != 0); }
            else if (key == "enable_frustum_culling") { int v; file >> v; settings->enableFrustumCulling = (v != 0); }
            else if (key == "max_draw_calls") file >> settings->maxDrawCalls;
            else if (key == "upscaling_mode") { int v; file >> v; settings->upscalingMode = (UpscalingMode)v; }
            else if (key == "upscaling_quality") { int v; file >> v; settings->upscalingQuality = (UpscalingQuality)v; }
        }
        file.close();
        TraceLog(LOG_INFO, "Graphics settings loaded");
    }
    else {
        // Default settings
        settings->resolutionIndex = 2; // 1280x720
        settings->vsync = true;
        settings->msaa = false;
        settings->msaaSamples = 4;
        settings->targetFPS = 60;
        settings->renderScale = 1.0f;
        settings->showFPS = false;
        settings->enableLOD = true;
        settings->enableFrustumCulling = true;
        settings->maxDrawCalls = 1000;
        settings->upscalingMode = UPSCALING_NONE;
        settings->upscalingQuality = UPSCALE_QUALITY_QUALITY;
    }
}

void ApplyGraphicsSettings(const GraphicsSettings& settings) {
    const Resolution& res = AVAILABLE_RESOLUTIONS[settings.resolutionIndex];

    // Apply resolution
    if (!IsWindowFullscreen()) {
        SetWindowSize(res.width, res.height);
    }

    // Apply V-Sync
    if (settings.vsync) {
        SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    }
    else {
        SetTargetFPS(settings.targetFPS > 0 ? settings.targetFPS : 0);
    }

    // MSAA configuration hint (applied at window creation)
    if (settings.msaa) {
        SetConfigFlags(FLAG_MSAA_4X_HINT); // Raylib will handle this
    }

    TraceLog(LOG_INFO, TextFormat("Applied graphics: %s, VSync: %s, MSAA: %s, FPS: %d, Frustum Culling: %s",
        res.label, settings.vsync ? "ON" : "OFF",
        settings.msaa ? TextFormat("%dx", settings.msaaSamples) : "OFF",
        settings.targetFPS,
        settings.enableFrustumCulling ? "ON" : "OFF"));
}

// Draw the load/save menu - UPDATED with sound
void DrawLoadMenu(int screenW, int screenH, int* selectedSlot, GameState currentState) {
    int menuW = 600;
    int menuH = 400;
    int menuX = (screenW - menuW) / 2;
    int menuY = (screenH - menuH) / 2;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);

    const char* title = (currentState == GameState::Paused) ? "SAVE GAME" : "LOAD GAME";
    DrawText(title, menuX + 20, menuY + 10, 28, PIPBOY_GREEN);

    // Navigation
    bool useController = isControllerEnabled && IsGamepadAvailable(0);
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        *selectedSlot = (*selectedSlot - 1 + MAX_SAVE_SLOTS) % MAX_SAVE_SLOTS;
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        *selectedSlot = (*selectedSlot + 1) % MAX_SAVE_SLOTS;
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }

    // Draw slots
    int slotY = menuY + 60;
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        bool fileExists = SaveFileExists(i + 1);
        Color bgColor = (*selectedSlot == i) ? PIPBOY_SELECTED : PIPBOY_DARK;
        Color fgColor = fileExists ? PIPBOY_GREEN : PIPBOY_DIM;

        DrawRectangle(menuX + 20, slotY, menuW - 40, 60, bgColor);
        DrawRectangleLines(menuX + 20, slotY, menuW - 40, 60, (*selectedSlot == i) ? PIPBOY_GREEN : PIPBOY_DIM);

        DrawText(TextFormat("Slot %d", i + 1), menuX + 30, slotY + 10, 20, fgColor);
        if (fileExists) {
            DrawText("Save file exists", menuX + 30, slotY + 35, 16, PIPBOY_DIM);
        }
        else {
            DrawText("Empty", menuX + 30, slotY + 35, 16, PIPBOY_DIM);
        }

        // Mouse selection
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuW - 20 &&
            mousePos.y >= slotY && mousePos.y <= slotY + 60) {
            *selectedSlot = i;
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.5f);
        }

        slotY += 70;
    }

    // Instructions
    if (currentState == GameState::Paused) {
        DrawText("Press ENTER to save", menuX + 20, menuY + menuH - 60, 18, PIPBOY_GREEN);
    }
    else {
        DrawText("Press ENTER to load", menuX + 20, menuY + menuH - 60, 18, PIPBOY_GREEN);
    }
    DrawText("Press ESC to cancel", menuX + 20, menuY + menuH - 35, 18, PIPBOY_DIM);
}

// Draw the controller bindings menu - UPDATED with sound
void DrawControllerBindings(int screenW, int screenH, int* activeBindingIndex, bool* isBindingMode, int* controllerSettingsSelection, ControllerBinding* currentBindings) {
    int menuW = 700;
    int menuH = 500;
    int menuX = (screenW - menuW) / 2;
    int menuY = (screenH - menuH) / 2;

    DrawRectangle(menuX, menuY, menuW, menuH, PIPBOY_DARK);
    DrawRectangleLines(menuX, menuY, menuW, menuH, PIPBOY_GREEN);
    DrawText("CONTROLLER BINDINGS", menuX + 20, menuY + 10, 26, PIPBOY_GREEN);

    if (*isBindingMode) {
        DrawRectangle(0, 0, screenW, screenH, Color{ 0, 0, 0, 180 });
        DrawRectangle(screenW / 2 - 200, screenH / 2 - 50, 400, 100, PIPBOY_DARK);
        DrawRectangleLines(screenW / 2 - 200, screenH / 2 - 50, 400, 100, PIPBOY_GREEN);
        DrawText("Press any button to bind...", screenW / 2 - 140, screenH / 2 - 20, 20, PIPBOY_GREEN);
        DrawText("Press ESC to cancel", screenW / 2 - 100, screenH / 2 + 10, 16, PIPBOY_DIM);

        // Check for button press
        if (IsGamepadAvailable(0)) {
            for (int btn = 0; btn < GAMEPAD_BUTTON_COUNT; btn++) {
                if (IsGamepadButtonPressed(0, btn)) {
                    currentBindings[*activeBindingIndex].isAxis = false;
                    currentBindings[*activeBindingIndex].inputId = btn;
                    currentBindings[*activeBindingIndex].threshold = 0.0f;
                    *isBindingMode = false;
                    *activeBindingIndex = -1;
                    if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.7f);
                    break;
                }
            }
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            *isBindingMode = false;
            *activeBindingIndex = -1;
            if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_BACK, 0.5f);
        }

        return;
    }

    bool useController = isControllerEnabled && IsGamepadAvailable(0);

    // Navigation
    if (IsKeyPressed(KEY_UP) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_UP))) {
        *controllerSettingsSelection = (*controllerSettingsSelection - 1 + ACTION_COUNT) % ACTION_COUNT;
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }
    if (IsKeyPressed(KEY_DOWN) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_DPAD_DOWN))) {
        *controllerSettingsSelection = (*controllerSettingsSelection + 1) % ACTION_COUNT;
        if (g_SoundManager) g_SoundManager->PlaySound(SND_UI_SELECT, 0.3f);
    }

    // Draw bindings
    int bindY = menuY + 50;
    const char* actionNames[] = { "Jump", "Sprint", "Inventory", "Crafting", "Map", "Flashlight", "Use Item", "Shoot" };

    for (int i = 0; i < ACTION_COUNT; i++) {
        Color bgColor = (*controllerSettingsSelection == i) ? PIPBOY_SELECTED : PIPBOY_DARK;
        Color fgColor = (*controllerSettingsSelection == i) ? PIPBOY_GREEN : PIPBOY_DIM;

        DrawRectangle(menuX + 20, bindY, menuW - 40, 45, bgColor);
        DrawRectangleLines(menuX + 20, bindY, menuW - 40, 45, (*controllerSettingsSelection == i) ? PIPBOY_GREEN : PIPBOY_DIM);

        DrawText(actionNames[i], menuX + 30, bindY + 13, 18, fgColor);

        const char* bindingText;
        if (currentBindings[i].isAxis) {
            bindingText = TextFormat("Axis %d (%.1f)", currentBindings[i].inputId, currentBindings[i].threshold);
        }
        else {
            bindingText = currentBindings[i].actionName;
        }
        DrawText(bindingText, menuX + 300, bindY + 13, 18, fgColor);

        // Mouse selection
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            mousePos.x >= menuX + 20 && mousePos.x <= menuX + menuW - 20 &&
            mousePos.y >= bindY && mousePos.y <= bindY + 45) {
            *controllerSettingsSelection = i;
        }

        bindY += 50;
    }

    // Handle rebinding
    if (IsKeyPressed(KEY_ENTER) || (useController && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) {
        *isBindingMode = true;
        *activeBindingIndex = *controllerSettingsSelection;
    }

    DrawText("Press ENTER to rebind", menuX + 20, menuY + menuH - 60, 16, PIPBOY_DIM);
    DrawText("Press ESC to go back", menuX + 20, menuY + menuH - 35, 16, PIPBOY_DIM);
}