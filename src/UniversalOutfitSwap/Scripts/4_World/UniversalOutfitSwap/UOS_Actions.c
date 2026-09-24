class ActionUOSBase : ActionInteractBase
{
    protected int m_UOSOperation;

    void ActionUOSBase()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
        m_LockTargetOnUse = true;
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINone();
        m_ConditionTarget = new CCTObject(UAMaxDistances.DEFAULT);
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        // In multiplayer, never evaluate visibility against compiled defaults.
        // The server sends the authoritative eligibility subset to this player.
        if (!player || !player.UOS_HasEligibilityConfig())
            return false;

        if (!UOS_Eligibility.IsOperationEnabled(m_UOSOperation))
            return false;

        ItemBase storage = UOS_Eligibility.ResolveTarget(target);
        if (!UOS_Eligibility.IsEligible(player, storage))
            return false;

        return UOS_Eligibility.HasUsefulWork(player, storage, m_UOSOperation);
    }

    override void OnExecuteServer(ActionData action_data)
    {
        super.OnExecuteServer(action_data);

        if (!action_data || !action_data.m_Player || !action_data.m_Target)
            return;

        ItemBase storage = UOS_Eligibility.ResolveTarget(action_data.m_Target);
        string resultText;
        bool success = UOS_Transaction.Execute(action_data.m_Player, storage, m_UOSOperation, resultText);

        if (success)
            return; // The pending transaction sends its final result.

        UOS_Config cfg = UOS_ConfigManager.Get();
        if (!cfg)
            return;

        if ((success && cfg.NotifyOnSuccess) || (!success && cfg.NotifyOnFailure))
            UOS_Notifications.Send(action_data.m_Player, resultText);
    }
};

class ActionUOSSwapOutfit : ActionUOSBase
{
    void ActionUOSSwapOutfit()
    {
        m_UOSOperation = UOS_Operation.UOS_OP_SWAP;
        m_Text = "Swap Outfit";
    }
};

class ActionUOSStoreOutfit : ActionUOSBase
{
    void ActionUOSStoreOutfit()
    {
        m_UOSOperation = UOS_Operation.UOS_OP_STORE;
        m_Text = "Store Outfit";
    }
};

class ActionUOSEquipOutfit : ActionUOSBase
{
    void ActionUOSEquipOutfit()
    {
        m_UOSOperation = UOS_Operation.UOS_OP_EQUIP;
        m_Text = "Equip Outfit";
    }
};
