# First-test checklist

Use expendable gear and preferably a disposable test character for the first server run.

## 1. Build / authoritative compile sanity

Run:

```powershell
.\tools\validate-dayz-compile.ps1
```

Expected: `DAYZ SCRIPT COMPILE: PASS`. This builds a fresh PBO, starts the real
DayZ dedicated server with a disposable profile/storage directory, fails on
Game/World/Mission script compiler errors, and passes only after the UOS
`MissionServer.OnInit()` marker is observed.

Then load the same build on client and server for the multiplayer checks below.
Confirm the normal server profile creates
`$profile:UniversalOutfitSwap/UniversalOutfitSwap.json`.

## 2. Eligibility

Test at least:

- RaG single/triple/big equipment locker with sufficient clothing slots -> actions should appear when open.
- RaG three-door locker -> actions should appear when open.
- MMG loadout/equipment locker -> actions should appear when open.
- RaG gun wall -> outfit actions should **not** appear.
- object with only a backpack/vest slot -> outfit actions should **not** appear.
- closed openable locker -> actions should not appear with default config.

If a legitimate storage does not qualify, add its exact classname temporarily to `IncludeClasses` and restart.

## 3. Swap Outfit: both sides occupied

Regression: first put different jackets in the player's and locker's Body slots,
leaving other wearable slots empty. Swap twice; each operation must exchange
the jackets immediately and the second must restore the original arrangement.
Repeat with cargo in both jackets and with a second client observing. Then run
the full outfit and mixed-slot cases below. Check the server log for
`Swap preflight rejected` or
`Synchronization timed out` if the action fails; include the exact notification.

Player and locker should each have a visibly different item in every practical wearable slot.

Include stateful items:

- jacket with cargo;
- backpack with cargo;
- vest with attached pouches/holster if available;
- belt with attachments;
- helmet with NVG/light if available;
- damaged or wet item;
- charged battery in an attached device.

Run **Swap Outfit** and verify:

- item identities actually exchange;
- after success, take every received wearable into hands, move it, and drop it;
- repeat Swap twice without relogging, including Store and Equip afterward;
- verify the owning client and a second observer agree with the server;
- cargo remains inside the same clothing;
- nested attachments remain attached;
- health/wetness/energy state remains intact;
- no item is duplicated or deleted;
- relog/restart preserves the result normally.

## 4. Swap Outfit: empty-slot behavior

Test mixed slots:

- player item / locker empty;
- player empty / locker item;
- both empty;
- both occupied.

Expected: true slot-for-slot exchange. If locker `Vest` is empty and player has a vest, the vest ends up in the locker and the player ends without one.

## 5. Store Outfit

Start with an empty eligible locker.

Expected: player's supported wearable items move to matching locker slots.

Then occupy one locker destination slot and retry.

Expected: transaction is refused before moving anything.

## 6. Equip Outfit

Start with empty corresponding player slots and stored gear.

Expected: stored outfit moves onto player.

Then occupy one player destination slot and retry.

Expected: transaction is refused before moving anything.

## 7. Lock/access behavior

Test a CodeLock-protected RaG/MMG object if applicable:

- unauthorized/locked state must not allow inventory bypass; target/player inventory lock state is explicitly rejected;
- authorized/unlocked/open state should work.

If the action appears while locked but the transaction is rejected, record the exact message and RPT. UI visibility and server authority are separate; the important requirement is no bypass.

## 8. Concurrent users

Have two players target the same locker and activate an outfit action nearly simultaneously.

Expected: DayZ action target locking and inventory validation allow one coherent transaction; no duplication/deletion. Record RPT if either action reports a failure or timeout.

## 9. Failure and reservation cleanup

Reject a slot during preflight/preparation or make a required juncture unavailable.
Expected: no outfit commands are dispatched and no items move. Reservations
acquired solely for this unsent batch must be released; other reservations must
remain intact. Retry after the interfering operation completes.

After dispatch, timeout/disconnect must not report success or issue inverse
commands while the original commands may still be in flight.

## Suggested first compatibility matrix

| Storage | Expected |
|---|---|
| RaG single locker | Outfit actions if it has >=6 standard wearable slots |
| RaG triple locker | Outfit actions |
| RaG military big locker | Outfit actions |
| RaG military three-door locker | Outfit actions |
| RaG gun wall | No outfit actions |
| RaG manikin | Likely qualifies; useful test |
| MMG Equipment/TA50/Solo loadout locker | Outfit actions |
| Vehicle / incidental Back slot holder | No action through this ItemBase patch / capability filter |

## 0.1.2 whole-outfit performance regression

- Load the signed 0.1.2 build on both server and client; keep 0.1.1 as baseline.
- Fill all 11 supported slots with different outfits, including loaded jackets,
  backpacks and nested attachments. Run Swap in both directions repeatedly.
- Observe the full outfit change together, without the 0.1.1 per-item cadence.
  Repeat under realistic latency and with a second client observing.
- After success, move and drop every received wearable; verify cargo/attachments.
- Repeat Store, Equip, and mixed empty/occupied slots.
- Enable DebugLogging to record batch submission count, simulation timestamp,
  and elapsed completion time; compare with a screen recording of the baseline.
- Repeated outfit actions cannot overlap on either player or storage.
- During latency/disconnect, no false success and no competing inverse commands.
- Confirm preparation failure moves nothing and leaves items usable afterward.

Run the fast local checks with `node tools/test-transaction-flow.mjs` and
`.\tools\check-source.ps1`. Scheduling tests execute extracted script control
flow with mocked engine calls; they do not compile Enforce Script or prove native
visual simultaneity.

Before multiplayer testing, also run
`.\tools\validate-dayz-compile.ps1`. That check uses the real DayZ dedicated
server compiler. Multiplayer visual/move/drop checks remain required separately.
