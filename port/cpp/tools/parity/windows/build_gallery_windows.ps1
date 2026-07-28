<#
.SYNOPSIS
  Build the examples gallery for MAUI_BACKEND=windows on the guest (WinUI 3, MSVC + Ninja).

.DESCRIPTION
  The gallery is the C++ PARITY COLUMN: one app that renders whichever demo page MAUI_SAMPLE_PAGE names,
  which is what the E2E runner launches once per page/theme. It lives in the STANDALONE examples/ project
  (port/cpp/examples), so this is a second CMake configure, not another target of the framework build.

  Consumed in-tree (MAUI_EXAMPLES_FRAMEWORK_DIR) rather than through an installed prefix: the framework
  changes constantly during the backend fan-out, and add_subdirectory means one rebuild covers both.

  Run configure_port_windows.ps1 FIRST - this reuses the Windows App SDK + C++/WinRT projection it
  provisioned (the same paths, via provision_winui_sdk.ps1, which is idempotent).
#>
[CmdletBinding()]
param(
    [string]$SourceDir = "C:\maui-src\cpp\examples",
    [string]$FrameworkDir = "C:\maui-src\cpp",
    [string]$BuildDir = "C:\maui-src\cpp\examples\build-win",
    [string]$BuildType = "Debug",
    [string[]]$Targets = @("gallery"),
    [int]$Jobs = 8
)

$ErrorActionPreference = "Continue"
# Re-split on commas. Invoked as `powershell -File this.ps1 -Targets gallery,gallery_xaml`, PowerShell
# hands the parameter the LITERAL STRING "gallery,gallery_xaml" -- the comma is array syntax only when
# the script is dot-sourced or called in-language, which the SSH lane never does. cmake then looks for
# one target named "gallery,gallery_xaml" and fails. Space-separating is worse: only the first token
# binds to -Targets and the second lands on the next positional parameter ($SourceDir), which surfaces
# much later as "the source directory C:/.../gallery_xaml does not exist". Splitting here makes every
# invocation form work.
$Targets = @($Targets | ForEach-Object { $_ -split ',' } | Where-Object { $_ })

function Info($m) { Write-Host "[gallery] $m" -ForegroundColor Cyan }
function Warn($m) { Write-Host "[gallery] !   $m" -ForegroundColor Yellow }

$ninja = (Get-Command ninja -ErrorAction SilentlyContinue).Source
if (-not $ninja) { $ninja = "C:\Users\Testings-VM\AppData\Local\Microsoft\WinGet\Links\ninja.exe" }
if (-not (Test-Path $ninja)) { throw "ninja not found (looked on PATH and at $ninja)" }

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsRoot = (& $vswhere -latest -products * -property installationPath 2>&1 | Select-Object -First 1)
$msvcRoot = Join-Path $vsRoot "VC\Tools\MSVC"
# NOT $targets: PowerShell variable names are case-insensitive, so that would silently overwrite the
# $Targets PARAMETER and cmake would be handed compiler directory names as build targets.
$clTargets = Get-ChildItem $msvcRoot -Recurse -Filter cl.exe -ErrorAction SilentlyContinue |
             ForEach-Object { $_.Directory.Parent.Name + "\" + $_.Directory.Name } | Sort-Object -Unique
if ($clTargets -contains "Hostarm64\arm64") {
    $arch = "arm64"; $abi = "arm64"; $triplet = "arm64-windows"
} else {
    $arch = "arm64_x64"; $abi = "x64"; $triplet = "x64-windows"
    Warn "NOT native arm64 - the MAUI reference board is arm64; do not publish a score across ABIs."
}
Info "vcvarsall $arch (ABI $abi)"
cmd /c "`"$(Join-Path $vsRoot 'VC\Auxiliary\Build\vcvarsall.bat')`" $arch >nul 2>&1 && set" |
    ForEach-Object {
        if ($_ -match "^([^=]+)=(.*)$") {
            Set-Item -Path ("Env:" + $matches[1]) -Value $matches[2] -Force -ErrorAction SilentlyContinue
        }
    }

Info "provisioning / locating the Windows App SDK + C++/WinRT projection"
$sdkOut = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "provision_winui_sdk.ps1") 2>&1
$wasdk = ($sdkOut | Select-String '^WINAPPSDK=(.+)$').Matches.Groups[1].Value
$generated = ($sdkOut | Select-String '^WINUI_GENERATED=(.+)$').Matches.Groups[1].Value
if (-not $wasdk -or -not $generated) { $sdkOut | ForEach-Object { Write-Host "   $_" }; throw "provision_winui_sdk.ps1 did not report both paths" }

$env:VCPKG_ROOT = "C:\vcpkg"
$cmakeArgs = @(
    "-S", $SourceDir,
    "-B", $BuildDir,
    "-G", "Ninja",
    "-DCMAKE_MAKE_PROGRAM=$ninja",
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DMAUI_EXAMPLES_FRAMEWORK_DIR=$FrameworkDir",
    "-DMAUI_BACKEND=windows",
    "-DVCPKG_TARGET_TRIPLET=$triplet",
    "-DVCPKG_HOST_TRIPLET=$triplet",
    "-DMAUI_TARGET_ABI=$abi",
    "-DMAUI_WINAPPSDK=$wasdk",
    "-DMAUI_WINUI_GENERATED=$generated",
    "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
)
Info ("cmake " + ($cmakeArgs -join " "))
& cmake @cmakeArgs 2>&1 | Select-Object -Last 30
if ($LASTEXITCODE -ne 0) { Info "configure exit: $LASTEXITCODE"; exit $LASTEXITCODE }

$buildArgs = @("--build", $BuildDir, "-j", "$Jobs", "--target") + $Targets
Info ("cmake " + ($buildArgs -join " "))
# Tee to a file, then print the DIAGNOSTIC lines rather than the last N lines of the stream. MSVC emits
# hundreds of lines of template-instantiation notes after a warning, so a trailing window routinely
# scrolls the actual "FAILED:" block off the top and the failure reads as if there were no error at all.
$buildLog = Join-Path $BuildDir "last-build.log"
& cmake @buildArgs 2>&1 | Tee-Object -FilePath $buildLog | Out-Null
$code = $LASTEXITCODE
if ($code -ne 0) {
    Select-String -Path $buildLog -Pattern "FAILED:|error [A-Z]+[0-9]+|fatal error" |
        Select-Object -First 40 | ForEach-Object { $_.Line }
}
Info "build exit: $code"
if ($code -eq 0) {
    foreach ($t in $Targets) {
        $exe = Get-ChildItem -Path $BuildDir -Filter "$t.exe" -Recurse -EA SilentlyContinue | Select-Object -First 1
        if ($exe) { Write-Output ("GALLERY_EXE_" + $t.ToUpper() + "=" + $exe.FullName) }
    }
}
exit $code
