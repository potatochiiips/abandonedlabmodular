#include "items.h"

const char* GetItemName(int itemId) { 
    switch(itemId) {
        case ITEM_WATER_BOTTLE: return "Water Bottle";
        case ITEM_LAB_KEY: return "Lab Key";
        case ITEM_FLASHLIGHT: return "Flashlight";
        case ITEM_WOOD: return "Wood";
        case ITEM_STONE: return "Stone";
        case ITEM_POTATO_CHIPS: return "Chips";
        case ITEM_PISTOL: return "Pistol";
        case ITEM_MAG: return "Magazine";
        default: return "Empty";
    }
}