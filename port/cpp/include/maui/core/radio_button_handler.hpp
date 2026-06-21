#pragma once
// maui::core::radio_button_handler  <=  Microsoft.Maui.Handlers.RadioButtonHandler
//
// The handler for a radio button: IsChecked / Content / the text style / the stroke flow
// virtual→native through the property mapper, and a native tap flows native→virtual via
// i_radio_button::send_is_checked(true) (a radio tap SELECTS — it never unchecks itself; the group's
// mutual exclusion unchecks the others at the Controls layer, see radio_button_group). Ported from
// RadioButtonHandler.cs (the Mapper keys) + RadioButtonHandler.iOS.cs.
//
// Native shape per backend (documented deviations):
//   - AppKit: a real NSButtonTypeRadio NSButton (title = the string content, state = IsChecked). The
//     native exclusion of an NSButton radio group (sibling buttons sharing one action) is NOT used —
//     the cross-platform RadioButtonGroup is authoritative, exactly as C# keeps the exclusion in the
//     Controls layer on every platform.
//   - UIKit: C# renders RadioButton via a ControlTemplate (the handler's PlatformView is a plain
//     ContentView and most Map* are [MissingMapper]). The port's templated content is deferred, so the
//     ios partial implements the NATIVE DEFAULT FALLBACK instead: a UIButton(System) whose
//     selected-state rides UIButton.selected with the circle / filled-circle SF-symbol pair (the
//     DefaultTemplate's Ellipse indicator, translated), title = the string content, and C#'s
//     MapIsChecked AccessibilityValue "1"/"0" push kept verbatim.
//   - The C# Mapper's Background key (suppressed on iOS) rides the shared view_mapper instead; the
//     stroke/corner keys ([MissingMapper] on iOS) map to the layer per the ButtonExtensions recipe —
//     the AppKit/UIKit translation of the DefaultTemplate's Border bindings.
//
// Partial-class split (PROFILE §5): the mapper TABLES + ctor are cross-platform
// (radio_button_handler.cpp); the platform recipe — create/connect/disconnect/map_*/measure — is per
// backend under src/platform/<backend>/radio_button_handler.{cpp,mm}. Only one backend is linked.
//
// radio_button_platform is a single cross-platform struct: `native` holds the real backend view (an
// NSButton* on apple / a UIButton* on ios, retained in the .mm; unused headless); the value fields
// mirror every mapped property; and on_select is the inbound channel (the platform partial wires it;
// headless tests invoke it directly to simulate a native tap).

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_radio_button.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    struct radio_button_platform : view_platform_base
    {
        radio_button_platform() = default;
        ~radio_button_platform() override; // backend-defined: releases the retained native button on apple/ios
        radio_button_platform(const radio_button_platform&) = delete;
        radio_button_platform(radio_button_platform&&) = delete;
        radio_button_platform& operator=(const radio_button_platform&) = delete;
        radio_button_platform& operator=(radio_button_platform&&) = delete;

        void* native = nullptr;
        // Headless mirrors of every mapped property (the Apple/iOS builds push to `native` instead).
        bool is_checked = false;
        std::string content;
        maui::graphics::color text_color;
        font text_font;
        double character_spacing = 0;
        maui::graphics::color stroke_color;
        double stroke_thickness = 0;
        int corner_radius = 0;
        // The inbound channel hook (wired by the platform partial; headless tests invoke it directly).
        move_only_function<void()> on_select;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the radio NSButton (defined in
        // src/platform/apple/radio_button_handler.mm). Omitted on headless, which keeps the base mirrors.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend: push the fundamental IView properties to the fallback UIButton (defined in
        // src/platform/ios/radio_button_handler.mm). update_background paints the VisualElement background
        // (the RadioButtonBorder yellow fill) — a Custom UIButton honors backgroundColor directly.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
#endif
    };

    class radio_button_handler : public view_handler<radio_button_handler, i_radio_button, radio_button_platform>
    {
    public:
        radio_button_handler();

        static property_mapper<i_radio_button, radio_button_handler>& mapper();
        static command_mapper<i_radio_button, radio_button_handler>& command_mapper();

        // Platform recipe (per backend). create + disconnect need no handler state (static); connect
        // captures `this` to route the native tap back to the virtual view.
        static std::unique_ptr<radio_button_platform> create_platform_view();
        void on_connect_handler(radio_button_platform& platform);
        static void on_disconnect_handler(radio_button_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe), one per RadioButtonHandler.Mapper key.
        static void map_is_checked(radio_button_handler& handler, i_radio_button& view);
        static void map_content(radio_button_handler& handler, i_radio_button& view);
        static void map_text_color(radio_button_handler& handler, i_radio_button& view);
        static void map_character_spacing(radio_button_handler& handler, i_radio_button& view);
        static void map_font(radio_button_handler& handler, i_radio_button& view);
        static void map_stroke_color(radio_button_handler& handler, i_radio_button& view);
        static void map_stroke_thickness(radio_button_handler& handler, i_radio_button& view);
        static void map_corner_radius(radio_button_handler& handler, i_radio_button& view);
    };
} // namespace maui::core
