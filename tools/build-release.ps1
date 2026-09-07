[CmdletBinding()]
param(
    [string]$AddonBuilder = "E:\Steamlibrary\steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe",
    [string]$PrivateKeyBase = "",
    [string]$OutputRoot = "",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

& (Join-Path $PSScriptRoot "check-source.ps1")
$SourceDir = Join-Path $RepoRoot "src\UniversalOutfitSwap"
$PackagingDir = Join-Path $RepoRoot "packaging"

if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $RepoRoot "dist\@UniversalOutfitSwap"
}

$AddonsDir = Join-Path $OutputRoot "addons"

if (-not (Test-Path -LiteralPath $AddonBuilder)) {
    throw "AddonBuilder not found: $AddonBuilder"
}

if (-not (Test-Path -LiteralPath $SourceDir)) {
    throw "Source directory not found: $SourceDir"
}

if ($Clean -and (Test-Path -LiteralPath $OutputRoot)) {
    Remove-Item -LiteralPath $OutputRoot -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $AddonsDir | Out-Null
Copy-Item -LiteralPath (Join-Path $PackagingDir "mod.cpp") -Destination (Join-Path $OutputRoot "mod.cpp") -Force

# Never allow an old package to make a failed AddonBuilder invocation look
# successful. This output directory belongs exclusively to this addon.
Get-ChildItem -LiteralPath $AddonsDir -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -eq 'UniversalOutfitSwap.pbo' -or $_.Name -like 'UniversalOutfitSwap.pbo.*.bisign' } |
    Remove-Item -Force

$arguments = @(
    $SourceDir,
    $AddonsDir,
    '-prefix=UniversalOutfitSwap',
    '-packonly'
)

$keyCandidate = ""
$privateKey = ""
if (-not [string]::IsNullOrWhiteSpace($PrivateKeyBase)) {
    $keyCandidate = $PrivateKeyBase
    if ($keyCandidate.EndsWith('.biprivatekey', [System.StringComparison]::OrdinalIgnoreCase)) {
        $keyCandidate = $keyCandidate.Substring(0, $keyCandidate.Length - '.biprivatekey'.Length)
    }

    $privateKey = $keyCandidate + '.biprivatekey'
    if (-not (Test-Path -LiteralPath $privateKey)) {
        throw "Private key not found: $privateKey"
    }

    # AddonBuilder 1.0.240639 expects the complete .biprivatekey filename,
    # not the extensionless key base.
    $arguments += "-sign=$privateKey"
}

Write-Host "Building Universal Outfit Swap..."
Write-Host "Source : $SourceDir"
Write-Host "Output : $AddonsDir"
if ([string]::IsNullOrWhiteSpace($PrivateKeyBase)) {
    Write-Host "Signing: disabled (unsigned test build)"
}
else {
    Write-Host "Signing: enabled"
}

& $AddonBuilder @arguments
if ($LASTEXITCODE -ne 0) {
    throw "AddonBuilder failed with exit code $LASTEXITCODE"
}

$pbo = Get-ChildItem -LiteralPath $AddonsDir -Filter '*.pbo' | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $pbo) {
    throw "AddonBuilder returned success but no PBO was found in $AddonsDir"
}

# AddonBuilder names the PBO after the source directory. Normalize case/name for release output.
$expectedPbo = Join-Path $AddonsDir 'UniversalOutfitSwap.pbo'
if ($pbo.FullName -ne $expectedPbo) {
    Move-Item -LiteralPath $pbo.FullName -Destination $expectedPbo -Force

    # A signature is named after the PBO filename. Normalize it in parallel.
    Get-ChildItem -LiteralPath $AddonsDir -Filter ($pbo.Name + '.*.bisign') -ErrorAction SilentlyContinue | ForEach-Object {
        $suffix = $_.Name.Substring($pbo.Name.Length)
        Move-Item -LiteralPath $_.FullName -Destination (Join-Path $AddonsDir ('UniversalOutfitSwap.pbo' + $suffix)) -Force
    }
}

if (-not [string]::IsNullOrWhiteSpace($keyCandidate)) {
    $publicKey = $keyCandidate + '.bikey'
    if (Test-Path -LiteralPath $publicKey) {
        $keysDir = Join-Path $OutputRoot 'keys'
        New-Item -ItemType Directory -Force -Path $keysDir | Out-Null
        Copy-Item -LiteralPath $publicKey -Destination (Join-Path $keysDir (Split-Path $publicKey -Leaf)) -Force
        Write-Host "Public key copied to: $keysDir"
    }
    else {
        Write-Warning "Matching public key not found: $publicKey (build is still signed)."
    }
}

Write-Host "Build complete: $OutputRoot"
