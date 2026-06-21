#pragma once
// maui::core::editor_handler  <=  Microsoft.Maui.Handlers.EditorHandler
//
// The handler for a multi-line text editor. Text/placeholder/read-only/alignment/appearance flow
// virtual→native through the property mapper; a native edit flows native→virtual by calling
// i_editor::send_text_changed(old, new) (the control turns it into its `text_changed` event) and
// i_editor::send_completed() on end-of-edit (EditorHandler.iOS's MauiTextViewEventProxy: Ended →
// Completed). Ported from EditorHandler.cs (cross-platform) + EditorHandler.iOS.cs (the MauiTextView
// recipe; the AppKit twin hosts an NSTextView in an NSScrollView).
//
// Partial-class split (PROFILE §5): the mapper TABLE + ctor are cross-platform (editor_handler.cpp); the
// platform recipe — create/connect/disconnect/map_*/measure — is per backend under
// src/platform/<backend>/editor_handler.{cpp,mm}. Only one backend is linked.
//
// Out of the C# mapper, this cut: MapBackground rides the shared view_mapper; MapKeyboard pushes
// UIKeyboardType + the autocapitalization/spellcheck/autocorrection traits onto the UITextView on iOS
// (a documented no-op on AppKit, which has no soft keyboard); the iOS-only MapIsEnabled override
// collapses into the shared view_mapper's is_enabled push.
//
// editor_platform is a single cross-platform struct (so the CRTP Platform type stays complete
// everywhere): `native` holds the real backend view (an NSScrollView* hosting the NSTextView on apple,
// a MauiIosTextView* (UITextView) on ios, both retained in the .mm; unused headless), the value fields
// mirror every mapped property (the headless tests observe them), `last_known_text` lets the inbound
// edit supply the *old* value, and the move_only_function hooks are the inbound channel the platform
// partial wires up (headless tests invoke them directly to simulate a native edit / end-of-edit).

#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_editor.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    struct editor_platform : view_platform_base
    {
        editor_platform() = default;
        ~editor_platform() override; // backend-defined: releases the retained native view on apple/ios
        editor_platform(const editor_platform&) = delete;
        editor_platform(editor_platform&&) = delete;
        editor_platform& operator=(const editor_platform&) = delete;
        editor_platform& operator=(editor_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple builds write to `native` instead).
        std::string text;
        std::string placeholder;
        bool is_read_only = false;
        int max_length = std::numeric_limits<int>::max(); // C# default: no effective cap
        maui::graphics::color text_color;
        maui::graphics::color placeholder_color;
        font text_font;
        double character_spacing = 0;
        text_alignment horizontal_alignment = text_alignment::start;
        text_alignment vertical_alignment = text_alignment::start; // Editor default: Start (not center)
        bool is_text_prediction_enabled = true;                    // C# InputView default
        bool is_spell_check_enabled = true;                        // C# InputView default
        int cursor_position = 0;
        int selection_length = 0;
        // The realized keyboard input type (InputView.Keyboard default = Keyboard.Default). Every backend
        // records this mirror; the iOS twin additionally pushes UIKeyboardType + the traits (MapKeyboard);
        // AppKit has no soft keyboard (documented no-op).
        maui::core::keyboard keyboard = maui::core::keyboard::default_keyboard();
        std::string last_known_text;

        // Inbound channel hooks (wired by the platform partial; headless tests invoke them directly).
        move_only_function<void(const std::string& old_value, const std::string& new_value)> on_text_changed;
        move_only_function<void()> on_completed;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSScrollView/NSTextView pair (defined
        // in src/platform/apple/editor_handler.mm). Omitted on headless, which keeps the base mirrors.
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
        // iOS backend: push the four fundamental IView properties to the UITextView (defined in
        // src/platform/ios/editor_handler.mm). The remaining generic-IView pushes keep the
        // view_platform_base mirrors until the shared ios view/visual/semantics op helpers land
        // (the coordinator's retrofit; see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // Background IS pushed: VisualElement.Background paints the UITextView layer (solid backgroundColor /
        // gradient or image sublayer) via the shared apply_background — clip_views' red editor fills under the
        // clip mask, and the image-backed Background described above renders.
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the MauiIosEditorTextView (UITextView)'s layer (the shared
        // apply_and_store_clip; MauiIosEditorTextView.layoutSubviews re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif
    };

    class editor_handler : public view_handler<editor_handler, i_editor, editor_platform>
    {
    public:
        editor_handler();

        static property_mapper<i_editor, editor_handler>& mapper();
        static command_mapper<i_editor, editor_handler>& command_mapper();

        // Platform recipe (per backend). create + disconnect need no handler state (static); connect
        // captures `this` to route the native view's edits back to the virtual view.
        static std::unique_ptr<editor_platform> create_platform_view();
        void on_connect_handler(editor_platform& platform);
        static void on_disconnect_handler(editor_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe), each pushing one virtual-view property to the view.
        static void map_text(editor_handler& handler, i_editor& view);
        static void map_placeholder(editor_handler& handler, i_editor& view);
        static void map_placeholder_color(editor_handler& handler, i_editor& view);
        static void map_is_read_only(editor_handler& handler, i_editor& view);
        static void map_max_length(editor_handler& handler, i_editor& view);
        static void map_text_color(editor_handler& handler, i_editor& view);
        static void map_font(editor_handler& handler, i_editor& view);
        static void map_character_spacing(editor_handler& handler, i_editor& view);
        static void map_horizontal_text_alignment(editor_handler& handler, i_editor& view);
        static void map_vertical_text_alignment(editor_handler& handler, i_editor& view);
        static void map_is_text_prediction_enabled(editor_handler& handler, i_editor& view);
        static void map_is_spell_check_enabled(editor_handler& handler, i_editor& view);
        static void map_keyboard(editor_handler& handler, i_editor& view);
        static void map_cursor_position(editor_handler& handler, i_editor& view);
        static void map_selection_length(editor_handler& handler, i_editor& view);
    };
} // namespace maui::core
