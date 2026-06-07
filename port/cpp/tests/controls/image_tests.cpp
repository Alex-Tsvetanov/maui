// Tests for the image control + its headless handler seam — a display-only control. Setting aspect (and a
// file source) flows virtual→native into the headless image_platform mirror (image_aspect / source_file +
// source_loaded). The source load is synchronous this cut.
#include "maui/controls/image.hpp"

#include <memory>

#include "maui/controls/file_image_source.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_handler.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image;
    using maui::controls::image_source;
    using maui::core::aspect;
    using maui::core::i_element_handler;
    using maui::core::i_file_image_source;
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
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<image>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<image_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        image control;
        control.set_aspect(aspect::fill);
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->image_aspect, aspect::fill);
    }

    // ---- source (file source, synchronous load; headless mirror) ----

    TEST(image, source_defaults_to_null_and_owns_a_set_source)
    {
        image control;
        EXPECT_EQ(control.source(), nullptr);

        control.set_source(image_source::from_file("/tmp/picture.png"));
        auto* src = control.source(); // raw borrow into the owned shared_ptr
        ASSERT_NE(src, nullptr);
        EXPECT_FALSE(src->is_empty());
        const auto* file_src = dynamic_cast<const i_file_image_source*>(src);
        ASSERT_NE(file_src, nullptr);
        EXPECT_EQ(file_src->file(), "/tmp/picture.png");
    }

    TEST(image, factory_makes_an_empty_source_for_an_empty_path)
    {
        auto src = image_source::from_file("");
        ASSERT_NE(src, nullptr);
        EXPECT_TRUE(src->is_empty());
    }

    TEST(image_seam, attaching_handler_loads_initial_source)
    {
        image control;
        control.set_source(image_source::from_file("/tmp/initial.png"));
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_file, "/tmp/initial.png");
    }

    TEST(image_seam, setting_file_source_maps_to_platform)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // No source yet -> nothing loaded.
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_TRUE(platform->source_file.empty());

        control.set_source(image_source::from_file("/tmp/a.png"));
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_file, "/tmp/a.png");

        // Replacing with a different source re-loads.
        control.set_source(image_source::from_file("/tmp/b.png"));
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_file, "/tmp/b.png");
    }

    TEST(image_seam, empty_source_does_not_load)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(image_source::from_file(""));
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_TRUE(platform->source_file.empty());
    }

    TEST(image_seam, clearing_source_back_to_null_clears_the_load)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(image_source::from_file("/tmp/c.png"));
        ASSERT_TRUE(platform->source_loaded);

        control.set_source(nullptr);
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_TRUE(platform->source_file.empty());
    }
} // namespace
