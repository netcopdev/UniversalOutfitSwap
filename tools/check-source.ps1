[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$version = (Get-Content -LiteralPath (Join-Path $RepoRoot 'VERSION') -Raw).Trim()

$privateKeys = Get-ChildItem -LiteralPath $RepoRoot -Recurse -File -Filter '*.biprivatekey' -ErrorAction SilentlyContinue
if ($privateKeys) {
    throw "Private signing key found inside repository. Remove it before build/archive: $($privateKeys.FullName -join ', ')"
}

$configPath = Join-Path $RepoRoot 'src\UniversalOutfitSwap\config.cpp'
$constantsPath = Join-Path $RepoRoot 'src\UniversalOutfitSwap\Scripts\3_Game\UniversalOutfitSwap\UOS_Constants.c'
$actionRegistrationPath = Join-Path $RepoRoot 'src\UniversalOutfitSwap\Scripts\4_World\UniversalOutfitSwap\UOS_ActionRegistration.c'
$actionsPath = Join-Path $RepoRoot 'src\UniversalOutfitSwap\Scripts\4_World\UniversalOutfitSwap\UOS_Actions.c'
$configSyncPath = Join-Path $RepoRoot 'src\UniversalOutfitSwap\Scripts\4_World\UniversalOutfitSwap\UOS_ConfigSync.c'
$missionServerPath = Join-Path $RepoRoot 'src\UniversalOutfitSwap\Scripts\5_Mission\UniversalOutfitSwap\UOS_MissionServer.c'
$modCppPath = Join-Path $RepoRoot 'packaging\mod.cpp'

$expected = @($configPath, $constantsPath, $actionRegistrationPath, $actionsPath, $configSyncPath, $missionServerPath, $modCppPath)
foreach ($file in $expected) {
    if (-not (Test-Path -LiteralPath $file)) {
        throw "Required file missing: $file"
    }
}

$constants = Get-Content -LiteralPath $constantsPath -Raw
$actionRegistration = Get-Content -LiteralPath $actionRegistrationPath -Raw
$actions = Get-Content -LiteralPath $actionsPath -Raw
$configSync = Get-Content -LiteralPath $configSyncPath -Raw
$missionServer = Get-Content -LiteralPath $missionServerPath -Raw
$config = Get-Content -LiteralPath $configPath -Raw
$modCpp = Get-Content -LiteralPath $modCppPath -Raw
$worldScripts = Get-ChildItem -LiteralPath (Join-Path $RepoRoot 'src\UniversalOutfitSwap\Scripts\4_World') -Recurse -File -Filter '*.c' |
    ForEach-Object { Get-Content -LiteralPath $_.FullName -Raw }

if (-not $constants.Contains("UOS_VERSION = `"$version`"")) {
    throw "VERSION and UOS_VERSION differ. Expected $version"
}
if (-not $config.Contains("version = `"$version`"")) {
    throw "VERSION and config.cpp differ. Expected $version"
}
if (-not $modCpp.Contains("version = `"$version`"")) {
    throw "VERSION and packaging/mod.cpp differ. Expected $version"
}
if (($worldScripts -join "`n") -match '(?<!UOS_Operation\.)\bUOS_OP_(SWAP|STORE|EQUIP)\b') {
    throw 'Unqualified UOS_Operation enum member found in 4_World scripts.'
}
foreach ($actionType in @('ActionUOSSwapOutfit', 'ActionUOSStoreOutfit', 'ActionUOSEquipOutfit')) {
    if (-not $actionRegistration.Contains("actions.Insert($actionType);")) {
        throw "Action type is not registered with ActionConstructor: $actionType"
    }
}
if (-not $actions.Contains('player.UOS_HasEligibilityConfig()')) {
    throw 'Action visibility is not gated on synchronized client eligibility config.'
}
foreach ($requiredSyncText in @(
    'rpc.Send(player, UOS_RPC_CONFIG_SYNC, true, identity);',
    'UOS_ConfigManager.ApplyClientEligibilityConfig(',
    'm_UOSEligibilityConfigSynced = true;'
)) {
    if (-not $configSync.Contains($requiredSyncText)) {
        throw "Client eligibility config synchronization requirement missing: $requiredSyncText"
    }
}
foreach ($serverOnlyField in @('NotifyOnSuccess', 'NotifyOnFailure', 'DebugLogging')) {
    if ($configSync.Contains("cfg.$serverOnlyField")) {
        throw "Server-only config field must not be included in client eligibility sync: $serverOnlyField"
    }
}
foreach ($readyHook in @('OnClientReadyEvent', 'OnClientReconnectEvent', 'OnClientRespawnEvent')) {
    if (-not $missionServer.Contains($readyHook)) {
        throw "Client eligibility config resend hook missing: $readyHook"
    }
}
if (($worldScripts -join "`n") -match '\bServer(SwapEntities|TakeToDst)\s*\(') {
    throw 'Deferred player inventory helper found in multi-slot transaction code.'
}
$transactionScripts = $worldScripts -join "`n"
if ($transactionScripts -match '\b(LocationSwap|LocationSyncMoveEntity|SendServerSwap|SendServerMove)\s*\(') {
    throw 'Immediate mutation/generic server command found in living-player transaction.'
}
foreach ($required in @('TryAcquireTwoInventoryJuncturesFromServer(', 'InventoryInputUserData.SerializeSwap(', 'InventoryInputUserData.SerializeMove(', 'SendSyncJuncture(DayZPlayerSyncJunctures.SJ_INVENTORY, transfer.Command)', 'Outfit synchronization timed out', 's_Active.RemoveItem(this)')) {
    if (-not $transactionScripts.Contains($required)) {
        throw "Synchronized transaction requirement missing: $required"
    }
}
Write-Host "Source audit OK - Universal Outfit Swap v$version"
