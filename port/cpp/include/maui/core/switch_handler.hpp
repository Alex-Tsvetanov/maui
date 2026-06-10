#pragma once
// maui::core::switch_handler  <=  Microsoft.Maui.Handlers.SwitchHandler
//
// The handler for the two-state toggle (maui::controls::toggle_switch): IsOn / ThumbColor / TrackColor
// flow virtual→native through the mapper, and the native toggle flows native→virtual by writing
// i_switch::set_is_on (which the control turns into its `toggled` event). Ported from SwitchHandler.cs
// (cross-platform) + SwitchHandler.iOS.cs / SwitchExtensions.cs (the platform recipe; the AppKit
// backend translates the UISwitch recipe to NSSwitch).
//
// Same partial-class split + single cross-platform switch_platform struct as button_handler: the
// mapper TABLES and ctor are cross-platform (src/core/switch_handler.cpp); create / connect /
// disconnect / map_* / measure are per backend under src/platform/<backend>/switch_handler.{cpp,mm}.
//
// Not ported (deferred, documented): SwitchHandler.iOS's foreground/trait-change observers (UIKit-26
// theme-reset workarounds re-applying colors after lifecycle events) and the MACCATALYST notification
// dance — both are version-specific re-application timing, not mapping behavior.

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_switch.hpp"
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
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto
    // it (headless keeps the base mirrors; the Apple/iOS builds override update_* to push to the
    // NSSwitch/UISwitch).
    struct switch_platform : view_platform_base
    {
        switch_platform() = default;
        ~switch_platform() override; // backend-defined: releases the retained native switch on Apple/iOS
        switch_platform(const switch_platform&) = delete;
        switch_platform(switch_platform&&) = delete;
        switch_platform& operator=(const switch_platform&) = delete;
        switch_platform& operator=(switch_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple/iOS builds push to `native` instead). For
        // the headless backend `is_on` doubles as the NATIVE on/off state: a test simulates a user
        // toggle by flipping it and invoking on_value_changed (the UISwitch.ValueChanged analog).
        bool is_on = false;
        maui::graphics::color track_color;
        maui::graphics::color thumb_color;
        move_only_function<void()> on_value_changed;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSSwitch (defined in
        // src/platform/apple/switch_handler.mm). Same ODR note as button_platform: one backend per
        // build, identical class layout.
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
        // iOS backend: push the four fundamental IView properties to the UISwitch (defined in
        // src/platform/ios/switch_handler.mm). The remaining generic-IView pushes keep the
        // view_platform_base mirrors, matching the other ios platform structs (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class switch_handler : public view_handler<switch_handler, i_switch, switch_platform>
    {
    public:
        switch_handler();

        // Shared mapper tables (cross-platform — defined in src/core/switch_handler.cpp). `mapper`
        // chains the shared view_mapper, mirroring SwitchHandler.Mapper over ViewHandler.ViewMapper.
        static property_mapper<i_switch, switch_handler>& mapper();
        static command_mapper<i_switch, switch_handler>& command_mapper();

        // Platform recipe (defined per backend: src/platform/<backend>/switch_handler.{cpp,mm}).
        static std::unique_ptr<switch_platform> create_platform_view();
        void on_connect_handler(switch_platform& platform);
        static void on_disconnect_handler(switch_platform& platform);

        // i_view_handler measure/arrange seam (platform-specific sizing).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe). Each backend's map_is_on also re-runs the
        // track_color mapper (C# MapIsOn → UpdateIsOn(handler) → handler.UpdateValue(TrackColor): the
        // effective track color depends on the toggle state).
        static void map_is_on(switch_handler& handler, i_switch& view);
        static void map_track_color(switch_handler& handler, i_switch& view);
        static void map_thumb_color(switch_handler& handler, i_switch& view);
    };
} // namespace maui::core
