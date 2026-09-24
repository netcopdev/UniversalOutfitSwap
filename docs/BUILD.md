# Build and signing

Universal Outfit Swap is a script/config PBO. It does not define `CfgVehicles`, models, or textures, so the supplied build script uses AddonBuilder with `-packonly`.

## Prerequisites

- Windows
- DayZ Tools installed through Steam
- PowerShell 5.1+ or PowerShell 7+
- a BI private signing key only when producing a signed build

## Unsigned first-test build

From the repository root:

```powershell
.\tools\build-release.ps1 -Clean
```

Default output:

```text
dist\@UniversalOutfitSwap\
  addons\UniversalOutfitSwap.pbo
  mod.cpp
```

## Signed build

Pass the key base path without `.biprivatekey` (the build script appends it for AddonBuilder):

```powershell
.\tools\build-release.ps1 -Clean -PrivateKeyBase 'P:\keys\netcopdev'
```

The script verifies that:

```text
P:\keys\netcopdev.biprivatekey
```

exists before invoking AddonBuilder. If a matching public key exists:

```text
P:\keys\netcopdev.bikey
```

it is copied to:

```text
dist\@UniversalOutfitSwap\keys\
```

The private key is never copied into `dist` and should never be committed.

## Non-default DayZ Tools location

```powershell
.\tools\build-release.ps1 `
  -AddonBuilder 'D:\SteamLibrary\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe'
```

## Why `-packonly`

This PBO contains scripts plus a simple `CfgPatches`/`CfgMods` config and no game item definitions. There is nothing that requires Binarize for the first-test release.


## Authoritative DayZ script compile smoke test

AddonBuilder packages the PBO but does not compile Enforce Script. Before treating a build as test-ready, run the real dedicated-server compile smoke test:

```powershell
.\tools\validate-dayz-compile.ps1
```

The script:

1. locates DayZ Server and DayZ Tools through the configured Steam libraries;
2. builds a fresh unsigned `UniversalOutfitSwap.pbo`;
3. starts `DayZServer_x64.exe` with a disposable profile/storage directory and the vanilla Chernarus mission;
4. fails immediately when the server reports `Can't compile "Game"`, `"World"`, or `"Mission" script module`;
5. passes only after the server reaches `MissionServer.OnInit()` and logs the UOS version initialization marker.

If auto-discovery is unsuitable, provide explicit paths:

```powershell
.\tools\validate-dayz-compile.ps1 -DayZServer 'D:\SteamLibrary\steamapps\common\DayZServer\DayZServer_x64.exe' -AddonBuilder 'D:\SteamLibrary\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe'
```

Use `-Mission '<path>'` for a different installed mission. Use `-SkipBuild` only when intentionally validating an already-built PBO.

The compile smoke test proves that the packaged mod loads and its Game/World/Mission scripts compile against the installed DayZ server build. It does **not** replace multiplayer/in-game inventory synchronization testing.
