#pragma once
// maui::core::date_picker_handler  <=  Microsoft.Maui.Handlers.DatePickerHandler
//
// The handler for the date picker. Date/Minimum/Maximum/Format/appearance flow virtual→native
// through the property mapper; a native pick commits back through i_date_picker::set_date (the
// control's coercion clamps it). Ported from DatePickerHandler.cs (cross-platform) +
// DatePickerHandler.iOS.cs / Platform/iOS/MauiDatePicker.cs + DatePickerExtensions.cs (the
// UITextField-whose-inputView-is-a-UIDatePicker recipe — replicated 1:1 on the ios backend; the
// AppKit backend translates it idiomatically to NSDatePicker, deviations documented in the .mm).
//
// Partial-class split (PROFILE §5): mapper tables + ctor here/cpp; the platform recipe per backend
// under src/platform/<backend>/date_picker_handler.{cpp,mm}.
//
// date_picker_platform mirrors every mapped property for the headless backend (`native` holds the
// real backend view elsewhere). `date` is the native dialog's CURRENT value (UIDatePicker.Date always
// holds one — a null virtual Date falls back to Today, per DatePickerExtensions.UpdateDate); `text`
// is the formatted display string. `on_done` is the inbound channel: the headless stand-in for the
// Done-accessory tap that commits the dialog value (OnDoneClicked → SetVirtualViewDate); tests set
// `date` then invoke it.

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/date_time.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_date_picker.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

#ifdef MAUI_PLATFORM_ANDROID
namespace maui::platform::android
{
    // The click/dialog trampoline target the android partial owns (src/platform/android/
    // android_dialog_ops.hpp). Forward-declared: this cross-platform header must not see the JNI seam,
    // and a shared_ptr to an incomplete type is well-formed as long as it is only default-constructed
    // and destroyed here (the android partial, which sees the definition, does the rest).
    struct dialog_trampoline;
} // namespace maui::platform::android
#endif

namespace maui::core
{
    struct date_picker_platform : view_platform_base
    {
        date_picker_platform() = default;
        ~date_picker_platform() override; // backend-defined: releases the retained native view on Apple/iOS
        date_picker_platform(const date_picker_platform&) = delete;
        date_picker_platform(date_picker_platform&&) = delete;
        date_picker_platform& operator=(const date_picker_platform&) = delete;
        date_picker_platform& operator=(date_picker_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple/iOS builds write to `native` instead).
        date_time date;                        // the native dialog value (falls back to Today)
        std::optional<date_time> minimum_date; // UIDatePicker.MinimumDate (null = no bound)
        std::optional<date_time> maximum_date;
        std::string text; // the formatted display string (MauiDatePicker.Text)
        maui::graphics::color text_color;
        font text_font;
        double character_spacing = 0;

        // Inbound channel (wired by the platform partial; headless tests invoke it directly): commit
        // the dialog's current `date` — the Done-tap analog (OnDoneClicked → SetVirtualViewDate). The
        // Windows backend does NOT use this channel (its native CalendarDatePicker.DateChanged fires
        // with the new value directly, no separate "commit" step — see date_picker_handler.cpp), exactly
        // like picker_platform's on_done is unused by picker_handler.cpp's Windows SelectionChanged.
        move_only_function<void()> on_done;

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: the three event registration tokens on_connect_handler produces (Opened /
        // Closed / DateChanged — DatePickerHandler.Windows.cs's ConnectHandler), so on_disconnect_handler
        // can revoke EXACTLY what it registered. int64 (winrt::event_token's underlying type), not the
        // WinRT type itself — this cross-platform header must not see the C++/WinRT projection, matching
        // picker_platform's identical token fields.
        std::int64_t opened_token = 0;
        std::int64_t closed_token = 0;
        std::int64_t date_changed_token = 0;
#endif
#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: push the generic IView properties to the native CalendarDatePicker via the
        // shared winui_visual_ops helpers (src/platform/windows/), exactly like picker_platform's
        // identical block. Selected by MAUI_PLATFORM_WINDOWS, PUBLIC on maui_core for that backend only.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
#endif

#ifdef MAUI_PLATFORM_APPLE
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
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // BackgroundColor IS pushed to the MauiDatePicker (a RoundedRect UITextField): a solid fill goes to
        // the UIView backgroundColor property (flat fill, bezel suppressed, like MAUI); gradient/image use
        // the shared backing-layer helper. Mirrors the picker handler's update_background.
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the MauiIosDatePicker (UITextField)'s layer (the shared
        // apply_and_store_clip; MauiIosDatePicker.layoutSubviews re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend: a non-editable android.widget.EditText stand-in for MauiDatePicker (the field text
        // is the formatted Date; the DatePickerDialog is deferred). Defined in
        // src/platform/android/date_picker_handler.cpp; base body FIRST then widget push. IsEnabled IS pushed.
        // Clip IS pushed (wave 24): the generic-view clip rides a ViewOutlineProvider + setClipToOutline(true)
        // (android_clip_ops.hpp apply_outline_clip) — convex shapes (ellipse/rect/rounded-rect, the
        // clip_views page's shared EllipseGeometry) clip exactly; a non-convex path keeps the headless mirror
        // (the honest deferral documented there). update_clip stashes the borrow in clip_shape so
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
        // The android.app.DatePickerDialog MauiDatePicker's OnClick opens (DatePickerHandler.Android.cs's
        // _dialog), pinned as a JNI global reference while it is shown; nullptr when no dialog is up —
        // which is also ShowPickerDialog's "already showing" guard. Released (dismiss + DeleteGlobalRef)
        // by release_dialog_seam from BOTH on_disconnect_handler and ~date_picker_platform.
        void* dialog = nullptr;
        // The trampoline the click listener and the dialog's OnDateSet/OnDismiss listeners carry as their
        // peer. Heap-allocated and registry-registered so a late callback into a torn-down handler
        // resolves to nothing instead of dereferencing freed storage (android_dialog_ops.hpp's header).
        std::shared_ptr<maui::platform::android::dialog_trampoline> dialog_peer;
#endif
    };

    class date_picker_handler : public view_handler<date_picker_handler, i_date_picker, date_picker_platform>
    {
    public:
        date_picker_handler();

        static property_mapper<i_date_picker, date_picker_handler>& mapper();
        static command_mapper<i_date_picker, date_picker_handler>& command_mapper();

        static std::unique_ptr<date_picker_platform> create_platform_view();
        void on_connect_handler(date_picker_platform& platform);
        static void on_disconnect_handler(date_picker_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe). map_format shares map_date's body — C#'s
        // UpdateFormat routes straight into UpdateDate (a format change re-renders the text).
        static void map_format(date_picker_handler& handler, i_date_picker& view);
        static void map_date(date_picker_handler& handler, i_date_picker& view);
        static void map_minimum_date(date_picker_handler& handler, i_date_picker& view);
        static void map_maximum_date(date_picker_handler& handler, i_date_picker& view);
        static void map_text_color(date_picker_handler& handler, i_date_picker& view);
        static void map_font(date_picker_handler& handler, i_date_picker& view);
        static void map_character_spacing(date_picker_handler& handler, i_date_picker& view);
        // DatePickerHandler.MapIsOpen: become first responder when IsOpen, else resign.
        static void map_is_open(date_picker_handler& handler, i_date_picker& view);
    };
} // namespace maui::core
