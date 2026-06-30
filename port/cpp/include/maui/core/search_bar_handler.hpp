#pragma once
// maui::core::search_bar_handler  <=  Microsoft.Maui.Handlers.SearchBarHandler
//
// The handler for a search input. Text/placeholder/appearance/alignment flow virtual→native through
// the property mapper; a native edit flows native→virtual via i_search_bar::send_text_changed(old, new)
// and the search action (the keyboard's Search key / NSSearchField's action) via
// i_search_bar::send_search_button_pressed(). Ported from SearchBarHandler.cs (cross-platform) +
// SearchBarHandler.iOS.cs (the MauiSearchBar/UISearchBar recipe, translated to AppKit's NSSearchField).
//
// Partial-class split (PROFILE §5): the mapper TABLE + ctor are cross-platform (search_bar_handler.cpp);
// the platform recipe — create/connect/disconnect/map_*/measure — is per backend under
// src/platform/<backend>/search_bar_handler.{cpp,mm}. Only one backend is linked.
//
// Out of the C# mapper this cut: MapBackground rides the shared view_mapper; MapKeyboard pushes
// UIKeyboardType + the autocapitalization/spellcheck/autocorrection traits onto the UISearchBar on iOS
// (a documented no-op on AppKit, which has no soft keyboard); the iOS/Android MapIsEnabled override
// collapses into the shared view_mapper is_enabled push.
//
// search_bar_platform is a single cross-platform struct: `native` holds the real backend view (an
// NSSearchField* on apple, a UISearchBar* on ios, retained in the .mm; unused headless), the value
// fields mirror every mapped property, `last_known_text` lets the inbound edit supply the *old* value,
// and the move_only_function hooks are the inbound channel the platform partial wires up (headless
// tests invoke them directly to simulate a native edit / search press).

#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_search_bar.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    struct search_bar_platform : view_platform_base
    {
        search_bar_platform() = default;
        ~search_bar_platform() override; // backend-defined: releases the retained native view on apple/ios
        search_bar_platform(const search_bar_platform&) = delete;
        search_bar_platform(search_bar_platform&&) = delete;
        search_bar_platform& operator=(const search_bar_platform&) = delete;
        search_bar_platform& operator=(search_bar_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple builds write to `native` instead).
        std::string text;
        std::string placeholder;
        bool is_read_only = false;
        int max_length = std::numeric_limits<int>::max(); // C# default: no effective cap
        maui::graphics::color text_color;
        maui::graphics::color placeholder_color;
        maui::graphics::color cancel_button_color;
        maui::graphics::color search_icon_color;
        font text_font;
        double character_spacing = 0;
        text_alignment horizontal_alignment = text_alignment::start;
        text_alignment vertical_alignment = text_alignment::center;
        bool is_text_prediction_enabled = true; // C# InputView default
        bool is_spell_check_enabled = true;     // C# InputView default
        int cursor_position = 0;
        int selection_length = 0;
        return_type bar_return_type = return_type::search; // C# SearchBar default
        // The realized keyboard input type (InputView.Keyboard default = Keyboard.Default). Every backend
        // records this mirror; the iOS twin additionally pushes UIKeyboardType + the traits (MapKeyboard);
        // AppKit has no soft keyboard (documented no-op).
        maui::core::keyboard keyboard = maui::core::keyboard::default_keyboard();

        // The last text the bar is known to hold, so an inbound edit can report the *old* value.
        std::string last_known_text;

        // Inbound channel hooks (wired by the platform partial; headless tests invoke them directly).
        move_only_function<void(const std::string& old_value, const std::string& new_value)> on_text_changed;
        move_only_function<void()> on_search_button_pressed;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSSearchField (defined in
        // src/platform/apple/search_bar_handler.mm). Omitted on headless, which keeps the base mirrors.
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
        // iOS backend: push the four fundamental IView properties to the UISearchBar (defined in
        // src/platform/ios/search_bar_handler.mm). The remaining generic-IView pushes keep the
        // view_platform_base mirrors until the shared ios op helpers land (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // Background IS pushed: VisualElement.Background paints the UISearchBar layer (solid backgroundColor /
        // gradient or image sublayer) via the shared apply_background — clip_views' red search bar fills under
        // the clip mask, mirroring the apple backend.
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the MauiIosSearchBar (UISearchBar)'s layer (the shared
        // apply_and_store_clip; MauiIosSearchBar.layoutSubviews re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend: a plain android.widget.EditText stand-in for MauiSearchView (text + placeholder/
        // hint; the search/cancel chrome + submit are deferred). Defined in
        // src/platform/android/search_bar_handler.cpp; base body FIRST then widget push. IsEnabled IS pushed.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class search_bar_handler : public view_handler<search_bar_handler, i_search_bar, search_bar_platform>
    {
    public:
        search_bar_handler();

        static property_mapper<i_search_bar, search_bar_handler>& mapper();
        static command_mapper<i_search_bar, search_bar_handler>& command_mapper();

        // Platform recipe (per backend). create + disconnect need no handler state (static); connect
        // captures `this` to route the native bar's edits/search back to the virtual view.
        static std::unique_ptr<search_bar_platform> create_platform_view();
        void on_connect_handler(search_bar_platform& platform);
        static void on_disconnect_handler(search_bar_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe), each pushing one virtual-view property to the bar.
        static void map_text(search_bar_handler& handler, i_search_bar& view);
        static void map_placeholder(search_bar_handler& handler, i_search_bar& view);
        static void map_placeholder_color(search_bar_handler& handler, i_search_bar& view);
        static void map_is_read_only(search_bar_handler& handler, i_search_bar& view);
        static void map_max_length(search_bar_handler& handler, i_search_bar& view);
        static void map_text_color(search_bar_handler& handler, i_search_bar& view);
        static void map_font(search_bar_handler& handler, i_search_bar& view);
        static void map_character_spacing(search_bar_handler& handler, i_search_bar& view);
        static void map_horizontal_text_alignment(search_bar_handler& handler, i_search_bar& view);
        static void map_vertical_text_alignment(search_bar_handler& handler, i_search_bar& view);
        static void map_is_text_prediction_enabled(search_bar_handler& handler, i_search_bar& view);
        static void map_is_spell_check_enabled(search_bar_handler& handler, i_search_bar& view);
        static void map_keyboard(search_bar_handler& handler, i_search_bar& view);
        static void map_cursor_position(search_bar_handler& handler, i_search_bar& view);
        static void map_selection_length(search_bar_handler& handler, i_search_bar& view);
        static void map_cancel_button_color(search_bar_handler& handler, i_search_bar& view);
        static void map_search_icon_color(search_bar_handler& handler, i_search_bar& view);
        static void map_return_type(search_bar_handler& handler, i_search_bar& view);
    };
} // namespace maui::core
