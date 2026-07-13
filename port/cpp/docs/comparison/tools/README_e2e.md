# E2E visual-comparison runner

Deploys the three framework columns to a test environment, drives a scripted interaction per example,
captures the app window after each step, and pixel-scores MAUI vs the C++ columns.

- **Host orchestrator:** [`run_comparison.py`](run_comparison.py) — runs on your Mac; drives everything over SSH.
- **Guest agent:** [`vm_agent_macos.py`](vm_agent_macos.py) — copied to the VM; wraps the macOS primitives.
- **Config:** [`../config/local.example.toml`](../config/local.example.toml) — copy to `local.toml`, edit.
- **Scenarios:** [`../scenarios/`](../scenarios/) — one `<tag>.toml` per example (optional; absent → one idle screenshot).

## What runs where (today)

Only the **macOS VM** has a desktop GUI to run: the port has no Windows/Linux backend yet (`headless`
builds there but renders nothing), so those VMs are config stubs. iOS/Android capture stays in the
existing `port/tools/e2e/e2e.py` / `tools/parity` pipeline.

## One-time VM setup (macOS in UTM)

1. **SSH, passwordless.** From the host:
   ```
   ssh-copy-id testinguser@Testings-Virtual-Machine.local
   ssh -o BatchMode=yes testinguser@Testings-Virtual-Machine.local true   # must succeed silently
   ```
   The second line is the exact readiness probe the runner uses. **Retype the hostname by hand** — a
   stray zero-width character after `.local` (easy to paste in) breaks DNS with a very confusing error.

2. **Live desktop (Aqua) session — the #1 gotcha.** `screencapture` and `cliclick` produce black /
   no-op output from a bare SSH context with no logged-in console session. Set the VM to **auto-login**
   to the desktop (System Settings → Users & Groups → Automatic login).

3. **TCC permissions on the VM** (System Settings → Privacy & Security):
   - **Screen Recording** — for `screencapture`.
   - **Accessibility** — for `cliclick` (synthetic clicks/keys) and Quartz scroll events.
   Grant them to the process SSH runs commands under (Terminal/`sshd-keygen-wrapper`/your login shell).
   If capture still comes back black, wrap the capture in the user's GUI session: `launchctl asuser $UID …`.

4. **Tools** (Homebrew installs to `/opt/homebrew/bin`, which a non-interactive SSH shell does NOT have
   on PATH — the runner therefore calls every tool by absolute path, configured in `[tools]`):
   ```
   brew install cliclick displayplacer
   ```
   **pyobjc is optional** (it often fails to build `pyobjc-core` on a VM — don't fight it). With it, window
   capture uses the tight per-window `screencapture -l <id>`; without it, the window rect comes from
   AppleScript (System Events, needs the Accessibility grant you already set) and capture uses
   `-R <x,y,w,h>` (whole-window region — hide the dock so nothing composites over it). Scroll never needs
   pyobjc (ctypes → CoreGraphics). If you *want* the tighter capture and the build fails, upgrade pip first:
   `pip3 install --upgrade pip && pip3 install pyobjc-core pyobjc-framework-Quartz` (a wheel, no compiler),
   or install Xcode CLT (`xcode-select --install`).

   Sanity-check the PATH issue is handled:
   ```
   ssh testinguser@Testings-Virtual-Machine.local "/bin/zsh -c '/opt/homebrew/bin/cliclick p'"
   ```

5. **The `maui_xaml` column** (C# MauiReference.app) needs the .NET runtime on the VM, or build it
   self-contained/Release. The Debug Catalyst bundle references the installed SDK.

## Build the apps (on the host, before running)

The runner deploys pre-built artifacts; it does not build them.
- `cpp` / `cpp_xaml`: the Catalyst gallery builds → `port/cpp/examples/build-maccatalyst/{gallery,gallery_xaml}/*.app`.
- `maui_xaml`: `dotnet build port/maui-reference/app -f net10.0-maccatalyst`.

## Run

```
cd <repo root>
cp port/cpp/docs/comparison/config/local.example.toml port/cpp/docs/comparison/config/local.toml
# edit local.toml (host/user/paths)

python3 port/cpp/docs/comparison/tools/run_comparison.py --config port/cpp/docs/comparison/config/local.toml --plan
python3 port/cpp/docs/comparison/tools/run_comparison.py --config .../local.toml --only button
python3 port/cpp/docs/comparison/tools/run_comparison.py --config .../local.toml            # all pages
```

Output:
```
port/cpp/docs/comparison/<YYYY-MM-DD-HH_MM_SS>/
  <tag>/<platform>/<column>/NNNN.png      # + NNNN.json sidecar (step, theme, bounds, commit, time)
  <tag>/<platform>/compare/NNNN-report.json   # SSIM + %pixels-differ, maui_xaml vs cpp / cpp_xaml
  summary.json  report.md  run-manifest.json
```
`NNNN` is the running frame number across a column's `themes × steps` (the requested path has no theme
axis, so multi-theme sweeps continue the sequence; the sidecar records each frame's theme + step).

## Scenarios

`scenarios/<tag>.toml` — `themes` + a list of `[[steps]]`. Each step is an optional action then a
capture. Coordinates are **absolute screen pixels** against the pinned resolution. Absent scenario file →
a single idle screenshot (`0001.png`).

**The shipped `button`/`entry`/`scroll_view` scenarios are calibrated for a 1280-wide display (1280×800).**
Geometry model (measured on the C++ gallery, Mac Catalyst): the window is 1024 pt wide, **horizontally
centered**, pinned to the top (`y_origin = 30`). So centered controls sit at **screen x = display_width/2
= 640**, and control **y is measured from the fixed top** (resolution-independent). Re-calibrate the `x`
if you change the display *width*; `y` only changes if the window size changes. Coordinates were tuned
against the **C++ gallery** window — if the `maui_xaml` (C# MauiReference) window differs, recalibrate for
that column or use the DevFlow `automation_id` path (resolution-independent, no coordinates).

Display-mode note: not every VM offers 1280×800. Check `displayplacer list` on the VM; **any 1280-wide
mode** (e.g. 1280×960, 1280×732) shares the same `x=640` / `y` calibration. To get exactly 1280×800, add
that mode in the UTM VM's display settings. If `set-resolution` can't find the configured mode it logs a
warning and continues at the current resolution — at which point absolute coords will be off, so make
sure the display width matches what the scenarios were calibrated for.

## Interaction drivers

Set per column via `driver`. All three fall back to coordinates for anything they can't do, so
coordinate-only scenarios (steps with `at = [x, y]`, no `automation_id`) work regardless of setup.

- **`coordinate`** (default) — `cliclick` at absolute coordinates. Works for every column.
- **`maui_devflow`** (`maui_xaml`) — taps by `automation_id` via Microsoft's experimental **MAUI DevFlow**
  CLI. Enable DevFlow in the reference app build:
  ```
  dotnet build port/maui-reference/app -f net10.0-maccatalyst -p:EnableDevFlow=true
  ```
  This adds the `Microsoft.Maui.DevFlow.Agent` package and calls `AddMauiDevFlowAgent()` under the
  `DEVFLOW` compile constant (gated so normal builds never pull the prerelease package). Install the CLI
  on the VM (`dotnet tool install -g Microsoft.Maui.Cli --prerelease`); set the tool path via the
  column's `devflow_cli`.
- **`cpp_devflow`** (`cpp` / `cpp_xaml`) — taps by `automation_id` via the C++ port's own in-app DevFlow
  agent (`port/cpp/src/devflow`, protocol in `port/cpp/docs/DEVFLOW_PROTOCOL.md`): JSON-over-HTTP on
  `127.0.0.1:<devflow_port>`, driven with the VM's `curl` over SSH (no port-forward needed). Requires
  building the gallery with `-DMAUI_DEVFLOW=ON`; the runner launches the app with `MAUI_DEVFLOW_PORT`
  set to start the agent. Set `driver = "cpp_devflow"` and `devflow_port = 8765` on those columns.
