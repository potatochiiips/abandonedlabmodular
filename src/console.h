#pragma once
#include "globals.h"

// Global console variables (defined in console.cpp)
extern char consoleInput[MAX_COMMAND_LENGTH];
extern int consoleInputLength;
extern std::vector<std::string> consoleHistory;

// Process a command and modify game state directly
void ProcessConsoleCommand(std::vector<std::string>& history, float* health, float* stamina, float* hunger, float* thirst, bool* isNoclip, float* fov);

// Renders the console overlay
void DrawConsole(int screenW, int screenH, const std::vector<std::string>& history, const char* input, int inputLength);

// Handles keyboard input for the console; requires game state pointers to avoid crashes
void UpdateConsoleInput(float* health, float* stamina, float* hunger, float* thirst, bool* isNoclip, float* fov);