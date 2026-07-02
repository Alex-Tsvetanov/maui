// Tests for the image_button control + its headless handler seam, ported from
// src/Controls/tests/Core.UnitTests/ImageButtonUnitTest.cs (the clicked/pressed/released
// enabled-gating trio, TestSource/TestSourceDoubleSet, the IsLoading lifecycle, the PressedVisualState
// drive) plus the seam suite every control carries. The measure aspect-math tests lean on the
// Controls-layer legacy pipeline and stay un-ported (the handler's measure is authoritative — the image
// control's documented scope).
#include "maui/controls/image_button.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <cstddef>

#include "maui/controls/file_image_source.hpp"
#include "maui/controls/setter.hpp"
#include "maui/controls/view.hpp"
#include "maui/controls/visual_state_manager.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/image_button_handler.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image_button;
    using maui::controls::image_source;
    using maui::core::aspect;
    using maui::core::cancellation_token;
    using maui::core::i_element_handler;
    using maui::core::image_button_handler;
    using maui::core::image_bytes;
    using maui::core::manual_dispatcher;

    // A stream source that yields a fixed number of bytes (the image_tests convention — the byte COUNT
    // distinguishes which source applied via the "<bytes:N>" detail; the value is irrelevant headless).
    std::shared_ptr<maui::core::i_image_source> make_stream_source(std::size_t byte_count)
    {
        return image_source::from_stream(
            [byte_count](const cancellation_token&) { return image_bytes(byte_count, std::byte{0x7F}); });
    }

    // image_button pins IImageSourcePart.IsAnimationPlaying to false (C# constant). To exercise the
    // handler's post-load re-assertion in isolation, this override forces the flag true so the mirror
    // observably flips when map_source re-pushes map_is_animation_playing after the async load — the
    // headless analog of apple's animation_flag_set_before_load_plays_once_the_image_arrives.
    class animating_image_button : public image_button
    {
    public:
        [[nodiscard]] bool is_animation_playing() const override
        {
            return true;
        }
    };

    // ---- the control in isolation ----

    TEST(image_button, defaults_mirror_image_element)
    {
        image_button control;
        EXPECT_EQ(control.aspect(), aspect::aspect_fit);
        EXPECT_EQ(control.source(), nullptr); // ImageButtonUnitTest.TestSource: initially null
        EXPECT_FALSE(control.is_opaque());
        EXPECT_FALSE(control.is_loading());
        EXPECT_FALSE(control.is_pressed());
        EXPECT_FALSE(control.is_animation_playing()); // pinned false (IImageSourcePart explicit impl)
        EXPECT_EQ(control.stroke_thickness(), 0.0);
        // C# ImageButton.DefaultCornerRadius = -1 (the keep-native-default sentinel; the old 0 here
        // encoded a port bug that squared native-rounded buttons — caught by the Windows parity pass).
        EXPECT_EQ(control.corner_radius(), -1);
    }

    // ImageButtonUnitTest.TestSource + TestSourceDoubleSet.
    TEST(image_button, source_set_and_double_set)
    {
        image_button control;
        int source_changes = 0;
        control.property_changed.connect([&source_changes](std::string_view name) {
            if (name == "source")
            {
                ++source_changes;
            }
        });

        auto source = image_source::from_file("File.png");
        control.set_source(source);
        EXPECT_EQ(control.source(), source.get());
        EXPECT_EQ(source_changes, 1);

        // Re-setting the same instance is a no-op (TestSourceDoubleSet).
        control.set_source(source);
        EXPECT_EQ(source_changes, 1);
    }

    // ImageButtonUnitTest.TestClickedvent: clicked fires only while enabled.
    TEST(image_button, clicked_gated_on_is_enabled)
    {
        for (const bool enabled : {true, false})
        {
            image_button control;
            control.set_is_enabled(enabled);
            bool activated = false;
            control.clicked.connect([&activated] { activated = true; });
            control.send_clicked();
            EXPECT_EQ(activated, enabled);
        }
    }

    // ImageButtonUnitTest.TestPressedEvent.
    TEST(image_button, pressed_gated_on_is_enabled)
    {
        for (const bool enabled : {true, false})
        {
            image_button control;
            control.set_is_enabled(enabled);
            bool fired = false;
            control.pressed.connect([&fired] { fired = true; });
            control.send_pressed();
            EXPECT_EQ(fired, enabled);
            EXPECT_EQ(control.is_pressed(), enabled);
        }
    }

    // ImageButtonUnitTest.TestReleasedEvent: released ALWAYS fires, even disabled.
    TEST(image_button, released_always_fires)
    {
        for (const bool enabled : {true, false})
        {
            image_button control;
            control.set_is_enabled(enabled);
            bool fired = false;
            control.released.connect([&fired] { fired = true; });
            control.send_released();
            EXPECT_TRUE(fired);
            EXPECT_FALSE(control.is_pressed());
        }
    }

    // ImageButtonUnitTest.ButtonClickWhenCommandCanExecuteFalse collapses to the command stand-in: the
    // command runs before the event on a click.
    TEST(image_button, command_runs_before_clicked)
    {
        image_button control;
        std::vector<std::string> order;
        control.command = [&order] { order.emplace_back("command"); };
        control.clicked.connect([&order] { order.emplace_back("event"); });
        control.send_clicked();
        ASSERT_EQ(order.size(), 2U);
        EXPECT_EQ(order[0], "command");
        EXPECT_EQ(order[1], "event");
    }

    // ImageButtonUnitTest.PressedVisualState: pressing drives the Pressed VSM state; releasing leaves
    // it (back to Normal). Observed through a state setter on opacity.
    TEST(image_button, pressed_visual_state_drive)
    {
        image_button control;
        maui::controls::visual_state normal{std::string{maui::controls::common_states::normal}};
        normal.add(maui::controls::setter::of(maui::controls::opacity_property(), 1.0));
        maui::controls::visual_state pressed_state{std::string{maui::controls::common_states::pressed}};
        pressed_state.add(maui::controls::setter::of(maui::controls::opacity_property(), 0.5));
        maui::controls::visual_state_group group{"CommonStates"};
        group.add(std::move(normal));
        group.add(std::move(pressed_state));
        control.visual_states().add_group(std::move(group));
        control.change_visual_state();
        EXPECT_EQ(control.opacity(), 1.0);

        control.send_pressed();
        EXPECT_EQ(control.opacity(), 0.5); // the Pressed state applied

        control.send_released();
        EXPECT_EQ(control.opacity(), 1.0); // back to Normal
    }

    // The IsLoading lifecycle (TestImageSourceToNullCancelsLoading's observable surface): the loader
    // pushes update_is_loading and the read-only is_loading follows.
    TEST(image_button, update_is_loading_drives_read_only_state)
    {
        image_button control;
        EXPECT_FALSE(control.is_loading());
        control.update_is_loading(true);
        EXPECT_TRUE(control.is_loading());
        control.update_is_loading(false);
        EXPECT_FALSE(control.is_loading());
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(image_button_seam, attaching_handler_maps_initial_properties)
    {
        image_button control;
        control.set_source(image_source::from_file("File.png"));
        control.set_aspect(aspect::aspect_fill);
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        auto* platform = handler->typed_platform_view();
        EXPECT_EQ(platform->image_aspect, aspect::aspect_fill);
        EXPECT_EQ(platform->source_kind, "file");
        EXPECT_EQ(platform->source_file, "File.png");
        EXPECT_TRUE(platform->source_loaded);
    }

    TEST(image_button_seam, setting_properties_maps_to_platform)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_aspect(aspect::center);
        EXPECT_EQ(platform->image_aspect, aspect::center);

        control.set_is_opaque(true);
        EXPECT_TRUE(platform->opaque);

        control.set_padding(maui::core::thickness{4, 8, 4, 8});
        EXPECT_EQ(platform->padding.left, 4.0);
        EXPECT_EQ(platform->padding.top, 8.0);

        control.set_stroke_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(platform->stroke_color, maui::graphics::color(1.0F, 0.0F, 0.0F));

        control.set_stroke_thickness(2.0);
        EXPECT_EQ(platform->stroke_thickness, 2.0);

        control.set_corner_radius(6);
        EXPECT_EQ(platform->corner_radius, 6);
    }

    TEST(image_button_seam, clearing_source_clears_the_native_mirror)
    {
        image_button control;
        control.set_source(image_source::from_file("File.png"));
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_TRUE(platform->source_loaded);

        control.set_source(nullptr);
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "");
    }

    TEST(image_button_seam, simulated_native_tap_fires_released_then_clicked)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        std::vector<std::string> order;
        control.pressed.connect([&order] { order.emplace_back("pressed"); });
        control.released.connect([&order] { order.emplace_back("released"); });
        control.clicked.connect([&order] { order.emplace_back("clicked"); });

        platform->on_press();
        EXPECT_TRUE(control.is_pressed());
        platform->on_click();
        ASSERT_EQ(order.size(), 3U);
        EXPECT_EQ(order[0], "pressed");
        EXPECT_EQ(order[1], "released"); // Released precedes Clicked on touch-up-inside
        EXPECT_EQ(order[2], "clicked");
        EXPECT_FALSE(control.is_pressed());
    }

    TEST(image_button_seam, clearing_handler_disconnects)
    {
        image_button control;
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(image_button_seam, handler_resolved_from_default_registry)
    {
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<image_button>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<image_button_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        image_button control;
        control.set_source(image_source::from_file("Registered.png"));
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->source_file, "Registered.png");
    }

    // ---- IsAnimationPlaying re-asserted after an async source load (the inherited ImageHandler.MapSource
    // pipeline — ImageHandler.iOS.cs:68 / .Android.cs:73 → UpdateValue(IsAnimationPlaying)). Mirrors
    // image_tests' is_animation_playing_defaults_false_and_maps + the apple before-load re-assert. ----

    // The mapper carries the "is_animation_playing" key: mapping it pushes the view's flag to the mirror.
    TEST(image_button_seam, is_animation_playing_maps_to_platform)
    {
        animating_image_button control; // overrides the pinned-false flag to true
        auto handler = std::make_shared<image_button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        image_button_handler::map_is_animation_playing(*handler, control);
        EXPECT_TRUE(platform->animation_playing);
    }

    // A freshly-loaded animated source re-asserts the animation flag once the async load completes: the
    // mirror starts false and flips true only after the pumped apply re-pushes map_is_animation_playing
    // (the headless twin of apple's animation_flag_set_before_load_plays_once_the_image_arrives).
    TEST(image_button_seam, async_load_reasserts_is_animation_playing)
    {
        animating_image_button control;
        auto handler = std::make_shared<image_button_handler>();
        manual_dispatcher disp;
        control.set_handler(handler);
        handler->source_loader().set_dispatcher(disp);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Attaching the handler maps the initial properties, so the mirror already reflects the flag.
        // Reset it to false so the re-push during the load is the sole cause of the later flip back.
        platform->animation_playing = false;

        // Set an async (stream) source: the apply is marshalled onto the dispatcher, so nothing — neither
        // the image nor the re-asserted animation flag — applies until it is pumped.
        control.set_source(make_stream_source(4));
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_FALSE(platform->animation_playing);

        const std::size_t ran = disp.run_pending();
        EXPECT_GE(ran, 1U);
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_TRUE(platform->animation_playing); // re-pushed by map_source after apply_loaded_result
    }
} // namespace
