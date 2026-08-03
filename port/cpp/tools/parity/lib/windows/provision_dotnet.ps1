<#
.SYNOPSIS
  Install the .NET SDK + MAUI Windows workload on the Windows E2E guest.

.DESCRIPTION
  Needed to build the MAUI REFERENCE app (port/maui-reference/app) on the guest. MAUI's Windows target
  is WinUI 3 and cannot be cross-built from macOS, so the reference column for Windows parity has to be
  produced here.

  Uses the official dotnet-install.ps1 rather than winget: it pins the channel/architecture explicitly
  (this guest is ARM64), needs no store dependency, and is the documented way to get a specific band.

  Everything is idempotent. Re-running with the SDK already present skips the download.

.PARAMETER Channel
  .NET channel, e.g. "10.0". Must match the MauiVersion band the reference app pins.

.PARAMETER InstallDir
  Where to place the SDK. Kept OUT of Program Files so workload installs need no further elevation.

.PARAMETER SkipWorkload
  Install the SDK only (useful to split a long download into two steps).

.EXAMPLE
  .\provision_dotnet.ps1 -Channel 10.0
#>
[CmdletBinding()]
param(
    [string]$Channel = "10.0",
    [string]$InstallDir = "C:\dotnet",
    [switch]$SkipWorkload
)

$ErrorActionPreference = "Stop"
function Info($m) { Write-Host "[dotnet] $m" -ForegroundColor Cyan }
function Ok($m)   { Write-Host "[dotnet] OK  $m" -ForegroundColor Green }
function Warn($m) { Write-Host "[dotnet] !   $m" -ForegroundColor Yellow }

# Native commands and $ErrorActionPreference="Stop" do not mix: PowerShell turns ANY stderr write from an
# external exe into a terminating NativeCommandError, and `dotnet` writes first-run/telemetry banners
# there. So every dotnet call goes through this helper, which merges stderr into stdout, keeps
# ErrorActionPreference relaxed for the duration, and judges success by the EXIT CODE (the only reliable
# signal) instead of by whether anything reached stderr.
function Invoke-Dotnet {
    param([Parameter(Mandatory)][string[]]$Arguments, [switch]$AllowFailure)
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        $out = & $script:dotnet @Arguments 2>&1 | Out-String
        $code = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $prev
    }
    if ($code -ne 0 -and -not $AllowFailure) {
        throw ("dotnet " + ($Arguments -join " ") + " failed (exit $code):`n" + $out)
    }
    return [pscustomobject]@{ Output = $out; ExitCode = $code }
}

$arch = if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") { "arm64" } else { "x64" }
Info "guest architecture: $arch"

$dotnet = Join-Path $InstallDir "dotnet.exe"
$script:dotnet = $dotnet

# ---------------------------------------------------------------- SDK
if (Test-Path $dotnet) {
    Ok ("SDK already present: " + (Invoke-Dotnet @("--version")).Output.Trim())
} else {
    Info "downloading dotnet-install.ps1"
    $script = Join-Path $env:TEMP "dotnet-install.ps1"
    # TLS 1.2 explicitly: a fresh Windows image can still default to a protocol the CDN rejects, and the
    # failure surfaces as an opaque "underlying connection was closed".
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest -UseBasicParsing "https://dot.net/v1/dotnet-install.ps1" -OutFile $script
    Info "installing .NET SDK channel $Channel ($arch) into $InstallDir (this takes a few minutes)"
    & $script -Channel $Channel -Architecture $arch -InstallDir $InstallDir
    if (-not (Test-Path $dotnet)) { throw "dotnet.exe not found at $dotnet after install" }
    $script:dotnet = $dotnet
    Ok ("SDK installed: " + (Invoke-Dotnet @("--version")).Output.Trim())
}

# ---------------------------------------------------------------- PATH + DOTNET_ROOT
# Set MACHINE-scope so a non-interactive SSH session (which does not run a login profile) still finds
# dotnet, and so the session-1 scheduled task inherits it too.
Info "putting dotnet on the machine PATH and setting DOTNET_ROOT"
$machinePath = [Environment]::GetEnvironmentVariable("Path", "Machine")
if ($machinePath -notlike "*$InstallDir*") {
    [Environment]::SetEnvironmentVariable("Path", "$machinePath;$InstallDir", "Machine")
    Ok "PATH += $InstallDir"
} else {
    Ok "PATH already contains $InstallDir"
}
[Environment]::SetEnvironmentVariable("DOTNET_ROOT", $InstallDir, "Machine")
# Telemetry off: it adds a first-run banner that pollutes command output the host parses.
[Environment]::SetEnvironmentVariable("DOTNET_CLI_TELEMETRY_OPTOUT", "1", "Machine")
$env:Path = "$env:Path;$InstallDir"
$env:DOTNET_ROOT = $InstallDir

# ---------------------------------------------------------------- MAUI workload
if ($SkipWorkload) {
    Warn "-SkipWorkload: stopping before the MAUI workload"
    exit 0
}

$installed = (Invoke-Dotnet @("workload","list") -AllowFailure).Output
if ($installed -match "maui-windows") {
    Ok "maui-windows workload already installed"
} else {
    Info "installing the maui-windows workload (large download; several minutes)"
    # maui-windows only, NOT the full `maui` workload: the mobile bands are irrelevant on this guest and
    # would add gigabytes plus an Android SDK dependency.
    Invoke-Dotnet @("workload","install","maui-windows","--skip-sign-check") | Out-Null
    $after = (Invoke-Dotnet @("workload","list") -AllowFailure).Output
    if ($after -notmatch "maui-windows") { throw "maui-windows workload still missing after install" }
    Ok "maui-windows workload installed"
}

Write-Host ""
Info "versions:"
(Invoke-Dotnet @("--info") -AllowFailure).Output -split "`n" |
    Select-String "Version:|RID:|Base Path:" | ForEach-Object { "   " + $_.Line.Trim() }
Write-Host ""
Ok "ready - build the reference app with tools/parity/windows/build_maui_reference.ps1"
