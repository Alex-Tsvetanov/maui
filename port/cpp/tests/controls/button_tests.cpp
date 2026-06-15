// Tests for the button control (maui::controls::button) and the headless handler seam — the M2 Rosetta
// Stone, end to end on the headless backend: control -> handler -> platform (virtual→native Text) and
// platform -> handler -> control (native tap → clicked). The Apple backend (.mm) is the real-native
// twin verified separately; here the headless button_platform lets the seam be unit-tested.
#include "maui/controls/button.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/button_content_layout.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_text.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::button_content_layout;
    using maui::controls::image_source;
    using maui::core::button_handler;
    using maui::core::i_button;
    using maui::core::i_element_handler;
    using maui::core::i_text;

    // ---- the control in isolation (no handler) ----

    TEST(button, text_defaults_empty_and_is_settable)
    {
        button control;
        EXPECT_EQ(control.text(), "");
        control.set_text("Hello");
        EXPECT_EQ(control.text(), "Hello");
    }

    TEST(button, send_clicked_raises_clicked_event)
    {
        button control;
        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });
        control.send_clicked();
        EXPECT_EQ(clicks, 1);
    }

    TEST(button, clicked_runs_command_before_event)
    {
        button control;
        std::vector<int> order;
        control.command = [&order] { order.push_back(1); };
        control.clicked.connect([&order] { order.push_back(2); });
        control.send_clicked();
        EXPECT_EQ(order, (std::vector<int>{1, 2}));
    }

    TEST(button, disabled_button_suppresses_click)
    {
        button control;
        control.set_is_enabled(false);
        int clicks = 0;
        bool command_ran = false;
        control.clicked.connect([&clicks] { ++clicks; });
        control.command = [&command_ran] { command_ran = true; };
        control.send_clicked();
        EXPECT_EQ(clicks, 0);
        EXPECT_FALSE(command_ran);
    }

    TEST(button, pressed_then_released_tracks_is_pressed)
    {
        button control;
        int presses = 0;
        int releases = 0;
        control.pressed.connect([&presses] { ++presses; });
        control.released.connect([&releases] { ++releases; });

        control.send_pressed();
        EXPECT_TRUE(control.is_pressed());
        EXPECT_EQ(presses, 1);

        control.send_released();
        EXPECT_FALSE(control.is_pressed());
        EXPECT_EQ(releases, 1);
    }

    TEST(button, disabled_release_still_clears_is_pressed)
    {
        button control;
        control.send_pressed();
        EXPECT_TRUE(control.is_pressed());
        control.set_is_enabled(false);
        control.send_released(); // Released clears IsPressed even when disabled (ButtonElement)
        EXPECT_FALSE(control.is_pressed());
    }

    TEST(button, usable_through_interface_references)
    {
        button control;
        control.set_text("Tap");
        i_text& as_text = control;
        i_button& as_button = control;
        EXPECT_EQ(as_text.text(), "Tap");
        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });
        as_button.send_clicked();
        EXPECT_EQ(clicks, 1);
    }

    // ---- the handler seam (control <-> handler <-> headless platform) ----

    TEST(button_seam, attaching_handler_creates_platform_and_maps_initial_text)
    {
        button control;
        control.set_text("Start");
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &control);
        EXPECT_EQ(handler->typed_platform_view()->title, "Start"); // mapper ran on connect
    }

    TEST(button_seam, setting_text_updates_the_platform_view)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        control.set_text("Changed"); // -> on_property_changed -> handler.update_value("text") -> map_text
        EXPECT_EQ(handler->typed_platform_view()->title, "Changed");
    }

    TEST(button_seam, native_click_flows_back_to_clicked_event)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        int clicks = 0;
        control.clicked.connect([&clicks] { ++clicks; });
        // Simulate the native control firing its tap action.
        handler->typed_platform_view()->on_click();
        EXPECT_EQ(clicks, 1);
    }

    TEST(button_seam, native_press_and_release_flow_back)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        int presses = 0;
        int releases = 0;
        control.pressed.connect([&presses] { ++presses; });
        control.released.connect([&releases] { ++releases; });

        handler->typed_platform_view()->on_press();
        EXPECT_TRUE(control.is_pressed());
        EXPECT_EQ(presses, 1);

        handler->typed_platform_view()->on_release();
        EXPECT_FALSE(control.is_pressed());
        EXPECT_EQ(releases, 1);
    }

    TEST(button_seam, clearing_handler_disconnects)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        control.set_handler(nullptr);
        EXPECT_EQ(control.handler(), nullptr);
        EXPECT_EQ(handler->platform_view(), nullptr); // disconnected + torn down
        EXPECT_EQ(handler->virtual_view(), nullptr);
    }

    TEST(button_seam, appearance_properties_map_to_platform)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_text_color(maui::graphics::color(1.0F, 0.0F, 0.0F));
        EXPECT_EQ(platform->text_color, maui::graphics::color(1.0F, 0.0F, 0.0F));

        control.set_font(maui::core::font::of_size("Arial", 18));
        EXPECT_EQ(platform->text_font.family(), "Arial");
        EXPECT_EQ(platform->text_font.size(), 18.0);

        control.set_character_spacing(2.5);
        EXPECT_EQ(platform->character_spacing, 2.5);

        control.set_padding(maui::core::thickness(4));
        EXPECT_EQ(platform->padding, maui::core::thickness(4));

        control.set_stroke_color(maui::graphics::color(0.0F, 1.0F, 0.0F));
        EXPECT_EQ(platform->stroke_color, maui::graphics::color(0.0F, 1.0F, 0.0F));

        control.set_stroke_thickness(3.0);
        EXPECT_EQ(platform->stroke_thickness, 3.0);

        control.set_corner_radius(8);
        EXPECT_EQ(platform->corner_radius, 8);
    }

    TEST(button_seam, handler_resolved_from_default_registry)
    {
        // button -> button_handler is self-registered in button.cpp (MAUI_REGISTER_HANDLER).
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<button>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<button_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        button control;
        control.set_text("Registered");
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->title, "Registered");
    }

    // ---- image surface (Button.ImageSource + ContentLayout + the ImageButtonMapper) ----

    TEST(button, image_source_defaults_null_and_is_settable)
    {
        button control;
        EXPECT_EQ(control.image_source(), nullptr); // C# Button.ImageSource default: null

        auto source = image_source::from_file("Logo.png");
        control.set_image_source(source);
        EXPECT_EQ(control.image_source(), source.get());
    }

    TEST(button, content_layout_defaults_to_left_and_default_spacing)
    {
        button control;
        // C# Button.ContentLayoutProperty default: new ButtonContentLayout(ImagePosition.Left, DefaultSpacing).
        EXPECT_EQ(control.content_layout().position, button_content_layout::image_position::left);
        EXPECT_EQ(control.content_layout().spacing, button_content_layout::default_spacing);
    }

    TEST(button, content_layout_is_settable)
    {
        button control;
        control.set_content_layout(button_content_layout{button_content_layout::image_position::bottom, 4.0});
        EXPECT_EQ(control.content_layout().position, button_content_layout::image_position::bottom);
        EXPECT_EQ(control.content_layout().spacing, 4.0);
    }

    TEST(button, image_source_change_fires_once_per_distinct_instance)
    {
        button control;
        int source_changes = 0;
        control.property_changed.connect([&source_changes](std::string_view name) {
            if (name == "source")
            {
                ++source_changes;
            }
        });

        auto source = image_source::from_file("Logo.png");
        control.set_image_source(source);
        EXPECT_EQ(source_changes, 1);

        // Re-setting the SAME instance does not fire (value-precedence change detection).
        control.set_image_source(source);
        EXPECT_EQ(source_changes, 1);
    }

    TEST(button_seam, file_image_source_loads_synchronously_to_the_platform)
    {
        button control;
        control.set_image_source(image_source::from_file("Logo.png"));
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        // map_image_source's file fast-path records the headless mirrors (the image_handler convention).
        EXPECT_EQ(platform->source_kind, "file");
        EXPECT_EQ(platform->source_file, "Logo.png");
        EXPECT_TRUE(platform->source_loaded);
    }

    TEST(button_seam, setting_image_source_after_attach_maps_to_platform)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_FALSE(platform->source_loaded); // no source yet

        control.set_image_source(image_source::from_file("Later.png"));
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_file, "Later.png");
    }

    TEST(button_seam, clearing_image_source_clears_the_native_mirror)
    {
        button control;
        control.set_image_source(image_source::from_file("Logo.png"));
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_TRUE(platform->source_loaded);

        control.set_image_source(nullptr);
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "");
        EXPECT_EQ(platform->source_file, "");
    }

    TEST(button_seam, content_layout_is_pushed_to_the_handler)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        // The initial mapper pass on connect pushes ContentLayout once.
        const int initial_pushes = platform->content_layout_push_count;

        control.set_content_layout(button_content_layout{button_content_layout::image_position::right, 6.0});
        // A change re-runs map_content_layout (stored + pushed — the composition is deferred).
        EXPECT_EQ(platform->content_layout_push_count, initial_pushes + 1);
    }

    TEST(button_seam, finishing_a_load_re_pushes_content_layout)
    {
        // C# Button.IImageSourcePart.UpdateIsLoading: a load FINISH (was loading → not loading) re-pushes
        // ContentLayout so the text+image composition re-measures (deferred here; the push is the seam).
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        const int before = platform->content_layout_push_count;
        control.update_is_loading(true); // begin: no re-push
        EXPECT_EQ(platform->content_layout_push_count, before);
        control.update_is_loading(false); // finish: re-push ContentLayout
        EXPECT_EQ(platform->content_layout_push_count, before + 1);
    }

    TEST(button, update_is_loading_without_prior_loading_does_not_re_push)
    {
        // A standalone "not loading" (no prior loading) must NOT re-push (C#'s `_wasImageLoading` gate).
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        const int before = platform->content_layout_push_count;
        control.update_is_loading(false);
        EXPECT_EQ(platform->content_layout_push_count, before);
    }
} // namespace
