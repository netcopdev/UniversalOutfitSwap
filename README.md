# Universal Outfit Swap

Universal Outfit Swap is a small DayZ utility addon that turns compatible existing storage objects into outfit stations without adding new storage models or depending on a specific storage mod.

The first test release is intentionally limited to wearable equipment. It detects compatible storage at runtime from standard DayZ attachment slots and adds three actions:

- **Swap Outfit** — slot-for-slot exchange between player and storage.
- **Store Outfit** — move the player's outfit into empty matching storage slots.
- **Equip Outfit** — move the stored outfit onto empty matching player slots.

The actual item entities are moved. Clothing cargo, attached pouches/holsters/NVG, batteries, health, wetness, quantities and other entity state are preserved naturally; there is no serialization or respawning.

## v0.1.2 test scope

Supported canonical wearable slots:

`Headgear`, `Eyewear`, `Mask`, `Armband`, `Gloves`, `Body`, `Vest`, `Back`, `Hips`, `Legs`, `Feet`

A storage object is eligible by default when:

1. it is an `ItemBase` target;
2. it is not a transport, is not ruined, and neither player nor target inventory is currently locked;
3. it exposes at least 6 canonical wearable slots;
4. it exposes the required core slots `Body`, `Legs`, and `Feet`;
5. if its config says `openable = 1`, it is currently open.

This avoids treating incidental attachment holders (for example an object with only a `Back` slot) as outfit stations.

## Compatibility intent

There are **no hardcoded RaG or MMG dependencies**. RaG and MMG lockers are first test targets because they expose standard wearable attachment slots, but any other storage mod using the same slots can qualify automatically.

For the RaG investigation that led to this addon: several RaG lockers expose loadout slots even when their models do not visually render those attachments. Visual proxy rendering is unrelated to swapping; this addon operates on the inventory entities themselves.

## Installation for test

Build the mod, then load it on both server and client:

```text
-mod=@UniversalOutfitSwap
```

The server creates this file on first start:

```text
$profile:UniversalOutfitSwap/UniversalOutfitSwap.json
```

See [docs/CONFIGURATION.md](docs/CONFIGURATION.md), [docs/BUILD.md](docs/BUILD.md), [docs/API_AUDIT.md](docs/API_AUDIT.md), [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md), and [TESTING.md](TESTING.md).

## Build

The source PBO is script/config only, so the build uses AddonBuilder `-packonly`.

Unsigned test build:

```powershell
.\tools\build-release.ps1
```

Authoritative DayZ script compile smoke test:

```powershell
.\tools\validate-dayz-compile.ps1
```

Signed build, supplying your key only at build time:

```powershell
.\tools\build-release.ps1 -PrivateKeyBase 'P:\keys\netcopdev'
```

`-PrivateKeyBase` is the key path **without** `.biprivatekey`; the build script appends the extension when passing the key to AddonBuilder. No private key belongs in this repository.

If DayZ Tools is not installed at the default Steam location, pass:

```powershell
-AddonBuilder 'D:\SteamLibrary\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe'
```

## Important first-test limitation

The transaction preflights and reserves the complete outfit before dispatching
all slot commands in one server frame. It checks the whole outfit before reporting
success. DayZ does not expose a native atomic whole-outfit command: a timeout or
execution failure may leave a partial result, which is reported without submitting
competing reverse commands.

## Weapons / full loadouts

Not in v0.1.0. Player weapon slots (`Shoulder`, `Melee`) do not map universally to storage-mod slots (`Shoulder1`, `Shoulder2`, ...). A later version can add an explicit policy once tested rather than inventing a mapping.

## License

MIT. See [LICENSE](LICENSE).
