#pragma once
// maui::core::check_box_handler  <=  Microsoft.Maui.Handlers.CheckBoxHandler
//
// The handler for the binary-choice check box (maui::controls::check_box): IsChecked / Foreground flow
// virtual→native through the mapper, and the native toggle flows native→virtual by writing
// i_check_box::set_is_checked (which the control turns into its `checked_changed` event). Ported from
// CheckBoxHandler.cs (cross-platform) + CheckBoxHandler.iOS.cs (the platform recipe — whose platform
// view is the DRAWN Platform/iOS/MauiCheckBox.cs, ported as an Obj-C UIButton subclass in the ios .mm;
// the AppKit backend uses the native NSButton checkbox style instead).
//
// Same partial-class split + single cross-platform check_box_platform struct as button_handler.

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_check_box.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto
    // it (headless keeps the base mirrors; Apple/iOS override update_* to push to the native control).
    struct check_box_platform : view_platform_base
    {
        check_box_platform() = default;
        ~check_box_platform() override; // backend-defined: releases the retained native control on Apple/iOS
        check_box_platform(const check_box_platform&) = delete;
        check_box_platform(check_box_platform&&) = delete;
        check_box_platform& operator=(const check_box_platform&) = delete;
        check_box_platform& operator=(check_box_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple/iOS builds push to `native` instead). For
        // the headless backend `is_checked` doubles as the NATIVE checked state: a test simulates a user
        // tap by flipping it and invoking on_checked_changed (the MauiCheckBox.CheckedChanged analog).
        // `foreground` is a NON-owning borrow of the paint the control owns (null = platform default).
        bool is_checked = false;
        const maui::graphics::paint* foreground = nullptr;
        move_only_function<void()> on_checked_changed;

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: Checked and Unchecked are two SEPARATE ToggleButton events (C#'s
        // ConnectHandler subscribes the SAME OnChecked body to both), so two revoke tokens are needed —
        // the same shape as date_picker_platform's opened_token/closed_token. Stored as int64
        // (winrt::event_token's underlying type) rather than the WinRT type so this cross-platform header
        // never has to see the C++/WinRT projection (matching button_platform's click_token).
        std::int64_t checked_token = 0;
        std::int64_t unchecked_token = 0;
#endif

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend: push the generic IView properties to the native CheckBox via the shared
        // winui_visual_ops helpers (src/platform/windows/) — same five-override shape as
        // button_platform/picker_platform. Selected by MAUI_PLATFORM_WINDOWS, which is PUBLIC on
        // maui_core for that backend only, so a given build sees exactly one backend's overrides.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
#endif

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSButton-checkbox (defined in
        // src/platform/apple/check_box_handler.mm). Same ODR note as button_platform.
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
        // iOS backend: push the four fundamental IView properties to the drawn MauiCheckBox (defined in
        // src/platform/ios/check_box_handler.mm). The remaining generic-IView pushes keep the
        // view_platform_base mirrors, matching the other ios platform structs (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // Background IS pushed: VisualElement.Background paints the MauiCheckBox's layer via the shared
        // apply_background (mirroring the apple backend).
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the MauiCheckBox (UIControl)'s layer (the shared
        // apply_and_store_clip; MauiCheckBox.layoutSubviews re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (M-android fan-out): push the generic IView properties to the real
        // android.widget.CheckBox over JNI (defined in src/platform/android/check_box_handler.cpp). Each
        // override calls the view_platform_base body FIRST (the VM-less cross-platform suite observes the
        // headless mirror) then pushes to the widget when one exists; transform / flow-direction /
        // semantics route through the shared android ops. Shadow / Clip / InputTransparent keep ONLY the
        // base mirror (WrapperView-only on Android). IsEnabled IS pushed (a CheckBox is interactive).
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

    class check_box_handler : public view_handler<check_box_handler, i_check_box, check_box_platform>
    {
    public:
        check_box_handler();

        // Shared mapper tables (cross-platform — defined in src/core/check_box_handler.cpp). `mapper`
        // chains the shared view_mapper, mirroring CheckBoxHandler.Mapper over ViewHandler.ViewMapper.
        static property_mapper<i_check_box, check_box_handler>& mapper();
        static command_mapper<i_check_box, check_box_handler>& command_mapper();

        // Platform recipe (defined per backend: src/platform/<backend>/check_box_handler.{cpp,mm}).
        static std::unique_ptr<check_box_platform> create_platform_view();
        void on_connect_handler(check_box_platform& platform);
        static void on_disconnect_handler(check_box_platform& platform);

        // i_view_handler measure/arrange seam. The iOS partial ports CheckBoxHandler.iOS's
        // GetDesiredSize minimum-size floor (MinimumSize 44pt when the constraint leaves it free).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

#ifdef MAUI_PLATFORM_WINDOWS
        // WinUI 3 backend ONLY: the measured size is a hard lower bound. A WinUI FrameworkElement measures
        // to max(MinWidth, Width), so the Fluent CheckBox style's own minimum outranks an explicit
        // WidthRequest — MEASURED on the ground truth (captures/windows/maui/border_playground_light.png:
        // that page's `WidthRequest="48"` checkbox occupies 120 DIP, the same 120 every unrequested
        // checkbox on the board takes). Without this opt-in view<>::measure applies the cross-platform
        // ResolveConstraints clamp and reports the 48, collapsing the row's gap to its label. This is the
        // "revisit with the capture as evidence" case windows/button_handler.cpp's own
        // content_is_minimum_size() note anticipates, and CheckBox's 120 IS that evidence. Button stays
        // false — no board page shows MAUI refusing to shrink a Windows Button, so whether its own style
        // carries a floor is untested either way; only CheckBox has a measurement and only CheckBox opts
        // in. Declared behind MAUI_PLATFORM_WINDOWS (PUBLIC on maui_core
        // for that backend only) so every TU of a given build sees one backend's overrides and the class
        // layout stays ODR-consistent — the same guard progress_bar_handler.hpp documents. No data fields.
        [[nodiscard]] bool content_is_minimum_size() const override;
#endif

        // Property map functions (platform recipe).
        static void map_is_checked(check_box_handler& handler, i_check_box& view);
        static void map_foreground(check_box_handler& handler, i_check_box& view);
    };
} // namespace maui::core
