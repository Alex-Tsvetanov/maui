<#
.SYNOPSIS
  Refresh the guest source tree (C:\maui-src) from the read-only UTM share of the host repo (Z:\).

.DESCRIPTION
  WHY THIS EXISTS. C:\maui-src was a scp'd TARBALL, not a checkout, and it went stale silently: on
  2026-08-11 the Windows column rendered a six-day-old tree for a full day and every score in that
  column was read as a port defect. SYNC_STAMP.txt was meant to prevent exactly that and did not,
  because a stamp ASSERTS about the tree instead of MEASURING it -- the file now carries its own
  correction, "VERIFY BY FILE DATE, NOT BY THIS STAMP".

  The UTM read-only share removes the class. Z:\ is not a copy of the host repo, it IS the host repo,
  so "is the guest stale?" becomes mechanically answerable. -Check answers it and writes nothing.

  WHY NOT ROBOCOPY. Robocopy CANNOT PAIR files across this share and silently reports a correct tree
  as totally divergent. Measured against maui-reference\pages, where PowerShell's own enumeration
  proves 196 of 196 source names are already present in the destination:

      robocopy <share> <dest> /MIR /L          ->  196 "New File" AND 197 "*EXTRA File"
      robocopy <share> <dest> /MIR /L /FFT     ->  identical; /FFT does not help recursively
      Compare-Object over Get-ChildItem        ->  common=196, only-in-dest=1 (a stale .bak)

  The single-file form (naming the file explicitly) pairs correctly with /FFT, which is what makes the
  recursive failure so easy to mistake for a real diff. Acting on that output would have purged and
  re-copied all ~2500 files on every run -- forcing a full rebuild each time -- while hiding genuine
  deletions in the noise. SPICE WebDAV does not report NTFS-precision timestamps and robocopy's
  matcher does not cope. Get-ChildItem does, and is cheap: it is BATCHED over WebDAV (718 files
  enumerated in 673 ms) where per-file metadata costs ~100 ms.

  Deletion is deliberate, not tidiness: the CMake projects use file(GLOB ...), so a source file removed
  on the host must disappear here or it keeps getting compiled.

  Only SOURCE subtrees are listed, each mapped explicitly. The guest's build outputs
  (cpp\build-win and cpp\examples\build-win) are never a destination, so they cannot be purged -- which
  a blanket mirror of cpp\ or examples\ WOULD do, since they exist only on the guest.

.EXAMPLE
  powershell -File sync_from_share.ps1 -Check     # stale? exit 1 = yes. Writes nothing.
  powershell -File sync_from_share.ps1            # make it current
#>
[CmdletBinding()]
param(
    [string]$ShareRoot = "Z:\",
    [string]$Dest      = "C:\maui-src",
    [switch]$Check
)

$ErrorActionPreference = "Stop"

# The guest tree is FLATTENED relative to the repo: Z:\port\cpp maps to C:\maui-src\cpp, because the
# build scripts' defaults were written against the tarball layout (build_gallery_windows.ps1
# -SourceDir C:\maui-src\cpp\examples, build_maui_reference.ps1 -SourceDir C:\maui-src\maui-reference\app).
# Keeping the mapping here means those defaults stay correct and nothing else has to move.
#
# Recurse = $false lists only the loose files beside a directory that also holds gigabytes of build
# output, so the walk never descends into it.
$Trees = @(
    @{ From = "port\cpp\src";                   To = "cpp\src" }
    @{ From = "port\cpp\include";               To = "cpp\include" }
    @{ From = "port\cpp\cmake";                 To = "cpp\cmake" }
    @{ From = "port\cpp\tools";                 To = "cpp\tools" }
    @{ From = "port\cpp\examples\gallery";      To = "cpp\examples\gallery" }
    @{ From = "port\cpp\examples\gallery_xaml"; To = "cpp\examples\gallery_xaml" }
    @{ From = "port\cpp\examples\cmake";        To = "cpp\examples\cmake" }
    # bin/obj are MSBuild output -- 3.3 GB of it, and none of it is an input.
    @{ From = "port\maui-reference\app";        To = "maui-reference\app"; Exclude = @("bin", "obj") }
    @{ From = "port\maui-reference\pages";      To = "maui-reference\pages" }
    @{ From = "port\tools";                     To = "tools" }
    @{ From = "port\cpp";                       To = "cpp";             Recurse = $false }
    @{ From = "port\cpp\examples";              To = "cpp\examples";    Recurse = $false }
    @{ From = "port\maui-reference";            To = "maui-reference";  Recurse = $false }
)

if (-not (Test-Path $ShareRoot)) {
    Write-Error "share not mounted at $ShareRoot -- check UTM's Shared Directory and the SPICE guest tools"
    exit 2
}

# WebDAV timestamps are coarser than NTFS, so an exact equality test would copy every file forever.
# Two seconds is the FAT granularity robocopy's own /FFT assumes, for the same underlying reason.
$TOLERANCE = [TimeSpan]::FromSeconds(2)

function Get-Entries {
    param([string]$Root, [bool]$Recurse, [string[]]$Exclude)
    if (-not (Test-Path $Root)) { return @{} }
    $items = if ($Recurse) { Get-ChildItem $Root -File -Recurse -ErrorAction SilentlyContinue }
             else          { Get-ChildItem $Root -File -ErrorAction SilentlyContinue }
    $map = @{}
    $prefix = (Resolve-Path $Root).Path.TrimEnd('\') + '\'
    foreach ($i in $items) {
        $rel = $i.FullName.Substring($prefix.Length)
        if ($Exclude) {
            $parts = $rel.Split('\')
            # Only the DIRECTORY components, so a file literally named "bin" is not mistaken for the
            # bin directory. Guarded on Count -gt 1 because PowerShell wraps negative indices:
            # for a root-level file $parts[0..-1] would span the whole array and match the filename.
            $skip = $false
            if ($parts.Count -gt 1) {
                $dirs = $parts[0..($parts.Count - 2)]
                foreach ($e in $Exclude) { if ($dirs -contains $e) { $skip = $true; break } }
            }
            if ($skip) { continue }
        }
        $map[$rel] = $i
    }
    return $map
}

$toCopy = New-Object System.Collections.ArrayList
$toDrop = New-Object System.Collections.ArrayList
$sw = [Diagnostics.Stopwatch]::StartNew()

foreach ($t in $Trees) {
    $src = Join-Path $ShareRoot $t.From
    $dst = Join-Path $Dest $t.To
    if (-not (Test-Path $src)) { Write-Warning "missing on share, skipped: $src"; continue }
    $recurse = if ($t.ContainsKey('Recurse')) { [bool]$t.Recurse } else { $true }

    $s = Get-Entries -Root $src -Recurse $recurse -Exclude $t.Exclude
    $d = Get-Entries -Root $dst -Recurse $recurse -Exclude $t.Exclude

    foreach ($rel in $s.Keys) {
        $si = $s[$rel]
        $di = $d[$rel]
        if ($null -eq $di -or
            $si.Length -ne $di.Length -or
            [Math]::Abs(($si.LastWriteTimeUtc - $di.LastWriteTimeUtc).Ticks) -gt $TOLERANCE.Ticks) {
            [void]$toCopy.Add(@{ Src = $si.FullName; Dst = (Join-Path $dst $rel) })
        }
    }
    # A non-recursive pass only ever compares the loose files it listed, so it must not propose
    # deleting anything: every subdirectory beneath it is invisible to this pass by design.
    if ($recurse) {
        foreach ($rel in $d.Keys) {
            if (-not $s.ContainsKey($rel)) { [void]$toDrop.Add((Join-Path $dst $rel)) }
        }
    }
}

$sw.Stop()
$scan = [math]::Round($sw.Elapsed.TotalSeconds, 1)

if ($toCopy.Count -eq 0 -and $toDrop.Count -eq 0) {
    Write-Host "guest source tree matches the host share (scanned in $scan s)"
    exit 0
}

if ($Check) {
    Write-Host "STALE - $($toCopy.Count) file(s) to update, $($toDrop.Count) to remove (scanned in $scan s)"
    $toCopy | Select-Object -First 10 | ForEach-Object { Write-Host "  update  $($_.Dst)" }
    if ($toCopy.Count -gt 10) { Write-Host "  ... and $($toCopy.Count - 10) more" }
    $toDrop | Select-Object -First 10 | ForEach-Object { Write-Host "  remove  $_" }
    if ($toDrop.Count -gt 10) { Write-Host "  ... and $($toDrop.Count - 10) more" }
    Write-Host "Run without -Check before capturing, or the Windows column renders old code."
    exit 1
}

$copied = 0
foreach ($c in $toCopy) {
    $dir = Split-Path $c.Dst -Parent
    if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    # Copy-Item goes through File.Copy, which carries the SOURCE's LastWriteTime across. That is what
    # makes the next run cheap -- and what keeps the build incremental, since an unchanged file never
    # gets a fresh mtime and never retriggers a compile.
    Copy-Item -LiteralPath $c.Src -Destination $c.Dst -Force
    $copied++
}
foreach ($p in $toDrop) { Remove-Item -LiteralPath $p -Force -ErrorAction SilentlyContinue }

$total = [math]::Round($sw.Elapsed.TotalSeconds, 1)
Write-Host "synced from host: $copied updated, $($toDrop.Count) removed"
Write-Host "REBUILD before capturing - synced sources are not compiled artifacts."
exit 0
