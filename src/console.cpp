#include "console.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <cmath>

// Global console variable definitions
char consoleInput[MAX_COMMAND_LENGTH] = "";
int consoleInputLength = 0;
std::vector<std::string> consoleHistory;

void ProcessConsoleCommand(std::vector<std::string>& consoleHistory, float* health, float* stamina, float* hunger, float* thirst, bool* isNoclip, float* fov) {
    // [Implementation of ProcessConsoleCommand]
    if (consoleInputLength == 0) return;

    std::string commandLine = consoleInput;
    consoleHistory.push_back("> " + commandLine);
    
    std::stringstream ss(commandLine);
    std::string command;
    ss >> command;

    // Use well-defined char->lower conversion
    std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    if (command == "help") {
        consoleHistory.push_back("Available commands: help, noclip, setstat <stat> <value>, setfov <value>");
    } else if (command == "noclip") {
        *isNoclip = !(*isNoclip);
        consoleHistory.push_back(TextFormat("Noclip %s", *isNoclip ? "enabled." : "disabled."));
    } else if (command == "setstat") {
        std::string statName;
        float value;
        ss >> statName >> value;
        
        if (statName.empty() || ss.fail()) {
            consoleHistory.push_back("Usage: setstat <stat> <value>");
        } else {
            std::transform(statName.begin(), statName.end(), statName.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            
            bool known = true;
            if (statName == "health") *health = fmaxf(0.0f, fminf(100.0f, value));
            else if (statName == "stamina") *stamina = fmaxf(0.0f, fminf(100.0f, value));
            else if (statName == "hunger") *hunger = fmaxf(0.0f, fminf(100.0f, value));
            else if (statName == "thirst") *thirst = fmaxf(0.0f, fminf(100.0f, value));
            else known = false;

            if (!known) {
                consoleHistory.push_back("Unknown stat. Use health, stamina, hunger, or thirst.");
            } else {
                consoleHistory.push_back(TextFormat("%s set to %.0f", statName.c_str(), value));
            }
        }
    } else if (command == "setfov") {
        float value;
        ss >> value;
        if (ss.fail() || value < 30.0f || value > 120.0f) {
            consoleHistory.push_back("Usage: setfov <value> (30-120)");
        } else {
            *fov = value;
            consoleHistory.push_back(TextFormat("FOV set to %.0f", value));
        }
    } else {
        consoleHistory.push_back("Unknown command. Type 'help'.");
    }

    // Clear input buffer
    consoleInput[0] = '\0';
    consoleInputLength = 0;
}

void DrawConsole(int screenW, int screenH, const std::vector<std::string>& history, const char* input, int inputLength) {
    DrawRectangle(0, 0, screenW, screenH / 2, Color{0, 0, 0, 220});
    DrawRectangleLines(0, 0, screenW, screenH / 2, PIPBOY_GREEN);

    int maxLines = (screenH / 2 - 50) / 15;
    int histSize = static_cast<int>(history.size());
    int startLine = (histSize > maxLines) ? histSize - maxLines : 0;
    
    // Draw history
    for(int i = startLine; i < histSize; ++i) {
        DrawText(history[i].c_str(), 10, 10 + (i - startLine) * 15, 15, PIPBOY_GREEN);
    }

    // Draw input line
    DrawRectangle(0, screenH / 2 - 25, screenW, 25, PIPBOY_SELECTED);
    DrawText(TextFormat("] %s_", input ? input : ""), 10, screenH / 2 - 20, 18, PIPBOY_GREEN);
}
void UpdateConsoleInput() {
    int key = GetKeyPressed();
    while (key != 0) {
        if (key == KEY_BACKSPACE) {
            if (consoleInputLength > 0) {
                consoleInput[--consoleInputLength] = '\0';
            }
        } else if (key == KEY_ENTER) {
            // Process command
            ProcessConsoleCommand(consoleHistory, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        } else if (consoleInputLength < MAX_COMMAND_LENGTH - 1 && isprint(key)) {
            consoleInput[consoleInputLength++] = static_cast<char>(key);
            consoleInput[consoleInputLength] = '\0';
        }
        key = GetKeyPressed();
    }
}
