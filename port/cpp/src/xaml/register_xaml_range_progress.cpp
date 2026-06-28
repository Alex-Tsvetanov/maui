// maui::xaml — XAML registrations for the "range_progress" control group:
//   Slider, Stepper, ProgressBar, ActivityIndicator.
//
// All four controls are pure leaf views — no child sink or content-property registration is needed
// for any of them.
//
// Converter status: double (convert_double), bool (convert_bool), and maui::graphics::color
// (convert_color) are already registered in register_standard_xaml_converters, so 13 of the 14
// property slots work from XAML markup text immediately. The only missing converter is
// std::shared_ptr<maui::core::i_image_source> for Slider.ThumbImageSource (same gap that exists for
// Image.Source today) — that property registration is deferred via a // TODO comment below.
//
// Pattern mirrors the Entry block in xaml_standard_types.cpp and uses the register_view_properties<T>
// helper from register_xaml_helpers.hpp to flatten the shared IView/VisualElement attribute surface.
//
// See register_xaml_groups.hpp for the function signature declaration.

#include "register_xaml_groups.hpp"
#include "register_xaml_helpers.hpp"

#include "maui/controls/activity_indicator.hpp"
#include "maui/controls/progress_bar.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/stepper.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    void register_xaml_range_progress(xaml_type_registry& types, xaml_property_registry& properties,
                                      xaml_converter_registry& /*converters*/)
    {
        namespace controls = maui::controls;

        // ---- Slider (Slider.cs: continuous value range + track/thumb colors + ThumbImageSource) ----
        types.register_type<controls::slider>("Slider");
        register_view_properties<controls::slider>(properties);
        properties.register_bindable_property<controls::slider>("Minimum", controls::slider::minimum_property());
        properties.register_bindable_property<controls::slider>("Maximum", controls::slider::maximum_property());
        properties.register_bindable_property<controls::slider>("Value", controls::slider::value_property());
        properties.register_bindable_property<controls::slider>("MinimumTrackColor",
                                                                controls::slider::minimum_track_color_property());
        properties.register_bindable_property<controls::slider>("MaximumTrackColor",
                                                                controls::slider::maximum_track_color_property());
        properties.register_bindable_property<controls::slider>("ThumbColor", controls::slider::thumb_color_property());
        // TODO: register ThumbImageSource once a std::shared_ptr<maui::core::i_image_source> converter
        // is added to register_standard_xaml_converters (same blocker as Image.Source).
        // properties.register_bindable_property<controls::slider>("ThumbImageSource",
        //                                                         controls::slider::thumb_image_source_property());

        // ---- Stepper (Stepper.cs: discrete increment/decrement within [Minimum, Maximum]) ----
        types.register_type<controls::stepper>("Stepper");
        register_view_properties<controls::stepper>(properties);
        properties.register_bindable_property<controls::stepper>("Minimum", controls::stepper::minimum_property());
        properties.register_bindable_property<controls::stepper>("Maximum", controls::stepper::maximum_property());
        properties.register_bindable_property<controls::stepper>("Value", controls::stepper::value_property());
        properties.register_bindable_property<controls::stepper>("Increment", controls::stepper::increment_property());

        // ---- ProgressBar (ProgressBar.cs: [0,1] progress fraction + fill color) ----
        types.register_type<controls::progress_bar>("ProgressBar");
        register_view_properties<controls::progress_bar>(properties);
        properties.register_bindable_property<controls::progress_bar>("Progress",
                                                                      controls::progress_bar::progress_property());
        properties.register_bindable_property<controls::progress_bar>(
            "ProgressColor", controls::progress_bar::progress_color_property());

        // ---- ActivityIndicator (ActivityIndicator.cs: IsRunning spinner + Color) ----
        types.register_type<controls::activity_indicator>("ActivityIndicator");
        register_view_properties<controls::activity_indicator>(properties);
        properties.register_bindable_property<controls::activity_indicator>(
            "IsRunning", controls::activity_indicator::is_running_property());
        properties.register_bindable_property<controls::activity_indicator>(
            "Color", controls::activity_indicator::color_property());
    }
} // namespace maui::xaml
