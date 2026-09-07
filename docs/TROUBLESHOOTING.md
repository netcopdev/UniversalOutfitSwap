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
6. if target config has `openable = 1`, it is open;
7. the action itself is enabled in server JSON.

For v0.1.0, server JSON is not synchronized to clients for action visibility. A permissive server-only `IncludeClasses` override cannot make an action appear if the client-side compiled default filter rejects the object.

## Action appears but server refuses it

This usually means the authoritative server state differs from what the client used to display the action. Common reasons are:

- server JSON disables the action or excludes the classname;
- target was closed/locked between action selection and execution;
- inventory became locked/reserved;
- an attached item rejects the opposite attachment location.

The server revalidates immediately before the transaction.

## Transaction reports rollback

Preserve:

- server RPT;
- exact target classname;
- action used;
- player and locker items in every relevant wearable slot;
- which of those items contain cargo or nested attachments.

A rollback message means preflight succeeded but DayZ rejected a later server move, often because another moved item changed attachment exclusions.

## CodeLock / storage-lock mods

Universal Outfit Swap does not call or bypass third-party lock APIs. It rejects closed openable targets and engine inventory-lock state, then relies on normal DayZ inventory validation for each move. Test your actual lock combination before treating the addon as production-safe.
