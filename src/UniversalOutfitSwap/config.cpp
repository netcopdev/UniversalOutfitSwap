class CfgPatches
{
    class UniversalOutfitSwap
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts"
        };
    };
};

class CfgMods
{
    class UniversalOutfitSwap
    {
        dir = "UniversalOutfitSwap";
        name = "Universal Outfit Swap";
        type = "mod";
        author = "netcopdev";
        version = "0.1.2";
        dependencies[] =
        {
            "Game",
            "World",
            "Mission"
        };

        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] =
                {
                    "UniversalOutfitSwap/Scripts/3_Game"
                };
            };

            class worldScriptModule
            {
                value = "";
                files[] =
                {
                    "UniversalOutfitSwap/Scripts/4_World"
                };
            };

            class missionScriptModule
            {
                value = "";
                files[] =
                {
                    "UniversalOutfitSwap/Scripts/5_Mission"
                };
            };
        };
    };
};
