// Tests for the shared ViewMapper (maui::core::view_mapper) on the headless backend — the generic IView
// properties (Visibility / Opacity / IsEnabled / AutomationId) flowing control -> handler ->
// view_platform_base mirrors. Exercised through both the button and the label control (each handler's
// platform view derives view_platform_base and the handler chains view_mapper into its own mapper).
#include "maui/controls/button.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

#include <memory>

#include "maui/core/button_handler.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::entry;
    using maui::controls::image;
    using maui::controls::label;
    using maui::controls::vertical_stack_layout;
    using maui::core::button_handler;
    using maui::core::entry_handler;
    using maui::core::image_handler;
    using maui::core::label_handler;
    using maui::core::layout_handler;
    using maui::core::view_platform_base;
    using maui::core::visibility;

    // ---- button: the four generic IView properties reach the platform base ----

    TEST(view_mapper_button, platform_base_is_available_after_attach)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        // button_platform derives view_platform_base, so the handler exposes a non-null platform base.
        EXPECT_NE(handler->platform_base(), nullptr);
    }

    TEST(view_mapper_button, defaults_map_on_connect)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        // VisualElement defaults: visible (hidden == false), opacity 1, enabled, empty automation id.
        EXPECT_FALSE(base->hidden);
        EXPECT_EQ(base->alpha, 1.0);
        EXPECT_TRUE(base->enabled);
        EXPECT_EQ(base->automation_id, "");
    }

    TEST(view_mapper_button, setting_visibility_maps_to_hidden)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(base->hidden);

        control.set_visibility(visibility::collapsed);
        EXPECT_TRUE(base->hidden); // collapsed also hides

        control.set_visibility(visibility::visible);
        EXPECT_FALSE(base->hidden);
    }

    TEST(view_mapper_button, setting_opacity_maps_to_alpha)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_opacity(0.25);
        EXPECT_EQ(base->alpha, 0.25);
    }

    TEST(view_mapper_button, opacity_is_clamped_to_unit_range)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        // VisualElement.OpacityProperty clamps to [0,1] (coerceValue).
        control.set_opacity(2.0);
        EXPECT_EQ(control.opacity(), 1.0);
        EXPECT_EQ(base->alpha, 1.0);

        control.set_opacity(-1.0);
        EXPECT_EQ(control.opacity(), 0.0);
        EXPECT_EQ(base->alpha, 0.0);
    }

    TEST(view_mapper_button, setting_is_enabled_maps_to_enabled)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_is_enabled(true);
        EXPECT_TRUE(base->enabled);
    }

    TEST(view_mapper_button, setting_automation_id_maps_to_mirror)
    {
        button control;
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_automation_id("submit_button");
        EXPECT_EQ(base->automation_id, "submit_button");
    }

    TEST(view_mapper_button, initial_values_map_on_attach)
    {
        // Values set BEFORE the handler is attached must be pushed when the mapper runs on connect.
        button control;
        control.set_visibility(visibility::collapsed);
        control.set_opacity(0.5);
        control.set_is_enabled(false);
        control.set_automation_id("preset");

        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);
        EXPECT_TRUE(base->hidden);
        EXPECT_EQ(base->alpha, 0.5);
        EXPECT_FALSE(base->enabled);
        EXPECT_EQ(base->automation_id, "preset");
    }

    // ---- label: the same generic properties reach its platform base (recipe generalizes) ----

    TEST(view_mapper_label, generic_view_properties_map_to_platform)
    {
        label control;
        auto handler = std::make_shared<label_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(base->hidden);

        control.set_opacity(0.75);
        EXPECT_EQ(base->alpha, 0.75);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_automation_id("caption");
        EXPECT_EQ(base->automation_id, "caption");
    }

    // ---- entry: the retrofit reaches the editable-field handler ----

    TEST(view_mapper_entry, generic_view_properties_map_to_platform)
    {
        entry control;
        auto handler = std::make_shared<entry_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(base->hidden);

        control.set_opacity(0.6);
        EXPECT_EQ(base->alpha, 0.6);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_automation_id("email_entry");
        EXPECT_EQ(base->automation_id, "email_entry");
    }

    // ---- image: the retrofit reaches the (minimal) image handler ----

    TEST(view_mapper_image, generic_view_properties_map_to_platform)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::collapsed);
        EXPECT_TRUE(base->hidden);

        control.set_opacity(0.3);
        EXPECT_EQ(base->alpha, 0.3);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_automation_id("avatar");
        EXPECT_EQ(base->automation_id, "avatar");
    }

    // ---- layout: the panel handler also gets the generic properties. is_enabled keeps the base mirror
    // (a plain NSView panel has no native enabled state); the headless mirror still records every value. ----

    TEST(view_mapper_layout, generic_view_properties_map_to_platform)
    {
        vertical_stack_layout control;
        auto handler = std::make_shared<layout_handler>();
        control.set_handler(handler);
        view_platform_base* base = handler->platform_base();
        ASSERT_NE(base, nullptr);

        control.set_visibility(visibility::hidden);
        EXPECT_TRUE(base->hidden);

        control.set_opacity(0.4);
        EXPECT_EQ(base->alpha, 0.4);

        control.set_is_enabled(false);
        EXPECT_FALSE(base->enabled);

        control.set_automation_id("form_stack");
        EXPECT_EQ(base->automation_id, "form_stack");
    }
} // namespace
