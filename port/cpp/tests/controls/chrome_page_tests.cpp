// Tests for the W1-11 chrome demo page wiring (src/samples/pages/chrome_page.hpp) — backend-agnostic:
// the toolbar/menu/context-flyout activations drive the readout, the page surfaces the chrome
// collections, and the tooltip/context flyout are attached. (The native materialization is covered by
// the chrome handler/backends tests; this exercises the sample's cross-platform wiring only.)
#include "src/samples/pages/chrome_page.hpp"

#include <optional>
#include <string>

#include "maui/controls/tool_tip_properties.hpp"
#include <gtest/gtest.h>

namespace
{
    TEST(chrome_page, surfaces_toolbar_and_menu_bar_items)
    {
        maui::samples::chrome_page sample;
        EXPECT_EQ(sample.page().toolbar_items().count(), 2U);
        EXPECT_EQ(sample.page().menu_bar_items().count(), 1U);
        EXPECT_EQ(sample.file_menu().item_count(), 4U); // New / Open / separator / Recent
    }

    TEST(chrome_page, toolbar_item_activation_drives_the_readout)
    {
        maui::samples::chrome_page sample;
        sample.save_item().activate();
        EXPECT_EQ(sample.readout().text(), "Last: Saved");
        sample.about_item().activate();
        EXPECT_EQ(sample.readout().text(), "Last: About");
        EXPECT_TRUE(sample.about_item().is_secondary());
    }

    TEST(chrome_page, menu_and_context_items_drive_the_readout)
    {
        maui::samples::chrome_page sample;
        sample.recent_first().activate();
        EXPECT_EQ(sample.readout().text(), "Last: File > Recent > First");

        ASSERT_EQ(sample.context_menu().item_count(), 3U); // Copy / separator / Paste
        sample.context_menu().item_at(0)->send_clicked();  // the native inbound channel
        EXPECT_EQ(sample.readout().text(), "Last: Copy");
    }

    TEST(chrome_page, button_carries_tooltip_and_context_flyout)
    {
        maui::samples::chrome_page sample;
        EXPECT_EQ(maui::controls::tool_tip_properties::get_text(sample.action_button()),
                  std::optional<std::string>("Press or right-click me"));
        EXPECT_EQ(sample.action_button().context_flyout(), &sample.context_menu());
    }
} // namespace
