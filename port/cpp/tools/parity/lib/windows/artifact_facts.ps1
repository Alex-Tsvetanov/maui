<#
.SYNOPSIS
  Report what the guest's BUILD OUTPUT actually is: artifact mtimes, the source dirs that really
  compile into it, and (on demand) whether a named symbol is present in the objects.

.DESCRIPTION
  The Windows lane declares `artifact_remote` for all three columns, so recapture.py's build() skips
  it entirely ("the guest builds its own artifacts") and the guest builds are run BY HAND. Nothing
  between a hand-run build and a three-hour capture asserts that the binaries are newer than the
  source they claim to render. freshness.py asks this script for the facts to make that assertion.

  ASCII ONLY, and invoked as `-ExecutionPolicy Bypass -File`. PowerShell 5.1 on this guest rejects
  non-ASCII .ps1 files, and pushing a `-Command` string through zsh + ssh mangles `$_` into a bare
  `.DirectoryName` (measured: 50s of CommandNotFoundException). Both are project rules, not taste.

.PARAMETER Artifacts
  Comma-separated guest paths to stat. Missing ones are reported with exists=false rather than
  throwing: a DECLARED-BUT-MISSING artifact is a loud skip on this lane, never a silent fallback.

.PARAMETER BuildRoot
  Where the object tree lives. The compiled source dirs are DERIVED from it rather than restated
  here, for the reason examples_configure_args() gives about deriving from CMakeCache: a second
  hand-written copy of CMake's platform selection would drift. Measured 2026-08-22, this reports
  src/platform/headless and src/platform/windows and NOT src/platform/{ios,apple,apple_shared,
  android} -- which is exactly the scoping a mtime guard needs, because an iOS .mm edited after the
  Windows build is not stale, it is irrelevant. Hand-excluding "the other platforms" would ALSO have
  wrongly excluded headless, which does compile in.

.PARAMETER Symbol
  Optional. Instead of the facts, grep the object tree for this symbol via dumpbin /SYMBOLS.
  See Find-Dumpbin for why the location is hard-coded to a deep search.

.PARAMETER ObjectFilter
  Wildcard narrowing the objects -Symbol searches (default *.obj = all 631, roughly two minutes of
  guest CPU). NARROW IT WHENEVER A CAPTURE IS RUNNING: the guest is a single-core-ish VM and load on
  it is what produces splash and half-drawn frames (capture_guard.py's header documents a run that
  lost 18 frames that way). `-ObjectFilter xaml_visitors*` is one dumpbin call.
#>
[CmdletBinding()]
param(
    [string]$Artifacts = "",
    [string]$BuildRoot = "C:/maui-build/examples",
    [string]$Symbol = "",
    [string]$ObjectFilter = "*.obj"
)

$ErrorActionPreference = "Stop"

function Find-Dumpbin {
    # -Depth 12, NOT the obvious -Depth 7. MSVC nests dumpbin about eleven levels below the Visual
    # Studio root (Installer-versioned toolset dir + Host<arch>/<arch>), so a shallower search reports
    # "not installed" for a tool that is present -- which is what turned a one-command symbol check
    # into a 20-minute forensic exercise on 2026-08-22. Prefer the known-good path, then search.
    $known = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostarm64\arm\dumpbin.exe"
    if (Test-Path $known) { return $known }
    $hit = Get-ChildItem -Path "C:\Program Files (x86)\Microsoft Visual Studio" -Filter dumpbin.exe `
                         -Recurse -Depth 12 -File -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $hit) { throw "dumpbin.exe not found under Visual Studio (searched -Depth 12)" }
    return $hit.FullName
}

if ($Symbol) {
    $dumpbin = Find-Dumpbin
    $total = 0
    Get-ChildItem -Path $BuildRoot -Recurse -Filter $ObjectFilter -File -ErrorAction SilentlyContinue | ForEach-Object {
        $n = @(& $dumpbin /SYMBOLS $_.FullName 2>$null | Select-String -SimpleMatch $Symbol).Count
        if ($n -gt 0) {
            $total += $n
            Write-Output ("{0} {1}" -f $n, $_.FullName)
        }
    }
    # ZERO HITS IS NOT PROOF OF A STALE BUILD. A `static` / anonymous-namespace function can be
    # INLINED AWAY in an optimised build and legitimately leave no symbol -- rule that out (give the
    # function external linkage, or check a caller) before concluding the object predates the edit.
    Write-Output ("TOTAL {0} hit(s) for '{1}'" -f $total, $Symbol)
    exit 0
}

$facts = @{}
$facts["now"] = [DateTime]::UtcNow.ToString("o")

$art = @{}
foreach ($a in ($Artifacts -split ',' | Where-Object { $_ })) {
    $p = $a.Trim()
    $i = Get-Item -LiteralPath $p -ErrorAction SilentlyContinue
    if ($i) {
        # MD5 TOO, not just mtime+length. A repeat-measurement ("did the verdict change with the
        # binary held still?") is only as good as its proof that the binary WAS held still, and
        # mtime+length can both survive a rebuild. The android lane pinned its equivalent claim with
        # base.apk md5 endpoints; this is the same control for this lane. Note the commit is NOT that
        # control: _git_commit() runs per unit, so HEAD moves DURING a long pass and a run directory
        # can carry several. A run does not have a commit -- the artifact has a hash.
        $art[$p] = @{ exists = $true; mtime = $i.LastWriteTimeUtc.ToString("o"); length = $i.Length;
                      md5 = (Get-FileHash -LiteralPath $p -Algorithm MD5).Hash }
    } else {
        $art[$p] = @{ exists = $false; mtime = ""; length = 0; md5 = "" }
    }
}
$facts["artifacts"] = $art

$dirs = @{}
Get-ChildItem -Path $BuildRoot -Recurse -Filter *.obj -File -ErrorAction SilentlyContinue | ForEach-Object {
    # <build>/CMakeFiles/<target>.dir/<path-relative-to-that-target's-source-root>/<file>.cpp.obj
    $rel = $_.FullName -replace '.*\.dir\\', ''
    $d = Split-Path $rel -Parent
    if ($d) { $dirs[($d -replace '\\', '/')] = 1 }
}
# Only the framework-rooted ones are a real host path (src/...). The gallery targets' objects are
# rooted at their own examples dir (main.cpp, Views_bytes/*) and Views_bytes is GENERATED, so those
# carry no host counterpart and are deliberately dropped; freshness.py states the two examples dirs
# directly instead. Derive what drifts (which platform compiles), state what cannot.
$facts["source_dirs"] = @($dirs.Keys | Where-Object { $_ -like 'src/*' } | Sort-Object)

ConvertTo-Json $facts -Depth 4 -Compress
