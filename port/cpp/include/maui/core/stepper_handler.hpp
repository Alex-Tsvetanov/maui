#pragma once
// maui::core::stepper_handler  <=  Microsoft.Maui.Handlers.StepperHandler
//
// The handler for the minus/plus numeric stepper (maui::controls::stepper): Interval / Maximum /
// Minimum / Value flow virtual→native through the mapper, and the native step flows back through
// i_range::set_value. Ported from StepperHandler.cs (cross-platform) + StepperHandler.iOS.cs /
// StepperExtensions.cs (the platform recipe; the AppKit backend translates the UIStepper recipe to
// NSStepper). The ios partial also ports the iOS-26 boundary stepValue adjustment
// (AdjustStepValueForBoundaries — the port's simulator floor IS 26, where UIStepper stops clamping at
// the range edges; see src/platform/ios/stepper_handler.mm).
//
// Not ported (deferred, documented): the iOS/Catalyst FlowDirection mapper override (the iOS-26 RTL
// transform dance — the port's shared flow-direction push is the view_mapper's) and the iOS-26
// "Liquid Glass" landscape width compensation in GetDesiredSize (a cosmetic, empirically-measured
// overflow constant; the port keeps the native fitting size).

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_stepper.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto
    // it (headless keeps the base mirrors; Apple/iOS override update_* to push to the native stepper).
    struct stepper_platform : view_platform_base
    {
        stepper_platform() = default;
        ~stepper_platform() override; // backend-defined: releases the retained native stepper on Apple/iOS
        stepper_platform(const stepper_platform&) = delete;
        stepper_platform(stepper_platform&&) = delete;
        stepper_platform& operator=(const stepper_platform&) = delete;
        stepper_platform& operator=(stepper_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple/iOS builds push to `native` instead). For
        // the headless backend `value` doubles as the NATIVE stepper value: a test simulates a button
        // tap by stepping it and invoking on_value_changed (the UIStepper.ValueChanged analog).
        double minimum = 0;
        double maximum = 100;
        double increment = 1;
        double value = 0;
        move_only_function<void()> on_value_changed;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSStepper (defined in
        // src/platform/apple/stepper_handler.mm). Same ODR note as button_platform.
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
        // iOS backend: push the four fundamental IView properties to the UIStepper (defined in
        // src/platform/ios/stepper_handler.mm). The remaining generic-IView pushes keep the
        // view_platform_base mirrors, matching the other ios platform structs (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class stepper_handler : public view_handler<stepper_handler, i_stepper, stepper_platform>
    {
    public:
        stepper_handler();

        // Shared mapper tables (cross-platform — defined in src/core/stepper_handler.cpp). `mapper`
        // chains the shared view_mapper, mirroring StepperHandler.Mapper over ViewHandler.ViewMapper.
        static property_mapper<i_stepper, stepper_handler>& mapper();
        static command_mapper<i_stepper, stepper_handler>& command_mapper();

        // Platform recipe (defined per backend: src/platform/<backend>/stepper_handler.{cpp,mm}).
        static std::unique_ptr<stepper_platform> create_platform_view();
        void on_connect_handler(stepper_platform& platform);
        static void on_disconnect_handler(stepper_platform& platform);

        // i_view_handler measure/arrange seam (platform-specific sizing).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe). The "increment" key carries C#'s MapIncrement,
        // remapped from the Controls layer (Stepper.Mapper.cs routes the control's Increment change to
        // the IStepper.Interval mapping — the port keys the mapper on "increment" directly).
        static void map_increment(stepper_handler& handler, i_stepper& view);
        static void map_minimum(stepper_handler& handler, i_stepper& view);
        static void map_maximum(stepper_handler& handler, i_stepper& view);
        static void map_value(stepper_handler& handler, i_stepper& view);
    };
} // namespace maui::core
