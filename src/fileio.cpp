#include "fileio.h"
#include <sys/stat.h> // for stat()

// Implements file saving and loading logic using the globals.h structs.
// [Content of SaveFileExists, SaveGame, LoadGame implementations from source]
bool SaveFileExists(int slotIndex) {
    std::string filename = TextFormat(SAVE_FILE_NAME_FORMAT, slotIndex);
    struct stat buffer;   
    return (stat(filename.c_str(), &buffer) == 0); 
}
// ... SaveGame and LoadGame implementations here (omitted for brevity, copied from source)
void SaveGame(int slotIndex, Vector3 pos, float yaw, float pitch, float hp, float stam, float hung, float thirst, InventorySlot* inv, float batt, bool lightOn, char map[MAP_SIZE][MAP_SIZE], float fov) {
    std::string filename = TextFormat(SAVE_FILE_NAME_FORMAT, slotIndex);
    std::ofstream outfile(filename);

    if (outfile.is_open()) {
        // --- Player & Camera State ---
        outfile << "pos " << pos.x << " " << pos.y << " " << pos.z << "\n";
        outfile << "yaw " << yaw << "\n";
        outfile << "pitch " << pitch << "\n";
        outfile << "fov " << fov << "\n"; // Save FOV
        
        // --- Stats ---
        outfile << "health " << hp << "\n";
        outfile << "stamina " << stam << "\n";
        outfile << "hunger " << hung << "\n";
        outfile << "thirst " << thirst << "\n";
        
        // --- Flashlight ---
        outfile << "battery " << batt << "\n";
        outfile << "lightOn " << lightOn << "\n";
        
        // --- Inventory ---
        outfile << "inventory_start\n";
        for (int i = 0; i < TOTAL_INVENTORY_SLOTS; i++) {
            outfile << inv[i].itemId << " " << inv[i].quantity << " " << inv[i].ammo << "\n";
        }
        outfile << "inventory_end\n";
        
        // --- Map Data (Simple block of text) ---
        outfile << "map_start\n";
        for (int i = 0; i < MAP_SIZE; i++) {
            for (int j = 0; j < MAP_SIZE; j++) {
                outfile << (int)map[i][j];
            }
            outfile << "\n";
        }
        outfile << "map_end\n";

        outfile.close();
        TraceLog(LOG_INFO, TextFormat("Game saved to slot %d.", slotIndex));
    } else {
        TraceLog(LOG_ERROR, TextFormat("Failed to open save file %s for writing.", filename.c_str()));
    }
}

bool LoadGame(int slotIndex, Vector3* pos, float* yaw, float* pitch, float* hp, float* stam, float* hung, float* thirst, InventorySlot* inv, float* batt, bool* lightOn, char map[MAP_SIZE][MAP_SIZE], float* fov) {
    std::string filename = TextFormat(SAVE_FILE_NAME_FORMAT, slotIndex);
    std::ifstream infile(filename);

    if (!infile.is_open()) {
        TraceLog(LOG_WARNING, TextFormat("Save file %s not found.", filename.c_str()));
        return false;
    }

    std::string line;
    std::string key;
    
    bool readingInventory = false;
    bool readingMap = false;
    int invIndex = 0;
    int mapRow = 0;

    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        
        if (readingInventory) {
            if (line == "inventory_end") {
                readingInventory = false;
                continue;
            }
            if (invIndex < TOTAL_INVENTORY_SLOTS) {
                ss >> inv[invIndex].itemId >> inv[invIndex].quantity >> inv[invIndex].ammo;
                invIndex++;
            }
            continue;
        }
        
        if (readingMap) {
            if (line == "map_end") {
                readingMap = false;
                continue;
            }
            if (mapRow < MAP_SIZE) {
                if (line.length() >= MAP_SIZE) {
                    for (int j = 0; j < MAP_SIZE; j++) {
                        map[mapRow][j] = line[j] - '0'; // Convert char to int tile value
                    }
                    mapRow++;
                }
            }
            continue;
        }

        ss >> key;
        if (key == "pos") {
            ss >> pos->x >> pos->y >> pos->z;
        } else if (key == "yaw") {
            ss >> *yaw;
        } else if (key == "pitch") {
            ss >> *pitch;
        } else if (key == "fov") {
            ss >> *fov;
        } else if (key == "health") {
            ss >> *hp;
        } else if (key == "stamina") {
            ss >> *stam;
        } else if (key == "hunger") {
            ss >> *hung;
        } else if (key == "thirst") {
            ss >> *thirst;
        } else if (key == "battery") {
            ss >> *batt;
        } else if (key == "lightOn") {
            int lO;
            ss >> lO;
            *lightOn = (lO != 0);
        } else if (key == "inventory_start") {
            readingInventory = true;
        } else if (key == "map_start") {
            readingMap = true;
        }
    }

    infile.close();
    TraceLog(LOG_INFO, TextFormat("Game loaded from slot %d.", slotIndex));
    return true;
}