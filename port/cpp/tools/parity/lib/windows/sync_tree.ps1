<#
.SYNOPSIS
  Mirror a source tree from the read-only host share (Z:\) onto the guest's local disk.

.DESCRIPTION
  Dot-source this and call Sync-Tree. It exists because BOTH Windows build lanes turned out to need a
  local copy, for different reasons, and a second hand-written copy of this logic would drift:

    * MSBUILD CANNOT ENUMERATE THE SHARE. `<ProbeZ Include="Z:\...\**\*.cs" />` matches 1 file where
      Python walking the same tree finds 210. MSBuild's FileMatcher falls back to treating a pattern
      as a literal path when enumeration fails, so the build dies with
      `CS2001: Source file '**/*.cs' could not be found`. No property fixes that.

    * CMAKE CONFIGURE IS FLAKY ON THE SHARE. Configure stats every source named by an
      add_library/add_executable -- ~2000 in a burst -- and a few intermittently fail, which CMake
      reports as "Cannot find source file" naming a file that is demonstrably present:
          run 1 -> src/essentials/main_thread.cpp, tests/controls/indicator_view_tests.cpp
          run 2 -> src/controls/items/items_source_factory.cpp
          run 3 -> src/platform/windows/editor_handler.cpp, src/controls/items/collection_view.cpp
      Different files every time, all present, and a retry did NOT clear it. Sequential checking is
      fine -- a 505-file EXISTS sweep misses none, in both mixed and forward-slash path forms -- so
      it is the burst, not the paths or the tree.

  READING the share is reliable; driving a BUILD off it is not. So the share stays the source of
  truth (it IS the host repo, which is what makes a stale guest tree structurally impossible) and the
  build reads a local mirror refreshed from it on every build.

  Not robocopy: it cannot pair files across this share, reporting a tree whose 196 of 196 names are
  already present as 196 "New File" PLUS 197 "*EXTRA File". Get-ChildItem reports correctly.
#>

function Sync-Tree {
    <#
    .PARAMETER From    source root on the share
    .PARAMETER To      destination root on local disk
    .PARAMETER Exclude directory NAMES pruned at any depth (never entered, so their size is irrelevant)
    .OUTPUTS @(copied, removed, total)
    #>
    param(
        [Parameter(Mandatory)][string]$From,
        [Parameter(Mandatory)][string]$To,
        [string[]]$Exclude = @("bin", "obj", "build", "build-win", "captures", "docs", ".git")
    )

    function Walk-Tree {
        param([string]$Root, [string[]]$Skip)
        $map = @{}
        if (-not (Test-Path $Root)) { return $map }
        $prefix = (Resolve-Path $Root).Path.TrimEnd('\') + '\'
        $stack = New-Object System.Collections.Stack
        $stack.Push((Resolve-Path $Root).Path)
        while ($stack.Count -gt 0) {
            $dir = $stack.Pop()
            # ENUMERATE WITH RETRY, AND FAIL LOUDLY. This used to be a bare
            # `Get-ChildItem -ErrorAction SilentlyContinue`, which swallowed a transient share read
            # failure and returned an EMPTY listing for that directory. An empty SOURCE listing is
            # indistinguishable from "the host deleted these files", so the removal pass below
            # deleted them -- silently, and reported as a normal "N removed".
            # MEASURED 2026-08-19: cpp/examples/gallery_xaml/Views (391 files, the LARGEST single
            # directory in the tree, and so the likeliest to trip the flakiness this file's header
            # already documents for MSBuild and CMake) enumerated as 0 on the share. All 391 were
            # removed from the mirror and none re-copied, and the build died with
            #   C1083: Cannot open include file: 'Views/absolute_layout.xaml.hpp'
            # naming generated headers that were present on the host, and on Z:, the entire time.
            # The header's "READING the share is reliable" is therefore FALSE for burst directory
            # enumeration -- it is reliable when asked once, which is how it was checked.
            # A short retry clears it; three consecutive failures are real and must abort the sync
            # rather than be converted into deletions.
            $items = $null
            for ($try = 1; $try -le 3; $try++) {
                $err = $null
                $got = @(Get-ChildItem -LiteralPath $dir -Force -ErrorAction SilentlyContinue -ErrorVariable err)
                if (-not $err) { $items = $got; break }
                Start-Sleep -Milliseconds (200 * $try)
            }
            if ($null -eq $items) {
                throw "Sync-Tree: cannot enumerate $dir after 3 tries -- ABORTING rather than treating an unreadable directory as an empty one (that would delete its mirror copy)"
            }
            foreach ($i in $items) {
                if ($i.PSIsContainer) {
                    # PRUNE, never filter afterwards. Get-ChildItem -Recurse descends first and
                    # filters its output, which on this tree means walking port/cpp/docs -- 10 GB of
                    # committed board PNGs against ~19 MB of source. That ran 76 minutes once.
                    if ($Skip -notcontains $i.Name -and $i.Name -notlike "build-*") { $stack.Push($i.FullName) }
                }
                else { $map[$i.FullName.Substring($prefix.Length)] = $i }
            }
        }
        return $map
    }

    $s = Walk-Tree -Root $From -Skip $Exclude

    # FIRST COPY ONLY: hand the bulk to robocopy /MT. Its defect on this share is PAIRING an existing
    # destination file with its source, which is irrelevant when the destination is empty -- there is
    # nothing to pair. Copying itself is fine and vastly faster multithreaded: measured 0.044 s/file
    # at /MT:32 against 0.43 s/file for sequential Copy-Item, i.e. ~2.5 min instead of ~25 for this
    # tree. /PURGE is deliberately NOT passed, so this can only ever add files.
    if (-not (Test-Path $To) -or -not (Get-ChildItem -LiteralPath $To -Recurse -File -ErrorAction SilentlyContinue | Select-Object -First 1)) {
        $rc = @($From, $To, "/E", "/MT:32", "/NFL", "/NDL", "/NJH", "/NJS", "/NP", "/R:1", "/W:1")
        foreach ($e in $Exclude) { $rc += @("/XD", $e) }
        $rc += @("/XD", "build-*")
        robocopy @rc | Out-Null
        if ($LASTEXITCODE -ge 8) { throw "robocopy bulk copy failed ($LASTEXITCODE): $From -> $To" }
    }

    $d = Walk-Tree -Root $To -Skip $Exclude
    $copied = 0
    foreach ($rel in $s.Keys) {
        $si = $s[$rel]; $di = $d[$rel]
        # ONE-SIDED, and the file gets a FRESH mtime on arrival. Both halves are required and they are
        # coupled - see the incident note below.
        #
        # 2s tolerance: WebDAV timestamps are coarser than NTFS, so exact equality would copy forever.
        # The comparison is `si NEWER than di`, not `si DIFFERS from di`: a source that is OLDER than its
        # copy is still a source that has not changed, and the symmetric form treated "older" as "differs"
        # only when the sizes happened to match, which is not a distinction worth acting on.
        if ($null -eq $di -or $si.Length -ne $di.Length -or
            ($si.LastWriteTimeUtc - $di.LastWriteTimeUtc).Ticks -gt [TimeSpan]::FromSeconds(2).Ticks) {
            $dst = Join-Path $To $rel
            $dir = Split-Path $dst -Parent
            if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
            Copy-Item -LiteralPath $si.FullName -Destination $dst -Force
            # STAMP IT WITH ARRIVAL TIME. Copy-Item preserves the SOURCE mtime, which this comment used to
            # defend as an optimisation ("an unchanged file never gets a new mtime, so nothing
            # recompiles"). It is also how a FRESH file can land wearing an OLD timestamp and be ignored by
            # ninja for the rest of the day. MEASURED 2026-08-22: xaml_visitors.cpp was edited on the host
            # at 00:28:48 and synced here at 03:02, so it arrived stamped 00:28:48 - OLDER than the
            # 00:36:07 object built from the PREVIOUS content. ninja called the object up to date;
            # `dumpbin /SYMBOLS` on it showed 0 hits for the function the edit added. The whole Windows
            # column then rendered pre-edit code while every artifact-level freshness check passed.
            #
            # The stamp and the one-sided guard above MUST land together: stamping alone would make every
            # destination newer than its source, and a symmetric |si - di| > 2s guard would then re-copy
            # the entire tree on every sync, forever.
            (Get-Item -LiteralPath $dst).LastWriteTimeUtc = [DateTime]::UtcNow
            $copied++
        }
    }
    # Removal is correctness, not tidiness: CMake and MSBuild both glob, so a source deleted on the
    # host but left here keeps getting compiled.
    $removed = 0
    # BACKSTOP for any partial-source failure the retry above does not catch. Deleting a large
    # fraction of the mirror in one pass is never a legitimate incremental sync of a source tree that
    # the host builds from; it is the signature of a source walk that came back short. Abort with the
    # numbers rather than proceed -- a re-run after a real bulk deletion is cheap, and a wrongly
    # emptied mirror costs a whole board run (it cost this one).
    $doomed = @($d.Keys | Where-Object { -not $s.ContainsKey($_) })
    if ($d.Count -gt 0 -and $doomed.Count -gt [Math]::Max(50, [int]($d.Count * 0.25))) {
        throw ("Sync-Tree: refusing to remove " + $doomed.Count + " of " + $d.Count +
               " mirrored files (source walk found " + $s.Count + "). That ratio means the SOURCE walk " +
               "came back short, not that the host deleted them. Re-run; if the host really did delete " +
               "this many, clear $To and let the bulk copy re-seed it.")
    }
    foreach ($rel in $d.Keys) {
        if (-not $s.ContainsKey($rel)) {
            Remove-Item -LiteralPath $d[$rel].FullName -Force -ErrorAction SilentlyContinue
            $removed++
        }
    }
    return @($copied, $removed, $s.Count)
}
