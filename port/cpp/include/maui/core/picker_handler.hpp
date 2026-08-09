#pragma once
// maui::core::picker_handler  <=  Microsoft.Maui.Handlers.PickerHandler
//
// The handler for the item picker. Items/selection/title/appearance flow virtual→native through the
// property mapper (items travel through the i_item_delegate face — get_count/get_item — exactly like
// the C# platform recipes); a native row pick flows native→virtual via i_picker::set_selected_index
// (the control stores it at from_handler specificity and raises selected_index_changed). Ported from
// PickerHandler.cs (cross-platform) + PickerHandler.iOS.cs / Platform/iOS/MauiPicker.cs +
// PickerExtensions.cs (the UITextField-whose-inputView-is-a-UIPickerView recipe — replicated 1:1 on
// the ios backend; the AppKit backend translates it idiomatically to NSPopUpButton, deviations
// documented in the .mm).
//
// Partial-class split (PROFILE §5): mapper tables + ctor here/cpp; the platform recipe per backend
// under src/platform/<backend>/picker_handler.{cpp,mm}.
//
// picker_platform mirrors every mapped property for the headless backend (`native` holds the real
// backend view elsewhere). `text` is the MauiPicker.Text analog — the display string the recipe
// derives from the selection (GetItem(selectedIndex), or empty + the title placeholder at -1).
// `on_done` is the inbound channel: the headless stand-in for the Done-accessory tap that commits the
// native wheel row (FinishSelectItem); tests invoke it directly with the picked row.

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_picker.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/text_alignment.hpp"
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
    struct picker_platform : view_platform_base
    {
        picker_platform() = default;
        ~picker_platform() override; // backend-defined: releases the retained native view on Apple/iOS
        picker_platform(const picker_platform&) = delete;
        picker_platform(picker_platform&&) = delete;
        picker_platform& operator=(const picker_platform&) = delete;
        picker_platform& operator=(picker_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple/iOS builds write to `native` instead).
        std::vector<std::string> items;
        int selected_index = -1;
        std::string text; // the displayed selection (MauiPicker.Text)
        std::string title;
        maui::graphics::color title_color;
        maui::graphics::color text_color;
        font text_font;
        double character_spacing = 0;
        text_alignment horizontal_alignment = text_alignment::start;
        text_alignment vertical_alignment = text_alignment::center;

        // Inbound channel (wired by the platform partial; headless tests invoke it directly): commit
        // the native wheel's pending row — the Done-tap / popup-action analog (FinishSelectItem).
        move_only_function<void(int row)> on_done;

#ifdef MAUI_PLATFORM_IOS
        // MAC CATALYST ONLY (the slot is declared for the whole iOS backend because the struct layout
        // must not depend on TARGET_OS_MACCATALYST, which is a per-TU compiler define rather than a
        // build-wide one like MAUI_PLATFORM_*): the presented UIAlertController hosting the wheel.
        //
        // Catalyst does not present a UITextField's inputView — it has no software keyboard to present
        // one into — so PickerHandler.iOS.cs's `#else` arm gives the picker NO inputView and shows a
        // UIAlertController instead. This holds that controller so a disconnect can dismiss it; leaving
        // it up over a torn-down handler is a modal the user cannot dismiss and a dangling delegate.
        // Retained through the same __bridge_retained convention as `native`; nullptr on iOS proper.
        void* catalyst_controller = nullptr;
#endif

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: the three event registration tokens on_connect_handler produces
        // (SelectionChanged / DropDownOpened / DropDownClosed — PickerHandler.Windows.cs's
        // ConnectHandler), so on_disconnect_handler can revoke EXACTLY what it registered. Stored as
        // int64 (winrt::event_token's underlying type) rather than the WinRT type itself, like
        // button_platform's click_token — this cross-platform header must not see the C++/WinRT
        // projection.
        std::int64_t selection_changed_token = 0;
        std::int64_t drop_down_opened_token = 0;
        std::int64_t drop_down_closed_token = 0;
        // PickerHandler.Windows's UpdatingItemSource: true while map_items rebuilds the native item
        // list, so the SelectionChanged that rebuild triggers is NOT written back into the virtual
        // view's SelectedIndex (which would stomp the value the reload is about to re-push via
        // SetUpdatingItemSource(false)'s UpdateValue(SelectedIndex)).
        bool updating_item_source = false;
#endif

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: push the generic IView properties to the native element via the shared
        // winui_visual_ops helpers (src/platform/windows/). Selected by MAUI_PLATFORM_WINDOWS, which is
        // PUBLIC on maui_core for that backend only - so every TU of a given build sees exactly one
        // backend's overrides and the class layout stays ODR-consistent.
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
        // BackgroundColor IS pushed to the UITextField (it respects backgroundColor directly, unlike a
        // UIButton): the MauiPicker is a plain UIView, so the shared apply_background paints its layer.
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the MauiIosPicker (UITextField)'s layer (the shared
        // apply_and_store_clip; MauiIosPicker.layoutSubviews re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend: a non-editable android.widget.EditText stand-in for MauiPicker (the selected
        // item is the field text, the title is the hint). Generic IView pushes over JNI
        // (src/platform/android/picker_handler.cpp); base body FIRST then widget push. IsEnabled IS pushed
        // (interactive); shadow/clip/input_transparent keep ONLY the base mirror.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_semantics(const maui::core::semantics* value) override;
        // The single-choice android.app.AlertDialog the field's Click opens (PickerHandler.Android.cs's
        // _dialog), pinned as a JNI global reference while it is shown; nullptr when no dialog is up —
        // which is also OnClick's `_dialog == null` guard. Released (dismiss + DeleteGlobalRef) by
        // release_dialog_seam from BOTH on_disconnect_handler and ~picker_platform.
        void* dialog = nullptr;
        // The trampoline the click listener and the dialog's row/dismiss listeners carry as their peer.
        // Heap-allocated and registry-registered so a late callback into a torn-down handler resolves to
        // nothing instead of dereferencing freed storage (android_dialog_ops.hpp's header).
        std::shared_ptr<maui::platform::android::dialog_trampoline> dialog_peer;
#endif
    };

    class picker_handler : public view_handler<picker_handler, i_picker, picker_platform>
    {
    public:
        picker_handler();

        static property_mapper<i_picker, picker_handler>& mapper();
        static command_mapper<i_picker, picker_handler>& command_mapper();

        static std::unique_ptr<picker_platform> create_platform_view();
        void on_connect_handler(picker_platform& platform);
        static void on_disconnect_handler(picker_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

#ifdef MAUI_PLATFORM_IOS
        // MAC CATALYST ONLY (see picker_platform::catalyst_controller for why the declaration is not
        // itself guarded on TARGET_OS_MACCATALYST): present the UIAlertController that hosts the wheel.
        // The iOS build compiles this to a no-op, so the editing-began path can call it unconditionally.
        //
        // PUBLIC because the Obj-C editing proxy — a file-scope class in the .mm, not a friend — calls
        // it from EditingDidBegin, mirroring MauiPickerProxy.OnStarted's `#if MACCATALYST` tail.
        void present_catalyst_picker();
#endif

        // Property map functions (platform recipe). map_items is the C# Reload/UpdatePicker; the
        // selection map shares its body (both route through the UpdatePicker(picker, index) helper).
        static void map_items(picker_handler& handler, i_picker& view);
        static void map_selected_index(picker_handler& handler, i_picker& view);
        static void map_title(picker_handler& handler, i_picker& view);
        static void map_title_color(picker_handler& handler, i_picker& view);
        static void map_text_color(picker_handler& handler, i_picker& view);
        static void map_font(picker_handler& handler, i_picker& view);
        static void map_character_spacing(picker_handler& handler, i_picker& view);
        static void map_horizontal_text_alignment(picker_handler& handler, i_picker& view);
        static void map_vertical_text_alignment(picker_handler& handler, i_picker& view);
        // PickerHandler.MapIsOpen: become first responder when IsOpen, else resign (focus the native
        // field opens its inputView wheel; resigning dismisses it).
        static void map_is_open(picker_handler& handler, i_picker& view);
    };
} // namespace maui::core
