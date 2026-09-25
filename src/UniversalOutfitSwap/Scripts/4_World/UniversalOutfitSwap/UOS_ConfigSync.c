class UOS_ConfigSync
{
    protected static void WriteStringArray(ScriptRPC rpc, array<string> values)
    {
        int count = 0;
        if (values)
            count = values.Count();

        rpc.Write(count);
        for (int i = 0; i < count; i++)
            rpc.Write(values.Get(i));
    }

    protected static bool ReadStringArray(ParamsReadContext ctx, out array<string> values)
    {
        values = new array<string>();

        int count;
        if (!ctx.Read(count) || count < 0 || count > 128)
            return false;

        for (int i = 0; i < count; i++)
        {
            string value;
            if (!ctx.Read(value))
                return false;
            values.Insert(value);
        }

        return true;
    }

    static void SendToClient(PlayerBase player, PlayerIdentity identity)
    {
        if (!GetGame() || !GetGame().IsServer() || !player || !identity)
            return;

        UOS_Config cfg = UOS_ConfigManager.Get();
        if (!cfg)
            return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(UOS_CONFIG_SYNC_PROTOCOL);
        rpc.Write(cfg.Enabled);
        rpc.Write(cfg.EnableSwapOutfit);
        rpc.Write(cfg.EnableStoreOutfit);
        rpc.Write(cfg.EnableEquipOutfit);
        rpc.Write(cfg.MinWearableSlots);
        rpc.Write(cfg.RequireOpenWhenOpenable);
        WriteStringArray(rpc, cfg.RequiredSlots);
        WriteStringArray(rpc, cfg.IncludeClasses);
        WriteStringArray(rpc, cfg.ExcludeClasses);
        rpc.Send(player, UOS_RPC_CONFIG_SYNC, true, identity);
    }

    static bool ReadFromServer(ParamsReadContext ctx)
    {
        int protocol;
        bool enabled;
        bool enableSwapOutfit;
        bool enableStoreOutfit;
        bool enableEquipOutfit;
        int minWearableSlots;
        bool requireOpenWhenOpenable;
        array<string> requiredSlots;
        array<string> includeClasses;
        array<string> excludeClasses;

        if (!ctx.Read(protocol) || protocol != UOS_CONFIG_SYNC_PROTOCOL)
            return false;
        if (!ctx.Read(enabled))
            return false;
        if (!ctx.Read(enableSwapOutfit))
            return false;
        if (!ctx.Read(enableStoreOutfit))
            return false;
        if (!ctx.Read(enableEquipOutfit))
            return false;
        if (!ctx.Read(minWearableSlots))
            return false;
        if (!ctx.Read(requireOpenWhenOpenable))
            return false;
        if (!ReadStringArray(ctx, requiredSlots))
            return false;
        if (!ReadStringArray(ctx, includeClasses))
            return false;
        if (!ReadStringArray(ctx, excludeClasses))
            return false;

        UOS_ConfigManager.ApplyClientEligibilityConfig(
            enabled,
            enableSwapOutfit,
            enableStoreOutfit,
            enableEquipOutfit,
            minWearableSlots,
            requireOpenWhenOpenable,
            requiredSlots,
            includeClasses,
            excludeClasses);
        return true;
    }
};

modded class PlayerBase
{
    protected bool m_UOSEligibilityConfigSynced;

    bool UOS_HasEligibilityConfig()
    {
        if (!GetGame() || !GetGame().IsMultiplayer() || GetGame().IsServer())
            return true;

        return m_UOSEligibilityConfigSynced;
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        if (rpc_type != UOS_RPC_CONFIG_SYNC || !GetGame() || GetGame().IsServer())
            return;

        if (UOS_ConfigSync.ReadFromServer(ctx))
        {
            m_UOSEligibilityConfigSynced = true;
            UOS_Log.Info("Eligibility config synchronized from server.");
        }
    }
};
