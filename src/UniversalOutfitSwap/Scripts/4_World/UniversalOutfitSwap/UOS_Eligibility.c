class UOS_Eligibility
{
    static ItemBase ResolveTarget(ActionTarget actionTarget)
    {
        if (!actionTarget)
            return null;

        ItemBase item = ItemBase.Cast(actionTarget.GetObject());
        if (item)
            return item;

        return ItemBase.Cast(actionTarget.GetParent());
    }

    static bool IsEligible(PlayerBase player, ItemBase storage)
    {
        if (!player || !storage)
            return false;

        UOS_Config cfg = UOS_ConfigManager.Get();
        if (!cfg || !cfg.Enabled)
            return false;

        if (storage == player)
            return false;

        if (storage.IsDamageDestroyed())
            return false;

        if (!player.GetInventory() || player.GetInventory().IsInventoryLocked())
            return false;

        if (!storage.GetInventory() || storage.GetInventory().IsInventoryLocked())
            return false;

        // The action is injected into ItemBase, which already excludes actual Transport
        // classes. This additional check protects against unusual mod inheritance.
        if (storage.IsTransport())
            return false;

        string typeName = storage.GetType();

        if (cfg.ExcludeClasses && cfg.ExcludeClasses.Find(typeName) != -1)
            return false;

        if (cfg.IncludeClasses && cfg.IncludeClasses.Find(typeName) != -1)
            return IsAccessStateAcceptable(storage, cfg);

        if (!HasRequiredSlots(storage, cfg.RequiredSlots))
            return false;

        if (CountWearableSlots(storage) < cfg.MinWearableSlots)
            return false;

        if (!IsAccessStateAcceptable(storage, cfg))
            return false;

        return true;
    }

    static bool IsOperationEnabled(int operation)
    {
        UOS_Config cfg = UOS_ConfigManager.Get();
        if (!cfg || !cfg.Enabled)
            return false;

        switch (operation)
        {
            case UOS_Operation.UOS_OP_SWAP:
                return cfg.EnableSwapOutfit;
            case UOS_Operation.UOS_OP_STORE:
                return cfg.EnableStoreOutfit;
            case UOS_Operation.UOS_OP_EQUIP:
                return cfg.EnableEquipOutfit;
        }

        return false;
    }

    static bool HasUsefulWork(PlayerBase player, ItemBase storage, int operation)
    {
        array<string> slots;
        UOS_Slots.GetWearableSlots(slots);

        bool hasPlayerItem = false;
        bool hasStorageItem = false;

        foreach (string slotName : slots)
        {
            int slotId = InventorySlots.GetSlotIdFromString(slotName);
            if (slotId == InventorySlots.INVALID)
                continue;

            if (!HasSlot(storage, slotId))
                continue;

            EntityAI playerItem = player.GetInventory().FindAttachment(slotId);
            EntityAI storageItem = storage.GetInventory().FindAttachment(slotId);

            switch (operation)
            {
                case UOS_Operation.UOS_OP_SWAP:
                    if (playerItem)
                        hasPlayerItem = true;
                    if (storageItem)
                        hasStorageItem = true;
                    break;

                case UOS_Operation.UOS_OP_STORE:
                    if (playerItem)
                        return true;
                    break;

                case UOS_Operation.UOS_OP_EQUIP:
                    if (storageItem)
                        return true;
                    break;
            }
        }

        if (operation == UOS_Operation.UOS_OP_SWAP)
            return hasPlayerItem && hasStorageItem;

        return false;
    }

    static int CountWearableSlots(EntityAI entity)
    {
        if (!entity || !entity.GetInventory())
            return 0;

        int count = 0;
        array<string> slots;
        UOS_Slots.GetWearableSlots(slots);

        foreach (string slotName : slots)
        {
            int slotId = InventorySlots.GetSlotIdFromString(slotName);
            if (slotId != InventorySlots.INVALID && HasSlot(entity, slotId))
                count++;
        }

        return count;
    }

    static bool HasRequiredSlots(EntityAI entity, array<string> requiredSlots)
    {
        if (!requiredSlots || requiredSlots.Count() == 0)
            return true;

        foreach (string slotName : requiredSlots)
        {
            int slotId = InventorySlots.GetSlotIdFromString(slotName);
            if (slotId == InventorySlots.INVALID || !HasSlot(entity, slotId))
                return false;
        }

        return true;
    }

    static bool HasSlot(EntityAI entity, int slotId)
    {
        if (!entity || !entity.GetInventory())
            return false;

        // HasAttachmentSlot answers whether this entity exposes the destination
        // attachment slot. GetSlotIdCount/GetSlotId describe slots the entity
        // itself can be attached to and are therefore the wrong capability test.
        return entity.GetInventory().HasAttachmentSlot(slotId);
    }

    protected static bool IsAccessStateAcceptable(ItemBase storage, UOS_Config cfg)
    {
        if (!cfg.RequireOpenWhenOpenable)
            return true;

        if (!IsConfiguredOpenable(storage))
            return true;

        return storage.IsOpen();
    }

    static bool IsConfiguredOpenable(ItemBase storage)
    {
        if (!storage)
            return false;

        string path = "CfgVehicles " + storage.GetType() + " openable";
        if (GetGame().ConfigGetInt(path) > 0)
            return true;

        // Many modded lockers implement Open()/Close()/IsOpen() but omit the
        // vanilla `openable` config property. Presence of a Close action is a
        // stronger capability signal than the config value alone.
        array<ActionBase_Basic> actions;
        storage.GetActions(InteractActionInput, actions);
        if (!actions)
            return false;

        foreach (ActionBase_Basic action : actions)
        {
            if (action && action.Type().ToString().Contains("Close"))
                return true;
        }

        return false;
    }
};
