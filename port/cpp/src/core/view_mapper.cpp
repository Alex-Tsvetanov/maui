// view_mapper — the shared generic-IView property mapper (ViewHandler.ViewMapper). Maps the fundamental
// IView properties (incl. the render transform, FlowDirection, and the visual-layer Background / Shadow /
// Clip) through i_view_handler::platform_base() onto the platform view's view_platform_base face. See
// view_mapper.hpp.

#include "maui/core/view_mapper.hpp"

#include <optional>
#include <string>

#include "maui/core/i_context_flyout_element.hpp"
#include "maui/core/i_tool_tip_element.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/view_size_ops.hpp"

namespace maui::core
{
    namespace
    {
        // Each map_* mirrors ViewHandler.MapVisibility/MapOpacity/MapIsEnabled/MapAutomationId, which
        // call PlatformView.Update*; here the platform-view face is view_platform_base (null when the
        // handler's platform view does not derive it — then the map is a documented no-op).
        void map_visibility(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_visibility(view.visibility());
            }
        }

        void map_opacity(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_opacity(view.opacity());
            }
        }

        void map_is_enabled(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_is_enabled(view.is_enabled());
            }
        }

        // ViewHandler.MapMinimumWidth / MapMinimumHeight (ViewHandler.cs:52-55). Both keys route here —
        // the per-platform Update extensions are a matched pair on the same native view, and pushing both
        // whenever either moves is the transform-bundle precedent above. The push is a per-BACKEND free
        // function (view_size_ops.hpp) because the minimum is genuinely platform-divergent: apple resolves
        // it in the leaf desired-size path, android only through View.SetMinimum*, and view<>::measure's
        // measure_minimum_width/height is the other half of that same split.
        void map_minimum_size(i_view_handler& handler, i_view& view)
        {
            apply_native_minimum_size(handler.native_view(), view.minimum_width(), view.minimum_height());
        }

        void map_automation_id(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_automation_id(view.automation_id());
            }
        }

        // The render transform is rebuilt as a whole: ONE shared mapper reads all ten ITransform
        // scalars off the view (the ten transform keys all route here), bundles them into a
        // transform_spec, and pushes the whole bundle — so any single scalar change re-applies the
        // entire transform, exactly as TransformationExtensions.UpdateTransformation rebuilds the
        // CATransform3D from every scalar.
        void map_transform(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_transform(transform_spec{.translation_x = view.translation_x(),
                                                      .translation_y = view.translation_y(),
                                                      .scale = view.scale(),
                                                      .scale_x = view.scale_x(),
                                                      .scale_y = view.scale_y(),
                                                      .rotation = view.rotation(),
                                                      .rotation_x = view.rotation_x(),
                                                      .rotation_y = view.rotation_y(),
                                                      .anchor_x = view.anchor_x(),
                                                      .anchor_y = view.anchor_y()});
            }
        }

        void map_flow_direction(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_flow_direction(view.flow_direction());
            }
        }

        // The three visual-layer maps mirror ViewHandler.MapBackground / MapShadow / MapClip. Each reads
        // the borrow off the view (the control owns the object) and pushes it to the platform base, which
        // on a real backend applies it to the native layer (PaintExtensions / ShadowExtensions /
        // WrapperView.SetClip). A null borrow clears the property.
        void map_background(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_background(view.background());
            }
        }

        void map_shadow(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_shadow(view.shadow());
            }
        }

        // chrome (W1-11): pushed to the platform_base mirror AND, like tool_tip/context_flyout, to the
        // native view via the per-backend view_chrome_ops free function (C#'s ContainerView.UpdateClip
        // shape — handler.native_view() already resolves to the container when one exists, matching
        // ViewHandler.cs:472's `((PlatformView?)handler.ContainerView)?.UpdateClip(view)`). Apple/iOS
        // define apply_native_clip as a no-op here because they already push Clip per-control from their
        // own platform_arrange (see view_chrome_ops.hpp); Windows is where this call actually lands.
        void map_clip(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_clip(view.clip());
            }
            apply_native_clip(handler.native_view(), view.clip());
        }

        // ViewHandler.MapSemantics / MapInputTransparent: push the accessibility metadata (a non-owning
        // borrow the control owns) + the input-transparent flag to the platform base. Headless records the
        // mirror; the native accessibility / hit-test push is deferred (apple keeps the base mirror).
        void map_semantics(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_semantics(view.semantics());
            }
        }

        void map_input_transparent(i_view_handler& handler, i_view& view)
        {
            if (auto* base = handler.platform_base())
            {
                base->update_input_transparent(view.input_transparent());
            }
        }

        // chrome (W1-11): ViewHandler.MapToolTip — `view is IToolTipElement` (the dynamic_cast probe is
        // C#'s type check) → record the mirror, then push the text to the NATIVE view through the
        // per-backend free function (C#'s ToPlatform().UpdateToolTip extension shape), so every control
        // gets the native behavior with no per-control platform override.
        void map_tool_tip(i_view_handler& handler, i_view& view)
        {
            const auto* tool_tip_element = dynamic_cast<const i_tool_tip_element*>(&view);
            const std::optional<std::string> text =
                tool_tip_element != nullptr ? tool_tip_element->tool_tip() : std::nullopt;
            if (auto* base = handler.platform_base())
            {
                base->update_tool_tip(text);
            }
            apply_native_tool_tip(handler.native_view(), text);
        }

        // ViewHandler.MapContainerView: the "special" mapper that runs before every other property (its
        // key sits first in the chained view_mapper). It reconciles has_container with needs_container —
        // setting it flips the native container wrap on/off via the handler's on_setup_container /
        // on_remove_container hooks (SwitchHandler => needs_container() true). C#'s WrapperView property
        // re-push (UpdateValue(ContainerView)) and the obsolete IBorder branch are not modeled (the port
        // wraps purely for the NeedsContainer cases — see view_handler.hpp + the switch handler).
        void map_container_view(i_view_handler& handler, i_view& /*view*/)
        {
            handler.set_has_container(handler.needs_container());
        }

        // chrome (W1-11): ViewHandler.MapContextFlyout — `view is IContextFlyoutElement` → record the
        // mirror, then materialize the menu on the native view (NSView.menu on AppKit; a
        // UIContextMenuInteraction on iOS; headless no-op).
        void map_context_flyout(i_view_handler& handler, i_view& view)
        {
            const auto* flyout_element = dynamic_cast<const i_context_flyout_element*>(&view);
            const i_flyout* flyout = flyout_element != nullptr ? flyout_element->context_flyout() : nullptr;
            if (auto* base = handler.platform_base())
            {
                base->update_context_flyout(flyout);
            }
            apply_native_context_flyout(handler.native_view(), flyout);
        }
    } // namespace

    property_mapper<i_view, i_view_handler>& view_mapper()
    {
        static property_mapper<i_view, i_view_handler> table{
            // C#'s "special" first key: the container wrap is reconciled before every other property.
            {"container_view", &map_container_view},
            {"visibility", &map_visibility},
            {"opacity", &map_opacity},
            {"is_enabled", &map_is_enabled},
            {"automation_id", &map_automation_id},
            // The size floor: a per-backend native push, not a cross-platform clamp (see map_minimum_size).
            {"minimum_width", &map_minimum_size},
            {"minimum_height", &map_minimum_size},
            // The ten transform scalars all route to the single map_transform (rebuilds the whole spec).
            {"translation_x", &map_transform},
            {"translation_y", &map_transform},
            {"scale", &map_transform},
            {"scale_x", &map_transform},
            {"scale_y", &map_transform},
            {"rotation", &map_transform},
            {"rotation_x", &map_transform},
            {"rotation_y", &map_transform},
            {"anchor_x", &map_transform},
            {"anchor_y", &map_transform},
            {"flow_direction", &map_flow_direction},
            // The visual-layer properties (keys match the controls/view.cpp descriptor names).
            {"background", &map_background},
            {"shadow", &map_shadow},
            {"clip", &map_clip},
            {"semantics", &map_semantics},
            {"input_transparent", &map_input_transparent},
            // chrome (W1-11): the attached ToolTip + ContextFlyout (keys match the view<> setters).
            {"tool_tip", &map_tool_tip},
            {"context_flyout", &map_context_flyout},
        };
        return table;
    }
} // namespace maui::core
