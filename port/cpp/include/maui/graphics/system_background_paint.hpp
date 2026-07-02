#pragma once
// maui::graphics::system_background_paint  <=  (no direct C# type — models UIColor.SystemBackground /
// ColorExtensions.BackgroundColor as a paint kind)
//
// A marker paint used ONLY as the legacy Frame's default background fill when the developer has not set
// a BackgroundColor. It exists because the port's `color` value type is a plain RGBA and cannot carry a
// "dynamic system color" that adapts to light/dark; instead this typed paint lets each backend's
// apply_background resolve it to the platform's *dynamic* system-background color HANDLER-SIDE:
//   - iOS / Mac Catalyst → UIColor.systemBackground, resolved against the native VIEW's traitCollection,
//   - macOS AppKit       → NSColor.windowBackgroundColor, resolved against the view's effectiveAppearance,
//   - headless / Android → its static fallback color (opaque white, the light-mode value).
// This mirrors the compatibility FrameRenderer.SetupLayer, which fills an UNSET Frame's layer with
// ColorExtensions.BackgroundColor (= the dynamic system background) when BackgroundColor is null.
//
// IMPORTANT (why per-VIEW resolution, not the bare CGColor accessor): UIColor.CGColor resolves a dynamic
// color against the GLOBAL UITraitCollection.current, which outside a view draw/layout callback is the
// screen's trait (e.g. the Mac's dark appearance) — baking the WRONG (dark) CGColor onto the layer. The
// frame's fill lives on a CALayer (needed for the rounded-corner clip), and a CALayer never auto-resolves
// a dynamic color per-view, so apply_background must resolve MANUALLY against the view's trait — exactly
// as MAUI's FrameRenderer relies on the platform view's own appearance.
//
// It derives solid_paint so backends WITHOUT a dynamic-color mechanism (headless, Android, and anything
// that has not been taught the marker) still get a sensible opaque-white solid fill for free; only the
// Apple backends override the resolution to the adapting dynamic color via a dynamic_cast to this type.

#include "maui/graphics/color.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::graphics
{
    class system_background_paint : public solid_paint
    {
    public:
        // The static fallback is opaque white — the light-mode value of UIColor.systemBackground — so a
        // backend that does not special-case this type renders a white card (correct in light mode).
        system_background_paint();

    private:
        // Out-of-line key function: pins this polymorphic type's vtable/type_info to one TU so the
        // cross-library dynamic_cast in the Apple backends' apply_background is reliable and RTTI is not
        // duplicated per include. (Not an override — a dedicated anchor virtual on this leaf type.)
        virtual void anchor();
    };
} // namespace maui::graphics
