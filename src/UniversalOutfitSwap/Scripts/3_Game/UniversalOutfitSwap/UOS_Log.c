class UOS_Log
{
    static void Info(string text)
    {
        Print(UOS_TAG + " " + text);
    }

    static void Debug(string text)
    {
        UOS_Config cfg = UOS_ConfigManager.Get();
        if (cfg && cfg.DebugLogging)
            Print(UOS_TAG + " [DEBUG] " + text);
    }
};
