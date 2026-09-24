# Known limitations — v0.1.2

1. **Wearables only.** No shoulder/melee weapon mapping yet.
2. **Best-effort whole-outfit transaction.** Every slot is prepared and reserved before dispatch. All commands are sent in one server frame, but native execution is not atomic; timeout or disconnect can leave a partial result.
3. **Exact classname include/exclude only.** No wildcard or inheritance matching in v0.1.2.
4. **Open-state capability is heuristic.** UOS treats storage as openable when `CfgVehicles ... openable` is enabled or the object registers a Close action, then requires `IsOpen()`. Storage exposing neither signal is treated as non-openable.
5. **Visual locker proxies are unrelated.** If a storage mod does not render clothing on its model, swapping still works but this addon does not create visual proxies.
