modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();

        UOS_ConfigManager.Get();
        UOS_Log.Info("v" + UOS_VERSION + " initialized.");
    }
};
