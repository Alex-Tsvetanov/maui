// maui::xaml — standard registrations: the v1 control set + built-in converters
// (xaml_standard_types.hpp).
//
// Derivations, per registration kind:
//   - element names + property names are the C# XAML markup names (PascalCase) of each control's
//     public bindable surface (Button.cs, Label.cs, Entry.cs, Image.cs, StackLayout/Grid, ContentPage
//     .cs, NavigationPage.cs, Window.cs) — including where the port renamed the backing member
//     (Button's markup BorderColor/BorderWidth map to the port's IButtonStroke-named
//     stroke_color/stroke_thickness descriptors);
//   - [ContentProperty] metadata: ContentPage.cs `[ContentProperty("Content")]`, Layout.cs
//     `[ContentProperty(nameof(Children))]`, Window.cs `[ContentProperty(nameof(Page))]`, Label.cs
//     `[ContentProperty(nameof(Text))]`;
//   - converters: TypeConversionExtensions.ConvertTo's built-in invariant-culture conversions plus
//     the full maui/xaml/xaml_converters.hpp table (color/point/rect/size/thickness/corner_radius/
//     grid_length/definition lists/layout_alignment/easing/the enum tables), each bridged through
//     registry_converter — so every registered enum/struct-typed attribute IS settable from markup
//     text (M7 converter parity).
//
// Documented v1 deferrals (registered surface only grows from here, it never silently shrinks):
//   - per-PROPERTY C# converters have no type-keyed slot: FontSizeConverter (double-typed named
//     sizes) and VisualElement.VisibilityConverter (bool-typed "visible"/"hidden" aliases) stay free
//     functions until a per-property converter override channel exists (STATUS.md M7 deferrals);
//   - Background/Clip/Shadow/Semantics/Style/StyleClass and the font sub-attributes
//     (FontFamily/FontSize/FontAttributes over the port's single font value) wait on unported types /
//     loader-side composition;
//   - Grid.RowDefinitions/ColumnDefinitions ("Auto,*,2*") CONVERT now but their attribute
//     registrations wait on the grid's definition-collection properties, and attached properties
//     (Grid.Row=…) are the M7 loader's dotted-name path — neither is a plain per-type property
//     registration.

#include "maui/xaml/xaml_standard_types.hpp"

#include <memory> // X1: std::shared_ptr<brush> converter registration

#include <string>
#include <string_view>
#include <vector>

#include "maui/animations/easing.hpp"
#include "maui/controls/brushes/brush.hpp"                // X1: the Brush-typed converter's value type
#include "maui/controls/brushes/brush_type_converter.hpp" // X1: maui::controls::convert_brush
#include "maui/controls/button.hpp"
#include "maui/controls/column_definition.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/layout.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/row_definition.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/view.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/size_f.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // ---- the converter-table adapter --------------------------------------------------------

        // Bridge a maui/xaml/xaml_converters.hpp free function into the registry's error contract:
        // the converter unit throws xaml_convert_error (C#'s per-converter Invalid/FormatException);
        // the registry's stored converters throw xaml_parse_exception (the one error
        // TypeConversionExtensions.ConvertTo surfaces). Same message, translated type.
        template <class T> [[nodiscard]] auto registry_converter(T (*convert)(std::string_view))
        {
            return [convert](const std::string& text) -> T {
                try
                {
                    return convert(text);
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what());
                }
            };
        }

        // ---- shared per-control property surfaces ---------------------------------------------------

        // The generic IView/VisualElement attribute surface every view<>-derived control shares. C#
        // finds these inherited bindables by walking base types (GetBindableProperty's
        // FlattenHierarchy); the explicit port FLATTENS them into each concrete type's registration,
        // re-using the same shared descriptors (view.cpp's non-template *_property() free functions)
        // so the apply_setter name — and therefore the value-precedence slot — is identical for every
        // control.
        template <class TControl> void register_view_properties(xaml_property_registry& properties)
        {
            namespace controls = maui::controls;
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
        }

        // Layout.cs [ContentProperty(nameof(Children))]: a child element appends to the i_container.
        // Registered under "Children" so the <Layout.Children> property-element spelling routes here.
        template <class TLayout> void register_layout_children(xaml_property_registry& properties)
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
    } // namespace

    void register_standard_xaml_types(xaml_type_registry& types)
    {
        namespace controls = maui::controls;
        types.register_type<controls::button>("Button");
        types.register_type<controls::label>("Label");
        types.register_type<controls::entry>("Entry");
        types.register_type<controls::image>("Image");
        types.register_type<controls::vertical_stack_layout>("VerticalStackLayout");
        types.register_type<controls::horizontal_stack_layout>("HorizontalStackLayout");
        types.register_type<controls::grid>("Grid");
        types.register_type<controls::content_page>("ContentPage");
        types.register_type<controls::navigation_page>("NavigationPage");
        types.register_type<controls::window>("Window");
    }

    void register_standard_xaml_properties(xaml_property_registry& properties)
    {
        namespace controls = maui::controls;

        // ---- Button (Button.cs; markup BorderColor/BorderWidth back the port's stroke descriptors) ----
        register_view_properties<controls::button>(properties);
        properties.register_bindable_property<controls::button>("Text", controls::button::text_property());
        properties.register_bindable_property<controls::button>("TextColor", controls::button::text_color_property());
        properties.register_bindable_property<controls::button>("CharacterSpacing",
                                                                controls::button::character_spacing_property());
        properties.register_bindable_property<controls::button>("Padding", controls::button::padding_property());
        properties.register_bindable_property<controls::button>("BorderColor",
                                                                controls::button::stroke_color_property());
        properties.register_bindable_property<controls::button>("BorderWidth",
                                                                controls::button::stroke_thickness_property());
        properties.register_bindable_property<controls::button>("CornerRadius",
                                                                controls::button::corner_radius_property());

        // ---- Label (Label.cs; [ContentProperty(nameof(Text))]) ----
        register_view_properties<controls::label>(properties);
        properties.register_bindable_property<controls::label>("Text", controls::label::text_property());
        properties.register_bindable_property<controls::label>("TextColor", controls::label::text_color_property());
        properties.register_bindable_property<controls::label>("CharacterSpacing",
                                                               controls::label::character_spacing_property());
        properties.register_bindable_property<controls::label>("Padding", controls::label::padding_property());
        properties.register_bindable_property<controls::label>("HorizontalTextAlignment",
                                                               controls::label::horizontal_text_alignment_property());
        properties.register_bindable_property<controls::label>("VerticalTextAlignment",
                                                               controls::label::vertical_text_alignment_property());
        properties.register_bindable_property<controls::label>("TextDecorations",
                                                               controls::label::text_decorations_property());
        properties.register_bindable_property<controls::label>("LineHeight", controls::label::line_height_property());
        properties.register_content_property<controls::label>("Text");

        // ---- Entry (Entry.cs + InputView/TextElement members) ----
        register_view_properties<controls::entry>(properties);
        properties.register_bindable_property<controls::entry>("Text", controls::entry::text_property());
        properties.register_bindable_property<controls::entry>("Placeholder", controls::entry::placeholder_property());
        properties.register_bindable_property<controls::entry>("PlaceholderColor",
                                                               controls::entry::placeholder_color_property());
        properties.register_bindable_property<controls::entry>("IsPassword", controls::entry::is_password_property());
        properties.register_bindable_property<controls::entry>("IsReadOnly", controls::entry::is_read_only_property());
        properties.register_bindable_property<controls::entry>("MaxLength", controls::entry::max_length_property());
        properties.register_bindable_property<controls::entry>("IsTextPredictionEnabled",
                                                               controls::entry::is_text_prediction_enabled_property());
        properties.register_bindable_property<controls::entry>("IsSpellCheckEnabled",
                                                               controls::entry::is_spell_check_enabled_property());
        properties.register_bindable_property<controls::entry>("CursorPosition",
                                                               controls::entry::cursor_position_property());
        properties.register_bindable_property<controls::entry>("SelectionLength",
                                                               controls::entry::selection_length_property());
        properties.register_bindable_property<controls::entry>("ReturnType", controls::entry::return_type_property());
        properties.register_bindable_property<controls::entry>("ClearButtonVisibility",
                                                               controls::entry::clear_button_visibility_property());
        properties.register_bindable_property<controls::entry>("TextColor", controls::entry::text_color_property());
        properties.register_bindable_property<controls::entry>("CharacterSpacing",
                                                               controls::entry::character_spacing_property());
        properties.register_bindable_property<controls::entry>("HorizontalTextAlignment",
                                                               controls::entry::horizontal_text_alignment_property());
        properties.register_bindable_property<controls::entry>("VerticalTextAlignment",
                                                               controls::entry::vertical_text_alignment_property());

        // ---- Image (Image.cs; IsLoading is read-only in C#, so it is not settable from markup) ----
        register_view_properties<controls::image>(properties);
        properties.register_bindable_property<controls::image>("Aspect", controls::image::aspect_property());
        properties.register_bindable_property<controls::image>("Source", controls::image::source_property());
        properties.register_bindable_property<controls::image>("IsOpaque", controls::image::is_opaque_property());
        properties.register_bindable_property<controls::image>("IsAnimationPlaying",
                                                               controls::image::is_animation_playing_property());

        // ---- the stack layouts (StackBase Spacing; Layout Padding/IsClippedToBounds/Children) ----
        register_view_properties<controls::vertical_stack_layout>(properties);
        properties.register_bindable_property<controls::vertical_stack_layout>(
            "Spacing", controls::vertical_stack_layout::spacing_property());
        properties.register_bindable_property<controls::vertical_stack_layout>(
            "Padding", controls::vertical_stack_layout::padding_property());
        properties.register_bindable_property<controls::vertical_stack_layout>("IsClippedToBounds",
                                                                               controls::clips_to_bounds_property());
        register_layout_children<controls::vertical_stack_layout>(properties);

        register_view_properties<controls::horizontal_stack_layout>(properties);
        properties.register_bindable_property<controls::horizontal_stack_layout>(
            "Spacing", controls::horizontal_stack_layout::spacing_property());
        properties.register_bindable_property<controls::horizontal_stack_layout>(
            "Padding", controls::horizontal_stack_layout::padding_property());
        properties.register_bindable_property<controls::horizontal_stack_layout>("IsClippedToBounds",
                                                                                 controls::clips_to_bounds_property());
        register_layout_children<controls::horizontal_stack_layout>(properties);

        // ---- Grid (GridLayout.cs spacing; definitions/attached props are deferred — header note) ----
        register_view_properties<controls::grid>(properties);
        properties.register_bindable_property<controls::grid>("RowSpacing", controls::grid::row_spacing_property());
        properties.register_bindable_property<controls::grid>("ColumnSpacing",
                                                              controls::grid::column_spacing_property());
        properties.register_bindable_property<controls::grid>("Padding", controls::grid::padding_property());
        properties.register_bindable_property<controls::grid>("IsClippedToBounds",
                                                              controls::clips_to_bounds_property());
        register_layout_children<controls::grid>(properties);

        // ---- ContentPage (ContentPage.cs [ContentProperty("Content")]; Page Title/Padding) ----
        register_view_properties<controls::content_page>(properties);
        properties.register_bindable_property<controls::content_page>("Title",
                                                                      controls::content_page::title_property());
        properties.register_bindable_property<controls::content_page>("Padding",
                                                                      controls::content_page::padding_property());
        properties.register_add_child<controls::content_page>(
            "Content", [](controls::content_page& page, maui::core::bindable_object& child) {
                auto* view = dynamic_cast<maui::core::i_view*>(&child);
                if (view == nullptr)
                {
                    return false;
                }
                page.set_content(*view);
                return true;
            });

        // ---- NavigationPage (NavigationPage.cs bar styling) ----
        register_view_properties<controls::navigation_page>(properties);
        properties.register_bindable_property<controls::navigation_page>(
            "BarBackgroundColor", controls::navigation_page::bar_background_color_property());
        properties.register_bindable_property<controls::navigation_page>(
            "BarTextColor", controls::navigation_page::bar_text_color_property());
        // C# NavigationPage carries NO [ContentProperty] — markup passes the root page through
        // <x:Arguments> into the NavigationPage(Page root) ctor, which PushPage()s it. The port's
        // explicit-registration analog routes the single child element to push(page, animated=false).
        // Documented deviation: push fires the pushed/navigated events (and appearing), which the C#
        // ctor path does not — the ctor's no-event PushPage seam (push_initial) is private.
        properties.register_add_child<controls::navigation_page>(
            [](controls::navigation_page& navigation, maui::core::bindable_object& child) {
                auto* page = dynamic_cast<controls::content_page*>(&child);
                if (page == nullptr)
                {
                    return false;
                }
                navigation.push(*page, /*animated=*/false);
                return true;
            });

        // ---- Window (Window.cs [ContentProperty(nameof(Page))]; Title is NON-bindable on the port's
        // window, so it registers as an explicit typed lambda; the geometry quartet is bindable) ----
        properties.register_property<controls::window, std::string>(
            "Title", [](controls::window& host, const std::string& value) { host.set_title(value); });
        properties.register_bindable_property<controls::window>("X", controls::window_x_property());
        properties.register_bindable_property<controls::window>("Y", controls::window_y_property());
        properties.register_bindable_property<controls::window>("Width", controls::window_width_property());
        properties.register_bindable_property<controls::window>("Height", controls::window_height_property());
        // C# Window.Page accepts a Page; the port has no shared page base yet, so the registration
        // exposes the existing window::set_content(element&) seam (every loadable root is an element).
        properties.register_add_child<controls::window>("Page",
                                                        [](controls::window& host, maui::core::bindable_object& child) {
                                                            auto* page = dynamic_cast<controls::element*>(&child);
                                                            if (page == nullptr)
                                                            {
                                                                return false;
                                                            }
                                                            host.set_content(*page);
                                                            return true;
                                                        });
    }

    void register_standard_xaml_converters(xaml_converter_registry& converters)
    {
        // The full xaml_converters.hpp table (M7 converter parity), each bridged through
        // registry_converter (xaml_convert_error -> xaml_parse_exception). The registry is
        // TYPE-keyed, so the per-PROPERTY C# converters cannot live here and stay free functions:
        // FontSizeConverter (double-typed FontSize names) and VisualElement.VisibilityConverter
        // (bool-typed IsVisible "visible"/"hidden" aliases) — see STATUS.md M7 deferrals.

        // TypeConversionExtensions.ConvertTo built-ins.
        converters.register_converter<std::string>(registry_converter(&convert_string));
        converters.register_converter<double>(registry_converter(&convert_double));
        converters.register_converter<float>(registry_converter(&convert_float));
        converters.register_converter<int>(registry_converter(&convert_int));
        converters.register_converter<bool>(registry_converter(&convert_bool));

        // Graphics values.
        converters.register_converter<maui::graphics::color>(registry_converter(&convert_color));
        converters.register_converter<maui::graphics::point>(registry_converter(&convert_point));
        converters.register_converter<maui::graphics::rect>(registry_converter(&convert_rect));
        converters.register_converter<maui::graphics::size>(registry_converter(&convert_size));
        converters.register_converter<maui::graphics::size_f>(registry_converter(&convert_size_f));
        converters.register_converter<maui::graphics::corner_radius>(registry_converter(&convert_corner_radius));

        // Core/Controls value types.
        converters.register_converter<maui::core::thickness>(registry_converter(&convert_thickness));
        converters.register_converter<maui::core::grid_length>(registry_converter(&convert_grid_length));
        converters.register_converter<std::vector<maui::controls::row_definition>>(
            registry_converter(&convert_row_definitions));
        converters.register_converter<std::vector<maui::controls::column_definition>>(
            registry_converter(&convert_column_definitions));
        converters.register_converter<maui::core::layout_alignment>(registry_converter(&convert_layout_alignment));
        converters.register_converter<maui::animations::easing>(registry_converter(&convert_easing));

        // Enums (incl. the [Flags] TextDecorations and the aliased FlowDirection).
        converters.register_converter<maui::core::text_alignment>(registry_converter(&convert_text_alignment));
        converters.register_converter<maui::core::aspect>(registry_converter(&convert_aspect));
        converters.register_converter<maui::core::visibility>(registry_converter(&convert_visibility));
        converters.register_converter<maui::core::return_type>(registry_converter(&convert_return_type));
        converters.register_converter<maui::core::clear_button_visibility>(
            registry_converter(&convert_clear_button_visibility));
        converters.register_converter<maui::core::flow_direction>(registry_converter(&convert_flow_direction));
        converters.register_converter<maui::core::text_decorations>(registry_converter(&convert_text_decorations));

        // X1 — the Brush converter (BrushTypeConverter): a Brush-typed XAML attribute (e.g.
        // VisualElement.Background) parses a color name/hex/CSS-color or a linear-/radial-gradient string
        // into a std::shared_ptr<maui::controls::brush>. Self-contained + appended so the sibling X4
        // converter additions union-merge trivially.
        converters.register_converter<std::shared_ptr<maui::controls::brush>>(
            registry_converter(&maui::controls::convert_brush));
    }

    void register_standard_xaml(xaml_type_registry& types, xaml_property_registry& properties,
                                xaml_converter_registry& converters)
    {
        register_standard_xaml_types(types);
        register_standard_xaml_properties(properties);
        register_standard_xaml_converters(converters);
    }
} // namespace maui::xaml
