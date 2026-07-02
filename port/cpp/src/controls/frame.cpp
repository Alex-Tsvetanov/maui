// maui::controls::frame — out-of-line definitions: the descriptors (Frame.cs defaults), the facade
// translation onto the border machinery (see frame.hpp), and the default-handler self-registration
// (the same border_handler — the facade needs no native code of its own).

#include "maui/controls/frame.hpp"

#include <memory>
#include <stdexcept>

#include "maui/controls/border.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/shadow.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/graphics/system_background_paint.hpp"

namespace maui::controls
{
    namespace
    {
        // The hard-coded shadow C#'s Frame returns for IView.Shadow when HasShadow (Frame.cs, iOS):
        // Radius 5, Opacity 0.8, Offset (0,0), Brush.Black.
        std::shared_ptr<maui::core::shadow> make_frame_shadow()
        {
            auto value = std::make_shared<maui::core::shadow>();
            value->set_radius(5.0);
            value->set_opacity(0.8);
            value->set_color(maui::graphics::color(0.0F, 0.0F, 0.0F));
            value->set_offset({0.0, 0.0});
            return value;
        }

        // The compatibility FrameRenderer.SetupLayer maps an UNSET CornerRadius (-1) to 5 at render time
        // (`if (cornerRadius == -1f) cornerRadius = 5f`). The port folds that renderer default into the
        // facade's StrokeShape: the sentinel resolves to a round_rectangle(5), not a plain rectangle.
        constexpr float k_default_corner_radius = 5.0F;

        std::shared_ptr<maui::graphics::i_shape> default_frame_stroke_shape()
        {
            return std::make_shared<maui::graphics::shapes::round_rectangle>(
                static_cast<double>(k_default_corner_radius));
        }
    } // namespace

    frame::frame() : border(padding_property())
    {
        this->set_style_target_type<frame>();
        // Facade invariants: no border until BorderColor is set (StrokeThickness 0 keeps the measure
        // inset Padding-only, the C# `BorderColor is not null ? 1 : 0` term), and the default
        // HasShadow=true materializes the canned frame shadow.
        set_stroke_thickness(0.0);
        set_shadow(make_frame_shadow());

        // The two FrameRenderer.SetupLayer defaults the border facade otherwise omits, so a bare
        // `new Frame { Content = … }` renders as MAUI's visible white rounded card (and the shadow has an
        // opaque body to cast around) instead of an invisible plain rectangle:
        //   (a) the CornerRadius=-1 sentinel → a round_rectangle(5) StrokeShape (see corner_radius_property);
        //   (b) a system-background fill when the developer has NOT set a Background. system_background_paint
        //       is resolved to the DYNAMIC UIColor.systemBackground handler-side (Apple backends, resolved
        //       against the native view's own trait/appearance) so it tracks light/dark; a developer-set
        //       Background overwrites it (last-write-wins), so containers_page's #2E249E fill and
        //       custom_swipe_item_view's explicit fills still win.
        set_stroke_shape(default_frame_stroke_shape());
        set_background(std::make_shared<maui::graphics::system_background_paint>());
    }

    const maui::core::bindable_property<maui::core::thickness>& frame::padding_property()
    {
        // C# Frame's IPaddingElement.PaddingDefaultValueCreator returns 20.
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding",
                                                                                     maui::core::thickness(20.0)};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& frame::border_color_property()
    {
        // C# BorderElement.BorderColorProperty default is null — "unset" is tracked via is_set().
        // The facade translation lives in propertyChanged (not just the imperative setter) so EVERY
        // set-path — set_border_color, the XAML loader's apply_setter, a data binding, a style setter —
        // materializes the stroke. Without this, a loader/binding-driven BorderColor would set only the
        // tracking slot and the border_handler would read a null stroke (no chrome rendered).
        static const maui::core::bindable_property<maui::graphics::color> descriptor{
            "border_color",
            maui::graphics::color{},
            {.property_changed = [](maui::core::bindable_object& owner, const maui::graphics::color& /*old*/,
                                    const maui::graphics::color& value) {
                auto& self = static_cast<frame&>(owner);
                // The FrameRenderer's fixed 1px border in the given color.
                self.set_stroke(std::make_shared<maui::graphics::solid_paint>(value));
                self.set_stroke_thickness(1.0);
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<float>& frame::corner_radius_property()
    {
        // C# Frame.CornerRadiusProperty default is -1 (the "use the platform default" sentinel).
        // propertyChanged drives the StrokeShape on every set-path (see border_color_property).
        static const maui::core::bindable_property<float> descriptor{
            "corner_radius",
            -1.0F,
            {.property_changed = [](maui::core::bindable_object& owner, const float& /*old*/, const float& value) {
                auto& self = static_cast<frame&>(owner);
                // FrameRenderer.SetupLayer maps the -1 sentinel to 5 at render time, so BOTH the default
                // AND an explicit CornerRadius=-1 render as a round_rectangle(5) — not a plain rectangle.
                const double radius =
                    value >= 0.0F ? static_cast<double>(value) : static_cast<double>(k_default_corner_radius);
                self.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(radius));
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& frame::has_shadow_property()
    {
        // C# Frame.HasShadowProperty default is true.
        // propertyChanged drives the canned frame shadow on every set-path (see border_color_property).
        static const maui::core::bindable_property<bool> descriptor{
            "has_shadow",
            true,
            {.property_changed = [](maui::core::bindable_object& owner, const bool& /*old*/, const bool& value) {
                static_cast<frame&>(owner).set_shadow(value ? make_frame_shadow() : nullptr);
            }}};
        return descriptor;
    }

    void frame::set_border_color(maui::graphics::color value)
    {
        // The facade translation onto stroke/StrokeThickness runs in border_color_property's
        // propertyChanged, so every set-path (this setter, the XAML loader, a binding) is identical.
        border_color_.set(value);
    }

    void frame::set_corner_radius(float value)
    {
        // C# CornerRadiusProperty.validateValue: value == -1 || value >= 0 (ArgumentException otherwise).
        if (value < 0.0F && value != -1.0F)
        {
            throw std::invalid_argument("frame corner_radius must be -1 or >= 0");
        }
        // The StrokeShape translation runs in corner_radius_property's propertyChanged.
        corner_radius_.set(value);
    }

    void frame::set_has_shadow(bool value)
    {
        // The Shadow translation runs in has_shadow_property's propertyChanged.
        has_shadow_.set(value);
    }
} // namespace maui::controls

// Self-register the default handler for frame — the same border_handler (the facade adds no native
// code; C#'s FrameRenderer lives in the out-of-scope Compatibility layer).
MAUI_REGISTER_HANDLER(maui::controls::frame, maui::core::border_handler)
