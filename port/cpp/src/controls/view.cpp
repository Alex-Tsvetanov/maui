// maui::controls::view — out-of-line definitions: the shared bindable-property descriptors for the
// generic IView properties (VisualElement.IsEnabled / Opacity / IsVisible(Visibility) +
// Element.AutomationId) plus the render-transform scalars and FlowDirection. These are NON-template free
// functions so there is exactly ONE descriptor per property — shared across every view<ViewInterface>
// instantiation — and the property name the view_mapper keys on is identical for every control. See
// view.hpp.
//
// Defaults mirror VisualElement.cs: IsEnabled = true, Opacity = 1.0 (clamped to [0,1], matching
// VisualElement's coerceValue), Visibility = Visible. AutomationId defaults to "" (C#'s is null; our
// value type is std::string). The render transform defaults to identity — translations/rotations 0,
// scales 1, anchors 0.5 — and FlowDirection to MatchParent (FlowDirection.cs). The property names match
// the view_mapper keys exactly.

#include "maui/controls/view.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <string>

#include "maui/controls/window.hpp" // request_containing_window_relayout needs the COMPLETE window type
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_shadow.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/semantics.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/paint.hpp"

namespace maui::controls
{
    // See the declaration in view.hpp for the fidelity note (C# Handler?.Invoke's no-Handler no-op).
    void request_containing_window_relayout(element& owner)
    {
        if (window* host = owner.containing_window())
        {
            host->request_relayout();
        }
    }

    const maui::core::bindable_property<bool>& is_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"is_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<double>& opacity_property()
    {
        static const maui::core::bindable_property<double> descriptor{
            "opacity",
            1.0,
            // VisualElement.OpacityProperty clamps to [0,1].
            {.coerce_value = [](maui::core::bindable_object& /*owner*/, const double& value) {
                return std::clamp(value, 0.0, 1.0);
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::visibility>& visibility_property()
    {
        static const maui::core::bindable_property<maui::core::visibility> descriptor{"visibility",
                                                                                      maui::core::visibility::visible};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& automation_id_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"automation_id", std::string{}};
        return descriptor;
    }

    // ---- render transform (VisualElement.cs identity defaults) ----
    const maui::core::bindable_property<double>& translation_x_property()
    {
        static const maui::core::bindable_property<double> descriptor{"translation_x", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& translation_y_property()
    {
        static const maui::core::bindable_property<double> descriptor{"translation_y", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& scale_property()
    {
        static const maui::core::bindable_property<double> descriptor{"scale", 1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& scale_x_property()
    {
        static const maui::core::bindable_property<double> descriptor{"scale_x", 1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& scale_y_property()
    {
        static const maui::core::bindable_property<double> descriptor{"scale_y", 1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& rotation_property()
    {
        static const maui::core::bindable_property<double> descriptor{"rotation", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& rotation_x_property()
    {
        static const maui::core::bindable_property<double> descriptor{"rotation_x", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& rotation_y_property()
    {
        static const maui::core::bindable_property<double> descriptor{"rotation_y", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& anchor_x_property()
    {
        static const maui::core::bindable_property<double> descriptor{"anchor_x", 0.5};
        return descriptor;
    }

    const maui::core::bindable_property<double>& anchor_y_property()
    {
        static const maui::core::bindable_property<double> descriptor{"anchor_y", 0.5};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::flow_direction>& flow_direction_property()
    {
        static const maui::core::bindable_property<maui::core::flow_direction> descriptor{
            "flow_direction", maui::core::flow_direction::match_parent};
        return descriptor;
    }

    // ---- visual layer (VisualElement Background / Shadow / Clip; defaults null). The key strings match
    // the view_mapper entries (background / shadow / clip) so on_property_changed routes to the right map.
    const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>>& background_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>> descriptor{"background"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_shadow>>& shadow_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_shadow>> descriptor{"shadow"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::graphics::i_shape>>& clip_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::graphics::i_shape>> descriptor{"clip"};
        return descriptor;
    }

    // Accessibility metadata (the control owns the semantics object via a property<shared_ptr<...>>, like
    // the visual-layer props) + the input-transparent flag (IView.InputTransparent, default false). Names
    // match the view_mapper keys.
    const maui::core::bindable_property<std::shared_ptr<maui::core::semantics>>& semantics_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::semantics>> descriptor{"semantics"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& input_transparent_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"input_transparent", false};
        return descriptor;
    }

    // ---- size requests (VisualElement.WidthRequest / HeightRequest / Minimum* / Maximum*) ----
    // Defaults match VisualElement.cs exactly: WidthRequest/HeightRequest/MinimumWidthRequest/
    // MinimumHeightRequest default to -1 (the "size to content" sentinel); MaximumWidthRequest/
    // MaximumHeightRequest default to +inf (no maximum). The names match the view_mapper keys (width/
    // height/minimum_width/minimum_height/maximum_width/maximum_height) so a request change re-runs the
    // matching map (C#'s OnRequestChanged invokes UpdateValue for each of IView.Width/Height/Minimum*/
    // Maximum*). These hold the developer's REQUEST — i_view::width()/... derive the IView values from
    // them (see view.hpp), leaving the arranged frame in frame_.
    const maui::core::bindable_property<double>& width_request_property()
    {
        static const maui::core::bindable_property<double> descriptor{"width", -1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& height_request_property()
    {
        static const maui::core::bindable_property<double> descriptor{"height", -1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& minimum_width_request_property()
    {
        static const maui::core::bindable_property<double> descriptor{"minimum_width", -1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& minimum_height_request_property()
    {
        static const maui::core::bindable_property<double> descriptor{"minimum_height", -1.0};
        return descriptor;
    }

    const maui::core::bindable_property<double>& maximum_width_request_property()
    {
        static const maui::core::bindable_property<double> descriptor{"maximum_width",
                                                                      std::numeric_limits<double>::infinity()};
        return descriptor;
    }

    const maui::core::bindable_property<double>& maximum_height_request_property()
    {
        static const maui::core::bindable_property<double> descriptor{"maximum_height",
                                                                      std::numeric_limits<double>::infinity()};
        return descriptor;
    }

    // The front-to-back ordering within a layout (VisualElement.ZIndex, default 0). A change re-orders the
    // element among its siblings: the layout managers arrange in z-index order and the layout panel
    // re-stacks its subviews (view<>::on_property_changed routes a z_index change to the parent layout's
    // handler update_z_index, mirroring ViewHandler.MapZIndex). The name is the key view<> raises.
    const maui::core::bindable_property<int>& z_index_property()
    {
        static const maui::core::bindable_property<int> descriptor{"z_index", 0};
        return descriptor;
    }

    // chrome (W1-11): the attached ToolTipProperties.Text storage (default empty; property<T>::is_set()
    // distinguishes "never set" — the C# IsSet(TextProperty) probe GetToolTip applies). The name is the
    // view_mapper key map_tool_tip listens on.
    const maui::core::bindable_property<std::string>& tool_tip_text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"tool_tip"};
        return descriptor;
    }

    // ---- layout alignment (View.HorizontalOptions / VerticalOptions) ----
    // C# View.HorizontalOptionsProperty / VerticalOptionsProperty: BindableProperty.Create with type
    // LayoutOptions and DEFAULT LayoutOptions.Fill (a propertyChanged that calls InvalidateMeasure). The
    // port's i_view surface is the resolved Primitives.LayoutAlignment (C#'s IView.HorizontalLayoutAlignment
    // => HorizontalOptions.ToCore()), and the legacy StackLayout-only Expands bit is NOT part of the IView
    // contract, so the bindable here stores the layout_alignment directly — default fill — and binding /
    // styles / setters flow through it exactly as for every other view property. The names are distinct
    // from the IView width/height/... mapper keys (there is no native mapper for alignment — it is a
    // layout-only property, consumed by view<>::arrange's ComputeFrame, never pushed to the platform view,
    // matching C# where alignment is not a handler-mapped property); on_property_changed's update_value is
    // a harmless no-op for them, like width_request / z_index before they reach a layout.
    const maui::core::bindable_property<maui::core::layout_alignment>& horizontal_layout_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::layout_alignment> descriptor{
            "horizontal_layout_alignment", maui::core::layout_alignment::fill};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::layout_alignment>& vertical_layout_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::layout_alignment> descriptor{
            "vertical_layout_alignment", maui::core::layout_alignment::fill};
        return descriptor;
    }

    // ---- margin (View.Margin) ----
    // C# View.MarginProperty: BindableProperty.Create(nameof(Margin), typeof(Thickness), typeof(View),
    // default(Thickness), propertyChanged: MarginPropertyChanged → InvalidateMeasureInternal(MarginChanged)).
    // Default is Thickness.Zero (the single-arg bindable_property ctor default-constructs the value). Like
    // the layout-alignment descriptors, there is NO native mapper keyed "margin" — margin is a layout-only
    // property the measure/arrange seam consumes (ComputeDesiredSize adds it, ComputeFrame subtracts it), so
    // on_property_changed's update_value("margin") is a harmless no-op; the change drives invalidate_measure.
    const maui::core::bindable_property<maui::core::thickness>& margin_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"margin"};
        return descriptor;
    }
} // namespace maui::controls
