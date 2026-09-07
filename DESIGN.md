# Design

## Goals

- Work with existing modded storage rather than introduce another locker ecosystem.
- Detect compatibility by inventory capability, not by RaG/MMG classname lists.
- Preserve complete entity trees by moving real items.
- Never serialize, delete, recreate, or virtualize player equipment.
- Avoid adding an outfit action to incidental holders with one or two wearable slots.
- Keep all authoritative inventory mutation server-side.
- Respect normal DayZ attachment acceptance/removal checks so locked or incompatible storage fails preflight instead of being bypassed.

## Action semantics

### Swap Outfit

For every canonical slot supported by the storage:

- player item + storage item: use DayZ server entity swap;
- player item + empty storage slot: move player item to storage;
- empty player slot + storage item: move storage item to player;
- both empty: no operation.

An empty slot is meaningful. A true swap can therefore leave the player without an item when the corresponding storage slot was empty.

### Store Outfit

Only player-to-storage moves are performed. If any relevant destination storage slot is occupied, the complete transaction is refused before the first move.

### Equip Outfit

Only storage-to-player moves are performed. If any relevant destination player slot is occupied, the complete transaction is refused before the first move.

This strict behavior is deliberate for v0.1.0. Silent per-slot skipping would make the final outfit less predictable.

## Eligibility

The actions are registered on `PlayerBase` and their conditions filter the current
target aggressively. Target-object actions are evaluated first, keeping a locker's
Open/Close interaction ahead of the outfit actions.

Default capability rules:

- at least `MinWearableSlots` canonical wearable slots (default 6);
- all configured `RequiredSlots` (default Body, Legs, Feet);
- not a transport;
- not ruined;
- player and target inventories not currently locked;
- open when either `CfgVehicles <type> openable` is true or the storage registers
  a Close action (for modded lockers that omit the config property).

`IncludeClasses` and `ExcludeClasses` are exact-classname escape hatches for unusual mods.

## Transaction model

1. Resolve target.
2. Re-run server-side eligibility checks.
3. Build every planned slot step.
4. Preflight each step with DayZ's inventory checks:
   - `GameInventory.CanSwapEntitiesEx` for occupied-to-occupied swaps;
   - `LocationCanMoveEntitySyncCheck` for one-way attachment moves.
5. Prepare all command payloads and reserve all required inventory junctures.
6. If any preparation or acquisition fails, release newly acquired unsent
   junctures and send nothing.
7. Dispatch every slot's standard inventory command in one server frame, at the
   same player simulation timestamp. There is no per-slot acknowledgment wait.
8. A 50 ms watchdog verifies the whole outfit before notifying success. It does
   not pace or dispatch moves. On timeout/disconnect it reports a partial result.

The active transaction registry excludes overlapping UOS operations for the same
player or locker until batch completion or timeout. Native DayZ commands own
replication and juncture cleanup after submission. The batch is not an engine
atomic primitive: native execution failures can still produce partial outcomes.
No competing inverse commands are sent against in-flight work.

## Why no temporary staging container?

DayZ already provides a direct entity swap primitive for two occupied inventory locations. Using it avoids a hidden staging entity and reduces failure surface. Empty-to-occupied cases use exact attachment destinations.

## Why no weapon swap yet?

Player and storage weapon slot conventions differ. Vanilla player equipment uses `Shoulder` and `Melee`; storage mods commonly define `Shoulder1...ShoulderN`. Universal behavior needs a deliberate policy such as first-two compatible storage slots, named mapping, or user-selected preset. That belongs after wearable swapping is proven stable.
