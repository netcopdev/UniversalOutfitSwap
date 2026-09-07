const string UOS_TAG = "[UniversalOutfitSwap]";
const string UOS_CONFIG_DIR = "$profile:UniversalOutfitSwap";
const string UOS_CONFIG_PATH = "$profile:UniversalOutfitSwap/UniversalOutfitSwap.json";
const string UOS_VERSION = "0.1.2";

enum UOS_Operation
{
    UOS_OP_SWAP = 0,
    UOS_OP_STORE = 1,
    UOS_OP_EQUIP = 2
};

class UOS_Slots
{
    static void GetWearableSlots(out array<string> slots)
    {
        slots = new array<string>();
        slots.Insert("Headgear");
        slots.Insert("Eyewear");
        slots.Insert("Mask");
        slots.Insert("Armband");
        slots.Insert("Gloves");
        slots.Insert("Body");
        slots.Insert("Vest");
        slots.Insert("Back");
        slots.Insert("Hips");
        slots.Insert("Legs");
        slots.Insert("Feet");
    }

    static void GetDefaultRequiredSlots(out array<string> slots)
    {
        slots = new array<string>();
        slots.Insert("Body");
        slots.Insert("Legs");
        slots.Insert("Feet");
    }
};
