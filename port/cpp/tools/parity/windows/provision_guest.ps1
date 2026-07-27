<#
.SYNOPSIS
  Provision a fresh Windows VM as a maui-port E2E test guest.

.DESCRIPTION
  Run this ONCE on the guest, in an ELEVATED PowerShell. It sets up, in order:

    1. OpenSSH Server        — how the host orchestrator drives the guest (key auth, no password).
    2. PowerShell as the SSH DefaultShell — REQUIRED. run_comparison.py and vm_smoke.py shell-quote
                               remote commands POSIX-style (shlex); cmd.exe passes single quotes
                               through literally, so every remote command would break in a way that
                               looks like the agent misbehaving rather than the shell.
    3. Python 3              — the guest agent needs nothing else (it is ctypes + stdlib only).
    4. Display/UX settings   — disable animations, sleep, screen blanking and the lock screen, so a
                               capture is never of a dimmed/locked screen and window geometry is not
                               mid-animation. This is the Windows equivalent of the macOS lane's
                               "clean WindowServer session" work.
    5. (optional) BUILD TOOLS for the WinUI 3 parity lane: VS Build Tools + Windows SDK + CMake +
                               Ninja, via -WithBuildTools.

  Everything is idempotent: re-running skips what is already in place.

.PARAMETER PublicKey
  The host's SSH public key (contents of ~/.ssh/id_ed25519.pub). Installed into administrators_authorized_keys
  (for an admin user, Windows OpenSSH reads THAT file, not the user's ~/.ssh/authorized_keys — the single
  most common reason key auth silently fails on Windows).

.PARAMETER WithBuildTools
  Also install the MSVC/WinUI 3 build chain. Skip it if you only intend to run the mingw-cross smoke
  app deployed from the dev machine.

.EXAMPLE
  .\provision_guest.ps1 -PublicKey (Get-Content ~/host_key.pub) -WithBuildTools
#>
[CmdletBinding()]
param(
    [string]$PublicKey,
    [switch]$WithBuildTools
)

$ErrorActionPreference = "Stop"

function Info($m) { Write-Host "[provision] $m" -ForegroundColor Cyan }
function Ok($m)   { Write-Host "[provision] OK  $m" -ForegroundColor Green }
function Warn($m) { Write-Host "[provision] !   $m" -ForegroundColor Yellow }

if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
        ).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw "Run this in an ELEVATED PowerShell (Run as Administrator)."
}

# ---------------------------------------------------------------- 1. OpenSSH Server
Info "ensuring OpenSSH Server is installed and running"
$ssh = Get-WindowsCapability -Online -Name "OpenSSH.Server*" | Select-Object -First 1
if ($ssh -and $ssh.State -ne "Installed") {
    Add-WindowsCapability -Online -Name $ssh.Name | Out-Null
    Ok "installed $($ssh.Name)"
} else {
    Ok "OpenSSH Server already present"
}
Set-Service -Name sshd -StartupType Automatic
Start-Service sshd
# TCP/22 inbound. Three distinct failure modes seen in practice, all of which look identical from the
# host (a bare connection timeout on port 22 while RDP still answers, so the network path is clearly
# fine), hence all three are handled:
#   (a) the capability's own rule is absent on some images;
#   (b) it exists but is DISABLED;
#   (c) it exists and is enabled but scoped to Private/Domain only, while a VM's NAT adapter is
#       classified Public — so it never applies. `-Profile Any` is what covers that, and it is stated
#       explicitly rather than relying on New-NetFirewallRule's default.
if (-not (Get-NetFirewallRule -Name "sshd-maui" -ErrorAction SilentlyContinue)) {
    New-NetFirewallRule -Name "sshd-maui" -DisplayName "OpenSSH Server (maui E2E)" `
        -Enabled True -Direction Inbound -Protocol TCP -Action Allow -LocalPort 22 `
        -Profile Any | Out-Null
    Ok "opened TCP/22 (all profiles)"
} else {
    Enable-NetFirewallRule -Name "sshd-maui" -ErrorAction SilentlyContinue
    Ok "TCP/22 rule already present (re-enabled)"
}
# Re-enable any shipped OpenSSH rules that are present but off, so `Get-NetFirewallRule *OpenSSH*`
# does not show a confusing mix of enabled/disabled entries later.
Get-NetFirewallRule -Name "*OpenSSH*" -ErrorAction SilentlyContinue |
    Where-Object { -not $_.Enabled } |
    ForEach-Object { Enable-NetFirewallRule -Name $_.Name -ErrorAction SilentlyContinue }

# Verify sshd is actually LISTENING, not merely "Running": a started service that failed to bind (e.g.
# a stale host-key permission problem) reports Running while port 22 answers nothing, which is
# otherwise indistinguishable from a firewall block.
$listening = Get-NetTCPConnection -LocalPort 22 -State Listen -ErrorAction SilentlyContinue
if ($listening) {
    Ok "sshd is listening on TCP/22"
} else {
    Warn "sshd is NOT listening on TCP/22 — check: Get-Service sshd; Get-EventLog -LogName Application -Source sshd -Newest 20"
}

# ---------------------------------------------------------------- 2. PowerShell as DefaultShell
Info "setting the SSH DefaultShell to PowerShell (required by the host orchestrator)"
$pwsh = (Get-Command powershell.exe).Source          # Windows PowerShell 5.1 — always present
# Create the key first: on some images HKLM:\SOFTWARE\OpenSSH does not exist until sshd has run once,
# and New-ItemProperty against a missing key is a hard error that would abort provisioning here.
if (-not (Test-Path "HKLM:\SOFTWARE\OpenSSH")) { New-Item -Path "HKLM:\SOFTWARE\OpenSSH" -Force | Out-Null }
New-ItemProperty -Path "HKLM:\SOFTWARE\OpenSSH" -Name DefaultShell -Value $pwsh `
    -PropertyType String -Force | Out-Null
Ok "DefaultShell = $pwsh"

# ---------------------------------------------------------------- 2b. authorized key
if ($PublicKey) {
    Info "installing the host public key"
    $key = $PublicKey.Trim()
    # For a member of Administrators, Windows OpenSSH reads ONLY this file (see sshd_config's
    # Match Group administrators block) — putting the key in ~/.ssh/authorized_keys appears to work
    # but authentication still fails.
    $admKeys = "$env:ProgramData\ssh\administrators_authorized_keys"
    # if/else, not `? :` — the ternary is PowerShell 7+ syntax and this script runs under Windows
    # PowerShell 5.1 (the shell we just set as DefaultShell, and the only one guaranteed present).
    $existing = ""
    if (Test-Path $admKeys) { $existing = Get-Content $admKeys -Raw }
    if ($existing -notmatch [regex]::Escape($key)) {
        Add-Content -Path $admKeys -Value $key
    }
    # Permissions MUST be Administrators+SYSTEM only, or sshd ignores the file silently. Grant by SID
    # (*S-1-5-32-544 = Administrators, *S-1-5-18 = SYSTEM) rather than by name: the localized group name
    # differs on a non-English Windows, where `icacls /grant "Administrators:F"` fails and the key file
    # keeps inherited ACLs — so sshd ignores it and auth fails with no useful message.
    icacls $admKeys /inheritance:r /grant "*S-1-5-32-544:F" /grant "*S-1-5-18:F" | Out-Null
    Ok "key installed in administrators_authorized_keys (perms tightened)"
} else {
    Warn "no -PublicKey given; install the host key yourself or SSH will need a password (BatchMode fails)"
}
Restart-Service sshd

# ---------------------------------------------------------------- 3. Python
Info "checking for Python 3"
$py = Get-Command py -ErrorAction SilentlyContinue
if (-not $py) {
    if (Get-Command winget -ErrorAction SilentlyContinue) {
        Info "installing Python via winget"
        winget install --id Python.Python.3.12 --accept-source-agreements --accept-package-agreements `
            --silent | Out-Null
        Ok "Python installed (open a new shell for PATH)"
    } else {
        Warn "no winget and no Python — install Python 3 from python.org, then re-run"
    }
} else {
    Ok "python present: $((& py --version) 2>&1)"
}

# ---------------------------------------------------------------- 4. deterministic UX for captures
Info "making the desktop capture-stable"
# No sleep / no monitor blanking: a blank or dimmed screen is captured as a legitimate-looking frame.
powercfg /change standby-timeout-ac 0
powercfg /change monitor-timeout-ac 0
powercfg /change disk-timeout-ac 0
# Disable window animations + transparency: an animating window reports its final rect before it has
# finished moving, and a translucent title bar composites the desktop behind it into the shot.
Set-ItemProperty -Path "HKCU:\Control Panel\Desktop\WindowMetrics" -Name MinAnimate -Value 0 -Force
Set-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize" `
    -Name EnableTransparency -Value 0 -Force -ErrorAction SilentlyContinue
# Full "visual effects: adjust for best performance" — turns off shadows, fades, smooth-scrolling.
Set-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\VisualEffects" `
    -Name VisualFXSetting -Value 2 -Force -ErrorAction SilentlyContinue
# Never show the lock screen on resume; the runner may reboot the VM between runs.
Set-ItemProperty -Path "HKCU:\Control Panel\Desktop" -Name ScreenSaveActive -Value 0 -Force
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DisableLockWorkstation `
    /t REG_DWORD /d 1 /f | Out-Null
Ok "sleep/blanking off, animations + transparency off, lock screen off"

# Set display scaling to 100%. The agent is PER_MONITOR_AWARE_V2 so it works at any scale, but at 100%
# the guest's logical and physical pixels coincide, which makes hand-checking scenario coordinates
# against a capture trivial instead of an arithmetic exercise.
Info "requesting 100% display scaling (log off/on to fully apply)"
Set-ItemProperty -Path "HKCU:\Control Panel\Desktop" -Name LogPixels -Value 96 -Force `
    -ErrorAction SilentlyContinue
Set-ItemProperty -Path "HKCU:\Control Panel\Desktop" -Name Win8DpiScaling -Value 1 -Force `
    -ErrorAction SilentlyContinue

# ---------------------------------------------------------------- 5. optional: WinUI 3 build chain
if ($WithBuildTools) {
    Info "installing the MSVC / WinUI 3 build chain (this takes a while)"
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        throw "winget is required for -WithBuildTools; install App Installer from the Store, or install VS Build Tools manually."
    }
    # VS Build Tools with the native-desktop workload + the Windows SDK. C++/WinRT support comes from
    # the Microsoft.Windows.CppWinRT NuGet package (restored per-project), not from the installer.
    $vsArgs = @(
        "--add", "Microsoft.VisualStudio.Workload.VCTools",
        "--add", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
        "--add", "Microsoft.VisualStudio.Component.Windows11SDK.22621",
        "--includeRecommended", "--quiet", "--wait", "--norestart"
    ) -join " "
    winget install --id Microsoft.VisualStudio.2022.BuildTools --accept-source-agreements `
        --accept-package-agreements --silent --override $vsArgs | Out-Null
    Ok "VS Build Tools + Windows SDK"

    foreach ($pkg in @("Kitware.CMake", "Ninja-build.Ninja", "Git.Git")) {
        winget install --id $pkg --accept-source-agreements --accept-package-agreements --silent |
            Out-Null
        Ok $pkg
    }
    Warn "open a NEW shell (or reboot) so PATH picks up cmake/ninja before building"
}

# ---------------------------------------------------------------- summary
Write-Host ""
Info "provisioning complete. Verify from the DEV MACHINE with:"
Write-Host "  ssh $env:USERNAME@$env:COMPUTERNAME 'exit 0'  && echo ssh-ok" -ForegroundColor White
Write-Host "  python3 port/cpp/tools/parity/windows/vm_smoke.py --host $env:COMPUTERNAME --user $env:USERNAME" `
    -ForegroundColor White
Write-Host ""
Warn "if you changed display scaling, log off and back on before the first run"
