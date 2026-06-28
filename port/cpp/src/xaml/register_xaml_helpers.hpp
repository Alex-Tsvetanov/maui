#pragma once
// maui::xaml — shared per-group registration helpers (internal header, NOT installed).
//
// These template helpers are split out of xaml_standard_types.cpp so every per-group registration TU
// (src/xaml/register_xaml_<group>.cpp) can reuse them without redeclaring them. They were previously
// in xaml_standard_types.cpp's anonymous namespace; moving them here gives ONE definition that all the
// parallel-authored group TUs share. Keep them as `inline` function templates: they have external
// linkage but the linker folds the (identical) instantiations, so including this header in many TUs is
// ODR-safe.
//
// Pattern (mirrors C# ApplyPropertiesVisitor's base-type flatten — see xaml_property_registry.hpp):
//   - register_view_properties<T> flattens the shared IView/VisualElement attribute surface every
//     view<>-derived control inherits, re-using the same shared descriptors (view.cpp's non-template
//     *_property() free functions) so apply_setter's name — and the value-precedence slot — is
//     identical for every control.
//   - register_layout_children<T> wires Layout.cs's [ContentProperty(nameof(Children))] child sink.

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/brushes/brush.hpp"             // Background (paint) attribute value type
#include "maui/controls/brushes/solid_color_brush.hpp" // BackgroundColor -> SolidColorBrush bridge
#include "maui/controls/element.hpp"
#include "maui/controls/style.hpp" // Style property value type (W3)
#include "maui/controls/view.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/font.hpp"            // FontSize/FontFamily compose onto the single font value (W6)
#include "maui/core/font_attributes.hpp" // FontAttributes -> with_attributes (W6)
#include "maui/core/i_view.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/xaml/xaml_converters.hpp"      // convert_font_size + xaml_convert_error (W6)
#include "maui/xaml/xaml_parse_exception.hpp" // the loader's single error channel (W6)
#include "maui/xaml/xaml_property_registry.hpp"

namespace maui::xaml
{
    // The generic IView/VisualElement attribute surface every view<>-derived control shares. C# finds
    // these inherited bindables by walking base types (GetBindableProperty's FlattenHierarchy); the
    // explicit port FLATTENS them into each concrete type's registration, re-using the same shared
    // descriptors (view.cpp's non-template *_property() free functions) so the apply_setter name — and
    // therefore the value-precedence slot — is identical for every control.
    template <class TControl> inline void register_view_properties(xaml_property_registry& properties)
    {
        namespace controls = maui::controls;
        properties.register_bindable_property<TControl>("Margin", controls::margin_property());
        // VisualElement layout-options + flow (descriptors + converters already exist; register-only).
        properties.register_bindable_property<TControl>("HorizontalOptions",
                                                        controls::horizontal_layout_alignment_property());
        properties.register_bindable_property<TControl>("VerticalOptions",
                                                        controls::vertical_layout_alignment_property());
        properties.register_bindable_property<TControl>("FlowDirection", controls::flow_direction_property());
        properties.register_bindable_property<TControl>("IsEnabled", controls::is_enabled_property());
        properties.register_bindable_property<TControl>("Opacity", controls::opacity_property());
        properties.register_bindable_property<TControl>("InputTransparent", controls::input_transparent_property());
        properties.register_bindable_property<TControl>("AutomationId", controls::automation_id_property());
        properties.register_bindable_property<TControl>("TranslationX", controls::translation_x_property());
        properties.register_bindable_property<TControl>("TranslationY", controls::translation_y_property());
        properties.register_bindable_property<TControl>("Scale", controls::scale_property());
        properties.register_bindable_property<TControl>("ScaleX", controls::scale_x_property());
        properties.register_bindable_property<TControl>("ScaleY", controls::scale_y_property());
        properties.register_bindable_property<TControl>("Rotation", controls::rotation_property());
        properties.register_bindable_property<TControl>("RotationX", controls::rotation_x_property());
        properties.register_bindable_property<TControl>("RotationY", controls::rotation_y_property());
        properties.register_bindable_property<TControl>("AnchorX", controls::anchor_x_property());
        properties.register_bindable_property<TControl>("AnchorY", controls::anchor_y_property());
        properties.register_bindable_property<TControl>("ZIndex", controls::z_index_property());
        properties.register_bindable_property<TControl>("WidthRequest", controls::width_request_property());
        properties.register_bindable_property<TControl>("HeightRequest", controls::height_request_property());
        properties.register_bindable_property<TControl>("MinimumWidthRequest",
                                                        controls::minimum_width_request_property());
        properties.register_bindable_property<TControl>("MinimumHeightRequest",
                                                        controls::minimum_height_request_property());
        properties.register_bindable_property<TControl>("MaximumWidthRequest",
                                                        controls::maximum_width_request_property());
        properties.register_bindable_property<TControl>("MaximumHeightRequest",
                                                        controls::maximum_height_request_property());
        // VisualElement.IsVisible (a bool bindable in C#) maps onto the port's visibility property
        // exactly the way VisualElement implements IView.Visibility: IsVisible.ToVisibility() —
        // true → Visible, false → Collapsed (VisibilityExtensions.cs).
        properties.register_property<TControl, bool>("IsVisible", [](TControl& control, const bool& value) {
            control.set_visibility(value ? maui::core::visibility::visible : maui::core::visibility::collapsed);
        });
        // VisualElement.BackgroundColor: MAUI's MapBackgroundColor routes to IView.Background =
        // new SolidColorBrush(BackgroundColor); the port has a single paint pipeline, so the Color
        // attribute bridges to set_background_brush (convert_color supplies the literal).
        properties.register_property<TControl, maui::graphics::color>(
            "BackgroundColor", [](TControl& control, const maui::graphics::color& value) {
                control.set_background_brush(std::make_shared<maui::controls::solid_color_brush>(value));
            });
        // VisualElement.Background (a Brush): convert_brush supplies solids/gradients from the literal.
        properties.register_property<TControl, std::shared_ptr<maui::controls::brush>>(
            "Background", [](TControl& control, const std::shared_ptr<maui::controls::brush>& value) {
                control.set_background_brush(value);
            });
        // NavigableElement.Style (W3): an inline <View.Style><Style/></View.Style> or Style="{StaticResource K}"
        // provides a shared_ptr<style>; set_style applies its setters at style specificity.
        properties.register_property<TControl, std::shared_ptr<maui::controls::style>>(
            "Style",
            [](TControl& control, const std::shared_ptr<maui::controls::style>& value) { control.set_style(value); });
    }

    // Font sub-attributes (W6): the port models a control's font as a single core::font value —
    // FontFamily/FontSize/FontAttributes are NOT separate bindables (the documented loader-side-composition
    // deferral) — so each markup attribute COMPOSES onto the current font value. All three are registered as
    // STRING-parsing properties: FontSize because both numeric ("22") and NamedSize ("Large") must route
    // through one entry (the per-property FontSizeConverter has no type-keyed converter slot — the original
    // deferral), FontAttributes because it is a [Flags] string ("Bold"/"Italic"/"Bold,Italic"/"None").
    // convert_font_size throws xaml_convert_error → translate to the loader's xaml_parse_exception channel
    // (the property lambda runs inside a guarded() block that only catches the latter). Call from each
    // control that exposes font()/set_font.
    template <class TControl> inline void register_font_properties(xaml_property_registry& properties)
    {
        properties.register_property<TControl, std::string>("FontSize", [](TControl& control, const std::string& text) {
            double size = 0;
            try
            {
                size = maui::xaml::convert_font_size(text);
            }
            catch (const xaml_convert_error& error)
            {
                throw xaml_parse_exception(error.what());
            }
            control.set_font(control.font().with_size(size));
        });
        // FontAttributes: the [Flags] None/Bold/Italic — folded onto the font's weight+slant via the same
        // FontExtensions.WithAttributes the span surface uses. Comma-separates like C# Enum.Parse([Flags]).
        properties.register_property<TControl, std::string>(
            "FontAttributes", [](TControl& control, const std::string& text) {
                maui::core::font_attributes attributes = maui::core::font_attributes::none;
                std::size_t start = 0;
                while (start <= text.size())
                {
                    const std::size_t comma = text.find(',', start);
                    const std::size_t end = comma == std::string::npos ? text.size() : comma;
                    std::string_view token{text.data() + start, end - start};
                    while (!token.empty() && token.front() == ' ')
                    {
                        token.remove_prefix(1);
                    }
                    while (!token.empty() && token.back() == ' ')
                    {
                        token.remove_suffix(1);
                    }
                    if (token == "Bold")
                    {
                        attributes |= maui::core::font_attributes::bold;
                    }
                    else if (token == "Italic")
                    {
                        attributes |= maui::core::font_attributes::italic;
                    }
                    else if (token != "None" && !token.empty())
                    {
                        throw xaml_parse_exception(std::string{"Invalid FontAttributes value: "} + std::string{token});
                    }
                    if (comma == std::string::npos)
                    {
                        break;
                    }
                    start = comma + 1;
                }
                control.set_font(maui::core::with_attributes(control.font(), attributes));
            });
        // FontFamily: rebuild the descriptor preserving the current size/weight/slant (no with_family
        // builder; of_size is the family-bearing factory).
        properties.register_property<TControl, std::string>(
            "FontFamily", [](TControl& control, const std::string& text) {
                const maui::core::font current = control.font();
                control.set_font(maui::core::font::of_size(text, current.size(), current.weight(), current.slant()));
            });
    }

    // Layout.cs [ContentProperty(nameof(Children))]: a child element appends to the i_container.
    // Registered under "Children" so the <Layout.Children> property-element spelling routes here.
    template <class TLayout> inline void register_layout_children(xaml_property_registry& properties)
    {
        properties.register_add_child<TLayout>("Children", [](TLayout& parent, maui::core::bindable_object& child) {
            auto* view = dynamic_cast<maui::core::i_view*>(&child);
            if (view == nullptr)
            {
                return false;
            }
            parent.add(*view);
            return true;
        });
    }
} // namespace maui::xaml
