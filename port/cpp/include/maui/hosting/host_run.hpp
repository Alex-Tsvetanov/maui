#pragma once
// maui::hosting::run_app — the PURE C++ application entry point each platform backend implements.
//
// No C# class maps here: this is the port's stand-in for what MAUI's platform startup wraps per OS
// (UIApplicationMain on iOS, the WinUI Application lifecycle on Windows, …). It is the single seam a
// portable main() calls (maui/maui_main.hpp) so the user writes ZERO platform code: they define an
// application subclass + the use_shared_maui_app configurator, and run_app does the rest on whichever
// backend is linked.
//
// The contract:
//   - `configure` receives a FRESH builder (from maui_app::create_builder()), is expected to call
//     use_maui_app<App>() (and any configure_handlers / configure_lifecycle_events it wants), and returns
//     the configured builder. It is exactly the user's use_shared_maui_app (maui_main.hpp), mirroring a C#
//     MauiProgram.CreateMauiApp that receives and returns the builder.
//   - run_app builds the app from that builder, mounts the application's window + element tree generically
//     (app_host.hpp), drives an initial layout pass, then enters the backend's run loop (none on headless —
//     boot + one settle pass is enough to prove the mount). Returns the process exit code.
//
// This header is PURE C++ — it names no platform type. Each backend supplies the run_app BODY:
//   - headless: src/platform/headless/host_run.cpp (this Stage) — no run loop; mount + one layout pass.
//   - apple/ios/windows/android: their own host_run translation unit (Stage 2) — same mount, plus the
//     native run loop and the safe-area-derived layout bounds. The builds key the right .cpp off the same
//     MAUI_BACKEND/platform macros src/platform/* already use, so only ONE run_app links per backend.

#include "maui/hosting/maui_app_builder.hpp"

namespace maui::hosting
{
    // The configurator signature: take a fresh builder, configure it (use_maui_app<App>(), …), return it.
    using app_configurator = maui_app_builder (*)(maui_app_builder);

    // Build + mount + run the application configured by `configure`, returning the process exit code.
    // `argc`/`argv` are forwarded to the backend run loop (UIApplicationMain et al.); headless ignores them.
    int run_app(int argc, char** argv, app_configurator configure);
} // namespace maui::hosting
