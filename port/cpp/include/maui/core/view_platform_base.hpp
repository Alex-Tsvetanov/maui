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
// update_* to push to its native view (see src/platform/apple/*_handler.mm). This M4c cut maps the
// four fundamental IView properties (Visibility, Opacity, IsEnabled, AutomationId) PLUS the render
// transform (the ten ITransform scalars, rebuilt as a whole — see TransformationExtensions.cs) and
// FlowDirection; Width/Height/Background/Clip/Shadow are still deferred (they need new value types or
// are layout-driven — see STATUS.md).
//
// Ownership (PROFILE §8): instances are owned solely by a handler's unique_ptr<Platform> and are never
// copied or moved — hence the deleted copy/move (mirroring the existing *_platform structs). The dtor
// is virtual (this is a polymorphic base) and defined out-of-line in view_platform_base.cpp.

#include <string>
#include <string_view>

#include "maui/core/flow_direction.hpp"
#include "maui/core/visibility.hpp"

namespace maui::core
{
    // The ten ITransform scalars as a single POD (the render transform is rebuilt as a whole from all
    // of them — see TransformationExtensions.UpdateTransformation — so the mapper passes one bundle, not
    // a value per scalar). Defaults are the identity transform (VisualElement's transform defaults):
    // translations 0, scales 1, rotations 0, anchors 0.5. ScaleX/ScaleY are the per-axis factors; the
    // uniform Scale multiplies both (see apply_transform).
    struct transform_spec
    {
        double translation_x = 0;
        double translation_y = 0;
        double scale = 1;
        double scale_x = 1;
        double scale_y = 1;
        double rotation = 0;
        double rotation_x = 0;
        double rotation_y = 0;
        double anchor_x = 0.5;
        double anchor_y = 0.5;
    };

    struct view_platform_base
    {
        view_platform_base() = default;
        virtual ~view_platform_base();
        view_platform_base(const view_platform_base&) = delete;
        view_platform_base(view_platform_base&&) = delete;
        view_platform_base& operator=(const view_platform_base&) = delete;
        view_platform_base& operator=(view_platform_base&&) = delete;

        // Headless mirrors of the mapped properties (a real backend pushes to its native view in the
        // update_* overrides instead; these let the headless tests observe each mapper ran). The render
        // transform is mirrored as a single transform_spec (it is rebuilt as a whole), and FlowDirection
        // as one enum.
        bool hidden = false;
        double alpha = 1.0;
        bool enabled = true;
        std::string automation_id;
        transform_spec transform{};
        maui::core::flow_direction flow_direction = maui::core::flow_direction::match_parent;

        // The default (headless) bodies write the mirrors above; backends override to push to the
        // native view. Defined out-of-line in view_platform_base.cpp.
        virtual void update_visibility(maui::core::visibility value);
        virtual void update_opacity(double value);
        virtual void update_is_enabled(bool value);
        virtual void update_automation_id(std::string_view value);
        // The whole render transform at once (any single scalar change re-pushes the full spec, matching
        // TransformationExtensions which rebuilds the CATransform3D from all ten scalars).
        virtual void update_transform(const transform_spec& value);
        virtual void update_flow_direction(maui::core::flow_direction value);
    };
} // namespace maui::core
