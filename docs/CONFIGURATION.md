# Configuration

Server path:

```text
$profile:UniversalOutfitSwap/UniversalOutfitSwap.json
```

Default shape:

```json
{
  "Enabled": true,
  "EnableSwapOutfit": true,
  "EnableStoreOutfit": true,
  "EnableEquipOutfit": true,
  "MinWearableSlots": 6,
  "RequireOpenWhenOpenable": true,
  "NotifyOnSuccess": true,
  "NotifyOnFailure": true,
  "DebugLogging": false,
  "RequiredSlots": ["Body", "Legs", "Feet"],
  "IncludeClasses": [],
  "ExcludeClasses": []
}
```

## IncludeClasses

Exact classnames in this list bypass the slot-count and required-slot tests. Open-state, ruined-state, transport, and normal inventory move validation still apply.

Use this only for a legitimate unusual outfit holder that intentionally has fewer canonical slots.

## ExcludeClasses

Exact classnames that must never act as outfit storage, even if they happen to expose a full clothing-slot set.

## Client/server behavior

The JSON file remains server-side. When a multiplayer client becomes ready, reconnects, or respawns, the server sends only the eligibility/action-visibility subset to that client:

- `Enabled`
- `EnableSwapOutfit`
- `EnableStoreOutfit`
- `EnableEquipOutfit`
- `MinWearableSlots`
- `RequireOpenWhenOpenable`
- `RequiredSlots`
- `IncludeClasses`
- `ExcludeClasses`

Until that snapshot arrives, UOS actions are hidden rather than evaluated against compiled defaults. `NotifyOnSuccess`, `NotifyOnFailure`, and `DebugLogging` remain server-only.

The server still revalidates its own authoritative configuration and inventory state before moving anything. The JSON is loaded once per server mission; restart the server after editing it.
