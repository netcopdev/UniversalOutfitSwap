class UOS_TransferStep
{
    string SlotName;
    int SlotId;
    EntityAI PlayerItem;
    EntityAI StorageItem;
    bool HadPlayerItem;
    bool HadStorageItem;
    bool Executed;

    void UOS_TransferStep(string slotName, int slotId, EntityAI playerItem, EntityAI storageItem)
    {
        SlotName = slotName;
        SlotId = slotId;
        PlayerItem = playerItem;
        StorageItem = storageItem;
        HadPlayerItem = playerItem != null;
        HadStorageItem = storageItem != null;
        Executed = false;
    }

    int ItemMoveCount()
    {
        int count = 0;
        if (PlayerItem)
            count++;
        if (StorageItem)
            count++;
        return count;
    }
};

class UOS_Transaction
{
    static bool Execute(PlayerBase player, ItemBase storage, int operation, out string resultText)
    {
        resultText = "";

        if (!GetGame().IsServer() || !player || !storage)
        {
            resultText = "Invalid player or storage target.";
            return false;
        }

        if (!UOS_Eligibility.IsOperationEnabled(operation))
        {
            resultText = "This outfit action is disabled by server configuration.";
            return false;
        }

        if (!UOS_Eligibility.IsEligible(player, storage))
        {
            resultText = "This object is not an eligible outfit storage container, or it is closed.";
            return false;
        }

        ref array<ref UOS_TransferStep> steps = new array<ref UOS_TransferStep>();
        if (!BuildAndValidateSteps(player, storage, operation, steps, resultText))
            return false;

        if (steps.Count() == 0)
        {
            resultText = "Nothing to move.";
            return false;
        }

        if (UOS_PendingTransaction.IsBusy(player, storage))
        {
            resultText = "An outfit transaction is already running for this player or storage.";
            return false;
        }

        return UOS_PendingTransaction.Start(player, storage, operation, steps, resultText);
    }
    protected static bool BuildAndValidateSteps(PlayerBase player, ItemBase storage, int operation, array<ref UOS_TransferStep> steps, out string errorText)
    {
        array<string> slotNames;
        UOS_Slots.GetWearableSlots(slotNames);

        foreach (string slotName : slotNames)
        {
            int slotId = InventorySlots.GetSlotIdFromString(slotName);
            if (slotId == InventorySlots.INVALID)
                continue;

            if (!UOS_Eligibility.HasSlot(storage, slotId))
                continue;

            EntityAI playerItem = player.GetInventory().FindAttachment(slotId);
            EntityAI storageItem = storage.GetInventory().FindAttachment(slotId);

            if (operation == UOS_Operation.UOS_OP_STORE && !playerItem)
                continue;

            if (operation == UOS_Operation.UOS_OP_EQUIP && !storageItem)
                continue;

            if (operation == UOS_Operation.UOS_OP_SWAP && !playerItem && !storageItem)
                continue;

            if (operation == UOS_Operation.UOS_OP_STORE && storageItem)
            {
                errorText = "Cannot store outfit: locker slot '" + slotName + "' is occupied.";
                return false;
            }

            if (operation == UOS_Operation.UOS_OP_EQUIP && playerItem)
            {
                errorText = "Cannot equip outfit: player slot '" + slotName + "' is occupied.";
                return false;
            }

            UOS_TransferStep step = new UOS_TransferStep(slotName, slotId, playerItem, storageItem);
            if (!ValidateStep(player, storage, step, operation, errorText))
                return false;

            steps.Insert(step);
        }

        return true;
    }

    static bool ValidateStep(PlayerBase player, ItemBase storage, UOS_TransferStep step, int operation, out string errorText)
    {
        if (operation == UOS_Operation.UOS_OP_SWAP && step.PlayerItem && step.StorageItem)
        {
            if (!GameInventory.CanSwapEntitiesEx(step.PlayerItem, step.StorageItem))
            {
                UOS_Log.Info("Swap preflight rejected slot=" + step.SlotName + " playerItem=" + step.PlayerItem.GetType() + " storageItem=" + step.StorageItem.GetType() + " storage=" + storage.GetType());
                errorText = "Cannot swap slot '" + step.SlotName + "': one of the items cannot occupy the opposite attachment slot.";
                return false;
            }

            return true;
        }

        if (step.PlayerItem && !step.StorageItem)
        {
            if (!CanMoveToAttachment(step.PlayerItem, storage, step.SlotId))
            {
                errorText = "Cannot move player's '" + step.SlotName + "' item into the storage slot.";
                return false;
            }

            return true;
        }

        if (!step.PlayerItem && step.StorageItem)
        {
            if (!CanMoveToAttachment(step.StorageItem, player, step.SlotId))
            {
                errorText = "Cannot move stored '" + step.SlotName + "' item onto the player.";
                return false;
            }

            return true;
        }

        return true;
    }

    protected static bool CanMoveToAttachment(EntityAI item, EntityAI destinationParent, int slotId)
    {
        if (!item || !destinationParent || !destinationParent.GetInventory())
            return false;

        InventoryLocation src = new InventoryLocation();
        if (!item.GetInventory().GetCurrentInventoryLocation(src))
            return false;

        InventoryLocation dst = new InventoryLocation();
        dst.SetAttachment(destinationParent, item, slotId);

        // Use the same synchronized location validation family as the actual
        // server-side move. This catches both source removal restrictions and
        // destination attachment restrictions during preflight.
        return GameInventory.LocationCanMoveEntitySyncCheck(src, dst);
    }

};
