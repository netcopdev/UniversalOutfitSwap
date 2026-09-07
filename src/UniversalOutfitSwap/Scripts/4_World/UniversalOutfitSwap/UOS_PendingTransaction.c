// Prepare and reserve the entire outfit before dispatching every standard
// inventory juncture in one server frame. Polling only observes completion;
// it never schedules individual items or mutates inventory locally.
class UOS_PreparedTransfer
{
    ref InventoryLocation Src1;
    ref InventoryLocation Src2;
    ref InventoryLocation Dst1;
    ref InventoryLocation Dst2;
    ref ScriptInputUserData Command;
    bool OwnsJuncture1;
    bool OwnsJuncture2;

    bool Prepare(PlayerBase player, ItemBase storage, UOS_TransferStep step, int operation, out string errorText)
    {
        if (!UOS_PendingTransaction.AtStep(player, storage, step, true))
        {
            errorText = "Inventory changed before submission at slot '" + step.SlotName + "'.";
            return false;
        }
        if (!UOS_Transaction.ValidateStep(player, storage, step, operation, errorText))
            return false;

        Src1 = new InventoryLocation();
        Dst1 = new InventoryLocation();
        Command = new ScriptInputUserData();
        if (step.PlayerItem && step.StorageItem)
        {
            Src2 = new InventoryLocation();
            Dst2 = new InventoryLocation();
            if (!GameInventory.MakeSrcAndDstForSwap(step.PlayerItem, step.StorageItem, Src1, Src2, Dst1, Dst2))
                return false;
            InventoryInputUserData.SerializeSwap(Command, Src1, Src2, Dst1, Dst2, false);
        }
        else
        {
            EntityAI item = step.PlayerItem;
            EntityAI destination = storage;
            if (!item)
            {
                item = step.StorageItem;
                destination = player;
            }
            if (!item || !item.GetInventory().GetCurrentInventoryLocation(Src1))
                return false;
            Dst1.SetAttachment(destination, item, step.SlotId);
            InventoryInputUserData.SerializeMove(Command, InventoryCommandType.SYNC_MOVE, Src1, Dst1);
        }
        return true;
    }

    bool Acquire(PlayerBase player)
    {
        // Record only newly acquired junctures. Preparation failure must never
        // release an unrelated inventory operation's existing reservation.
        bool hadFirst = GetGame().HasInventoryJuncture(player, Src1.GetItem());
        bool hadSecond = false;
        if (Src2)
            hadSecond = GetGame().HasInventoryJuncture(player, Src2.GetItem());

        bool acquired = true;
        if (Src2)
        {
            int result = TryAcquireTwoInventoryJuncturesFromServer(player, Src1, Src2, Dst1, Dst2);
            acquired = result == JunctureRequestResult.JUNCTURE_ACQUIRED || result == JunctureRequestResult.JUNCTURE_NOT_REQUIRED;
        }
        else if (player.NeedInventoryJunctureFromServer(Src1.GetItem(), Src1.GetParent(), Dst1.GetParent()))
        {
            acquired = GetGame().AddInventoryJunctureEx(player, Src1.GetItem(), Dst1, true, GameInventory.c_InventoryReservationTimeoutMS);
        }

        OwnsJuncture1 = !hadFirst && GetGame().HasInventoryJuncture(player, Src1.GetItem());
        if (Src2)
            OwnsJuncture2 = !hadSecond && GetGame().HasInventoryJuncture(player, Src2.GetItem());
        return acquired;
    }

    void ReleaseUnsent(PlayerBase player)
    {
        if (OwnsJuncture1 && Src1.GetItem())
            GetGame().ClearJunctureEx(player, Src1.GetItem());
        if (OwnsJuncture2 && Src2.GetItem())
            GetGame().ClearJunctureEx(player, Src2.GetItem());
        OwnsJuncture1 = false;
        OwnsJuncture2 = false;
    }
};

class UOS_PendingTransaction
{
    protected static ref array<ref UOS_PendingTransaction> s_Active = new array<ref UOS_PendingTransaction>();
    protected PlayerBase m_Player;
    protected ItemBase m_Storage;
    protected ref array<ref UOS_TransferStep> m_Steps;
    protected ref array<ref UOS_PreparedTransfer> m_Prepared;
    protected int m_Operation;
    protected int m_Deadline;
    protected int m_StartedAt;

    static bool IsBusy(PlayerBase player, ItemBase storage)
    {
        foreach (UOS_PendingTransaction pending : s_Active)
        {
            if (pending.m_Player == player || pending.m_Storage == storage)
                return true;
        }
        return false;
    }

    static bool Start(PlayerBase player, ItemBase storage, int operation, array<ref UOS_TransferStep> steps, out string errorText)
    {
        UOS_PendingTransaction pending = new UOS_PendingTransaction();
        pending.m_Player = player;
        pending.m_Storage = storage;
        pending.m_Operation = operation;
        pending.m_Steps = steps;
        pending.m_Prepared = new array<ref UOS_PreparedTransfer>();
        s_Active.Insert(pending);
        if (!pending.PrepareAndAcquire(errorText))
        {
            pending.ReleaseUnsent();
            s_Active.RemoveItem(pending);
            return false;
        }

        // There is no timer, acknowledgment wait, or deferred event between
        // sends: every slot is submitted at this player's current timestamp.
        pending.Dispatch();
        return true;
    }

    protected bool PrepareAndAcquire(out string errorText)
    {
        if (!m_Player.IsAlive() || !m_Player.GetIdentity() || !UOS_Eligibility.IsEligible(m_Player, m_Storage) || vector.Distance(m_Player.GetPosition(), m_Storage.GetPosition()) > UAMaxDistances.DEFAULT)
        {
            errorText = "Outfit storage is no longer accessible.";
            return false;
        }
        foreach (UOS_TransferStep step : m_Steps)
        {
            UOS_PreparedTransfer prepared = new UOS_PreparedTransfer();
            if (!prepared.Prepare(m_Player, m_Storage, step, m_Operation, errorText))
            {
                if (errorText == "")
                    errorText = "Cannot prepare outfit slot '" + step.SlotName + "'.";
                return false;
            }
            m_Prepared.Insert(prepared);
        }
        foreach (UOS_PreparedTransfer transfer : m_Prepared)
        {
            if (!transfer.Acquire(m_Player))
            {
                errorText = "Cannot reserve the complete outfit. Nothing was moved.";
                return false;
            }
        }
        return true;
    }

    protected void ReleaseUnsent()
    {
        foreach (UOS_PreparedTransfer transfer : m_Prepared)
            transfer.ReleaseUnsent(m_Player);
    }

    protected void Dispatch()
    {
        m_StartedAt = GetGame().GetTime();
        m_Deadline = m_StartedAt + GameInventory.c_InventoryReservationTimeoutMS + 2000;
        foreach (UOS_PreparedTransfer transfer : m_Prepared)
            m_Player.SendSyncJuncture(DayZPlayerSyncJunctures.SJ_INVENTORY, transfer.Command);
        UOS_Log.Debug("Outfit batch submitted: " + m_Prepared.Count().ToString() + " slots, STS=" + m_Player.GetSimulationTimeStamp().ToString());
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Tick, 50, true);
    }

    static bool AtStep(PlayerBase player, ItemBase storage, UOS_TransferStep step, bool original)
    {
        if (!player || !storage)
            return false;
        if ((step.HadPlayerItem && !step.PlayerItem) || (step.HadStorageItem && !step.StorageItem))
            return false;
        EntityAI playerExpected = step.StorageItem;
        EntityAI storageExpected = step.PlayerItem;
        if (original)
        {
            playerExpected = step.PlayerItem;
            storageExpected = step.StorageItem;
        }
        return player.GetInventory().FindAttachment(step.SlotId) == playerExpected && storage.GetInventory().FindAttachment(step.SlotId) == storageExpected;
    }

    void Tick()
    {
        if (!m_Player || !m_Storage || !m_Player.IsAlive() || !m_Player.GetIdentity())
        {
            Finish(false, "Outfit transaction interrupted. Some items may already have moved.");
            return;
        }

        bool complete = true;
        foreach (UOS_TransferStep step : m_Steps)
        {
            if (!AtStep(m_Player, m_Storage, step, false))
                complete = false;
        }
        if (complete)
        {
            string verb = "swapped";
            if (m_Operation == UOS_Operation.UOS_OP_STORE)
                verb = "stored";
            if (m_Operation == UOS_Operation.UOS_OP_EQUIP)
                verb = "equipped";
            Finish(true, "Outfit " + verb + ".");
            return;
        }
        if (GetGame().GetTime() >= m_Deadline)
        {
            // Native commands can still be in flight. Never issue inverse
            // commands or clear their ownership junctures from this watchdog.
            Finish(false, "Outfit synchronization timed out. Some items may already have moved; check both inventories.");
        }
    }

    protected void Finish(bool success, string message)
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Tick);
        UOS_Log.Info(message);
        int elapsed = GetGame().GetTime() - m_StartedAt;
        UOS_Log.Debug("Outfit batch elapsed=" + elapsed.ToString() + "ms");
        UOS_Config cfg = UOS_ConfigManager.Get();
        if (cfg && ((success && cfg.NotifyOnSuccess) || (!success && cfg.NotifyOnFailure)))
            UOS_Notifications.Send(m_Player, message);
        s_Active.RemoveItem(this);
    }
};
