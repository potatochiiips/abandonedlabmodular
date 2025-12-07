#pragma once
#include "globals.h"

// Global console variables (defined in console.cpp)
extern char consoleInput[MAX_COMMAND_LENGTH];
extern int consoleInputLength;
extern std::vector<std::string> consoleHistory;

void ProcessConsoleCommand(std::vector<std::string>& consoleHistory, float* health, float* stamina, float* hunger, float* thirst, bool* isNoclip, float* fov);
// Helper for main.cpp
void DrawConsole(int screenW, int screenH, const std::vector<std::string>& history, const char* input, int inputLength);
void UpdateConsoleInput();