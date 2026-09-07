class UOS_Notifications
{
    static void Send(PlayerBase player, string text)
    {
        if (!player || !player.GetIdentity())
            return;

        Param1<string> param = new Param1<string>("[Outfit Swap] " + text);
        GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, param, true, player.GetIdentity());
    }
};
