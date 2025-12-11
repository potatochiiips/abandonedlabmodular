#pragma once
#include "globals.h"

// Prototypes for all menu drawing functions
void DrawMenu(int screenW, int screenH, const std::vector<std::string>& options, int* selectedIndex, bool useController, const char* title);
void DrawLoadMenu(int screenW, int screenH, int* selectedSlot, GameState currentState);
void DrawSettingsMenu(int screenW, int screenH, bool* showMinimap, bool* isControllerEnabled, bool* isFullscreen, int* settingsSelection, GameState* nextState);
void DrawControllerBindings(int screenW, int screenH, int* activeBindingIndex, bool* isBindingMode, int* controllerSettingsSelection, ControllerBinding* currentBindings);
void DrawGraphicsSettingsMenu(int screenW, int screenH, GraphicsSettings* settings, int* selection, GameState* nextState);
void SaveGraphicsSettings(const GraphicsSettings& settings);
void LoadGraphicsSettings(GraphicsSettings* settings);
void ApplyGraphicsSettings(const GraphicsSettings& settings);
void DrawAudioSettingsMenu(int screenW, int screenH, int* selection, GameState* nextState);
