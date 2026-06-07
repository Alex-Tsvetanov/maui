#pragma once
// maui::core::view_platform_base  <=  the platform-view face of Microsoft.Maui.Handlers.ViewHandler's
// generic-IView property maps (ViewHandler.cs: MapVisibility / MapOpacity / MapIsEnabled /
// MapAutomationId, which call PlatformView.UpdateVisibility/UpdateOpacity/UpdateIsEnabled/
// UpdateAutomationId — the per-platform extension methods on the native view).
//
// A non-template base for the per-control platform-view structs (button_platform, label_platform, …):
// it gives the shared view_mapper (view_mapper.hpp) a single polymorphic type to push the four
// fundamental IView properties through, regardless of which control's platform view it actually is.
//
// The base bodies just store the headless mirrors below (so the deterministic headless tests can
// observe each map_* ran with the right value); a real backend's platform struct overrides each
// update_* to push to its native view (see src/platform/apple/*_handler.mm). This M4b cut maps EXACTLY
// four properties — Visibility, Opacity, IsEnabled, AutomationId; Width/Height/Background/transforms/
// Clip/Shadow/FlowDirection are deferred (see STATUS.md).
//
// Ownership (PROFILE §8): instances are owned solely by a handler's unique_ptr<Platform> and are never
// copied or moved — hence the deleted copy/move (mirroring the existing *_platform structs). The dtor
// is virtual (this is a polymorphic base) and defined out-of-line in view_platform_base.cpp.

#include <string>
#include <string_view>

#include "maui/core/visibility.hpp"

namespace maui::core
{
    struct view_platform_base
    {
        view_platform_base() = default;
        virtual ~view_platform_base();
        view_platform_base(const view_platform_base&) = delete;
        view_platform_base(view_platform_base&&) = delete;
        view_platform_base& operator=(const view_platform_base&) = delete;
        view_platform_base& operator=(view_platform_base&&) = delete;

        // Headless mirrors of the four mapped properties (a real backend pushes to its native view in
        // the update_* overrides instead; these let the headless tests observe each mapper ran).
        bool hidden = false;
        double alpha = 1.0;
        bool enabled = true;
        std::string automation_id;

        // The default (headless) bodies write the mirrors above; backends override to push to the
        // native view. Defined out-of-line in view_platform_base.cpp.
        virtual void update_visibility(maui::core::visibility value);
        virtual void update_opacity(double value);
        virtual void update_is_enabled(bool value);
        virtual void update_automation_id(std::string_view value);
    };
} // namespace maui::core
