# Changelog

## 0.1.2 - whole-outfit dispatch

- Prepare and reserve every outfit slot before sending any inventory commands.
- Dispatch all standard inventory junctures in one server frame instead of
  waiting for each item to finish and for the next timer tick.
- Keep whole-outfit completion verification and overlapping-operation exclusion.
- Release only newly acquired, unsent junctures if preparation fails.
- Add batch scheduling regression checks and debug elapsed-time logging.
- User confirmed the whole-outfit performance change works in-game on 2026-09-07.

## 0.1.1 - living-player inventory synchronization

- Replace immediate local swaps plus generic server commands with the living
  player's inventory sync juncture protocol for Swap, Store, and Equip.
- Process one slot at a time and delay success until final server destinations
  are confirmed; prevent overlapping outfit operations on a player or locker.
- Revalidate each submission and use synchronized rollback for confirmed moves
  after submission failure. Report uncertain/partial outcomes on timeout or
  interruption rather than claiming success.
- Remove the prior local-swap correction, which could leave client items unusable.
- User confirmed the updated swap works in-game on 2026-09-07.
## 0.1.0 - first test

- Universal capability-based wearable storage detection.
- `Swap Outfit` action.
- `Store Outfit` action with strict empty-destination preflight.
- `Equip Outfit` action with strict empty-destination preflight.
- Server-authoritative moves using real entities.
- Full preflight plus reverse-order rollback attempt.
- Openable-storage check.
- Server JSON config with required-slot threshold and include/exclude escape hatches.
- No hard dependency on RaG, MMG, CF, APH, CodeLock, or other storage frameworks.
- Use DayZ's server-synchronized location commands so multi-slot operations avoid
  the invalid deferred force-swap mode without desynchronizing clients.
- Register outfit operations as player actions so target storage actions,
  including Close, remain ahead of them in the interaction list.
- Recognize a registered Close action as an openable-storage capability for
  modded lockers that omit the vanilla `openable` config property.
- Hide Swap unless both sides contain relevant wearable items; hide Store or
  Equip when its respective source side is empty.
