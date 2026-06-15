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
// NeedsContainer (SwitchHandler.NeedsContainer => true) IS ported: needs_container() returns true and
// the per-backend on_setup_container / on_remove_container wrap the native switch in a container view
// (the UISwitch >101pt accessibility workaround) — driven by the shared view_mapper's container_view map.
//
// SwitchHandler.iOS's SwitchProxy color-re-application observers ARE ported on the iOS backend (the
// UIKit-26 theme-reset workarounds): WillEnterForeground re-applies the OFF track color, and the
// iOS-26 trait-change registration re-applies the thumb color after a light/dark switch — both with
// the empirically-required 10ms main-queue settle. Not ported (deferred, documented): the MACCATALYST
// NSWindowDidBecomeKey notification dance (no macOS backend here yet).

#include <atomic>
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
        // C# NeedsContainer => true: the container view that wraps the native switch (the UISwitch >101pt
        // accessibility workaround — the natural-sized switch stays inside a sizable container). The Apple
        // (NSView) / iOS (UIView) builds retain a real wrapper here; released in on_remove_container / the
        // platform dtor. Headless has no native tree, so it has no wrapper (the handler's has_container /
        // container_view mirrors record the state instead).
        void* container = nullptr;

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

        // SwitchProxy's color-re-application observers (SwitchHandler.iOS.cs). Held as void* (retained)
        // so the cross-platform struct stays Obj-C-free, matching window_platform::notification_trampoline;
        // the .mm bridges via __bridge_retained / CFRelease. Both MUST be torn down in BOTH
        // on_disconnect_handler AND the dtor — a surviving observer fires into freed memory (UAF).
        //
        // The WillEnterForeground notification token (NSObject* from addObserverForName:…usingBlock:):
        // on app return-from-background the block re-applies the OFF track color (the UISwitch resets it).
        void* foreground_observer = nullptr;
        // The iOS-26 trait-change registration (id<UITraitChangeRegistration> from
        // registerForTraitChanges:withHandler:): on a light/dark change the block re-applies the thumb
        // color (UIKit 26 resets thumbTintColor when the interface style flips).
        void* trait_change_registration = nullptr;
        // Liveness flag for the deferred (10ms) re-apply blocks. SwitchProxy guards the post-delay body
        // with WeakReferences; the port has no shared_from_this on the handler, so the blocks capture a
        // copy of THIS flag (keeping the flag — never the handler — alive) and check it after the delay.
        // Teardown (on_disconnect_handler + dtor) sets it false, so a block already in flight when the
        // handler is destroyed bails BEFORE dereferencing the freed handler (the battery alive_ pattern).
        std::shared_ptr<std::atomic<bool>> reapply_alive = std::make_shared<std::atomic<bool>>(true);
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

        // C# SwitchHandler.NeedsContainer => true (always): the natural-sized native switch is wrapped in
        // a container so background/visual chrome can grow without the >101pt UISwitch accessibility bug.
        // The CRTP base's needs_container() override reads this opt-in constant (the view_mapper's
        // container_view map then drives the wrap). Cross-platform — the WRAP itself is per backend
        // (on_setup_container / on_remove_container).
        static constexpr bool needs_container_v = true;

        // Platform recipe (defined per backend: src/platform/<backend>/switch_handler.{cpp,mm}).
        static std::unique_ptr<switch_platform> create_platform_view();
        void on_connect_handler(switch_platform& platform);
        static void on_disconnect_handler(switch_platform& platform);

        // C# ViewHandler.SetupContainer / RemoveContainer (the iOS WrapperView swap): wrap the native
        // switch in a container view (or unwrap it), keeping the handler's container_view current. The
        // view_mapper's container_view map drives these via set_has_container(needs_container()). Defined
        // per backend (the Apple/iOS .mm build a real wrapper; headless has no native tree — it keeps the
        // base container mirrors only, so it declares no hook and the wrap is a recorded no-op there).
#if defined(MAUI_PLATFORM_APPLE) || defined(MAUI_PLATFORM_IOS)
        void on_setup_container();
        void on_remove_container();
#endif

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
