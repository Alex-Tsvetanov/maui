<#
.SYNOPSIS
  Build the MAUI reference app (WinUI 3) on the Windows E2E guest.

.DESCRIPTION
  This produces the GROUND TRUTH column for Windows parity. MAUI's Windows backend is WinUI 3
  (Microsoft.UI.Xaml), which cannot be cross-built from macOS, so the reference has to be built here.

  The app is the same port/maui-reference/app project the iOS / Mac Catalyst / Android lanes use, with a
  Windows TFM appended by an IsOSPlatform('windows') condition, so this adds a column without forking
  the reference app.

  Built UNPACKAGED (WindowsPackageType=None, set in the csproj): the E2E runner deploys a folder and
  launches an .exe, exactly as the macOS lane copies a .app. MSIX would need an install step per run and
  could not be swapped by copying files.

.PARAMETER SourceDir
  port/maui-reference/app ON THE READ-ONLY HOST SHARE. Z:\ is the host repo itself rather than a copy,
  so this column cannot render stale code -- the failure that went unnoticed for a full day on
  2026-08-11 while SYNC_STAMP.txt claimed the tree was current.

.PARAMETER OutputRoot
  Where bin/ and obj/ go. UNLIKE the CMake lanes, MSBuild is NOT out-of-source: it writes obj/ and bin/
  beside the project by default, which a read-only source makes impossible. BaseIntermediateOutputPath
  and BaseOutputPath move both, and are passed to RESTORE as well as BUILD -- restore writes
  project.assets.json and the generated .nuget.g.props/.targets into obj/, and if the two commands
  disagree about where obj/ is, the build fails looking for assets restore already wrote elsewhere.
  Both MUST keep their trailing backslash; MSBuild concatenates these as string prefixes.
  Deliberately NOT `dotnet --artifacts-path`, which would also work but renames the leaf directory to
  <Config>_<tfm>_<rid> -- windows.toml pins a static artifact path, and this keeps the familiar
  bin/<Config>/<tfm>/<rid>/ layout it already encodes.

.PARAMETER Configuration
  Debug (default) or Release. Debug matches what the other lanes capture.

.PARAMETER Tfm
  Windows target framework moniker; must match the csproj.

.EXAMPLE
  .\build_maui_reference.ps1
#>
[CmdletBinding()]
param(
    [string]$SourceDir = "Z:\port\maui-reference\app",
    [string]$OutputRoot = "C:\maui-build\maui-reference",
    [string]$Configuration = "Debug",
    [string]$Tfm = "net10.0-windows10.0.19041.0"
)

$ErrorActionPreference = "Stop"
# Relaxed for the native dotnet invocations below: with "Stop", ANY stderr write from an
# external exe becomes a terminating NativeCommandError, and dotnet restore/build both write
# progress there. Success is judged by $LASTEXITCODE, the only reliable signal.
$nativeEAP = "Continue"
function Info($m) { Write-Host "[maui-ref] $m" -ForegroundColor Cyan }
function Ok($m)   { Write-Host "[maui-ref] OK  $m" -ForegroundColor Green }
function Warn($m) { Write-Host "[maui-ref] !   $m" -ForegroundColor Yellow }

if (-not (Test-Path $SourceDir)) { throw "SourceDir not found: $SourceDir (is the Z: share mounted?)" }

# --- stage the project onto C: -------------------------------------------------------------------
# MSBUILD CANNOT ENUMERATE THE SHARE RECURSIVELY, so unlike the CMake lanes this one cannot build in
# place. Measured with an otherwise identical probe project:
#
#     <ProbeZ Include="Z:\port\maui-reference\app\**\*.cs" />   ->  1 match
#     <ProbeC Include="C:\maui-build\cpp\**\*.txt" />           ->  8 matches
#
# while Python walking the same Z: tree finds 239 files, 210 of them .cs (4 top-level, 206 nested).
# MSBuild's FileMatcher falls back to treating a pattern as a LITERAL path when enumeration fails,
# which is why the build died with `CS2001: Source file '**/*.cs' could not be found` -- the glob
# reached csc verbatim. No MSBuild property fixes that; the enumeration itself is what fails.
# CMake is unaffected and globs the share correctly (it resolves all 505 .cpp under src/), so ONLY
# this lane stages.
#
# The staging is part of the BUILD, deliberately, not a step someone remembers to run. A separate
# manual sync is exactly what failed on 2026-08-11: the tree drifted six days and the column rendered
# old code for a full day while SYNC_STAMP.txt claimed otherwise.
#
# Not robocopy: it cannot pair files across this share at all, reporting a tree whose 196 of 196 names
# are already present as 196 "New File" plus 197 "*EXTRA File". Get-ChildItem reports correctly.
#
# THE WHOLE maui-reference DIRECTORY IS STAGED, not just app/. Three things outside the project dir
# are load-bearing, and staging app/ alone silently produced a project that compiled no XAML at all:
#   * ../pages/*.xaml -- THE canonical shared pages (ruling 6), pulled in by the csproj as
#     `<MauiXaml Include="..\pages\*.xaml" Link="Pages\%(Filename).xaml" />`. With app/ alone that
#     glob resolved to a directory that did not exist, so every page silently produced no generated
#     partial and the build died with ~200 x CS0103 "InitializeComponent does not exist" -- an error
#     that names the code-behind and says nothing about the missing XAML.
#   * ../Directory.Build.props|targets -- deliberately EMPTY isolation stubs whose entire job is to
#     stop MSBuild's upward traversal from reaching the dotnet/maui repo's arcade infrastructure.
#     Omitting them does not fail loudly; it changes what the reference app builds against.
#   * ../NuGet.config -- the package sources for restore.
# captures/ is excluded: 151 MB of PNGs that are output, not input.
function Sync-Tree {
    param([string]$From, [string]$To)
    $exclude = @("bin", "obj", "captures")
    function Walk([string]$Root) {
        $map = @{}
        if (-not (Test-Path $Root)) { return $map }
        $prefix = (Resolve-Path $Root).Path.TrimEnd('\') + '\'
        $stack = New-Object System.Collections.Stack
        $stack.Push((Resolve-Path $Root).Path)
        while ($stack.Count -gt 0) {
            $dir = $stack.Pop()
            foreach ($i in Get-ChildItem -LiteralPath $dir -Force -ErrorAction SilentlyContinue) {
                # Prune, never filter afterwards: descending into bin/obj would walk MSBuild output
                # that is orders of magnitude larger than the sources.
                if ($i.PSIsContainer) {
                    if ($exclude -notcontains $i.Name) { $stack.Push($i.FullName) }
                }
                else { $map[$i.FullName.Substring($prefix.Length)] = $i }
            }
        }
        return $map
    }
    $s = Walk $From
    $d = Walk $To
    $copied = 0
    foreach ($rel in $s.Keys) {
        $si = $s[$rel]; $di = $d[$rel]
        # 2s tolerance: WebDAV timestamps are coarser than NTFS, so exact equality would copy forever.
        if ($null -eq $di -or $si.Length -ne $di.Length -or
            [Math]::Abs(($si.LastWriteTimeUtc - $di.LastWriteTimeUtc).Ticks) -gt [TimeSpan]::FromSeconds(2).Ticks) {
            $dst = Join-Path $To $rel
            $dir = Split-Path $dst -Parent
            if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
            # File.Copy carries the source timestamp over, which keeps the NEXT run cheap and keeps
            # MSBuild incremental -- an unchanged file never gets a new mtime, so nothing recompiles.
            Copy-Item -LiteralPath $si.FullName -Destination $dst -Force
            $copied++
        }
    }
    # Removal is not tidiness: MSBuild's default item globs are recursive, so a .cs deleted on the
    # host but left here would still be compiled.
    $removed = 0
    foreach ($rel in $d.Keys) {
        if (-not $s.ContainsKey($rel)) { Remove-Item -LiteralPath $d[$rel].FullName -Force -ErrorAction SilentlyContinue; $removed++ }
    }
    return @($copied, $removed, $s.Count)
}

$stageSrc = Split-Path $SourceDir -Parent          # ...\maui-reference  (app/ AND pages/ AND the stubs)
$stageDst = Join-Path $OutputRoot "src"
$sw = [Diagnostics.Stopwatch]::StartNew()
$r = Sync-Tree -From $stageSrc -To $stageDst
$sw.Stop()
# Rebase onto the stage: everything downstream (csproj lookup, restore, build) uses the LOCAL copy.
$SourceDir = Join-Path $stageDst (Split-Path $SourceDir -Leaf)
Info ("staged {0} -> {1}: {2} of {3} file(s) updated, {4} removed ({5:N1}s)" -f `
      $stageSrc, $stageDst, $r[0], $r[2], $r[1], $sw.Elapsed.TotalSeconds)
Info "project root: $SourceDir"

$dotnet = "C:\dotnet\dotnet.exe"
if (-not (Test-Path $dotnet)) {
    $cmd = Get-Command dotnet -ErrorAction SilentlyContinue
    if (-not $cmd) { throw "dotnet not found; run provision_dotnet.ps1 first" }
    $dotnet = $cmd.Source
}
$env:DOTNET_CLI_TELEMETRY_OPTOUT = "1"
# Deterministic, quiet-ish output the host can parse; NuGet restore noise otherwise dominates the log.
$env:DOTNET_NOLOGO = "1"

$csproj = Get-ChildItem -Path $SourceDir -Filter *.csproj | Select-Object -First 1
if (-not $csproj) { throw "no .csproj under $SourceDir" }
Info "project: $($csproj.FullName)"
$ErrorActionPreference = $nativeEAP
Info "sdk    : $((& $dotnet --version 2>&1) -join ' ')"

# The RID must be explicit. MAUI's Windows default is win10-x64, which on an ARM64 guest would build an
# x64 app that then runs under emulation - a difference we do NOT want in the reference column, since the
# whole point is to capture what MAUI natively renders on this machine.
$rid = if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") { "win-arm64" } else { "win-x64" }
Info "rid    : $rid ($Configuration, $Tfm)"

# Trailing backslashes are load-bearing: MSBuild treats these as string prefixes, so without them the
# output lands in a sibling named e.g. "maui-referencebin". Passed to BOTH commands -- see .PARAMETER
# OutputRoot for why splitting them breaks restore.
$objDir = (Join-Path $OutputRoot "obj") + "\"
$binDir = (Join-Path $OutputRoot "bin") + "\"
$outArgs = @("-p:BaseIntermediateOutputPath=$objDir", "-p:BaseOutputPath=$binDir")
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
Info "obj    : $objDir"
Info "bin    : $binDir"

Info "restoring (TargetFrameworks pinned to $Tfm so the mobile TFMs are never evaluated)"
$ErrorActionPreference = $nativeEAP
& $dotnet restore $csproj.FullName -p:TargetFrameworks=$Tfm -r $rid @outArgs 2>&1 |
    Select-Object -Last 8
if ($LASTEXITCODE -ne 0) { throw "restore failed with exit code $LASTEXITCODE" }

Info "building (first build pulls the Windows App SDK; several minutes)"
& $dotnet build $csproj.FullName -p:TargetFrameworks=$Tfm -f $Tfm -c $Configuration -r $rid `
    @outArgs --no-restore --nologo -v minimal 2>&1 | Select-Object -Last 30
if ($LASTEXITCODE -ne 0) { throw "build failed with exit code $LASTEXITCODE" }

# Locate the produced exe. The layout is bin/<Config>/<tfm>/<rid>/<AppName>.exe, but the RID folder name
# has changed across SDK bands (win10-arm64 vs win-arm64), so search rather than assume.
# Rooted at $OutputRoot, NOT $SourceDir -- the share is read-only and nothing is produced there.
$binRoot = Join-Path $binDir "$Configuration\$Tfm"
$exe = Get-ChildItem -Path $binRoot -Filter "MauiReference.exe" -Recurse -ErrorAction SilentlyContinue |
       Select-Object -First 1
if (-not $exe) { throw "build reported success but MauiReference.exe was not found under $binRoot" }

Ok "exe: $($exe.FullName)"
Ok "dir: $($exe.Directory.FullName)"
Write-Host ""
Info "deploy that DIRECTORY as the maui_xaml column artifact; process = MauiReference.exe"
# Emit one machine-readable line the host can grep, so the host does not have to guess the RID folder.
Write-Output ("MAUI_REF_EXE=" + $exe.FullName)
Write-Output ("MAUI_REF_DIR=" + $exe.Directory.FullName)
