// font_image_source tests (headless). The font source carries a glyph + font + color; is_empty() tracks an
// empty glyph (C# FontImageSource.IsEmpty => string.IsNullOrEmpty(Glyph)). The font_image_source_service
// "rasterizes" it — on headless the result mirrors kind="font" + the glyph (the apple twin draws an
// NSImage; covered by font_image_source_apple_tests.mm). The image control loads a font source through the
// handler-owned loader, exactly like a uri/stream source. Ported in spirit from
// src/Core/tests/DeviceTests/Services/ImageSource/FontImageSourceServiceTests.* (the device-test oracle).
#include "maui/controls/font_image_source.hpp"

#include <cstddef>
#include <memory>
#include <string>

#include "maui/controls/file_image_source.hpp"
#include "maui/controls/image.hpp"
#include "maui/core/font.hpp"
#include "maui/core/font_image_source_service.hpp"
#include "maui/core/i_font_image_source.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/image_source_service_registry.hpp"
#include "maui/core/image_source_services.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::font_image_source;
    using maui::controls::image;
    using maui::controls::image_source;
    using maui::core::font;
    using maui::core::font_image_source_service;
    using maui::core::i_font_image_source;
    using maui::core::image_handler;
    using maui::core::manual_dispatcher;

    std::shared_ptr<maui::core::i_image_source> make_font_source(const std::string& glyph)
    {
        return image_source::from_font(glyph, font::of_size("Ionicons", font_image_source::default_size),
                                       maui::graphics::colors::red);
    }

    TEST(font_image_source, is_empty_tracks_the_glyph)
    {
        const font_image_source empty("", font::of_size("X", 30), maui::graphics::colors::black);
        EXPECT_TRUE(empty.is_empty());

        const font_image_source filled("A", font::of_size("X", 30), maui::graphics::colors::black);
        EXPECT_FALSE(filled.is_empty());
    }

    TEST(font_image_source, exposes_glyph_font_color)
    {
        const font_image_source src("Z", font::of_size("MyFamily", 24), maui::graphics::colors::blue);
        EXPECT_EQ(src.glyph(), "Z");
        EXPECT_EQ(src.font().family(), "MyFamily");
        EXPECT_EQ(src.font().size(), 24.0);
        const i_font_image_source& as_iface = src;
        EXPECT_EQ(as_iface.color(), maui::graphics::colors::blue);
    }

    TEST(font_image_source, factory_size_default_matches_csharp)
    {
        // image_source::from_font uses the caller-supplied font; the default_size constant is C#'s 30.
        EXPECT_EQ(font_image_source::default_size, 30.0);
    }

    // The registry resolves a font source to the font service (registered in the defaults).
    TEST(font_image_source, registry_resolves_font_source_to_font_service)
    {
        maui::core::image_source_service_registry registry;
        maui::core::register_default_image_source_services(registry);

        const auto src = make_font_source("A");
        const auto service = registry.resolve(*src);
        ASSERT_NE(service, nullptr);
        EXPECT_NE(dynamic_cast<font_image_source_service*>(service.get()), nullptr);
    }

    // The image control loads a font source through the handler-owned loader (async, pumped). The headless
    // mirror records kind="font" + the glyph as the detail.
    TEST(font_image_source, image_loads_font_source_through_loader)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        manual_dispatcher disp;
        control.set_handler(handler);
        handler->source_loader().set_dispatcher(disp);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(make_font_source("star"));
        EXPECT_FALSE(platform->source_loaded); // marshalled — nothing applied until pumped

        disp.run_pending();
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "font");
        EXPECT_EQ(platform->source_file, "star"); // detail = the glyph
    }

    // An empty-glyph font source renders nothing (IsEmpty short-circuits the load to a clear).
    TEST(font_image_source, empty_glyph_does_not_load)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        manual_dispatcher disp;
        control.set_handler(handler);
        handler->source_loader().set_dispatcher(disp);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(make_font_source(""));
        disp.run_pending();
        EXPECT_FALSE(platform->source_loaded);
    }
} // namespace
