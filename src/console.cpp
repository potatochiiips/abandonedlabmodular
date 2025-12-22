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
        history.push_back("Available commands:");
        history.push_back("  help - Show this help message");
        history.push_back("  noclip - Toggle noclip mode");
        history.push_back("  setstat <stat> <value> - Set player stat (health/stamina/hunger/thirst)");
        history.push_back("  setfov <value> - Set FOV (30-120)");
        history.push_back("  god - Toggle god mode (infinite health)");
        history.push_back("  spawn <item> - Spawn item in inventory");
        history.push_back("  clear - Clear console history");
        history.push_back("  teleport <x> <y> <z> - Teleport to coordinates");
        history.push_back("  time <hour> - Set time of day (0-24)");
        history.push_back("  weather <type> - Set weather (clear/rain/fog)");
    }
    else if (command == "noclip" && isNoclip) {
        *isNoclip = !(*isNoclip);
        history.push_back(TextFormat("Noclip %s", *isNoclip ? "enabled." : "disabled."));
    }
    else if (command == "god") {
        if (health) {
            *health = 100.0f;
            history.push_back("God mode activated - Health set to 100");
        }
    }
    else if (command == "clear") {
        history.clear();
        history.push_back("Console cleared.");
    }
    else if (command == "setstat") {
        std::string statName;
        float value;
        ss >> statName >> value;

        if (statName.empty() || ss.fail()) {
            history.push_back("Usage: setstat <stat> <value>");
            history.push_back("Stats: health, stamina, hunger, thirst");
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
    else if (command == "spawn") {
        std::string itemName;
        ss >> itemName;
        if (itemName.empty()) {
            history.push_back("Usage: spawn <item>");
            history.push_back("Items: water, key, flashlight, wood, stone, chips, pistol, mag, m16, m16mag, knife");
        }
        else {
            history.push_back(TextFormat("Spawning %s... (not implemented yet)", itemName.c_str()));
        }
    }
    else if (command == "teleport") {
        float x, y, z;
        ss >> x >> y >> z;
        if (ss.fail()) {
            history.push_back("Usage: teleport <x> <y> <z>");
        }
        else {
            history.push_back(TextFormat("Teleport to (%.1f, %.1f, %.1f) - not implemented yet", x, y, z));
        }
    }
    else if (command == "time") {
        int hour;
        ss >> hour;
        if (ss.fail() || hour < 0 || hour > 24) {
            history.push_back("Usage: time <hour> (0-24)");
        }
        else {
            history.push_back(TextFormat("Time set to %d:00 - not implemented yet", hour));
        }
    }
    else if (command == "weather") {
        std::string weather;
        ss >> weather;
        if (weather.empty()) {
            history.push_back("Usage: weather <type> (clear/rain/fog)");
        }
        else {
            history.push_back(TextFormat("Weather set to %s - not implemented yet", weather.c_str()));
        }
    }
    else {
        history.push_back("Unknown command. Type 'help' for list of commands.");
    }

    consoleInput[0] = '\0';
    consoleInputLength = 0;
}

void DrawConsole(int screenW, int screenH, const std::vector<std::string>& history, const char* input, int inputLength) {
    // Semi-transparent background
    DrawRectangle(0, 0, screenW, screenH / 2, Color{ 0, 0, 0, 200 });
    DrawRectangleLines(0, 0, screenW, screenH / 2, PIPBOY_GREEN);

    // Title
    DrawText("CONSOLE", 10, 5, 20, PIPBOY_GREEN);
    DrawText("Press ` or ESC to close", screenW - 200, 5, 14, PIPBOY_DIM);

    int fontSize = 15;
    int maxLines = (screenH / 2 - 60) / (fontSize + 2);
    int histSize = static_cast<int>(history.size());
    int startLine = (histSize > maxLines) ? histSize - maxLines : 0;

    for (int i = startLine; i < histSize; ++i) {
        DrawText(history[i].c_str(), 10, 30 + (i - startLine) * (fontSize + 2), fontSize, PIPBOY_GREEN);
    }

    // Input line background
    DrawRectangle(0, screenH / 2 - 30, screenW, 30, Color{ 0, 50, 0, 220 });
    DrawText(TextFormat("] %s_", input ? input : ""), 10, screenH / 2 - 25, 18, PIPBOY_GREEN);
}

void UpdateConsoleInput(float* health, float* stamina, float* hunger, float* thirst, bool* isNoclip, float* fov) {
    int key = GetCharPressed();
    while (key > 0) {
        // Handle printable characters
        if (key >= 32 && key <= 126 && consoleInputLength < MAX_COMMAND_LENGTH - 1) {
            consoleInput[consoleInputLength++] = static_cast<char>(key);
            consoleInput[consoleInputLength] = '\0';
        }
        key = GetCharPressed();
    }

    // Handle special keys
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (consoleInputLength > 0) {
            consoleInput[--consoleInputLength] = '\0';
        }
    }
    else if (IsKeyPressed(KEY_ENTER)) {
        ProcessConsoleCommand(consoleHistory, health, stamina, hunger, thirst, isNoclip, fov);
    }
}