<#
.SYNOPSIS
  Add the native ARM64 MSVC toolset to the guest's VS Build Tools install.

.DESCRIPTION
  The guest is ARM64 Windows 11, but the Build Tools install only shipped the cross compilers
  Hostarm64\x64, Hostarm64\x86 and Hostarm64\arm (ARM32 - the component named "VC.Tools.ARM", which is
  the one people tick by mistake). Without Hostarm64\arm64 every build is x64-under-emulation, and the
  MAUI reference column on this machine is native arm64 - so a cross-ABI parity score would be
  comparing two different binaries' rendering, not the port against MAUI.

  Two component IDs, both required for a native lane:
    Microsoft.VisualStudio.Component.VC.Tools.ARM64     - the arm64 target toolset
    Microsoft.VisualStudio.Component.VC.Tools.ARM64EC   - the arm64ec variant (WinUI/WindowsAppSDK
                                                          packages ship arm64ec-flavored libs)

  WHY THIS IS A SCRIPT AND NOT AN SSH ONE-LINER: setup.exe's argument parsing is fragile through
  Start-Process -ArgumentList (an ARRAY gets re-quoted by PowerShell 5.1 and setup.exe answers 87,
  ERROR_INVALID_PARAMETER, having seen none of the flags - its own telemetry log then reads
  "vs.willow.quiet : False"). Passing ONE pre-quoted argument string is what it actually accepts, and
  writing it here removes the SSH + PowerShell quoting layers that mangled it.

.PARAMETER InstallPath
  The VS instance to modify. Defaults to the Build Tools install this lane provisions.

.PARAMETER Wait
  Block until the installer finishes (default). -Wait:$false returns as soon as it is launched.
#>
[CmdletBinding()]
param(
    [string]$InstallPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools",
    [switch]$Wait = $true
)

$ErrorActionPreference = "Stop"
function Info($m) { Write-Host "[arm64] $m" -ForegroundColor Cyan }
function Ok($m)   { Write-Host "[arm64] OK  $m" -ForegroundColor Green }

$setup = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\setup.exe"
if (-not (Test-Path $setup)) { throw "VS setup.exe not found at $setup" }

function Get-TargetCompilers {
    $root = Join-Path $InstallPath "VC\Tools\MSVC"
    if (-not (Test-Path $root)) { return @() }
    Get-ChildItem $root -Recurse -Filter cl.exe -ErrorAction SilentlyContinue |
        ForEach-Object { $_.Directory.Parent.Name + "\" + $_.Directory.Name } | Sort-Object -Unique
}

$before = Get-TargetCompilers
Info ("compilers before: " + ($before -join ", "))
if ($before -contains "Hostarm64\arm64") { Ok "Hostarm64\arm64 already present - nothing to do"; exit 0 }

# ONE string, quoted by hand. See the .DESCRIPTION note: an ArgumentList array does not survive.
# NO --wait here: that option belongs to vs_installer.exe, and setup.exe answers 87 with
# "Option 'wait' is unknown". setup.exe hands the work to a detached elevated child and returns
# immediately either way, so completion is detected by POLLING for the toolset, below.
$argline = '--installPath "' + $InstallPath + '"' +
           ' --add Microsoft.VisualStudio.Component.VC.Tools.ARM64' +
           ' --add Microsoft.VisualStudio.Component.VC.Tools.ARM64EC' +
           ' --quiet --norestart'
Info "setup.exe modify $argline"

# -Wait, NOT a manual .WaitForExit(): with the latter PowerShell never caches the process handle and
# .ExitCode reads back EMPTY, so the check below rejects a run that actually succeeded.
$p = Start-Process -FilePath $setup -ArgumentList ("modify " + $argline) -Wait -PassThru -NoNewWindow
# 3010 is "success, reboot required" - a normal outcome for a toolset add and NOT a failure.
if ($p.ExitCode -ne 0 -and $p.ExitCode -ne 3010) {
    throw "setup.exe modify failed with exit code $($p.ExitCode) - see the newest %TEMP%\dd_installer_*.log, which names the offending option (87 = a flag setup.exe does not accept; --wait is vs_installer.exe's, not setup.exe's)"
}
if (-not $Wait) { Info "launched; poll for Hostarm64\arm64 yourself"; exit 0 }

Info "waiting for the install to land (download + install of the arm64 toolset; up to 30 min)"
$deadline = (Get-Date).AddMinutes(30)
while ((Get-Date) -lt $deadline) {
    if ((Get-TargetCompilers) -contains "Hostarm64\arm64") { break }
    Start-Sleep -Seconds 20
}
$after = Get-TargetCompilers
Info ("compilers after : " + ($after -join ", "))
if ($after -notcontains "Hostarm64\arm64") {
    throw "timed out: Hostarm64\arm64 still absent - check the newest %TEMP%\dd_installer_*.log"
}
Ok "Hostarm64\arm64 installed - reconfigure with configure_port_windows.ps1 to pick up the native ABI"
