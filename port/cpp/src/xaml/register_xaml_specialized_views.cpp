// maui::xaml — XAML registration for the "specialized_views" control group:
//   ImageButton, GraphicsView, WebView.
//
// Source of truth: xaml_specs.json group "specialized_views" + the control headers below.
// Pattern: mirrors register_xaml_text_input.cpp — register_view_properties<T> first, then one
// register_bindable_property<T> per property row from the spec.
//
// Naming alignment with the existing Button block (xaml_standard_types.cpp):
//   ImageButton XAML attrs BorderColor / BorderWidth map to the port's IButtonStroke descriptors
//   stroke_color_property() / stroke_thickness_property() — same naming as controls::button.
//
// Converters: No new converters are needed.  All value types for these three controls either already
// have converters registered in register_standard_xaml_converters (maui::core::aspect,
// maui::core::thickness, maui::graphics::color, double, int, bool, std::string) or are
// shared_ptr object types that have no text converter by design and are binding-only
// (shared_ptr<i_image_source>, shared_ptr<i_drawable>, shared_ptr<web_view_source>).
//
// Content model: all three controls are leaf controls with no [ContentProperty] — no
// register_add_child or register_content_property call is needed for any of them.

#include "register_xaml_helpers.hpp"

#include <memory>
#include <string>

#include "maui/controls/graphics_view.hpp"
#include "maui/controls/image_button.hpp"
#include "maui/controls/web_view.hpp"
#include "maui/controls/web_view_source.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/i_drawable.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    void register_xaml_specialized_views(xaml_type_registry& types, xaml_property_registry& properties,
                                         xaml_converter_registry& /*converters*/)
    {
        namespace controls = maui::controls;

        // ---- ImageButton (ImageButton.cs; [ContentProperty] absent — leaf control) ----
        // XAML attrs BorderColor/BorderWidth map to the port's IButtonStroke-named stroke descriptors,
        // exactly paralleling the button block in xaml_standard_types.cpp.
        // IsLoading is read-only (handler-only via update_is_loading — omit from markup).
        // IsAnimationPlaying is a constant false with no bindable_property accessor — omit entirely.
        types.register_type<controls::image_button>("ImageButton");
        register_view_properties<controls::image_button>(properties);
        properties.register_bindable_property<controls::image_button>("Aspect",
                                                                      controls::image_button::aspect_property());
        properties.register_bindable_property<controls::image_button>("Source",
                                                                      controls::image_button::source_property());
        properties.register_bindable_property<controls::image_button>("IsOpaque",
                                                                      controls::image_button::is_opaque_property());
        properties.register_bindable_property<controls::image_button>("Padding",
                                                                      controls::image_button::padding_property());
        properties.register_bindable_property<controls::image_button>("BorderColor",
                                                                      controls::image_button::stroke_color_property());
        properties.register_bindable_property<controls::image_button>(
            "BorderWidth", controls::image_button::stroke_thickness_property());
        properties.register_bindable_property<controls::image_button>("CornerRadius",
                                                                      controls::image_button::corner_radius_property());

        // ---- GraphicsView (GraphicsView.cs; [ContentProperty] absent — leaf canvas control) ----
        // Drawable is shared_ptr<i_drawable>: no text converter exists and none should be fabricated;
        // it is only usable via binding (e.g. Binding DrawableObject). Registering via
        // register_bindable_property lets the binding path route through apply_setter.
        types.register_type<controls::graphics_view>("GraphicsView");
        register_view_properties<controls::graphics_view>(properties);
        properties.register_bindable_property<controls::graphics_view>("Drawable",
                                                                       controls::graphics_view::drawable_property());

        // ---- WebView (WebView.cs; [ContentProperty] absent — leaf control) ----
        // Source is shared_ptr<web_view_source>: no text converter; binding-only (same rationale as
        // ImageButton.Source). UserAgent is a plain std::string — already has a converter.
        // CanGoBack / CanGoForward are handler-only read-only properties — omit from markup surface.
        types.register_type<controls::web_view>("WebView");
        register_view_properties<controls::web_view>(properties);
        properties.register_bindable_property<controls::web_view>("Source", controls::web_view::source_property());
        properties.register_bindable_property<controls::web_view>("UserAgent",
                                                                  controls::web_view::user_agent_property());
    }
} // namespace maui::xaml
