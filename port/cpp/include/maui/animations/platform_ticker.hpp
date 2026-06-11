#pragma once
// maui::animations::create_platform_ticker  <=  Microsoft.Maui.Animations.PlatformTicker (the
// per-backend partials PlatformTicker.{iOS,Standard,...}.cs)
//
// The factory for the backend's REAL ticker — exactly one definition is compiled per backend
// (PROFILE §3 partial-class split):
//   - headless (src/platform/headless/platform_ticker.cpp): the deterministic manual_ticker over the
//     supplied dispatcher — the port's PlatformTicker.Standard.cs (which is just the base Ticker).
//   - ios (src/platform/ios/platform_ticker.mm): CADisplayLink added to the current run loop in
//     common modes, 1:1 with PlatformTicker.iOS.cs (which also covers Mac Catalyst).
//   - apple/AppKit (src/platform/apple/platform_ticker.mm): an NSTimer on the main run loop at
//     1000/max_fps ms in common modes. DEVIATION (documented): C# has no AppKit backend — Mac
//     Catalyst is UIKit and uses CADisplayLink. AppKit only gained a per-view CADisplayLink in
//     macOS 14 (and the standalone CVDisplayLink is deprecated since macOS 15 and fires on a
//     background thread, which would need re-marshalling). The NSTimer ticker keeps the C# base
//     Ticker's timer semantics on the UI thread, matching what AppKit affords.
//
// The dispatcher reference is the headless ticker's time source; the Apple tickers drive off their
// native frame source and ignore it (like C#'s CADisplayLink ticker ignores the DI dispatcher). It
// must outlive the returned ticker.

#include <memory>

namespace maui::core
{
    class i_dispatcher;
} // namespace maui::core

namespace maui::animations
{
    class ticker;

    [[nodiscard]] std::shared_ptr<ticker> create_platform_ticker(maui::core::i_dispatcher& dispatcher);
} // namespace maui::animations
