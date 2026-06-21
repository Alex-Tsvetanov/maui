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
        // Background IS pushed: VisualElement.Background paints the MauiCheckBox's layer via the shared
        // apply_background (mirroring the apple backend).
        void update_background(const maui::graphics::paint* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the MauiCheckBox (UIControl)'s layer (the shared
        // apply_and_store_clip; MauiCheckBox.layoutSubviews re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
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

        // Property map functions (platform recipe).
        static void map_is_checked(check_box_handler& handler, i_check_box& view);
        static void map_foreground(check_box_handler& handler, i_check_box& view);
    };
} // namespace maui::core
