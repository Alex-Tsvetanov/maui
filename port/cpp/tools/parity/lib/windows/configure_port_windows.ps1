<#
.SYNOPSIS
  Configure the C++ port for MAUI_BACKEND=windows on the guest (MSVC + vcpkg + Ninja).

.DESCRIPTION
  Answers the question that gates all Windows backend work: do the port's existing ~400 cross-platform
  sources compile under MSVC? That is independent of any handler code and of the ABI question, so it is
  worth proving before writing a line of the backend.

  Three things here are load-bearing and each failed once already:

  1. NINJA BY FULL PATH. vcvarsall rebuilds PATH from scratch, and winget's shim directory
     (AppData\Local\Microsoft\WinGet\Links) does not survive it - so CMake reports "unable to find a
     build program corresponding to Ninja" even though `ninja` resolves fine in a normal shell.

  2. VCPKG_HOST_TRIPLET, not just the target one. vcpkg builds its HOST tools with the host triplet,
     which on an ARM64 machine defaults to arm64-windows; without an arm64 toolchain installed it dies
     with "Unable to find a valid toolchain for requested target architecture arm64" long before it
     reaches the target triplet.

  3. ABI AUTO-DETECTION. Prefer a native arm64 toolchain when present, else arm64_x64 (native host, x64
     target). The MAUI reference board was captured NATIVE arm64, so a non-native build must warn: a
     score across two ABIs compares two rendering paths, not parity.

  4. SOURCE IS THE READ-ONLY SHARE. Z:\ is the host repo mounted by UTM, not a copy of it, so the guest
     cannot render stale code -- the failure that cost a full day on 2026-08-11 and that SYNC_STAMP.txt
     was supposed to catch. Nothing is written into the source tree: CMake is an out-of-source build by
     construction, and the XAML bytes-mode codegen already emits into CMAKE_CURRENT_BINARY_DIR.
     Warm reads off the share measured 1.2 ms/file, FASTER than the guest's own C: (5.8 ms/file); only
     the first touch of a file is slow (~25 ms), and that cost is paid once per boot.
#>
[CmdletBinding()]
param(
    [string]$SourceDir = "Z:\port\cpp",
    [string]$BuildDir  = "C:\maui-build\cpp",
    [string]$BuildType = "Debug"
)

$ErrorActionPreference = "Continue"
function Info($m) { Write-Host "[port] $m" -ForegroundColor Cyan }
function Warn($m) { Write-Host "[port] !   $m" -ForegroundColor Yellow }

$ninja = (Get-Command ninja -ErrorAction SilentlyContinue).Source
if (-not $ninja) { $ninja = "C:\Users\Testings-VM\AppData\Local\Microsoft\WinGet\Links\ninja.exe" }
if (-not (Test-Path $ninja)) { throw "ninja not found (looked on PATH and at $ninja)" }
Info "ninja: $ninja"

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsRoot = (& $vswhere -latest -products * -property installationPath 2>&1 | Select-Object -First 1)
if (-not $vsRoot) { throw "vswhere found no Visual Studio installation" }
$vcvars = Join-Path $vsRoot "VC\Auxiliary\Build\vcvarsall.bat"

$msvcRoot = Join-Path $vsRoot "VC\Tools\MSVC"
$targets = Get-ChildItem $msvcRoot -Recurse -Filter cl.exe -ErrorAction SilentlyContinue |
           ForEach-Object { $_.Directory.Parent.Name + "\" + $_.Directory.Name } | Sort-Object -Unique
Info ("cl targets: " + ($targets -join ", "))

if ($targets -contains "Hostarm64\arm64") {
    $arch = "arm64"; $abi = "arm64"; $triplet = "arm64-windows"
} elseif ($targets -contains "Hostarm64\x64") {
    $arch = "arm64_x64"; $abi = "x64"; $triplet = "x64-windows"
} else {
    throw "no usable cl.exe target under $msvcRoot"
}
Info "toolchain: vcvarsall $arch  (ABI $abi, vcpkg triplet $triplet)"
if ($abi -ne "arm64") {
    Warn "NOT native arm64. The MAUI reference board is arm64 - do not publish a score across ABIs."
    Warn "Install 'MSVC v143 - VS 2022 C++ ARM64/ARM64EC build tools' to build natively."
}

Info "importing the MSVC environment"
cmd /c "`"$vcvars`" $arch >nul 2>&1 && set" | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") {
        Set-Item -Path ("Env:" + $matches[1]) -Value $matches[2] -Force -ErrorAction SilentlyContinue
    }
}
$cl = (Get-Command cl.exe -ErrorAction SilentlyContinue).Source
if (-not $cl) { throw "cl.exe not on PATH after importing vcvarsall $arch" }
Info "cl: $cl"

# The WinUI 3 dependencies. MAUI_BACKEND=windows compiles against the C++/WinRT projection, which is
# GENERATED (cppwinrt.exe over the App SDK winmds) rather than shipped, so it has to exist before cmake
# runs. provision_winui_sdk.ps1 is idempotent and prints the two paths cmake needs.
Info "provisioning the Windows App SDK + C++/WinRT projection"
$sdkOut = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "provision_winui_sdk.ps1") 2>&1
$sdkOut | ForEach-Object { Write-Host "   $_" }
$wasdk = ($sdkOut | Select-String '^WINAPPSDK=(.+)$').Matches.Groups[1].Value
$generated = ($sdkOut | Select-String '^WINUI_GENERATED=(.+)$').Matches.Groups[1].Value
$win2d = ($sdkOut | Select-String '^WIN2D=(.+)$').Matches.Groups[1].Value
$webview2 = ($sdkOut | Select-String '^WEBVIEW2=(.+)$').Matches.Groups[1].Value
if (-not $wasdk -or -not $generated -or -not $win2d -or -not $webview2) { throw "provision_winui_sdk.ps1 did not report all four paths" }
Info "WindowsAppSDK: $wasdk"
Info "projection   : $generated"
Info "Win2D        : $win2d"
Info "WebView2     : $webview2"

$env:VCPKG_ROOT = "C:\vcpkg"
Info "configuring (vcpkg builds gtest/benchmark/pugixml first; this is the slow part)"
# Arguments as an ARRAY, then splatted. Backtick-continued bareword arguments to a native command are
# not reliably variable-expanded: the previous form passed the LITERAL text "$triplet" to vcpkg, which
# rejected it with "Invalid triplet name ... on expression: $triplet". Quoting each element and splatting
# removes the ambiguity entirely.
$cmakeArgs = @(
    "-S", $SourceDir,
    "-B", $BuildDir,
    "-G", "Ninja",
    "-DCMAKE_MAKE_PROGRAM=$ninja",
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DMAUI_BACKEND=windows",
    "-DVCPKG_TARGET_TRIPLET=$triplet",
    "-DVCPKG_HOST_TRIPLET=$triplet",
    "-DMAUI_TARGET_ABI=$abi",
    "-DMAUI_WINAPPSDK=$wasdk",
    "-DMAUI_WINUI_GENERATED=$generated",
    "-DMAUI_WIN2D=$win2d",
    "-DMAUI_WEBVIEW2=$webview2",
    "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
)
Info ("cmake " + ($cmakeArgs -join " "))
& cmake @cmakeArgs 2>&1 | Select-Object -Last 40
$code = $LASTEXITCODE
Info "configure exit: $code"
if ($code -ne 0) {
    $log = Join-Path $BuildDir "vcpkg-manifest-install.log"
    if (Test-Path $log) { Info "vcpkg manifest log (tail):"; Get-Content $log -Tail 15 }
}
exit $code
