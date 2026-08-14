<#
.SYNOPSIS
  Build the C++ port's libraries for MAUI_BACKEND=windows on the guest (MSVC + Ninja).

.DESCRIPTION
  Run configure_port_windows.ps1 first. This builds LIBRARY targets only by default: the question it
  answers is whether the port's cross-platform sources compile under MSVC, and the test/example targets
  drag in extra surface that is not on that critical path.

  Success here is what makes writing src/platform/windows/* real work rather than blind work - the same
  footing that made the WinUI probe's failures diagnosable instead of mysterious.
#>
[CmdletBinding()]
param(
    [string]$BuildDir = "C:\maui-build\cpp",
    [string[]]$Targets = @("maui_graphics", "maui_core", "maui_controls", "maui_layouts", "maui_hosting"),
    [int]$Jobs = 8
)

$ErrorActionPreference = "Continue"
function Info($m) { Write-Host "[build] $m" -ForegroundColor Cyan }

$ninja = (Get-Command ninja -ErrorAction SilentlyContinue).Source
if (-not $ninja) { $ninja = "C:\Users\Testings-VM\AppData\Local\Microsoft\WinGet\Links\ninja.exe" }

# The MSVC environment must be re-imported: this is a fresh SSH session, and cl.exe needs ~20 variables
# that do not persist. Same arch selection as the configure so the two cannot drift.
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsRoot = (& $vswhere -latest -products * -property installationPath 2>&1 | Select-Object -First 1)
$msvcRoot = Join-Path $vsRoot "VC\Tools\MSVC"
# NOT $targets: PowerShell variable names are case-insensitive, so that would silently overwrite the
# $Targets PARAMETER and cmake would be handed compiler directory names as build targets.
$clTargets = Get-ChildItem $msvcRoot -Recurse -Filter cl.exe -ErrorAction SilentlyContinue |
             ForEach-Object { $_.Directory.Parent.Name + "\" + $_.Directory.Name } | Sort-Object -Unique
$arch = if ($clTargets -contains "Hostarm64\arm64") { "arm64" } else { "arm64_x64" }
Info "vcvarsall $arch"
cmd /c "`"$(Join-Path $vsRoot 'VC\Auxiliary\Build\vcvarsall.bat')`" $arch >nul 2>&1 && set" |
    ForEach-Object {
        if ($_ -match "^([^=]+)=(.*)$") {
            Set-Item -Path ("Env:" + $matches[1]) -Value $matches[2] -Force -ErrorAction SilentlyContinue
        }
    }

# $args is an AUTOMATIC variable in PowerShell; using it for our own list is asking for trouble.
$cmakeArgs = @("--build", $BuildDir, "-j", "$Jobs", "--target") + $Targets
Info ("cmake " + ($cmakeArgs -join " "))
# Tee, then print DIAGNOSTICS (see the gallery script for why a trailing window hides the real error).
$buildLog = Join-Path $BuildDir "last-build.log"
& cmake @cmakeArgs 2>&1 | Tee-Object -FilePath $buildLog | Out-Null
$code = $LASTEXITCODE
if ($code -ne 0) {
    Select-String -Path $buildLog -Pattern "FAILED:|error [A-Z]+[0-9]+|fatal error" |
        Select-Object -First 40 | ForEach-Object { $_.Line }
}
Info "build exit: $code"
exit $code
