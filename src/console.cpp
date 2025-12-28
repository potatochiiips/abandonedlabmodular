#include "console.h"
#include "daynight_system.h"
#include "weather_system.h"
#include "zombie_system.h"
#include "player.h"
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
        history.push_back("  setstat <stat> <value> - Set player stat");
        history.push_back("  setfov <value> - Set FOV (30-120)");
        history.push_back("  god - Toggle god mode");
        history.push_back("  spawn <item> - Spawn item");
        history.push_back("  clear - Clear console");
        history.push_back("  teleport <x> <y> <z> - Teleport");
        history.push_back("  time <hour> - Set time (0-24)");
        history.push_back("  weather <type> - Set weather");
        history.push_back("  spawnzombie - Spawn zombie nearby");
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
        }
        else {
            std::transform(statName.begin(), statName.end(), statName.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            bool known = true;
            if (statName == "health" && health) *health = fmaxf(0.0f, fminf(100.0f, value));
            else if (statName == "stamina" && stamina) *stamina = fmaxf(0.0f, fminf(100.0f, value));
            else if (statName == "hunger" && hunger) *hunger = fmaxf(0.0f, fminf(100.0f, value));
            else if (statName == "thirst" && thirst) *thirst = fmaxf(0.0f, fminf(100.0f, value));
            else known = false;

            if (!known) history.push_back("Unknown stat.");
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
        }
        else {
            extern InventorySlot inventory[TOTAL_INVENTORY_SLOTS];
            int itemId = ITEM_NONE;

            if (itemName == "water") itemId = ITEM_WATER_BOTTLE;
            else if (itemName == "key") itemId = ITEM_LAB_KEY;
            else if (itemName == "flashlight") itemId = ITEM_FLASHLIGHT;
            else if (itemName == "wood") itemId = ITEM_WOOD;
            else if (itemName == "stone") itemId = ITEM_STONE;
            else if (itemName == "chips") itemId = ITEM_POTATO_CHIPS;
            else if (itemName == "pistol") itemId = ITEM_PISTOL;
            else if (itemName == "mag") itemId = ITEM_MAG;
            else if (itemName == "m16") itemId = ITEM_M16;
            else if (itemName == "m16mag") itemId = ITEM_M16_MAG;
            else if (itemName == "knife") itemId = ITEM_KNIFE;

            if (itemId != ITEM_NONE) {
                extern bool AddItemToInventory(InventorySlot * inventory, int itemId, int quantity, int ammo);
                if (AddItemToInventory(inventory, itemId, 1, 0)) {
                    history.push_back(TextFormat("Spawned %s", itemName.c_str()));
                }
                else {
                    history.push_back("Inventory full!");
                }
            }
            else {
                history.push_back(TextFormat("Unknown item: %s", itemName.c_str()));
            }
        }
    }
    else if (command == "teleport") {
        float x, y, z;
        ss >> x >> y >> z;
        if (ss.fail()) {
            history.push_back("Usage: teleport <x> <y> <z>");
        }
        else {
            extern Vector3 playerPosition;
            extern Camera3D camera;
            playerPosition = Vector3{ x, y, z };
            camera.position = playerPosition;
            history.push_back(TextFormat("Teleported to (%.1f, %.1f, %.1f)", x, y, z));
        }
    }
    else if (command == "time") {
        int hour;
        ss >> hour;
        if (ss.fail() || hour < 0 || hour > 24) {
            history.push_back("Usage: time <hour> (0-24)");
        }
        else {
            if (g_DayNightCycle) {
                g_DayNightCycle->SetTime((float)hour);
                history.push_back(TextFormat("Time set to %d:00", hour));
            }
            else {
                history.push_back("Day/night system not available");
            }
        }
    }
    else if (command == "weather") {
        std::string weather;
        ss >> weather;
        if (weather.empty()) {
            history.push_back("Usage: weather <type> (clear/rain/fog/storm)");
        }
        else {
            if (g_WeatherSystem) {
                WeatherType type = WEATHER_CLEAR;
                if (weather == "clear") type = WEATHER_CLEAR;
                else if (weather == "rain") type = WEATHER_RAIN;
                else if (weather == "fog") type = WEATHER_FOG;
                else if (weather == "storm") type = WEATHER_STORM;

                g_WeatherSystem->SetWeather(type);
                history.push_back(TextFormat("Weather set to %s", weather.c_str()));
            }
            else {
                history.push_back("Weather system not available");
            }
        }
    }
    else if (command == "spawnzombie") {
        if (g_ZombieManager) {
            extern Vector3 playerPosition;
            Vector3 spawnPos = playerPosition;
            spawnPos.x += 5.0f;
            g_ZombieManager->SpawnZombie(spawnPos);
            history.push_back("Zombie spawned nearby");
        }
        else {
            history.push_back("Zombie system not available");
        }
    }
    else {
        history.push_back("Unknown command. Type 'help' for list of commands.");
    }

    consoleInput[0] = '\0';
    consoleInputLength = 0;
}

void DrawConsole(int screenW, int screenH, const std::vector<std::string>& history, const char* input, int inputLength) {
    DrawRectangle(0, 0, screenW, screenH / 2, Color{ 0, 0, 0, 200 });
    DrawRectangleLines(0, 0, screenW, screenH / 2, PIPBOY_GREEN);

    DrawText("CONSOLE", 10, 5, 20, PIPBOY_GREEN);
    DrawText("Press ` or ESC to close", screenW - 200, 5, 14, PIPBOY_DIM);

    int fontSize = 15;
    int maxLines = (screenH / 2 - 60) / (fontSize + 2);
    int histSize = static_cast<int>(history.size());
    int startLine = (histSize > maxLines) ? histSize - maxLines : 0;

    for (int i = startLine; i < histSize; ++i) {
        DrawText(history[i].c_str(), 10, 30 + (i - startLine) * (fontSize + 2), fontSize, PIPBOY_GREEN);
    }

    DrawRectangle(0, screenH / 2 - 30, screenW, 30, Color{ 0, 50, 0, 220 });
    DrawText(TextFormat("] %s_", input ? input : ""), 10, screenH / 2 - 25, 18, PIPBOY_GREEN);
}

// FIXED: Console input handling - check game state properly
void UpdateConsoleInput(float* health, float* stamina, float* hunger, float* thirst, bool* isNoclip, float* fov) {
    // FIXED: Only process typed characters, not special keys
    int key = GetCharPressed();
    while (key > 0) {
        // Only accept printable characters (space to ~)
        if (key >= 32 && key <= 126 && consoleInputLength < MAX_COMMAND_LENGTH - 1) {
            consoleInput[consoleInputLength] = (char)key;
            consoleInputLength++;
            consoleInput[consoleInputLength] = '\0';
        }
        key = GetCharPressed();
    }

    // Handle backspace separately
    if (IsKeyPressed(KEY_BACKSPACE) && consoleInputLength > 0) {
        consoleInputLength--;
        consoleInput[consoleInputLength] = '\0';
    }

    // Handle enter separately
    if (IsKeyPressed(KEY_ENTER)) {
        ProcessConsoleCommand(consoleHistory, health, stamina, hunger, thirst, isNoclip, fov);
    }
}