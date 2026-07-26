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

## 6. Lane 1 (WinUI 3) — what remains

The pipeline is the deliverable here; the WinUI 3 **backend** is a full platform port (a
`src/platform/windows/` handler fan-out mirroring the existing apple/android partials) and is not
attempted in this branch. What this branch leaves ready for it:

* `cmake/toolchains/windows-mingw.cmake` — the cross lane, with its non-parity scope stated in-file.
* **`MAUI_BACKEND=windows` already resolves sensibly.** It accepts `windows` (`CMakeLists.txt:34`), and
  the platform-source selection's final `else()` branch — commented "headless AND android (M-android
  scaffold)" — already hands any non-apple backend the pure-C++ **headless mirrors**. That is exactly how
  both the iOS and Android backends were bootstrapped ("the JNI fan-out swaps them out control by
  control"), so a `windows` branch is needed only when the first real WinUI source lands, and the WinUI
  handlers then replace the mirrors one control at a time. This is the established pattern to follow, not
  a gap to design around.
* Two things a **CMake-driven** Windows build needs that `build_core_check.sh` sidesteps by compiling the
  library sources directly (which is why there is no preset yet):
  - `find_package(GTest CONFIG REQUIRED)` and `benchmark` are **unconditional** (`CMakeLists.txt:55-56`),
    so any configure needs both built for the target triplet — i.e. vcpkg ports for `x64-mingw-*` or
    `x64-windows`.
  - `MAUI_PLATFORM_HEADLESS` — and with it the only linked `run_app` body — is defined **only** when
    `MAUI_BACKEND STREQUAL "headless"`. A `windows` build therefore has no `run_app` until
    `src/platform/windows/host_run.cpp` exists, which is why `core_probe.cpp` performs the
    build → `mount_window` → `drive_layout` sequence itself instead of calling `run_app`.
* `provision_guest.ps1 -WithBuildTools` installs the MSVC/SDK/CMake/Ninja chain on the guest.
* `docs/comparison/config/windows.example.toml` already contains the (commented) `maui_xaml` / `cpp` /
  `cpp_xaml` columns for the parity lane.

Decisions worth making before that work starts:

1. **Packaged vs unpackaged.** Prefer **unpackaged** with the Windows App SDK bootstrapper: the runner
   deploys a folder and launches an `.exe`, exactly like the macOS lane copies a `.app`. MSIX packaging
   would add an install/uninstall step per run for no parity benefit.
2. **`cl.exe` on the guest vs `clang-cl` cross from macOS.** Cross-compiling WinUI needs the Windows
   SDK + App SDK headers on the Mac (licence-encumbered, fragile). Building on the guest is the honest
   path and the runner already reaches the guest over SSH.
3. **C++/WinRT projection.** `cppwinrt.exe` from the `Microsoft.Windows.CppWinRT` NuGet package
   generates the headers; wire it as a CMake custom command so no NuGet/MSBuild project is needed.
