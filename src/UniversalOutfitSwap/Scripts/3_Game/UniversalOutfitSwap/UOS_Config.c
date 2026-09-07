class UOS_Config
{
    bool Enabled = true;

    bool EnableSwapOutfit = true;
    bool EnableStoreOutfit = true;
    bool EnableEquipOutfit = true;

    int MinWearableSlots = 6;
    bool RequireOpenWhenOpenable = true;
    bool NotifyOnSuccess = true;
    bool NotifyOnFailure = true;
    bool DebugLogging = false;

    ref array<string> RequiredSlots;
    ref array<string> IncludeClasses;
    ref array<string> ExcludeClasses;

    void UOS_Config()
    {
        UOS_Slots.GetDefaultRequiredSlots(RequiredSlots);
        IncludeClasses = new array<string>();
        ExcludeClasses = new array<string>();
    }
};

class UOS_ConfigManager
{
    protected static ref UOS_Config s_Config;
    protected static bool s_ServerLoaded;

    static UOS_Config Get()
    {
        if (!s_Config)
            s_Config = new UOS_Config();

        if (GetGame() && GetGame().IsServer() && !s_ServerLoaded)
            LoadServerConfig();

        return s_Config;
    }

    static void LoadServerConfig()
    {
        if (s_ServerLoaded)
            return;

        s_ServerLoaded = true;

        if (!FileExist(UOS_CONFIG_DIR))
            MakeDirectory(UOS_CONFIG_DIR);

        if (!FileExist(UOS_CONFIG_PATH))
        {
            JsonFileLoader<UOS_Config>.JsonSaveFile(UOS_CONFIG_PATH, s_Config);
            Print(UOS_TAG + " Created default config: " + UOS_CONFIG_PATH);
            return;
        }

        UOS_Config loaded = new UOS_Config();
        JsonFileLoader<UOS_Config>.JsonLoadFile(UOS_CONFIG_PATH, loaded);
        s_Config = loaded;

        Normalize();
        Print(UOS_TAG + " Loaded config: " + UOS_CONFIG_PATH);
    }

    protected static void Normalize()
    {
        if (!s_Config)
            s_Config = new UOS_Config();

        if (!s_Config.RequiredSlots)
            UOS_Slots.GetDefaultRequiredSlots(s_Config.RequiredSlots);

        if (!s_Config.IncludeClasses)
            s_Config.IncludeClasses = new array<string>();

        if (!s_Config.ExcludeClasses)
            s_Config.ExcludeClasses = new array<string>();

        if (s_Config.MinWearableSlots < 1)
            s_Config.MinWearableSlots = 1;
    }
};
