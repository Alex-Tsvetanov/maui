// Tests for the image control + its headless handler seam — a minimal display-only control (aspect only;
// no image source this cut). Setting aspect flows virtual→native into the headless image_platform mirror.
#include "maui/controls/image.hpp"

#include <memory>

#include "maui/core/aspect.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/image_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image;
    using maui::core::aspect;
    using maui::core::i_element_handler;
    using maui::core::i_image;
    using maui::core::image_handler;

    TEST(image, aspect_defaults_to_aspect_fit_and_is_settable)
    {
        image control;
        EXPECT_EQ(control.aspect(), aspect::aspect_fit);
        control.set_aspect(aspect::fill);
        EXPECT_EQ(control.aspect(), aspect::fill);
    }

    TEST(image, usable_through_interface_reference)
    {
        image control;
        control.set_aspect(aspect::center);
        i_image& as_image = control;
        EXPECT_EQ(as_image.aspect(), aspect::center);
    }

    TEST(image_seam, attaching_handler_maps_initial_aspect)
    {
        image control;
        control.set_aspect(aspect::aspect_fill);
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &control);
        EXPECT_EQ(handler->typed_platform_view()->image_aspect, aspect::aspect_fill);
    }

    TEST(image_seam, setting_aspect_maps_to_platform)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_aspect(aspect::fill);
        EXPECT_EQ(platform->image_aspect, aspect::fill);

        control.set_aspect(aspect::center);
        EXPECT_EQ(platform->image_aspect, aspect::center);
    }

    TEST(image_seam, handler_resolved_from_default_registry)
    {
        // image -> image_handler is self-registered in image.cpp (MAUI_REGISTER_HANDLER).
        std::shared_ptr<i_element_handler> handler = maui::core::default_handler_registry().create_handler<image>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<image_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        image control;
        control.set_aspect(aspect::fill);
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->image_aspect, aspect::fill);
    }
} // namespace
