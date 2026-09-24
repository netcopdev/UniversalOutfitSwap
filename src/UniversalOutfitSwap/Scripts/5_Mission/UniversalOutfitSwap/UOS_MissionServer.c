modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();

        UOS_ConfigManager.Get();
        UOS_Log.Info("v" + UOS_VERSION + " initialized.");
    }

    override void OnClientReadyEvent(PlayerIdentity identity, PlayerBase player)
    {
        super.OnClientReadyEvent(identity, player);
        UOS_ConfigSync.SendToClient(player, identity);
    }

    override void OnClientReconnectEvent(PlayerIdentity identity, PlayerBase player)
    {
        super.OnClientReconnectEvent(identity, player);
        UOS_ConfigSync.SendToClient(player, identity);
    }

    override void OnClientRespawnEvent(PlayerIdentity identity, PlayerBase player)
    {
        super.OnClientRespawnEvent(identity, player);
        UOS_ConfigSync.SendToClient(player, identity);
    }
};
