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

## Client/server note for v0.1.0

The JSON file is server-side. Clients use the compiled defaults for deciding whether to display an action; the server always revalidates using the authoritative server configuration before moving anything.

Therefore, after heavily changing eligibility/action-enable settings, a client may still briefly see an action the server refuses. This is a UI limitation of the first test release, not an authority bypass. A later release can sync settings to clients if this proves worth the additional RPC/config complexity.
