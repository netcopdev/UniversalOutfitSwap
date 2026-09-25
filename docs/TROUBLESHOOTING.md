# Troubleshooting first test

## Mod fails with `Can't compile World script module`

Preserve the first compiler error and the surrounding RPT lines. Later errors are often cascading failures.

Useful files to inspect first:

- `Scripts/4_World/UniversalOutfitSwap/UOS_Eligibility.c`
- `Scripts/4_World/UniversalOutfitSwap/UOS_Transaction.c`
- `Scripts/4_World/UniversalOutfitSwap/UOS_Actions.c`

The inventory-adapter surface is intentionally concentrated in those files.

## No outfit actions on a locker

With default settings, verify all of the following:

1. target is an `ItemBase` rather than a vehicle/transport;
2. target exposes at least 6 of the canonical wearable **attachment** slots;
3. it exposes `Body`, `Legs`, and `Feet`;
4. target is not ruined;
5. target and player inventories are not locked;
6. if the target is openable (via `CfgVehicles ... openable` or a registered Close action), it is open;
7. the action itself is enabled in server JSON.

On multiplayer clients, UOS actions remain hidden until the server eligibility snapshot arrives. The client RPT should contain `[UniversalOutfitSwap] Eligibility config synchronized from server.`. Server-side `IncludeClasses`, `ExcludeClasses`, thresholds, and action-enable settings then control client visibility.

## Action appears but server refuses it

The server always revalidates immediately before mutation, so a visible action can still become invalid after it was shown. Common reasons are:

- target was closed/locked between action selection and execution;
- inventory became locked/reserved;
- an attached item rejects the opposite attachment location.

The server revalidates immediately before the transaction.

## Transaction reports timeout or partial completion

Preserve:

- server RPT;
- exact target classname;
- action used;
- player and locker items in every relevant wearable slot;
- which of those items contain cargo or nested attachments.

v0.1.2 does not submit inverse commands after dispatch. If synchronization times out or the transaction is interrupted, some native inventory commands may already have completed. Check both inventories before retrying.

## CodeLock / storage-lock mods

Universal Outfit Swap does not call third-party lock APIs. An object that is open and whose normal DayZ inventory operations are permitted is intentionally treated as accessible. Closed openable targets and engine inventory-lock state are rejected, and every actual move still passes normal DayZ inventory validation.
