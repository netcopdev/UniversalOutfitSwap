# DayZ API audit

Audit date: **2026-09-06**

The first-test implementation was checked against Bohemia Interactive's public `DayZ-Script-Diff` repository before packaging. The audit is intended to reduce obvious compile/signature mistakes; an actual DayZ Tools build and in-game test remain authoritative.

## Inventory capability and lookup

The implementation relies on the current public script surface for:

- `GameInventory.HasAttachmentSlot(int slotId)` to test whether a storage entity exposes a canonical wearable attachment slot.
- `GameInventory.FindAttachment(int slotId)` to obtain the item currently occupying that slot.
- `InventorySlots.GetSlotIdFromString(string)` to resolve the canonical slot names.

`GetSlotIdCount()` / `GetSlotId()` are intentionally not used for storage capability detection; they describe slots the entity itself can occupy.

## Preflight and synchronization (0.1.2)

- Retain `CanSwapEntitiesEx` and `LocationCanMoveEntitySyncCheck` preflight.
- For occupied slots, acquire the normal pair of inventory junctures using
  `TryAcquireTwoInventoryJuncturesFromServer`, then `SerializeSwap`.
- For one-way attachment moves, acquire `AddInventoryJunctureEx` when needed
  and use `SerializeMove`, matching `DayZPlayerInventory.TakeToDst(SERVER)`.
- Send through the living player's `SendSyncJuncture(SJ_INVENTORY, ctx)`.
  Do not mutate locally or call the generic `SendServerSwap/SendServerMove`.
- Prepare all command payloads, then acquire all junctures before dispatch.
- Send every command in the same script call at one simulation timestamp.
  The watchdog only observes the batch; it does not schedule individual slots.
- Confirm all server destinations before notifying success. This is not an
  independent client ack or a native atomic whole-outfit swap.

The public `DayZPlayerInventory.OnServerInventoryCommand` calls
`ProcessInputData(ctx, true, true)`, whose first guard rejects that combination.
This explains why generic server inventory commands plus local mutation are not
an appropriate living-player transaction path. The previous local-swap fix
checked only authoritative locations and could not detect client desynchronization.

References read on 2026-09-07:

- [Player inventory](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/4_world/systems/inventory/dayzplayerinventory.c)
- [Juncture helpers](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/3_game/systems/inventory/junctures.c)

## Actions and concurrency

An active transaction registry excludes simultaneous UOS operations on either
its player or storage. Actions retain their normal target lock. Batch preparation
rechecks source occupants, attachment validity, storage access, and distance.
Other inventory interactions are not globally locked; native checks still apply.

If preparation/acquisition fails, no commands are sent and only newly acquired
junctures are released. Once sent, ownership cleanup belongs to DayZ. Timeout or
disconnect reports an uncertain partial result without submitting inverse moves.

## Deliberate first-test constraints

- No serialization or recreation of equipment.
- No custom RPC transaction protocol.
- No weapon-slot mapping yet.
- No attempt to bypass third-party inventory/lock restrictions.

## Public reference

Repository: `BohemiaInteractive/DayZ-Script-Diff`

Relevant public script areas checked:

- `scripts/3_game/systems/inventory/inventory.c`
- `scripts/3_game/systems/inventory/inventorylocation.c`
- `scripts/3_game/entities/man.c`
- `scripts/4_world/classes/useractionscomponent/actionbase.c`
- `scripts/4_world/classes/useractionscomponent/actions/actioninteractbase.c`

If a future DayZ update changes these APIs, keep compatibility changes concentrated in `UOS_Eligibility.c`, `UOS_Transaction.c`, and `UOS_Actions.c`.
