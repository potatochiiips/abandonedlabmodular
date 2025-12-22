#include "console.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <cmath>

// Global console variable definitions
char consoleInput[MAX_COMMAND_LENGTH] = "";
int consoleInputLength = 0;
std::vector<std::string> consoleHistory;

void ProcessConsoleCommand(std::vector<std::string>& history, float* health, float* stamina, float* hunger, float* thirst, bool* isNoclip, float* fov) {
    if (consoleInputLength == 0) return;

    std::string commandLine = consoleInput;
    history.push_back("> " + commandLine);

    std::stringstream ss(commandLine);
    std::string command;
    ss >> command;

    std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (command == "help") {
        history.push_back("Available: help, noclip, setstat <stat> <value>, setfov <value>");
    }
    else if (command == "noclip" && isNoclip) {
        *isNoclip = !(*isNoclip);
        history.push_back(TextFormat("Noclip %s", *isNoclip ? "enabled." : "disabled."));
    }
    else if (command == "setstat") {
        std::string statName;
        float value;
        ss >> statName >> value;

        if (statName.empty() || ss.fail()) {
            history.push_back("Usage: setstat <stat> <value>");
        }
        else {
            std::transform(statName.begin(), statName.end(), statName.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            bool known = true;
            if (statName == "health" && health) *health = fmaxf(0.0f, fminf(100.0f, value));
            else if (statName == "stamina" && stamina) *stamina = fmaxf(0.0f, fminf(100.0f, value));
            else if (statName == "hunger" && hunger) *hunger = fmaxf(0.0f, fminf(100.0f, value));
            else if (statName == "thirst" && thirst) *thirst = fmaxf(0.0f, fminf(100.0f, value));
            else known = false;

            if (!known) history.push_back("Unknown stat. Use health, stamina, hunger, or thirst.");
            else history.push_back(TextFormat("%s set to %.0f", statName.c_str(), value));
        }
    }
    else if (command == "setfov" && fov) {
        float value;
        ss >> value;
        if (ss.fail() || value < 30.0f || value > 120.0f) {
            history.push_back("Usage: setfov <value> (30-120)");
        }
        else {
            *fov = value;
            history.push_back(TextFormat("FOV set to %.0f", value));
        }
    }
    else {
        history.push_back("Unknown command. Type 'help'.");
    }

    consoleInput[0] = '\0';
    consoleInputLength = 0;
}

void DrawConsole(int screenW, int screenH, const std::vector<std::string>& history, const char* input, int inputLength) {
    // Semi-transparent background (alpha 160) to prevent a "black screen"
    DrawRectangle(0, 0, screenW, screenH / 2, Color{ 0, 0, 0, 160 });
    DrawRectangleLines(0, 0, screenW, screenH / 2, PIPBOY_GREEN);

    int fontSize = 15;
    int maxLines = (screenH / 2 - 40) / (fontSize + 2);
    int histSize = static_cast<int>(history.size());
    int startLine = (histSize > maxLines) ? histSize - maxLines : 0;

    for (int i = startLine; i < histSize; ++i) {
        DrawText(history[i].c_str(), 10, 10 + (i - startLine) * (fontSize + 2), fontSize, PIPBOY_GREEN);
    }

    // Input line background
    DrawRectangle(0, screenH / 2 - 25, screenW, 25, Color{ 0, 40, 0, 200 });
    DrawText(TextFormat("] %s_", input ? input : ""), 10, screenH / 2 - 20, 18, PIPBOY_GREEN);
}

void UpdateConsoleInput(float* health, float* stamina, float* hunger, float* thirst, bool* isNoclip, float* fov) {
    int key = GetKeyPressed();
    while (key != 0) {
        if (key == KEY_BACKSPACE) {
            if (consoleInputLength > 0) {
                consoleInput[--consoleInputLength] = '\0';
            }
        }
        else if (key == KEY_ENTER) {
            // Passing the pointers to ensure the logic has access to the game state
            ProcessConsoleCommand(consoleHistory, health, stamina, hunger, thirst, isNoclip, fov);
        }
        else if (consoleInputLength < MAX_COMMAND_LENGTH - 1 && key >= 32 && key <= 126) {
            consoleInput[consoleInputLength++] = static_cast<char>(key);
            consoleInput[consoleInputLength] = '\0';
        }
        key = GetKeyPressed();
    }
}