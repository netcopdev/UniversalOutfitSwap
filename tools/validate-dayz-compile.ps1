[CmdletBinding()]
param(
    [string]$DayZServer = "",
    [string]$AddonBuilder = "",
    [string]$Mission = "",
    [string]$OutputRoot = "",
    [int]$Port = 26020,
    [int]$TimeoutSeconds = 90,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$Version = (Get-Content -LiteralPath (Join-Path $RepoRoot "VERSION") -Raw).Trim()

function Get-SteamLibraryRoots
{
    $steamRoots = @()

    $programFilesX86 = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::ProgramFilesX86)
    if (-not [string]::IsNullOrWhiteSpace($programFilesX86)) {
        $steamRoots += Join-Path $programFilesX86 "Steam"
    }

    $programFiles = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::ProgramFiles)
    if (-not [string]::IsNullOrWhiteSpace($programFiles)) {
        $steamRoots += Join-Path $programFiles "Steam"
    }

    $libraryRoots = @()
    foreach ($steamRoot in ($steamRoots | Select-Object -Unique)) {
        if (-not (Test-Path -LiteralPath $steamRoot)) {
            continue
        }

        $libraryRoots += $steamRoot
        $libraryFoldersPath = Join-Path $steamRoot "steamapps\libraryfolders.vdf"
        if (-not (Test-Path -LiteralPath $libraryFoldersPath)) {
            continue
        }

        foreach ($line in (Get-Content -LiteralPath $libraryFoldersPath)) {
            if ($line -match '^\s*"path"\s+"([^"]+)"') {
                $libraryRoots += $Matches[1].Replace('\\', '\')
            }
        }
    }

    return @($libraryRoots | Select-Object -Unique)
}

function Find-SteamExecutable([string]$relativePath)
{
    foreach ($libraryRoot in (Get-SteamLibraryRoots)) {
        $candidate = Join-Path $libraryRoot $relativePath
        if (Test-Path -LiteralPath $candidate) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    return ""
}

function Get-SmokeText([string]$profilesDir, [string]$stdoutPath, [string]$stderrPath)
{
    $parts = @()

    foreach ($path in @($stdoutPath, $stderrPath)) {
        if (Test-Path -LiteralPath $path) {
            $parts += Get-Content -LiteralPath $path -Raw -ErrorAction SilentlyContinue
        }
    }

    if (Test-Path -LiteralPath $profilesDir) {
        $logFiles = Get-ChildItem -LiteralPath $profilesDir -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Extension -in @(".RPT", ".log") } |
            Sort-Object LastWriteTime

        foreach ($logFile in $logFiles) {
            $parts += Get-Content -LiteralPath $logFile.FullName -Raw -ErrorAction SilentlyContinue
        }
    }

    return ($parts -join [Environment]::NewLine)
}

function Get-CompileError([string]$text)
{
    if ([string]::IsNullOrWhiteSpace($text)) {
        return ""
    }

    $lines = $text -split "\r?\n"
    for ($i = 0; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -match 'Can''t compile "(Game|World|Mission)" script module!') {
            $end = [Math]::Min($i + 3, $lines.Count - 1)
            return (($lines[$i..$end] | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }) -join [Environment]::NewLine)
        }
    }

    return ""
}

function Get-LogTail([string]$text, [int]$lineCount = 40)
{
    if ([string]::IsNullOrWhiteSpace($text)) {
        return "(no server output captured)"
    }

    $lines = @($text -split "\r?\n")
    $start = [Math]::Max(0, $lines.Count - $lineCount)
    return (($lines[$start..($lines.Count - 1)]) -join [Environment]::NewLine)
}

if ([string]::IsNullOrWhiteSpace($DayZServer)) {
    $DayZServer = Find-SteamExecutable "steamapps\common\DayZServer\DayZServer_x64.exe"
}
if ([string]::IsNullOrWhiteSpace($DayZServer) -or -not (Test-Path -LiteralPath $DayZServer)) {
    throw "DayZServer_x64.exe was not found in the detected Steam libraries. Pass -DayZServer '<path>\DayZServer_x64.exe'."
}
$DayZServer = (Resolve-Path -LiteralPath $DayZServer).Path
$ServerRoot = Split-Path -Parent $DayZServer

if ([string]::IsNullOrWhiteSpace($AddonBuilder) -and -not $SkipBuild) {
    $AddonBuilder = Find-SteamExecutable "steamapps\common\DayZ Tools\Bin\AddonBuilder\AddonBuilder.exe"
}
if (-not $SkipBuild -and ([string]::IsNullOrWhiteSpace($AddonBuilder) -or -not (Test-Path -LiteralPath $AddonBuilder))) {
    throw "AddonBuilder.exe was not found in the detected Steam libraries. Pass -AddonBuilder '<path>\AddonBuilder.exe'."
}

if ([string]::IsNullOrWhiteSpace($Mission)) {
    $Mission = Join-Path $ServerRoot "mpmissions\dayzOffline.chernarusplus"
}
if (-not (Test-Path -LiteralPath $Mission)) {
    throw "DayZ mission was not found: $Mission. Pass -Mission '<mission directory>'."
}
$Mission = (Resolve-Path -LiteralPath $Mission).Path

if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $RepoRoot "dist\@UniversalOutfitSwap"
}

if (-not $SkipBuild) {
    Write-Host "Building compile-smoke package..."
    & (Join-Path $PSScriptRoot "build-release.ps1") -Clean -AddonBuilder $AddonBuilder -OutputRoot $OutputRoot
}

$PboPath = Join-Path $OutputRoot "addons\UniversalOutfitSwap.pbo"
if (-not (Test-Path -LiteralPath $PboPath)) {
    throw "Compile-smoke PBO not found: $PboPath"
}

$WorkRoot = Join-Path $RepoRoot "build\dayz-compile-smoke"
if (Test-Path -LiteralPath $WorkRoot) {
    Remove-Item -LiteralPath $WorkRoot -Recurse -Force
}

$ProfilesDir = Join-Path $WorkRoot "profiles"
$StorageDir = Join-Path $WorkRoot "storage"
New-Item -ItemType Directory -Force -Path $ProfilesDir, $StorageDir | Out-Null

$ConfigPath = Join-Path $WorkRoot "serverDZ.compile-smoke.cfg"
@"
hostname = "Universal Outfit Swap compile smoke";
password = "";
passwordAdmin = "uos-compile-smoke";
maxPlayers = 1;
verifySignatures = 0;
forceSameBuild = 0;
disableVoN = 1;
enableWhitelist = 0;
"@ | Set-Content -LiteralPath $ConfigPath -Encoding ASCII

$StdoutPath = Join-Path $WorkRoot "server.stdout.log"
$StderrPath = Join-Path $WorkRoot "server.stderr.log"
$SuccessMarker = "[UniversalOutfitSwap] v$Version initialized."

$ServerArguments = @(
    ('-config="{0}"' -f $ConfigPath),
    "-port=$Port",
    ('-profiles="{0}"' -f $ProfilesDir),
    ('-storage="{0}"' -f $StorageDir),
    ('-mission="{0}"' -f $Mission),
    ('-mod="{0}"' -f $OutputRoot),
    "-doLogs",
    "-limitFPS=10"
)

Write-Host "Starting DayZ dedicated-server compile smoke..."
Write-Host "Server : $DayZServer"
Write-Host "Mission: $Mission"
Write-Host "Mod    : $OutputRoot"
Write-Host "Logs   : $ProfilesDir"

$process = $null
$passed = $false
$lastText = ""

try {
    $process = Start-Process -FilePath $DayZServer -ArgumentList $ServerArguments -WorkingDirectory $ServerRoot -RedirectStandardOutput $StdoutPath -RedirectStandardError $StderrPath -PassThru
    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)

    while ([DateTime]::UtcNow -lt $deadline) {
        Start-Sleep -Milliseconds 250
        $lastText = Get-SmokeText $ProfilesDir $StdoutPath $StderrPath

        $compileError = Get-CompileError $lastText
        if (-not [string]::IsNullOrWhiteSpace($compileError)) {
            throw "DAYZ SCRIPT COMPILE: FAIL$([Environment]::NewLine)$compileError"
        }

        if ($lastText.Contains($SuccessMarker)) {
            $passed = $true
            break
        }

        if ($process.HasExited) {
            throw "DayZ server exited before the UOS initialization marker (exit code $($process.ExitCode)).$([Environment]::NewLine)$([Environment]::NewLine)$(Get-LogTail $lastText)"
        }
    }

    if (-not $passed) {
        throw "Timed out after $TimeoutSeconds seconds waiting for '$SuccessMarker'.$([Environment]::NewLine)$([Environment]::NewLine)$(Get-LogTail $lastText)"
    }

    Write-Host "DAYZ SCRIPT COMPILE: PASS"
    Write-Host "Observed: $SuccessMarker"
}
finally {
    if ($process -and -not $process.HasExited) {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        $process.WaitForExit()
    }
}
