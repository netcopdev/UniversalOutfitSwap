# Source/API notes

The implementation is intentionally based on public DayZ inventory/action APIs.

Key engine-side assumptions to verify during the first compile/run:

- `InventorySlots.GetSlotIdFromString` resolves standard slot IDs; `GameInventory.HasAttachmentSlot(slotId)` tests whether a target entity actually exposes that attachment slot.
- `GameInventory.CanSwapEntitiesEx` validates occupied-to-occupied swaps.
- `GameInventory.LocationCanMoveEntitySyncCheck` validates one-way attachment moves using the synchronized inventory-check context.
- Multi-slot operations reserve the whole outfit, then dispatch every living-player inventory sync juncture in one server frame. See API_AUDIT.md.
- OnExecuteServer starts the server transaction; DayZ executes its queued inventory commands.
- Action target locking is enabled through `m_LockTargetOnUse`.

The first real DayZ Tools compile is authoritative. If the stable 1.29 script surface differs in method visibility/signature from public generated references, adjust only the thin inventory-adapter methods in `UOS_Transaction.c` rather than changing the higher-level transaction design.

- `GameInventory.GetSlotIdCount/GetSlotId` describe slots an entity/item can itself occupy; they are deliberately **not** used to detect storage attachment capabilities.
