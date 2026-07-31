<#
.SYNOPSIS
  Restore the Windows App SDK and generate the C++/WinRT projection headers on the guest.

.DESCRIPTION
  The port's windows backend needs two paths CMake cannot discover, because neither is a CMake concept:

    MAUI_WINAPPSDK       the restored Microsoft.WindowsAppSDK NuGet package (headers + import libs +
                         the bootstrap DLL)
    MAUI_WINUI_GENERATED the winrt/*.h projection headers produced by running cppwinrt.exe over that
                         package's .winmd files plus the installed Windows SDK metadata

  This script produces both, idempotently, and prints them as two machine-readable lines the caller
  parses (WINAPPSDK= / WINUI_GENERATED=). configure_port_windows.ps1 calls it.

  It does NOT touch the MSVC environment: cppwinrt.exe and nuget.exe are plain executables, so this can
  run before or after vcvarsall without interacting with it.

  Deliberately NOT shared with build_winui_probe.ps1, which carries its own copy of these steps: that
  script is the standalone WinUI 3 FEASIBILITY probe, and its entire value is that it depends on nothing
  else in the tree, so a failure there is unambiguous. Keep the versions below in sync with it.

.PARAMETER WorkDir
  Where nuget.exe, the restored packages and the generated headers live. Shared with the probe on
  purpose: a restore already done for one is reused by the other.
#>
[CmdletBinding()]
param(
    [string]$WorkDir = "C:\maui-winui",
    [string]$WinAppSdkVersion = "1.7.250606001",
    [string]$CppWinRtVersion = "2.0.240405.15",
    [string]$WebView2Version = "1.0.2903.40",
    [string]$Win2DVersion = "1.3.2"
)

$ErrorActionPreference = "Stop"
function Info($m) { Write-Host "[winui-sdk] $m" -ForegroundColor Cyan }
function Ok($m)   { Write-Host "[winui-sdk] OK  $m" -ForegroundColor Green }
$nativeEAP = "Continue"

New-Item -ItemType Directory -Force -Path $WorkDir | Out-Null

# ---------------------------------------------------------------- nuget
$nuget = Join-Path $WorkDir "nuget.exe"
if (-not (Test-Path $nuget)) {
    Info "downloading nuget.exe"
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest -UseBasicParsing "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe" -OutFile $nuget
}

# Microsoft.Web.WebView2 is REQUIRED even though the port's backend has no WebView yet:
# Microsoft.UI.Xaml.winmd declares Microsoft.UI.Xaml.Controls.IWebView2, whose CoreWebView2 property type
# lives in the WebView2 winmd, and cppwinrt resolves the WHOLE type graph. Without it, generation fails
# with "Type 'Microsoft.Web.WebView2.Core.CoreWebView2' could not be found" - which reads as a cppwinrt
# bug rather than a missing package. The App SDK does not declare it as a NuGet dependency.
$packages = Join-Path $WorkDir "packages"
foreach ($pkg in @(@{n="Microsoft.WindowsAppSDK"; v=$WinAppSdkVersion},
                   @{n="Microsoft.Web.WebView2";  v=$WebView2Version},
                   @{n="Microsoft.Graphics.Win2D"; v=$Win2DVersion},
                   @{n="Microsoft.Windows.CppWinRT"; v=$CppWinRtVersion})) {
    $dest = Join-Path $packages ($pkg.n + "." + $pkg.v)
    if (Test-Path $dest) { Ok "$($pkg.n) $($pkg.v) already restored"; continue }
    Info "restoring $($pkg.n) $($pkg.v)"
    $ErrorActionPreference = $nativeEAP
    & $nuget install $pkg.n -Version $pkg.v -OutputDirectory $packages -NonInteractive 2>&1 | Select-Object -Last 3
    $code = $LASTEXITCODE
    $ErrorActionPreference = "Stop"
    if ($code -ne 0 -or -not (Test-Path $dest)) { throw "nuget install $($pkg.n) failed (exit $code)" }
}

$wasdk = Join-Path $packages ("Microsoft.WindowsAppSDK." + $WinAppSdkVersion)
$webview2 = Join-Path $packages ("Microsoft.Web.WebView2." + $WebView2Version)
$win2d = Join-Path $packages ("Microsoft.Graphics.Win2D." + $Win2DVersion)
$cppwinrtPkg = Join-Path $packages ("Microsoft.Windows.CppWinRT." + $CppWinRtVersion)

# ---------------------------------------------------------------- cppwinrt projection
$generated = Join-Path $WorkDir "generated"
if ((Test-Path (Join-Path $generated "winrt\Microsoft.UI.Xaml.Controls.h")) -and
    (Test-Path (Join-Path $generated "winrt\Microsoft.Graphics.Canvas.h"))) {
    Ok "projection already generated"
} else {
    $cppwinrt = Get-ChildItem -Path $cppwinrtPkg -Filter cppwinrt.exe -Recurse | Select-Object -First 1
    if (-not $cppwinrt) { throw "cppwinrt.exe not found in $cppwinrtPkg" }
    # The winmd folder has moved between App SDK versions (lib/uap10.0 vs lib/uap10.0.18362), so SEARCH.
    # An empty -in would silently produce a projection missing every Microsoft.UI type, and the first
    # error would be a confusing "no such header" hundreds of lines later.
    $winmds = @()
    foreach ($root in @($wasdk, $webview2, $win2d)) {
        $winmds += Get-ChildItem -Path $root -Filter *.winmd -Recurse -EA SilentlyContinue |
                   Select-Object -ExpandProperty FullName
    }
    $winmds = $winmds | Sort-Object -Unique
    if (-not $winmds) { throw "no .winmd files found under $wasdk or $webview2" }
    Info "cppwinrt: $($winmds.Count) winmd(s) + the installed Windows SDK metadata"
    New-Item -ItemType Directory -Force -Path $generated | Out-Null
    $cppwinrtArgs = @("-in", "local")   # "local" = the installed Windows SDK's union metadata
    foreach ($w in $winmds) { $cppwinrtArgs += @("-in", $w) }
    $cppwinrtArgs += @("-out", $generated, "-overwrite")
    $ErrorActionPreference = $nativeEAP
    & $cppwinrt.FullName @cppwinrtArgs 2>&1 | Select-Object -Last 6
    $code = $LASTEXITCODE
    $ErrorActionPreference = "Stop"
    if ($code -ne 0) { throw "cppwinrt failed (exit $code)" }
    if (-not (Test-Path (Join-Path $generated "winrt\Microsoft.UI.Xaml.Controls.h"))) {
        throw "cppwinrt reported success but winrt/Microsoft.UI.Xaml.Controls.h is missing"
    }
    if (-not (Test-Path (Join-Path $generated "winrt\Microsoft.Graphics.Canvas.h"))) {
        throw "cppwinrt reported success but winrt/Microsoft.Graphics.Canvas.h is missing (Win2D winmd not picked up?)"
    }
    Ok "projection generated into $generated"
}

# Forward slashes: these go straight into a CMake -D argument, where a backslash is an escape.
Write-Output ("WINAPPSDK=" + ($wasdk -replace '\\','/'))
Write-Output ("WINUI_GENERATED=" + ($generated -replace '\\','/'))
Write-Output ("WIN2D=" + ($win2d -replace '\\','/'))
