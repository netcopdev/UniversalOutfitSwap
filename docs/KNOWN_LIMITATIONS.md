# Known limitations — v0.1.2

1. **Wearables only.** No shoulder/melee weapon mapping yet.
2. **Best-effort whole-outfit transaction.** Every slot is prepared and reserved before dispatch. All commands are sent in one server frame, but native execution is not atomic; timeout or disconnect can leave a partial result.
3. **Server config is not synchronized to clients.** Server remains authoritative; unusual config changes can cause an action to be displayed client-side and then refused server-side.
4. **Exact classname include/exclude only.** No wildcard or inheritance matching in v0.1.0.
5. **Open-state detection uses `CfgVehicles ... openable`.** A custom mod that implements doors without `openable = 1` may be considered accessible while closed; use `ExcludeClasses` until a compatibility rule is added.
6. **Visual locker proxies are unrelated.** If a storage mod does not render clothing on its model, swapping still works but this addon does not create visual proxies.
