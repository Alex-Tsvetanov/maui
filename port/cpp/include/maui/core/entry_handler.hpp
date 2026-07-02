#pragma once
// maui::core::entry_handler  <=  Microsoft.Maui.Handlers.EntryHandler
//
// The handler for a single-line text entry — the first editable native control and the first INBOUND-text
// channel. Text/placeholder/secure/read-only/alignment/appearance flow virtual→native through the property
// mapper; a native edit flows native→virtual by calling i_entry::send_text_changed(old, new) (the control
// turns it into its `text_changed` event) and i_entry::send_completed() on end-of-edit. Ported from
// EntryHandler.cs (cross-platform) + EntryHandler.iOS.cs (the UITextField recipe, translated to AppKit's
// editable NSTextField / NSSecureTextField).
//
// Partial-class split (PROFILE §5): the mapper TABLES + ctor are cross-platform (entry_handler.cpp); the
// platform recipe — create/connect/disconnect/map_*/measure — is per backend under
// src/platform/<backend>/entry_handler.{cpp,mm}. Only one backend is linked.
//
// entry_platform is a single cross-platform struct (so the CRTP Platform type stays complete everywhere):
// `native` holds the real backend view (an editable NSTextField* on Apple, retained in the .mm; unused
// headless), the value fields mirror every mapped property (the headless tests observe them; the Apple
// build pushes to `native` instead), `last_known_text` lets the inbound edit supply the *old* value, and
// the move_only_function hooks are the inbound channel the platform partial wires up (the headless test
// invokes them directly to simulate a native edit / end-of-edit).
//
// entry_platform derives view_platform_base (the shared ViewMapper face) so the generic IView properties
// (Visibility/Opacity/IsEnabled/AutomationId) map onto the field too (Apple overrides update_*; headless
// keeps the base mirrors).

#include <cstdint>
#include <limits>
#include <memory>
#include <optional> // --- platform configuration (W2-24): the cursor-color mirror ---
#include <string>
#include <string_view>

#include "maui/core/clear_button_visibility.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_entry.hpp"
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
    struct entry_platform : view_platform_base
    {
        entry_platform() = default;
        ~entry_platform() override; // backend-defined: releases the retained native field on Apple
        entry_platform(const entry_platform&) = delete;
        entry_platform(entry_platform&&) = delete;
        entry_platform& operator=(const entry_platform&) = delete;
        entry_platform& operator=(entry_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple build writes to `native` instead).
        std::string text;
        std::string placeholder;
        bool is_password = false;
        bool is_read_only = false;
        int max_length = std::numeric_limits<int>::max(); // C# default: no effective cap
        maui::graphics::color text_color;
        maui::graphics::color placeholder_color;
        font text_font;
        double character_spacing = 0;
        text_alignment horizontal_alignment = text_alignment::start;
        text_alignment vertical_alignment = text_alignment::center;
        bool is_text_prediction_enabled = true; // C# InputView default
        bool is_spell_check_enabled = true;     // C# InputView default
        int cursor_position = 0;
        int selection_length = 0;
        return_type entry_return_type = return_type::default_;
        clear_button_visibility clear_button = clear_button_visibility::never;
        // The realized keyboard input type (InputView.Keyboard default = Keyboard.Default). Every backend
        // records this mirror; the iOS twin additionally pushes UIKeyboardType + the autocapitalization /
        // spellcheck / autocorrection traits (MapKeyboard); AppKit has no soft keyboard (documented no-op).
        maui::core::keyboard keyboard = maui::core::keyboard::default_keyboard();
        // --- platform configuration (W2-24): the realized iOSSpecific Entry.CursorColor (nullopt until
        // the knob is SET — TextExtensions.UpdateCursorColor's IsSet guard; every backend keeps this
        // mirror, the iOS twin additionally tints the UITextField). ---
        std::optional<maui::graphics::color> cursor_color;
        // The realized iOSSpecific Entry.AdjustsFontSizeToFitWidth (BindableProperty default false).
        // UpdateAdjustsFontSizeToFitWidth pushes UNCONDITIONALLY (no IsSet guard), so every backend
        // records this mirror on every map run; the iOS twin additionally pushes
        // UITextField.adjustsFontSizeToFitWidth.
        bool adjusts_font_size_to_fit_width = false;

        // The last text the entry is known to hold, so an inbound edit can report the *old* value.
        std::string last_known_text;

        // Inbound channel hooks (wired by the platform partial; headless tests invoke them directly).
        move_only_function<void(const std::string& old_value, const std::string& new_value)> on_text_changed;
        move_only_function<void()> on_completed;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSTextField (defined in
        // src/platform/apple/entry_handler.mm). Omitted on headless, which keeps the base mirrors.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        // Accessibility metadata + the input-transparent flag pushed to the NSTextField (M5d native a11y /
        // hit-test): semantics → accessibilityLabel/Help/heading role, input_transparent → -hitTest: gate.
        // (Read-only is mapped separately via map_is_read_only → NSTextField.editable; independent of this.)
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend (M6 fan-out): push the four fundamental IView properties to the UITextField
        // (defined in src/platform/ios/entry_handler.mm). The remaining generic-IView pushes keep the
        // view_platform_base mirrors until the shared ios view/visual/semantics op helpers land (the
        // coordinator's retrofit; see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // Background IS pushed: VisualElement.Background paints the UITextField layer's backgroundColor
        // (the shared apply_background), so a clipped red field (clip_views) fills under the clip mask.
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the UITextField's layer (the shared apply_and_store_clip;
        // MauiIosTextField.layoutSubviews re-frames the mask to the live bounds, the 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (M-android fan-out, wave 14): push the generic IView properties to the real
        // android.widget.EditText over JNI (defined in src/platform/android/entry_handler.cpp). Each
        // override calls the view_platform_base body FIRST (the VM-less cross-platform suite observes the
        // headless mirror) then pushes to the widget when one exists; transform / flow-direction /
        // semantics route through the shared android ops. Shadow and InputTransparent keep ONLY the base
        // mirror (WrapperView-only on Android, no plain-View analog), as the editor partial documents. Clip
        // IS pushed (wave 24): the generic-view clip rides a ViewOutlineProvider + setClipToOutline(true)
        // (android_clip_ops.hpp apply_outline_clip) — convex shapes (ellipse/rect/rounded-rect, the
        // clip_views page's shared EllipseGeometry) clip exactly; a non-convex path keeps the headless
        // mirror (the honest deferral documented there). update_clip stashes the borrow in clip_shape so
        // platform_arrange re-resolves it against the live bounds (the geometry is bounds-dependent — the
        // iOS reapply_clip analog).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_semantics(const maui::core::semantics* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        // The clip borrow platform_arrange re-resolves against the live bounds (null = no clip). Android-gated.
        const maui::graphics::i_shape* clip_shape = nullptr;
#endif

#ifdef MAUI_PLATFORM_WINDOWS
        // Windows (WinUI 3) backend: push the fundamental IView properties + Background to the real
        // Microsoft.UI.Xaml.Controls.TextBox (defined in src/platform/windows/entry_handler.cpp). Each
        // override calls the view_platform_base body FIRST — the windows preset also runs the
        // cross-platform suite on the host WITHOUT a XAML runtime (create_platform_view degrades to a
        // null native there), and that suite observes the headless mirrors — then pushes to the control
        // when one exists (the android partial's dual-drive pattern). Background joins the four because
        // C#'s EntryHandler has a Windows-specific MapBackground (TextBoxExtensions.UpdateBackground).
        // transform / flow_direction / semantics / shadow / clip / input_transparent keep the base
        // mirrors in this first cut (deferred — see the partial's header).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        // Native event wiring state (NO winrt types in shared headers — opaque ints only): the
        // TextChanged / KeyUp winrt::event_token values EntryHandler.Windows.cs ConnectHandler installs
        // (OnPlatformTextChanged / OnPlatformKeyUp — Enter → Completed). Revoked in
        // on_disconnect_handler.
        std::int64_t text_changed_token = 0;
        std::int64_t key_up_token = 0;
#endif
    };

    class entry_handler : public view_handler<entry_handler, i_entry, entry_platform>
    {
    public:
        entry_handler();

        static property_mapper<i_entry, entry_handler>& mapper();
        static command_mapper<i_entry, entry_handler>& command_mapper();

        // Platform recipe (per backend). create + disconnect need no handler state (static); connect
        // captures `this` to route the native field's edits back to the virtual view.
        static std::unique_ptr<entry_platform> create_platform_view();
        void on_connect_handler(entry_platform& platform);
        static void on_disconnect_handler(entry_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe), each pushing one virtual-view property to the field.
        static void map_text(entry_handler& handler, i_entry& view);
        static void map_placeholder(entry_handler& handler, i_entry& view);
        static void map_placeholder_color(entry_handler& handler, i_entry& view);
        static void map_is_password(entry_handler& handler, i_entry& view);
        static void map_is_read_only(entry_handler& handler, i_entry& view);
        static void map_max_length(entry_handler& handler, i_entry& view);
        static void map_text_color(entry_handler& handler, i_entry& view);
        static void map_font(entry_handler& handler, i_entry& view);
        static void map_character_spacing(entry_handler& handler, i_entry& view);
        static void map_horizontal_text_alignment(entry_handler& handler, i_entry& view);
        static void map_vertical_text_alignment(entry_handler& handler, i_entry& view);
        static void map_is_text_prediction_enabled(entry_handler& handler, i_entry& view);
        static void map_is_spell_check_enabled(entry_handler& handler, i_entry& view);
        static void map_keyboard(entry_handler& handler, i_entry& view);
        static void map_return_type(entry_handler& handler, i_entry& view);
        static void map_clear_button_visibility(entry_handler& handler, i_entry& view);
        static void map_cursor_position(entry_handler& handler, i_entry& view);
        static void map_selection_length(entry_handler& handler, i_entry& view);
        // --- platform configuration (W2-24): the iOSSpecific Entry.CursorColor map (the Entry.iOS.cs
        // MapCursorColor / TextExtensions.UpdateCursorColor port; reads the i_ios_entry_specifics face).
        static void map_cursor_color(entry_handler& handler, i_entry& view);
        // The iOSSpecific Entry.AdjustsFontSizeToFitWidth map (Entry.iOS.cs MapAdjustsFontSizeToFitWidth /
        // TextExtensions.UpdateAdjustsFontSizeToFitWidth port; reads the i_ios_entry_specifics face and
        // pushes unconditionally — no IsSet guard).
        static void map_adjusts_font_size_to_fit_width(entry_handler& handler, i_entry& view);
    };
} // namespace maui::core
