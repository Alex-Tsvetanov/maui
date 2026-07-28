# Windows toolchain — investigation, decision, and the VM runbook

Status: **pipeline built and verified off-guest; awaiting a live Windows VM.** Branch `windows-toolchain`.

This documents how the C++23 MAUI port builds Windows binaries and how they get deployed to and driven
on a Windows VM — the sibling of the macOS UTM lane (`docs/comparison/tools/README_e2e.md`).

---

## 1. The finding that decides everything: MAUI on Windows is WinUI 3

Read from the oracle (`src/`, per `port/CLAUDE.md`), not assumed:

| Control | MAUI's Windows platform view | Source |
|---|---|---|
| `Button` | `Microsoft.UI.Xaml.Controls.Button` (`MauiButton`) | `src/Core/src/Handlers/Button/ButtonHandler.Windows.cs` |
| `Label`  | `Microsoft.UI.Xaml.Controls.TextBlock` | `src/Core/src/Handlers/Label/LabelHandler.Windows.cs` |

The namespace is **`Microsoft.UI.Xaml`** — that is WinUI 3 / Windows App SDK, *not* UWP's system XAML
(`Windows.UI.Xaml`) and *not* Win32 common controls.

**Consequence:** only a WinUI 3 render can be visually compared against MAUI. Any other widget set
fails parity *by construction*, no matter how much it is polished.

This is not a theoretical worry — the project has already paid for it once. The Android `SearchBar` was
implemented with framework compound drawables on a plain `EditText` instead of the real AppCompat
`SearchView`, and its parity page is **permanently yellow**: the magnifier glyph, text inset and
underline can never match because the widget is not the reference's widget. Choosing Win32/GDI for
Windows would be that same decision applied to an entire platform, so it is explicitly rejected below.

---

## 2. Two lanes, deliberately separated

| | **Lane 1 — `windows` (parity)** | **Lane 2 — `windows-mingw` (pipeline/core)** |
|---|---|---|
| Widgets | WinUI 3 (`Microsoft.UI.Xaml`) via C++/WinRT | Win32 (`user32`/`gdi32`) |
| Compiler | MSVC `cl.exe` | mingw-w64 GCC 15.2 |
| Builds on | **the Windows guest** | **the macOS dev machine** (cross) |
| Needs | VS Build Tools, Windows SDK, Windows App SDK, cppwinrt | `brew install mingw-w64` — nothing else |
| Visually comparable to MAUI | **yes — this is the only comparable lane** | **no. never.** |
| Purpose | the real Windows backend | validate deploy→launch→capture; build/test the cross-platform core on Windows |

Lane 2 exists because it is the only way to have a *deployable Windows binary today* and therefore to
debug the VM pipeline before a single line of WinUI backend exists. It is labelled as non-comparable in
the source, the config, and the toolchain file so it cannot quietly become "the Windows backend".

Verified on this machine (macOS arm64 → Windows x64):

```
$ tools/parity/windows/build_smoke.sh
[smoke] built …/build/windows-smoke/maui_smoke.exe
…/maui_smoke.exe: PE32+ executable (GUI) x86-64, for MS Windows      # real PE, 0 warnings
```

mingw-w64 GCC 15.2 accepts `-std=c++23` and the library features the port uses (`<expected>`,
`<format>`, …). Three findings worth keeping:

1. **`std::print` does not link** — mingw's libstdc++ lacks `__open_terminal`/`__write_to_terminal`. Use
   `std::format` + `fputs`.
2. **`-static` is required, not `-static-libstdc++ -static-libgcc`.** Verified with `objdump -p` on a
   real binary: the latter still leaves **`libwinpthread-1.dll`** as a dynamic import (GCC's threading
   model links it separately). A stock guest has no mingw DLLs, so the app would have died at launch
   behind a modal dialog — which the runner reports only as "process exited early". `build_smoke.sh` now
   **gates** on the import list (any `lib*.dll` fails the build); only the UCRT `api-ms-win-crt-*.dll`
   (ships with Windows 10+) and the system DLLs may remain.
3. **Do not pass `-municode`/`-mwindows` via `-DCMAKE_EXE_LINKER_FLAGS=…`.** It replaces the toolchain's
   `_INIT` (losing `-static`) *and* breaks CMake's compiler ABI check, whose probe has a plain `main()`
   and cannot link a wide GUI entry point. Use `target_link_options(<tgt> PRIVATE -municode -mwindows)`.
   The toolchain file was verified end-to-end against a scratch CMake project (configure → build →
   `PE32+`), which is how findings 2 and 3 surfaced.

---

## 2b. The core cross-build found two real portability bugs

`tools/parity/windows/build_core_check.sh` cross-compiles the port's **platform-independent half**
(graphics, core, controls, layouts, hosting + the headless handler mirrors — 347 sources) for Windows and
links `core_probe.cpp` against it. `--syntax-only` is the fast portability gate; the full run archives
`libmaui_core_win.a` and produces a **runnable, self-checking** `maui_core_probe.exe`.

It immediately earned its keep. Two defects that block **any** non-libc++ toolchain — i.e. they would
equally have blocked the MSVC build that Lane 1 (WinUI 3) requires — were found and fixed:

1. **`src/core/date_time.cpp` included `<__chrono/duration.h>`** — a **libc++ private** header, added
   only to satisfy clang-tidy's include-cleaner (`<chrono>`, the portable header, was already included
   two lines below). `<__chrono/…>` does not exist in libstdc++ or MSVC STL, so the TU was
   uncompilable anywhere else. Now guarded by `#ifdef _LIBCPP_VERSION` — verified that the macro *is*
   defined under host clang, so include-cleaner still sees the include on macOS (no new finding) while
   GCC/MSVC skip it.
2. **`src/layouts/detail/flex.cpp` built a vector from `std::views::iota`'s iterator pair.**
   `iota_view`'s iterator is a C++20 `input_iterator` but **not** a *Cpp17*InputIterator (its
   `reference` is a prvalue), so libstdc++ **and** MSVC STL reject `vector(first, last)` — libc++ merely
   happens to accept it. Replaced with `reserve` + a range-for over the same `views::iota`, which keeps
   the ranges form the surrounding comment requires and works on all three standard libraries.

Both changed TUs were re-checked against host clang/libc++ (clean), and the full 347-source sweep is
Windows-clean. This is the concrete argument for keeping Lane 2: it is a cheap, VM-free portability gate
on the code Lane 1 will depend on.

One non-defect to be aware of: GCC's `-Wextra` reports `-Wmissing-field-initializers` on
`include/maui/controls/view.hpp`'s designated initializer. That is a GCC-vs-clang strictness difference,
not a portability problem.

---

## 3. Guest provisioning (do this once on the VM)

Run in an **elevated** PowerShell on the guest:

```powershell
.\provision_guest.ps1 -PublicKey (Get-Content C:\host_key.pub)          # pipeline only
.\provision_guest.ps1 -PublicKey (Get-Content C:\host_key.pub) -WithBuildTools   # + MSVC/WinUI lane
```

(`tools/parity/windows/provision_guest.ps1`, idempotent.) It handles four things that each cause a
confusing, non-obvious failure if missed:

1. **OpenSSH Server** + TCP/22, key auth, `BatchMode=yes` friendly.
2. **`DefaultShell` = PowerShell.** *Required.* The host orchestrator quotes remote commands with
   `shlex` (POSIX single quotes); `cmd.exe` passes single quotes through literally, so every remote
   command breaks in a way that looks like the agent misbehaving.
3. **The key goes in `%ProgramData%\ssh\administrators_authorized_keys`**, not `~/.ssh/authorized_keys`
   — for a member of `Administrators`, Windows sshd reads only that file, and its ACL must be
   Administrators+SYSTEM only or sshd ignores it *silently*. This is the most common Windows key-auth
   dead end.
4. **Capture-stable desktop**: no sleep/monitor-blanking (a dimmed screen captures as a plausible
   frame), no window animations or transparency (an animating window reports its final rect before it
   has stopped moving; a translucent title bar composites the desktop into the shot), no lock screen.
   This is the Windows counterpart of the macOS lane's "clean WindowServer session" work.

Python 3 is the **only** runtime dependency of the guest agent.

---

## 4. The guest agent and the host seam

`docs/comparison/tools/vm_agent_windows.py` is the sibling of `vm_agent_macos.py` and exposes the
**same ten subcommands** (`set-resolution clean launch window-id present click type scroll shot stop`),
each printing one JSON line — so the host orchestrator is unchanged apart from a small `os` switch.

It needs **no third-party packages at all** (the macOS agent needs `brew install cliclick
displayplacer` and optionally pyobjc): every primitive exists in Win32 via `ctypes`, and screenshots
are encoded by a ~15-line stdlib PNG writer (`zlib` + `struct`), so Pillow is not required either.

Three Windows-specific hazards it handles up front, each of which otherwise yields a
*plausible-but-wrong* capture:

1. **DPI virtualisation** — a DPI-unaware process is lied to about window rects and gets stretched
   captures. The agent declares `PER_MONITOR_AWARE_V2` at import, so every coordinate in it is a
   physical pixel that matches the capture.
2. **Occlusion** — `BitBlt` from the screen photographs whatever is on top. `PrintWindow` with
   `PW_RENDERFULLCONTENT` asks the window to render *itself*, so it cannot be occluded and needs no
   focus. This is the analogue of macOS `screencapture -l <id>`, and it is the default path.
   (`PW_RENDERFULLCONTENT` is also what makes DirectComposition-rendered **WinUI 3** windows come back
   non-blank, so Lane 1 needs it too.)
3. **Late layout** — a fresh window reports a rect before it has painted, and WinUI 3 inflates its XAML
   tree asynchronously. `window-id`/`present` poll for a stable non-degenerate rect and re-assert
   position+size, and `present` **fails loudly** rather than returning a short rect: on the macOS lane a
   silently-short window banked ~800 bad frames before anyone noticed.

Host-side changes in `run_comparison.py` (macOS behaviour byte-identical — verified with `--plan`):

* `GUEST_AGENTS[os]` picks the agent (it previously hardcoded `vm_agent_macos.py`).
* Per-OS guest primitives: `New-Item -Force` for mkdir, `scp -r` for deploy (no rsync on Windows),
  `shutdown /r /t 0 /f` for reboot, no `/usr/bin/env` prefix, and `exit 0` as the reachability probe
  (`true` is a POSIX builtin PowerShell does not have — it would have reported every VM as down).
* An unknown `os` now fails loudly instead of deploying a macOS agent to a Windows box.

---

## 4b. Session 1 — the one place this lane must differ from macOS

`sshd` on Windows runs in **session 0** (services, no desktop). The interactive console the VM displays
is **session 1**. `EnumWindows`, `SendInput` and `PrintWindow` are all **per-session**. Therefore:

* an app launched over SSH runs in session 0 and has **no window on any visible desktop** — verified on
  the UTM guest: the process reported `Responding=True` with `MainWindowHandle=0`;
* an agent living in the SSH session can neither see nor drive session 1's UI;
* reading `MainWindowHandle` *from* session 0 returns 0 **even when the window exists**, so that is not
  evidence of failure — check from inside session 1.

macOS has no equivalent (`open -g` reaches the user's Aqua session), which is why `vm_agent_macos.py`
needs nothing like this.

**The fix** (`tools/parity/windows/session1.py` + the agent's `serve` subcommand):

```
host                                    guest
ssh (session 0) ------ schtasks /it --> agent `serve` in SESSION 1, bound to 127.0.0.1:<gport>
ssh -N -L <lport>:127.0.0.1:<gport> --> (tunnel)
TCP to 127.0.0.1:<lport> -------------> one JSON request/response per connection
```

Because the agent itself lives in session 1, apps it `launch`es are session-1 children — visible and
driveable — so this fixes the app-launch problem for free.

Four decisions worth knowing:

1. **Loopback + SSH tunnel, not a network port.** This endpoint executes commands. Binding it to
   loopback and tunnelling keeps access behind the same SSH key that already administers the guest and
   adds no firewall hole. A **shared token** is required on every request as defence in depth (any local
   guest account could otherwise reach the loopback port), and the server **refuses to start** if the
   token file is unreadable rather than serving unauthenticated.
2. **A persistent server, not one task per call.** `schtasks /run` costs ~1–2 s. Fine for a smoke test;
   across a 182-page board (~2200 agent calls) it would burn most of an hour on task startup. The
   project already uses a long-lived TCP agent for this reason (`devflow_port = 8765` on macOS).
3. **`serve` reuses `main()`'s argparse dispatch** by capturing stdout, so each subcommand has exactly
   one definition and the served form cannot drift from the CLI form a human debugs with.
4. **Args/results cross via a generated `.cmd` + files**, never through nested
   `schtasks → cmd → PowerShell` quoting, which is a reliable source of silent breakage.

**Requirement this imposes:** the guest user must be **logged on at the console** (the UTM display) —
`/it` means "run in the interactive session", and there must be one. An RDP login moves the console
session, so log in on the UTM display. `vm_smoke.py` checks the agent's reported `session_id` and fails
fast with that remedy if it is 0.

Transport is exercised off-guest by `tests/test_agent_serve_protocol.py` (framing, token auth,
dispatch, malformed input, repeated connections, shutdown) — those are pure Python, and a bug there
otherwise presents as "the guest is broken".

---

## 5. Tomorrow's runbook (when the VM is up)

```bash
# on the dev machine, from port/cpp
tools/parity/windows/build_smoke.sh                       # cross-build the Win32 smoke exe
tools/parity/windows/build_core_check.sh                  # cross-build the core + the self-checking probe

python3 tools/parity/windows/vm_smoke.py \
        --host <vm-hostname> --user <vm-user>             # deploy + drive + verify, step by step
```

`vm_smoke.py` walks the chain one call at a time and prints PASS/FAIL per step — ssh, DefaultShell,
python, agent deploy, app deploy, `set-resolution`, `launch`, `window-id`, `present`+shot, pull,
**PNG decodes at the presented size**, `shot --window`, click/type/scroll, and finally **deploy + run the
core probe** (asserting its exit status, not its prose), then `stop` — so a failure on a brand-new guest
is attributed to one step instead of being buried in a 182-tag sweep. It saves the capture to
`build/windows-smoke/vm_smoke_capture.png` for eyeballing.

That last step is the one that answers "does the port itself work on Windows": the smoke window only
proves the pipeline can drive *a* window, whereas the probe exercises the port's own core. If
`build_core_check.sh` has not been run, the step is skipped with a note rather than failing.

Then the full board:

```bash
cp docs/comparison/config/windows.example.toml docs/comparison/config/windows.toml   # edit host/user
python3 docs/comparison/tools/run_comparison.py --config docs/comparison/config/windows.toml --plan
python3 docs/comparison/tools/run_comparison.py --config docs/comparison/config/windows.toml --only button
```

### What is already verified without a VM

* **The port's entire cross-platform core compiles AND links for Windows**: 393 translation units →
  a 64 MB `libmaui_core_win.a` → `maui_core_probe.exe` (`PE32+ console x86-64`, self-contained, zero
  undefined references). Compiling alone would not have shown this: omitting `src/essentials` passed the
  syntax sweep and failed only at link.
* Cross-build produces a real `PE32+ executable (GUI) x86-64`, zero warnings.
* `tools/tests/test_vm_agent_windows.py` — BGRA→RGB channel order (a B↔R swap would tint every
  capture and still look legitimate), stdlib PNG decodes in Pillow with exact pixels, a 1024×800 shot
  converts+encodes in **0.01 s**, and all ten macOS-parity subcommands are present.
* `verify_png` rejects garbage/empty/truncated files and accepts the agent's own writer's output —
  the two tools are checked against each other.
* `run_comparison.py --plan` for both a macOS and a Windows config, plus the unknown-`os` guard.

### What can only be verified on the guest

`set-resolution` (`ChangeDisplaySettingsExW`), `PrintWindow` against a real window, `SendInput`
click/type/scroll, and the PowerShell-quoting assumption end to end.

---

## 6. Lane 1 (WinUI 3) — the backend

**Status: the first vertical slice EXISTS and builds natively.** `MAUI_BACKEND=windows` now compiles the
port's ~400 cross-platform sources under MSVC and links five real WinUI 3 handlers. What that took, and
what is still mirror-only, is below.

### 6a. The toolchain facts that cost time

* **Native arm64 needs a component the installer does not tick for you.** The guest is ARM64 Windows 11,
  but VS Build Tools shipped only the cross compilers `Hostarm64\x64`, `Hostarm64\x86` and
  `Hostarm64\arm` — the last being **ARM32** (`VC.Tools.ARM`), which is the one people select by mistake.
  The native toolset is `Microsoft.VisualStudio.Component.VC.Tools.ARM64` (+ `...ARM64EC`);
  `tools/parity/windows/install_arm64_toolset.ps1` adds it non-interactively and is idempotent. This
  matters because the MAUI reference board on this machine is native arm64: a score across two ABIs
  compares two rendering paths, not parity.
* **`setup.exe modify` rejects `--wait`** (that flag belongs to `vs_installer.exe`) and answers 87
  ERROR_INVALID_PARAMETER having parsed nothing — its own log then reads `vs.willow.quiet : False`,
  which looks like the flags were ignored rather than rejected. The installer's newest
  `%TEMP%\dd_installer_*.log` names the offending option; read it before theorising.
* **PowerShell script execution is disabled by default.** Every `powershell -File` invocation over SSH
  needs `-ExecutionPolicy Bypass`, or it fails with a SecurityError that says nothing about SSH.
* **Three MSVC portability fixes in the port's own build**, each real and each committed:
  - `add_compile_options(-Wall -Wextra -Wpedantic)` was unconditional; MSVC answers
    `D8021 invalid numeric argument '/Wextra'`. Now `/W4 /utf-8 /bigobj /permissive- /Zc:__cplusplus
    /Zc:preprocessor` on MSVC. `/utf-8` is NOT optional — every header here has em-dashes in its comments.
  - The `maui_sanitizers` interface applied `-g -fno-omit-frame-pointer` unconditionally.
  - `MAUI_DEVFLOW` (the in-app automation agent) is a POSIX BSD-socket server that also demangles through
    `<cxxabi.h>`; it now defaults OFF under MSVC. Nothing depends on it — consumers gate on the macro, and
    the parity runner drives the UI from OUTSIDE the process.

### 6b. What the backend contains

`src/platform/windows/`, wired by the `MAUI_BACKEND STREQUAL "windows"` swap block inside the platform-
source `else()` branch — the same swap-one-control-at-a-time bring-up the android lane uses:

| unit | native type | oracle |
| --- | --- | --- |
| `window_handler.cpp` | `Microsoft.UI.Xaml.Window` | `WindowHandler.cs` + `MauiWinUIWindow.cs` |
| `content_page_handler.cpp` | `Controls.Canvas` (single-content host) | — |
| `layout_handler.cpp` | `Controls.Canvas` (child list + z-order) | `LayoutHandler.Windows.cs` |
| `label_handler.cpp` | `Controls.TextBlock` | `LabelHandler.Windows.cs` + `TextBlockExtensions.cs` |
| `button_handler.cpp` | `Controls.Button` | `ButtonHandler.Windows.cs` + `ButtonExtensions.cs` |
| `host_run.cpp` | the `Application::Start` run loop | the winui_probe |
| `winui_interop.{hpp,cpp}` | the `void*`-slot box + string/color conversions | — |

Every control NOT in that list keeps its headless mirror and renders nothing yet — the same staged state
the iOS and Android backends passed through.

Three things about the seam that are easy to get wrong:

1. **The `void* native` slot boxes a `winrt::Microsoft::UI::Xaml::UIElement`, not the derived type.**
   Projected derived types are distinct C++ classes, so storing a `TextBlock` would make the layout
   panel's generic upcast a `reinterpret_cast`; each handler does a checked `.as<T>()` instead. And the
   box is an aggregate wrapper, because every projected type deletes `operator new`.
2. **A Canvas, not a `LayoutPanel`.** MAUI's Windows panel calls back into CrossPlatformMeasure/Arrange;
   the port already runs its own layout and hands each child an absolute-in-parent frame, so the host
   must not impose XAML layout. Each child's `platform_arrange` writes its own `Canvas.Left/Top` +
   `Width`/`Height`.
3. **`XamlControlsResources` must be merged in `OnLaunched`, never in the Application constructor** —
   the latter dies with a stowed exception (`0xC000027B`) inside combase.

### 6c. Building it

```powershell
tools\parity\windows\install_arm64_toolset.ps1   # once: the native arm64 toolset
tools\parity\windows\configure_port_windows.ps1  # provisions the App SDK + cppwinrt projection, then cmake
tools\parity\windows\build_port_windows.ps1      # the five framework libraries
tools\parity\windows\build_gallery_windows.ps1   # the parity C++ column (examples/gallery)
```

`provision_winui_sdk.ps1` (called by the two configure scripts, idempotent) restores
Microsoft.WindowsAppSDK + Microsoft.Web.WebView2 + Microsoft.Windows.CppWinRT and runs `cppwinrt.exe`,
then prints the two paths CMake cannot discover on its own (`MAUI_WINAPPSDK`, `MAUI_WINUI_GENERATED`).
WebView2 is required even though nothing here uses a WebView: `Microsoft.UI.Xaml.winmd` references its
types and cppwinrt resolves the whole type graph.

### 6d. Still to do

* The handler fan-out for every remaining control (entry, switch, checkbox, slider, image, …).
* `NeedsContainer` / `WrapperView` — MAUI's Windows backend wraps a view in a container for Background,
  clipping, shadows and vertical text alignment; the port has no container seam on this backend.
* Window lifecycle events (`Activated` / `Closed` → `send_activated` / `send_destroying`) and the
  toolbar / menu-bar / title-bar chrome, which C# DOES materialize on Windows (unlike iOS).
* `gallery_xaml` (the "C++ & XAML" column). Its committed `.xaml.cpp` TUs use `#embed`, which MSVC does
  not implement — but that is NOT a dead end: the android lane hit the identical wall (NDK Clang 18) and
  solved it with `e2e.py gen --embed-mode=bytes`, which emits the same markup as an `unsigned char[]`
  literal through the same `build_page` path. `examples/gallery_xaml/CMakeLists.txt` now selects that
  lane automatically under MSVC. What remains UNVERIFIED is whether the rest of the XAML layer
  (`maui_xaml` + pugixml) compiles under MSVC — nothing has built it yet.

### 6e. Decisions taken (previously open)

1. **Unpackaged**, with the Windows App SDK bootstrapper: the runner deploys a folder and launches an
   `.exe`, exactly like the macOS lane copies a `.app`. The bootstrap DLL is copied next to the exe by
   `maui_add_app` — without it the process dies at load with `0xC0000135` before `main()`, and the runner
   sees only "the process exited early".
2. **`cl.exe` on the guest**, not `clang-cl` cross from macOS: cross-compiling WinUI needs the Windows SDK
   + App SDK headers on the Mac.
3. **C++/WinRT projection generated by a script, not a CMake custom command.** `cppwinrt.exe` needs the
   NuGet packages restored first, and teaching CMake about NuGet adds a moving part for no benefit; the
   script prints the paths and the configure passes them in.
