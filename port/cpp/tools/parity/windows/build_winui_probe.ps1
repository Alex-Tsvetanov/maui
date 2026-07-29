<#
.SYNOPSIS
  Restore the WinUI 3 dependencies and build the feasibility probe on the guest.

.DESCRIPTION
  Answers the gating question for the port's Windows backend: can a CODE-ONLY C++/WinRT WinUI 3 app be
  built with CMake + Ninja (no MSBuild, no XAML compiler) and show real Microsoft.UI.Xaml controls?

  Steps, each of which is a place this can realistically fail, so each reports separately:
    1. locate the MSVC toolchain (vcvarsall) and import its environment -- CMake cannot find cl.exe
       otherwise, and a "compiler not found" error would look like a CMake problem
    2. restore Microsoft.WindowsAppSDK + Microsoft.Windows.CppWinRT from NuGet
    3. run cppwinrt.exe to generate the C++/WinRT projection headers for the SDK + App SDK winmds
    4. configure + build with CMake/Ninja

  Why not MSBuild: the port is a CMake project, and its Windows backend has to build the same way as its
  other backends. The XAML compiler -- the main thing MSBuild brings -- is unnecessary because the port
  constructs its UI in C++ code, never from markup.

.PARAMETER ProbeDir
  The synced winui_probe source directory on the guest.

.PARAMETER WinAppSdkVersion
  Windows App SDK package version. Must match the MddBootstrapInitialize2 major/minor in main.cpp.
#>
[CmdletBinding()]
param(
    [string]$ProbeDir = "C:\maui-src\cpp\tools\parity\windows\winui_probe",
    [string]$WorkDir = "C:\maui-winui",
    [string]$WinAppSdkVersion = "1.8.251106002",
    [string]$CppWinRtVersion = "2.0.240405.15",
    [string]$WebView2Version = "1.0.2903.40"
)

$ErrorActionPreference = "Stop"
function Info($m) { Write-Host "[winui] $m" -ForegroundColor Cyan }
function Ok($m)   { Write-Host "[winui] OK  $m" -ForegroundColor Green }
function Warn($m) { Write-Host "[winui] !   $m" -ForegroundColor Yellow }
$nativeEAP = "Continue"

if (-not (Test-Path $ProbeDir)) { throw "ProbeDir not found: $ProbeDir (sync the repo to the guest first)" }
New-Item -ItemType Directory -Force -Path $WorkDir | Out-Null

# ---------------------------------------------------------------- 1. MSVC environment
Info "locating the MSVC toolchain"
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) { throw "vswhere.exe not found; is VS Build Tools installed?" }
$ErrorActionPreference = $nativeEAP
$vsRoot = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath 2>&1 | Select-Object -First 1)
$ErrorActionPreference = "Stop"
if (-not $vsRoot) { throw "no VS install with the C++ toolset found (vswhere returned nothing)" }
Ok "VS: $vsRoot"

# Pick the vcvarsall arch from what is ACTUALLY installed rather than from the host architecture.
# VC.Tools.x86.x64 provides only x86/x64 TARGETS, so an ARM64 guest can easily have Hostarm64\x64 (a
# native-host compiler targeting x64) but no arm64 target at all -- in which case `vcvarsall arm64`
# finds no cl.exe and the failure reads as a broken vcvars rather than a missing component.
# Preference order on an ARM64 host: native arm64, then arm64_x64 (native host, x64 target, runs under
# the OS's x64 emulation). The chosen target is REPORTED, because it must match the architecture the
# MAUI reference was built for or the parity comparison is measuring two different rendering paths.
$msvcRoot = Join-Path $vsRoot "VC\Tools\MSVC"
$targets = @()
if (Test-Path $msvcRoot) {
    $targets = Get-ChildItem $msvcRoot -Recurse -Filter cl.exe -EA SilentlyContinue |
               ForEach-Object { $_.Directory.Parent.Name + "\" + $_.Directory.Name } |
               Sort-Object -Unique
}
Info ("installed cl.exe targets: " + ($targets -join ", "))
if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") {
    if ($targets -contains "Hostarm64\arm64") { $hostArch = "arm64";     $abi = "arm64" }
    elseif ($targets -contains "Hostarm64\x64") { $hostArch = "arm64_x64"; $abi = "x64" }
    else { throw "no usable cl.exe target found under $msvcRoot" }
} else {
    $hostArch = "x64"; $abi = "x64"
}
Info "vcvarsall arch: $hostArch  (target ABI: $abi)"
if ($abi -ne "arm64") {
    Warn "building $abi, NOT native arm64. The MAUI reference must be built for the SAME ABI or the"
    Warn "comparison spans two rendering paths - rebuild it with build_maui_reference.ps1 -Rid win-$abi."
}
$vcvars = Join-Path $vsRoot "VC\Auxiliary\Build\vcvarsall.bat"
if (-not (Test-Path $vcvars)) { throw "vcvarsall.bat not found at $vcvars" }

# Import vcvarsall's environment into THIS session. cl.exe depends on ~20 environment variables (INCLUDE,
# LIB, PATH, WindowsSdkDir ...); running it in a child shell would set them and then throw them away.
Info "importing the MSVC environment ($hostArch)"
$envDump = Join-Path $WorkDir "vcvars.txt"
cmd /c "`"$vcvars`" $hostArch >nul 2>&1 && set" | Out-File -FilePath $envDump -Encoding ascii
if (-not (Test-Path $envDump)) { throw "failed to capture the vcvarsall environment" }
Get-Content $envDump | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") { Set-Item -Path ("Env:" + $matches[1]) -Value $matches[2] -Force }
}
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) { throw "cl.exe still not on PATH after importing vcvarsall" }
Ok "cl.exe: $($cl.Source)"

# ---------------------------------------------------------------- 2. NuGet packages
$nuget = Join-Path $WorkDir "nuget.exe"
if (-not (Test-Path $nuget)) {
    Info "downloading nuget.exe"
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest -UseBasicParsing "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe" `
        -OutFile $nuget
}
$packages = Join-Path $WorkDir "packages"
# Microsoft.Web.WebView2 is REQUIRED even though nothing here uses a WebView: Microsoft.UI.Xaml.winmd
# declares Microsoft.UI.Xaml.Controls.IWebView2, whose CoreWebView2 property type lives in the WebView2
# winmd. cppwinrt resolves the whole type graph, so without it generation fails with
# "Type 'Microsoft.Web.WebView2.Core.CoreWebView2' could not be found" -- which reads as a cppwinrt bug
# rather than a missing package. It is not pulled in automatically: the App SDK does not declare it as a
# NuGet dependency.
foreach ($pkg in @(@{n="Microsoft.WindowsAppSDK"; v=$WinAppSdkVersion},
                   @{n="Microsoft.Web.WebView2"; v=$WebView2Version},
                   @{n="Microsoft.Windows.CppWinRT"; v=$CppWinRtVersion})) {
    $dest = Join-Path $packages ($pkg.n + "." + $pkg.v)
    if (Test-Path $dest) { Ok "$($pkg.n) $($pkg.v) already restored"; continue }
    Info "restoring $($pkg.n) $($pkg.v)"
    $ErrorActionPreference = $nativeEAP
    & $nuget install $pkg.n -Version $pkg.v -OutputDirectory $packages -NonInteractive 2>&1 |
        Select-Object -Last 3
    $code = $LASTEXITCODE
    $ErrorActionPreference = "Stop"
    if ($code -ne 0 -or -not (Test-Path $dest)) { throw "nuget install $($pkg.n) failed (exit $code)" }
    Ok "$($pkg.n) restored"
}
$wasdk = Join-Path $packages ("Microsoft.WindowsAppSDK." + $WinAppSdkVersion)
$cppwinrtPkg = Join-Path $packages ("Microsoft.Windows.CppWinRT." + $CppWinRtVersion)

# ---------------------------------------------------------------- 3. cppwinrt projection
$cppwinrt = Get-ChildItem -Path $cppwinrtPkg -Filter cppwinrt.exe -Recurse |
            Select-Object -First 1
if (-not $cppwinrt) { throw "cppwinrt.exe not found in $cppwinrtPkg" }
$generated = Join-Path $WorkDir "generated"

# The App SDK's winmds. Their folder has moved between versions (lib/uap10.0 vs lib/uap10.0.18362), so
# search instead of assuming, and fail loudly if none are found -- an empty -in would silently produce a
# projection missing every Microsoft.UI type, and the first error would be a confusing "no such header".
$webview2 = Join-Path $packages ("Microsoft.Web.WebView2." + $WebView2Version)
# Scan BOTH packages: the App SDK's own winmds plus WebView2's, since the former references the latter.
$winmds = @()
foreach ($root in @($wasdk, $webview2)) {
    $winmds += Get-ChildItem -Path $root -Filter *.winmd -Recurse -EA SilentlyContinue |
               Select-Object -ExpandProperty FullName
}
$winmds = $winmds | Sort-Object -Unique
if (-not $winmds) { throw "no .winmd files found under $wasdk or $webview2" }
Info "cppwinrt: $($winmds.Count) App SDK winmd(s) + the platform SDK"

if (Test-Path (Join-Path $generated "winrt\Microsoft.UI.Xaml.Controls.h")) {
    Ok "projection already generated"
} else {
    New-Item -ItemType Directory -Force -Path $generated | Out-Null
    $args = @("-in", "local")           # "local" = the installed Windows SDK's union metadata
    foreach ($w in $winmds) { $args += @("-in", $w) }
    $args += @("-out", $generated, "-overwrite")
    $ErrorActionPreference = $nativeEAP
    & $cppwinrt.FullName @args 2>&1 | Select-Object -Last 6
    $code = $LASTEXITCODE
    $ErrorActionPreference = "Stop"
    if ($code -ne 0) { throw "cppwinrt failed (exit $code)" }
    if (-not (Test-Path (Join-Path $generated "winrt\Microsoft.UI.Xaml.Controls.h"))) {
        throw "cppwinrt reported success but winrt/Microsoft.UI.Xaml.Controls.h is missing"
    }
    Ok "projection generated into $generated"
}

# ---------------------------------------------------------------- 4. configure + build
$build = Join-Path $WorkDir "build"
Info "configuring CMake (Ninja)"
$ErrorActionPreference = $nativeEAP
& cmake -S $ProbeDir -B $build -G Ninja `
    -DCMAKE_BUILD_TYPE=Debug `
    "-DMAUI_WINUI_GENERATED=$($generated -replace '\\','/')" `
    "-DMAUI_WINAPPSDK=$($wasdk -replace '\\','/')" `
    "-DMAUI_TARGET_ABI=$abi" 2>&1 | Select-Object -Last 12
$code = $LASTEXITCODE
if ($code -ne 0) { $ErrorActionPreference = "Stop"; throw "cmake configure failed (exit $code)" }

Info "building"
& cmake --build $build 2>&1 | Select-Object -Last 30
$code = $LASTEXITCODE
$ErrorActionPreference = "Stop"
if ($code -ne 0) { throw "build failed (exit $code)" }

$exe = Join-Path $build "maui_winui_probe.exe"
if (-not (Test-Path $exe)) { throw "build succeeded but $exe is missing" }
Ok "built $exe"
Write-Output ("WINUI_PROBE_EXE=" + $exe)
