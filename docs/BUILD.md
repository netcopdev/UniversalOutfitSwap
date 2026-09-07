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
