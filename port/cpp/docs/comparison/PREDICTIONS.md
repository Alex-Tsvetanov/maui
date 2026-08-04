# Pre-registered predictions — cost of the implementation strategy

**Registered 2026-08-03, before any size, memory or timing number existed in this repository.**

The point of writing these down first is that they are falsifiable. A study whose predictions are
recovered after the numbers are in is advocacy; this file exists so that a wrong prediction stays
visible and gets reported as a wrong prediction. Do not edit a hypothesis after its measurement
lands — add the outcome underneath it.

The control condition that makes any of this a comparison rather than a claim: **~172 interfaces
verified pixel-equivalent across four toolkits and two themes** (`comparison.json`, rendered in
`README.md`). Both implementations drive the same native widgets and produce the same pixels, so a
difference in size, memory or time is attributable to the implementation strategy rather than to one
of them doing less work.

---

## H1 — binary size

> *"Since MAUI bakes .NET into each platform's binary, I expect MAUI's binary is a lot more bloated
> than a C++ equivalent."* — dipl., 2026-08-03

**Prediction:** the managed artifact is materially larger, and the delta is attributable
specifically to the bundled runtime + base class libraries rather than to framework code.

**Sharper, per-platform form** (this is the falsifiable part — a uniform "managed is bigger" is
predictable a priori and worth little): the delta should be **large on the Apple lanes**, where the
managed side bundles an AOT-compiled runtime and the native side links against system frameworks
only, and **much smaller on the Windows lane**, where *both* columns require the same platform
runtime (Windows App SDK / WinUI 3) and only the .NET portion differs.

### Outcome — 2026-08-03: **NOT YET TESTABLE. First measurement contradicts the naive form.**

`tools/measure_size.py` on the Mac Catalyst lane:

| Column | On disk | Main binary stripped | Build config |
| --- | --- | --- | --- |
| `maui_xaml` (managed) | 93.2 MB | 89.5 MB | **Debug** |
| `cpp` (native, code-first) | 56.5 MB | 24.7 MB | **`CMAKE_BUILD_TYPE` unset** — no `-O`, no `NDEBUG` |
| `cpp_xaml` (native, markup) | 96.5 MB | 47.1 MB | **`CMAKE_BUILD_TYPE` unset** |

Three things fall out, none of them the predicted result:

1. **Neither side is built for release.** The managed reference is a Debug build; the native builds
   have an empty `CMAKE_BUILD_TYPE`, i.e. no optimization and no `NDEBUG`. Publishing a ratio from
   these would be a strawman in *both* directions and is the first thing a reviewer would attack.
2. **Symbols dominate the native artifact.** The Catalyst native binary is 58.7 MB on disk, of which
   `__LINKEDIT` is 39.8 MB across 560 611 symbols. An actual `strip -S -x` takes it to 25.3 MB. The
   raw on-disk figure is mostly debug information, not program.
3. **The markup gallery is currently *larger* than the managed reference** (96.5 MB vs 93.2 MB
   unstripped). Whatever the eventual answer, "the native side is obviously smaller" is not what the
   artifacts on disk say today.

**H1 remains open.** It cannot be answered until both sides are rebuilt release-grade: managed with
`-c Release` plus trimming and AOT, native with `CMAKE_BUILD_TYPE=Release` and stripping. Until then
`measure_size.py` marks every record `release_grade: false` and the README refuses to print a
headline ratio. The requirement is recorded here so the eventual number cannot be quoted without it.

### Outcome — 2026-08-03 (second measurement): **release-grade. Directionally CONFIRMED, magnitude far below the naive claim. Control condition NOT yet re-verified.**

Both sides rebuilt release-grade. Native: new `build-{maccatalyst,apple}-release` dirs (the debug dirs
are untouched — the parity board deploys from them) at `CMAKE_BUILD_TYPE=Release`, verified as **828 /
856** `-O2`+`-DNDEBUG` lines in `build.ninja` against **0** in the debug dirs, `otool -l` platform 6
(macCatalyst), then `strip -S -x`. Managed: `dotnet publish -f net10.0-maccatalyst -c Release -r
maccatalyst-arm64`.

| Column | On disk | **Stripped (symmetric)** | Build config |
| --- | --- | --- | --- |
| `maui_xaml` (managed) | 78.7 MB | **47.3 MB** | Release, trimmed + AOT |
| `cpp` (native, code-first) | 9.1 MB | **9.1 MB** | Release, stripped |
| `cpp_xaml` (native, markup) | 22.1 MB | **22.1 MB** | Release, stripped |

**Ratios: 5.2× against the code-first gallery, 2.1× against the markup gallery.** The managed artifact
is materially larger — the prediction's direction holds — but "a lot more bloated" is a strain at 2.1×
against the like-for-like markup twin, which is the fairer comparison since it is the column that also
renders from XAML.

**Read the `Stripped` column, not `On disk`.** The native binaries were stripped in place (a real
release ships stripped), so their on-disk figure is already stripped while the managed one is not —
`On disk` therefore compares stripped against unstripped and overstates the gap.

**The symmetry was nearly a fabricated headline.** The managed main binary carries **33.3 MB of
`__LINKEDIT` across 1 060 792 symbols** and ships with no separate dSYM; `strip -S -x` takes it 65.9 →
33.0 MB. Stripping only the native side — which is what "strip the native release" naturally invites —
gives 78.7 vs 9.1 = **8.6×**, inflating the ratio by 65%. The `__LINKEDIT` trap recorded above for the
native artifact applies just as hard to the managed one.

**Trimming and AOT needed no speculative flags.** Plain `-c Release` did both; evidence from the
OUTPUT rather than the log: assemblies **225 → 88 files, 57.5 → 6.2 MB** (IL stripped) and the AOT code
moved *into* the executable (**10.4 → 65.9 MB**). The SDK's own evaluated defaults already differ by
configuration — Debug is `UseInterpreter=True`, `TrimMode=copy`, `MtouchLink=None`; Release is
`TrimMode=partial`. `PublishTrimmed` / `RunAOTCompilation` / `PublishAot` evaluate **empty** in both,
because the iOS/Catalyst targets set them during publish; passing them blind would have been
unverifiable either way. **Properties used: `-f net10.0-maccatalyst -c Release -r maccatalyst-arm64
-p:ValidateXcodeVersion=false`** (the last is a pre-existing host-Xcode-mismatch workaround, not a size
knob).

**Decomposition — the actual finding, and it is only partial.** Of the 15.0 MB of AOT metadata +
managed assemblies:

| Component | AOT data | Assemblies | Total |
| --- | --- | --- | --- |
| runtime + BCL | 6.23 MB | 2.30 MB | **8.53 MB** |
| framework (MAUI) | 2.48 MB | 3.48 MB | **5.96 MB** |
| app code | 0.32 MB | 0.17 MB | **0.49 MB** |

Runtime + BCL does lead, as predicted — but framework code is **40%** of the attributable bytes, not a
rounding error, so the sharper claim "attributable *specifically* to the runtime + BCL rather than to
framework code" is only half-supported.

**Honest limit:** that table covers 15.0 MB. The dominant component is the **31.5 MB stripped AOT image**,
a single linked Mach-O, and it is **not attributed**. Its 55 `mono_aot_module_*` markers are
data-segment structs rather than code boundaries, so splitting it per assembly needs a Mono AOT format
parse. Scaling by the `.aotdata` proportions (69 / 27 / 4) would suggest ≈21.7 MB runtime+BCL, ≈8.5 MB
framework, ≈1.1 MB app — recorded as a **proxy estimate, not a measurement**.

**Windows remains unmeasured** (`remote_only`): both columns build on the guest, and the host-side
`artifact` key is a source path. Not measured rather than measured wrongly.

**CONTROL CONDITION RE-VERIFIED — parity survives the optimizing build.** 10 pages × 2 themes × 3
columns captured from the *release* binaries and compared directly against the committed board (no
import: importing would have overwritten the board's frames with release-binary captures, a silent
rebaseline).

**54 of 54 comparable frames are byte-identical — exactly 0.000% differing. Zero frames moved.** The
remaining 4 of 58 are `shapes_demo`, which is not a board page and therefore has no committed
reference; that was a page-selection mistake on my part, not a build difference.

So `-O2`/`-DNDEBUG` and `strip -S -x` change nothing observable in the render, and the size comparison
above is between artifacts that are confirmed to draw the same pixels. That is what licenses
attributing the delta to implementation strategy rather than to a behavioural difference.

**Harness note worth keeping.** The first control attempt captured 4 frames and dropped 58, evenly
across ALL THREE columns (`cpp` 18, `cpp_xaml` 20, `maui_xaml` 20) with zero launch failures — the
managed reference failing identically is what identified it as infrastructure rather than a broken
native build. Cause: `reboot_before_run` had been disabled to dodge an earlier 1h34m hang in the
runner's own reboot-and-wait, and that setting is load-bearing — its comment predicts exactly this
("a confused WindowServer … opens app windows with bogus geometry that aren't AX-enumerable").
Rebooting the guest MANUALLY (back in ~30s, versus the runner's hang) and waiting for load to settle
gives the clean WindowServer without the hang: 63 frames, 0 dropped.

---

## H2a — resident memory

> *"Since MAUI is using C# and therefore garbage collector, I expect the C++ port to be much lighter
> in runtime RAM (because the objects are released the moment they stop being needed)."*
> — dipl., 2026-08-03

**Prediction:** lower resident set size for the native implementation, at rest and at peak.

**Stated confound, registered in advance:** a collected heap retains pages because it has not
*bothered* to release them, not because it needs them. A lower native RSS therefore partly measures
heap policy rather than object lifetime, and the result must be reported as such.

**Known limitation of the instrument:** the board relaunches the app for every (page, theme, column)
— the page is selected by an environment variable at launch. That is excellent for comparability
(every sample is a cold process, so nothing depends on page order) but it means the harness
**cannot** observe memory being reclaimed after navigating *away* from a page, which is where prompt
destruction would show most clearly. H2a as measured here is about footprint, not about reclamation.

### Outcome — pending.

---

## H2b — responsiveness

> *"…and much more responsive in time (because there is no garbage collector trying to sneak some
> runtime for itself)."* — dipl., 2026-08-03

This is registered as **three separate predictions**, because they will not behave alike.

**H2b-1 — cold start to first frame: large native advantage.** Runtime initialization, assembly
loading and AOT image mapping have no counterpart on the native side. Expected to be the largest and
most robust timing difference in the study.

**H2b-2 — layout (measure/arrange) on a deep tree: measurable native advantage.** This is the one
place where a *framework-attributable* difference must appear if the architecture means anything:
layout is computed in the portable layer, not delegated to the native toolkit, so this compares this
implementation's code against the reference's with the platform held constant.

**H2b-3 — steady-state frame time on a static interface: NO SIGNIFICANT DIFFERENCE.**

This last one is a deliberate prediction *against* the motivating intuition, registered by the
assistant on 2026-08-03 and accepted for registration by the dipl. Reasoning: both implementations
drive the same native widgets, so most of a frame is platform layout, rasterization and compositing
— identical code in both cases. The managed side is AOT-compiled on mobile, so there is no
steady-state JIT cost, and generational collection on a small heap typically costs a fraction of a
16.7 ms budget.

**If H2b-3 holds, it is reported as a positive finding, not buried.** A negative result contradicting
"managed means janky" is more informative than confirming the predictable, and it is what makes the
positive results in H1 and H2b-1 credible. A study in which every prediction conveniently held reads
as advocacy.

### Outcome — 2026-08-04: **H2b-1 CONFIRMED. Android 3.1x, iOS 1.7x. Three lanes unmeasurable.**

Measured by the recapture runner's TTFF instrument, 10 cold starts per column, same page.
The instrument implements the definition registered above verbatim: launch -> first captured
frame that is not the launch/blank screen; **window-exists deliberately not used**.

| lane | reference | code-first | from-markup | poll resolution |
| --- | --- | --- | --- | --- |
| Android | 1.442 s (p95 1.714) | **1.016 s** (1.252) | 1.060 s (1.184) | 0.239 s |
| iOS | 1.990 s (p95 2.259) | **1.148 s** (1.237) | 1.210 s (1.217) | 0.245 s |

Ratios **1.42x (Android)** and **1.73x (iOS)**, from run `2026-08-04-204818` — the last complete
pass, not the most favourable one. An earlier pass the same day reported Android at 3.1x with the
same code; the difference is emulator state, not the system. Recorded because it says something
about the quantity: cold start on an emulated device is sensitive to conditions outside the program
being measured, so a single run is weak evidence and the ratio should be re-measured before it is
leaned on.

**Direction and magnitude both hold** — this is the one cost hypothesis that came out as
predicted, unlike H1 whose magnitude was overstated. The two input paths of the system start
alike, which is what compile-time parsing predicts.

**Caveat that must travel with the number.** Poll resolution (~0.24 s) bounds precision. The gap
between the system's two input paths (0.044 s Android, 0.062 s iOS) is **below** the resolution and
is therefore NOT resolvable — reported as "alike", not as a measured small difference. The gap
against the reference (0.43 s and 0.84 s) is several times the resolution and is resolvable. iOS
lost 1 of 10 samples on both port columns.

**Three lanes report `measured: false` with a reason** rather than silently absent: Mac
Catalyst, AppKit and Windows all capture through a VM, and the round trip (~1-3 s) is the same
order as the signal (~1 s). An instrument measuring mostly its own latency is not a
measurement. Needs a guest-side timer.

**H2a, H2b-2 and H2b-3 remain unmeasured.** Confirming H2b-1 is *not* evidence for H2b-3 —
cold start and steady state measure different things, which is exactly why they were
registered separately.


---

## Validity requirements (agreed before measuring)

These are the attack surface; each is a field in the measurement record, not an assumption.

* **Build configuration parity.** Release on both sides, trimming and AOT enabled on the managed
  side. Recorded per artifact as `build_config` / `release_grade`; a comparison with a false on
  either side is not presented as a result.
* **Same device, same OS build, controlled thermal state**, many repetitions, distributions reported
  rather than single numbers.
* **Frame time reported at the 95th and 99th percentile**, never as a mean — jank is a tail
  phenomenon and a mean hides it.
* **Cold start distinguished from warm.**
* **Startup defined framework-agnostically** as launch → first captured frame that is not the
  launch/blank screen, using the capture path the parity board already relies on. Window-exists is
  *not* used as the signal: a window can precede first content paint by a long way on the managed
  side and barely at all on the native side, which would systematically flatter one column. The
  polling resolution is stated with the number rather than implying millisecond precision.
* **Lanes that cannot be measured are reported as unmeasured.** The Windows columns build on the
  guest (`artifact_remote`); the host-side `artifact` key there points at a *source* directory.
  Measuring it yielded 1170.9 MB for the managed column against 2.2 MB for the native one — a
  fabricated 500× ratio that would have gone into the README as the study's headline. `measure_size.py`
  now records those lanes as `remote_only` and measures nothing.
