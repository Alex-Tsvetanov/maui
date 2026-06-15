// Tests for the swipe_item_menu_item_handler (the coloured menu-button handler the SwipeView reveals) and
// the SwipeViewExtensions.GetTextColor helper — ported from SwipeItemMenuItemHandler.iOS.cs +
// SwipeViewExtensions.cs. Backend-agnostic: they exercise the cross-platform contract + the mapper through
// the handler's observable mirror (swipe_item_menu_item_platform), which every backend records as it
// pushes each property to the native button (the headless mirror IS the recorded state; the iOS/apple
// twins additionally drive a real UI*Button).
//
// Coverage:
//   - the handler connects directly to a swipe_item (the standalone-handler seam — see the handler header).
//   - the mapper mirrors record Text / Background / Visibility off the control's bindable surface.
//   - MapTextColor records the luminosity-derived effective colour (GetTextColor): white on a dark
//     background, black on a light one, and absent when there is no background.
//   - MapSource records whether a non-empty icon source was set.
//   - MapFont / MapCharacterSpacing record the inert MenuItem.ITextStyle defaults.
#include "maui/core/swipe_item_menu_item_handler.hpp"

#include <memory>

#include "maui/controls/swipe_item.hpp"
#include "maui/core/i_swipe_item_menu_item.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::swipe_item;
    using maui::core::swipe_item_menu_item_handler;

    // Attach a freshly-built handler to the item and connect it (ElementHandler.SetVirtualView), returning
    // the handler so the test can read back the mirror. The item OUTLIVES the handler (declared first by
    // the caller).
    std::shared_ptr<swipe_item_menu_item_handler> connect_handler(swipe_item& item)
    {
        auto handler = std::make_shared<swipe_item_menu_item_handler>();
        handler->set_virtual_view(item);
        return handler;
    }

    TEST(swipe_item_menu_item, connecting_creates_the_platform_button_and_records_the_item)
    {
        swipe_item item;
        auto handler = connect_handler(item);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->hosted_item, dynamic_cast<maui::core::i_swipe_item_menu_item*>(&item));
    }

    TEST(swipe_item_menu_item, map_text_records_the_title) // C# MapText
    {
        swipe_item item;
        item.set_text("Delete");
        auto handler = connect_handler(item);
        EXPECT_EQ(handler->typed_platform_view()->title, "Delete");

        item.set_text("Archive");
        handler->update_value("text");
        EXPECT_EQ(handler->typed_platform_view()->title, "Archive");
    }

    TEST(swipe_item_menu_item, map_background_records_the_paint_colour) // C# MapBackground
    {
        swipe_item item;
        item.set_background_color(maui::graphics::colors::red);
        auto handler = connect_handler(item);
        auto* platform = handler->typed_platform_view();
        EXPECT_TRUE(platform->has_background);
        EXPECT_EQ(platform->background_argb, maui::graphics::colors::red.to_uint());
    }

    TEST(swipe_item_menu_item, map_background_absent_when_no_colour_set)
    {
        swipe_item item; // BackgroundColor default null → background() returns nullptr
        auto handler = connect_handler(item);
        EXPECT_FALSE(handler->typed_platform_view()->has_background);
    }

    TEST(swipe_item_menu_item, text_color_is_white_on_a_dark_background) // C# GetTextColor + MapTextColor
    {
        swipe_item item;
        item.set_background_color(maui::graphics::colors::red); // luminosity 0.2126 < 0.75 → white
        auto handler = connect_handler(item);
        auto* platform = handler->typed_platform_view();
        EXPECT_TRUE(platform->has_title_color);
        EXPECT_EQ(platform->title_color_argb, maui::graphics::colors::white.to_uint());
    }

    TEST(swipe_item_menu_item, text_color_is_black_on_a_light_background) // C# GetTextColor + MapTextColor
    {
        swipe_item item;
        item.set_background_color(maui::graphics::colors::white); // luminosity 1.0 >= 0.75 → black
        auto handler = connect_handler(item);
        auto* platform = handler->typed_platform_view();
        EXPECT_TRUE(platform->has_title_color);
        EXPECT_EQ(platform->title_color_argb, maui::graphics::colors::black.to_uint());
    }

    TEST(swipe_item_menu_item, text_color_absent_when_no_background) // C# GetTextColor null branch
    {
        swipe_item item; // no background → GetTextColor returns null → SetTitleColor skipped
        auto handler = connect_handler(item);
        EXPECT_FALSE(handler->typed_platform_view()->has_title_color);
    }

    TEST(swipe_item_menu_item, get_text_color_extension_matches_the_luminosity_rule)
    {
        swipe_item dark;
        dark.set_background_color(maui::graphics::colors::red);
        const auto dark_color = maui::core::get_text_color(dark);
        ASSERT_TRUE(dark_color.has_value());
        EXPECT_EQ(dark_color.value_or(maui::graphics::color{}).to_uint(), maui::graphics::colors::white.to_uint());

        swipe_item light;
        light.set_background_color(maui::graphics::colors::white);
        const auto light_color = maui::core::get_text_color(light);
        ASSERT_TRUE(light_color.has_value());
        EXPECT_EQ(light_color.value_or(maui::graphics::color{}).to_uint(), maui::graphics::colors::black.to_uint());

        swipe_item none;
        EXPECT_FALSE(maui::core::get_text_color(none).has_value());
    }

    TEST(swipe_item_menu_item, map_visibility_records_collapsed) // C# MapVisibility
    {
        swipe_item item;
        item.set_is_visible(false);
        auto handler = connect_handler(item);
        EXPECT_EQ(handler->typed_platform_view()->item_visibility, maui::core::visibility::collapsed);

        item.set_is_visible(true);
        handler->update_value("visibility");
        EXPECT_EQ(handler->typed_platform_view()->item_visibility, maui::core::visibility::visible);
    }

    TEST(swipe_item_menu_item, map_source_records_absent_when_no_icon) // C# MapSource
    {
        swipe_item item; // no IconImageSource set → source() is null
        auto handler = connect_handler(item);
        EXPECT_FALSE(handler->typed_platform_view()->has_source);
    }

    TEST(swipe_item_menu_item, font_and_character_spacing_are_the_menu_item_defaults)
    {
        swipe_item item;
        auto handler = connect_handler(item);
        auto* platform = handler->typed_platform_view();
        // C# MenuItem.ITextStyle: CharacterSpacing => 0, Font => Font.Default.
        EXPECT_EQ(platform->character_spacing, 0.0);
        EXPECT_TRUE(platform->item_font.is_default());
    }

    TEST(swipe_item_menu_item, disconnect_drops_the_references_idempotently)
    {
        swipe_item item;
        auto handler = connect_handler(item);
        ASSERT_NE(handler->item_view(), nullptr);

        handler->disconnect_handler();
        EXPECT_EQ(handler->item_view(), nullptr);
        EXPECT_EQ(handler->typed_platform_view(), nullptr); // the platform button was released

        handler->disconnect_handler(); // idempotent — no crash, still null
        EXPECT_EQ(handler->item_view(), nullptr);
    }
} // namespace
