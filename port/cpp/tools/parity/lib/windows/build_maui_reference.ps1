<#
.SYNOPSIS
  Build the MAUI reference app (WinUI 3) on the Windows E2E guest.

.DESCRIPTION
  This produces the GROUND TRUTH column for Windows parity. MAUI's Windows backend is WinUI 3
  (Microsoft.UI.Xaml), which cannot be cross-built from macOS, so the reference has to be built here.

  The app is the same port/maui-reference/app project the iOS / Mac Catalyst / Android lanes use, with a
  Windows TFM appended by an IsOSPlatform('windows') condition, so this adds a column without forking
  the reference app.

  Built UNPACKAGED (WindowsPackageType=None, set in the csproj): the E2E runner deploys a folder and
  launches an .exe, exactly as the macOS lane copies a .app. MSIX would need an install step per run and
  could not be swapped by copying files.

.PARAMETER SourceDir
  port/maui-reference/app ON THE READ-ONLY HOST SHARE. Z:\ is the host repo itself rather than a copy,
  so this column cannot render stale code -- the failure that went unnoticed for a full day on
  2026-08-11 while SYNC_STAMP.txt claimed the tree was current.

.PARAMETER OutputRoot
  Where bin/ and obj/ go. UNLIKE the CMake lanes, MSBuild is NOT out-of-source: it writes obj/ and bin/
  beside the project by default, which a read-only source makes impossible. BaseIntermediateOutputPath
  and BaseOutputPath move both, and are passed to RESTORE as well as BUILD -- restore writes
  project.assets.json and the generated .nuget.g.props/.targets into obj/, and if the two commands
  disagree about where obj/ is, the build fails looking for assets restore already wrote elsewhere.
  Both MUST keep their trailing backslash; MSBuild concatenates these as string prefixes.
  Deliberately NOT `dotnet --artifacts-path`, which would also work but renames the leaf directory to
  <Config>_<tfm>_<rid> -- windows.toml pins a static artifact path, and this keeps the familiar
  bin/<Config>/<tfm>/<rid>/ layout it already encodes.

.PARAMETER Configuration
  Debug (default) or Release. Debug matches what the other lanes capture.

.PARAMETER Tfm
  Windows target framework moniker; must match the csproj.

.EXAMPLE
  .\build_maui_reference.ps1
#>
[CmdletBinding()]
param(
    [string]$SourceDir = "Z:\port\maui-reference\app",
    [string]$OutputRoot = "C:\maui-build\maui-reference",
    [string]$Configuration = "Debug",
    [string]$Tfm = "net10.0-windows10.0.19041.0"
)

$ErrorActionPreference = "Stop"
# Relaxed for the native dotnet invocations below: with "Stop", ANY stderr write from an
# external exe becomes a terminating NativeCommandError, and dotnet restore/build both write
# progress there. Success is judged by $LASTEXITCODE, the only reliable signal.
$nativeEAP = "Continue"
function Info($m) { Write-Host "[maui-ref] $m" -ForegroundColor Cyan }
function Ok($m)   { Write-Host "[maui-ref] OK  $m" -ForegroundColor Green }
function Warn($m) { Write-Host "[maui-ref] !   $m" -ForegroundColor Yellow }

if (-not (Test-Path $SourceDir)) { throw "SourceDir not found: $SourceDir (sync it from the host first)" }

$dotnet = "C:\dotnet\dotnet.exe"
if (-not (Test-Path $dotnet)) {
    $cmd = Get-Command dotnet -ErrorAction SilentlyContinue
    if (-not $cmd) { throw "dotnet not found; run provision_dotnet.ps1 first" }
    $dotnet = $cmd.Source
}
$env:DOTNET_CLI_TELEMETRY_OPTOUT = "1"
# Deterministic, quiet-ish output the host can parse; NuGet restore noise otherwise dominates the log.
$env:DOTNET_NOLOGO = "1"

$csproj = Get-ChildItem -Path $SourceDir -Filter *.csproj | Select-Object -First 1
if (-not $csproj) { throw "no .csproj under $SourceDir" }
Info "project: $($csproj.FullName)"
$ErrorActionPreference = $nativeEAP
Info "sdk    : $((& $dotnet --version 2>&1) -join ' ')"

# The RID must be explicit. MAUI's Windows default is win10-x64, which on an ARM64 guest would build an
# x64 app that then runs under emulation - a difference we do NOT want in the reference column, since the
# whole point is to capture what MAUI natively renders on this machine.
$rid = if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") { "win-arm64" } else { "win-x64" }
Info "rid    : $rid ($Configuration, $Tfm)"

# Trailing backslashes are load-bearing: MSBuild treats these as string prefixes, so without them the
# output lands in a sibling named e.g. "maui-referencebin". Passed to BOTH commands -- see .PARAMETER
# OutputRoot for why splitting them breaks restore.
$objDir = (Join-Path $OutputRoot "obj") + "\"
$binDir = (Join-Path $OutputRoot "bin") + "\"
$outArgs = @("-p:BaseIntermediateOutputPath=$objDir", "-p:BaseOutputPath=$binDir")
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
Info "obj    : $objDir"
Info "bin    : $binDir"

Info "restoring (TargetFrameworks pinned to $Tfm so the mobile TFMs are never evaluated)"
$ErrorActionPreference = $nativeEAP
& $dotnet restore $csproj.FullName -p:TargetFrameworks=$Tfm -r $rid @outArgs 2>&1 |
    Select-Object -Last 8
if ($LASTEXITCODE -ne 0) { throw "restore failed with exit code $LASTEXITCODE" }

Info "building (first build pulls the Windows App SDK; several minutes)"
& $dotnet build $csproj.FullName -p:TargetFrameworks=$Tfm -f $Tfm -c $Configuration -r $rid `
    @outArgs --no-restore --nologo -v minimal 2>&1 | Select-Object -Last 30
if ($LASTEXITCODE -ne 0) { throw "build failed with exit code $LASTEXITCODE" }

# Locate the produced exe. The layout is bin/<Config>/<tfm>/<rid>/<AppName>.exe, but the RID folder name
# has changed across SDK bands (win10-arm64 vs win-arm64), so search rather than assume.
# Rooted at $OutputRoot, NOT $SourceDir -- the share is read-only and nothing is produced there.
$binRoot = Join-Path $binDir "$Configuration\$Tfm"
$exe = Get-ChildItem -Path $binRoot -Filter "MauiReference.exe" -Recurse -ErrorAction SilentlyContinue |
       Select-Object -First 1
if (-not $exe) { throw "build reported success but MauiReference.exe was not found under $binRoot" }

Ok "exe: $($exe.FullName)"
Ok "dir: $($exe.Directory.FullName)"
Write-Host ""
Info "deploy that DIRECTORY as the maui_xaml column artifact; process = MauiReference.exe"
# Emit one machine-readable line the host can grep, so the host does not have to guess the RID folder.
Write-Output ("MAUI_REF_EXE=" + $exe.FullName)
Write-Output ("MAUI_REF_DIR=" + $exe.Directory.FullName)
