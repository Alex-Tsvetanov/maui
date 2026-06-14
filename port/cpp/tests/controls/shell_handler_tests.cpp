// shell_handler_tests — the W3-32 native shell chrome over the W2-21 model. HEADLESS structure mapping:
// the shell_handler resolves the shell model into a shell_render_tree (the tab host for the current item,
// one section renderer per visible section, each with a vc_stack of root + pushed pages, plus the flyout
// rows), and route navigation (go_to over the shell_navigation_manager) reconfigures that tree — the
// headless mirror of the iOS VC-stack reconfiguration the on-simulator e2e asserts.
//
// §8 teardown: the shell model (the publisher of on_property_changed → handler.update_value) is declared
// BEFORE the handler (the subscriber) in every test body, and the fixture-owned pages outlive both.

#include <memory>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell_handler.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "tests/controls/shell_test_base.hpp"
#include <gtest/gtest.h>

namespace maui::controls::shell_tests
{
    using maui::core::shell_handler;

    class shell_handler_test : public shell_test_base
    {
    protected:
        // Build the C# simple_go_to shell: two items (one/two), each with two sections (tab*), each with a
        // "content" shell_content over a fixture-owned page. Returns the shell BY VALUE is not possible
        // (non-copyable), so the caller declares it; this fills it. The model auto-selects //one/tabone.
        void build_two_item_shell(shell& sh)
        {
            auto one = std::make_shared<shell_item>();
            one->set_route("one");
            auto two = std::make_shared<shell_item>();
            two->set_route("two");

            one->add(make_simple_shell_section("tabone", "content"));
            one->add(make_simple_shell_section("tabtwo", "content"));
            two->add(make_simple_shell_section("tabthree", "content"));
            two->add(make_simple_shell_section("tabfour", "content"));

            sh.add_item(one);
            sh.add_item(two);
        }
    };

    // On connect, the handler resolves the model's CURRENT item into a tab host: one section renderer per
    // visible section, the selected_index at the current section, each section's vc_stack rooted at its
    // content page. This is the headless mirror of ShellItemRenderer's UITabBarController.
    TEST_F(shell_handler_test, connect_resolves_current_item_tab_host)
    {
        shell sh;
        build_two_item_shell(sh);
        ASSERT_EQ("//one/tabone/content", location_of(sh));

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        ASSERT_NE(handler->platform_view(), nullptr);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        const auto& item_renderer = platform->tree.current_item_renderer;
        EXPECT_EQ(item_renderer.item, sh.current_item());
        ASSERT_EQ(item_renderer.sections.size(), 2U); // tabone + tabtwo
        EXPECT_EQ(item_renderer.selected_index, 0);   // tabone is current
        EXPECT_EQ(item_renderer.sections[0].section, sh.current_section());
        // The current section's nav stack is just the root content page (no pushed pages yet).
        ASSERT_EQ(item_renderer.sections[0].vc_stack.size(), 1U);
        EXPECT_EQ(item_renderer.sections[0].vc_stack[0], sh.current_page());
        EXPECT_EQ(item_renderer.sections[0].root_page, sh.current_page());
    }

    // The flyout drawer rows mirror Shell.GetItems() (the visible items), one row per item, titled with
    // the item's route/title — the headless mirror of ShellFlyoutContentRenderer's table rows.
    TEST_F(shell_handler_test, connect_resolves_flyout_rows)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_EQ(platform->tree.flyout_rows.size(), 2U);
        EXPECT_EQ(platform->tree.flyout_rows[0].item, sh.items()[0].get());
        EXPECT_EQ(platform->tree.flyout_rows[1].item, sh.items()[1].get());
        // No template set → no per-row templated content (the native row falls back to a title label).
        EXPECT_EQ(platform->tree.flyout_rows[0].templated_content, nullptr);
    }

    // A flyout item template (W1-09 data_template) builds per-row content; setting it rebuilds the rows.
    TEST_F(shell_handler_test, flyout_item_template_builds_row_content)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        handler->set_flyout_item_template(data_template::of<content_page>());

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_EQ(platform->tree.flyout_rows.size(), 2U);
        EXPECT_NE(platform->tree.flyout_rows[0].templated_content, nullptr);
        EXPECT_NE(platform->tree.flyout_rows[1].templated_content, nullptr);
    }

    // THE E2E (headless mirror of the on-simulator route→VC-stack gate): go_to("//two/tabfour/")
    // reconfigures the model (current item → two, current section → tabfour), the model raises
    // on_property_changed → the handler rebuilds, and the resolved tab host now hosts item `two`'s sections
    // with tabfour selected. The active section + its vc_stack match the navigated model.
    TEST_F(shell_handler_test, go_to_route_reconfigures_tab_host)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_EQ(platform->tree.current_item_renderer.item, sh.items()[0].get()); // item one

        sh.go_to_async(shell_navigation_state{"//two/tabfour/"});
        ASSERT_EQ("//two/tabfour/content", location_of(sh));

        // The native container has been reconfigured to item `two` (tabthree + tabfour), tabfour selected.
        const auto& item_renderer = platform->tree.current_item_renderer;
        EXPECT_EQ(item_renderer.item, sh.items()[1].get()); // item two
        EXPECT_EQ(item_renderer.item, sh.current_item());
        ASSERT_EQ(item_renderer.sections.size(), 2U); // tabthree + tabfour
        EXPECT_EQ(item_renderer.selected_index, 1);   // tabfour is current
        EXPECT_EQ(item_renderer.sections[1].section, sh.current_section());
        EXPECT_EQ(item_renderer.sections[1].vc_stack[0], sh.current_page());
    }

    // Pushing a page (a route navigation that grows the current section stack) reconfigures the active
    // section's vc_stack: root content + the pushed page (stack[1..]). This is the per-section
    // UINavigationController stack the e2e asserts against.
    TEST_F(shell_handler_test, go_to_push_grows_active_section_vc_stack)
    {
        routing::register_route<content_page>("Details");

        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Push "Details" onto the current section (//one/tabone) — a route navigation.
        sh.go_to_async(shell_navigation_state{"Details"}, false);
        ASSERT_EQ("//one/tabone/content/Details", location_of(sh));

        const auto& item_renderer = platform->tree.current_item_renderer;
        ASSERT_GE(item_renderer.selected_index, 0);
        const auto& active = item_renderer.sections[static_cast<std::size_t>(item_renderer.selected_index)];
        // The active section's VC stack is now root content + the pushed Details page (2 entries).
        ASSERT_EQ(active.vc_stack.size(), 2U);
        EXPECT_EQ(active.vc_stack[0], active.root_page);
        EXPECT_EQ(active.vc_stack.back(), sh.current_page()); // the pushed page is the top VC
        // And it matches the model section stack (slot 0 root marker + the pushed page).
        EXPECT_EQ(sh.current_section()->stack().size(), 2U);
    }

    // FlyoutIsPresented is realized into the tree mirror (the drawer-open flag) through the mapper.
    TEST_F(shell_handler_test, flyout_is_presented_maps_to_tree)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_FALSE(platform->tree.flyout_presented);

        sh.set_flyout_is_presented(true);
        EXPECT_TRUE(platform->tree.flyout_presented);

        sh.set_flyout_is_presented(false);
        EXPECT_FALSE(platform->tree.flyout_presented);
    }

    // The "rebuild_shell" command re-resolves the tree (the control re-issues it after wiring — C#
    // ShellRenderer.SetupCurrentShellItem). Invoking it is idempotent and matches the current model.
    TEST_F(shell_handler_test, rebuild_command_reresolves_tree)
    {
        shell sh;
        build_two_item_shell(sh);

        auto handler = std::make_shared<shell_handler>();
        sh.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        sh.go_to_async(shell_navigation_state{"//two/tabthree/"});
        handler->invoke("rebuild_shell");

        const auto& item_renderer = platform->tree.current_item_renderer;
        EXPECT_EQ(item_renderer.item, sh.current_item());
        ASSERT_GE(item_renderer.selected_index, 0);
        EXPECT_EQ(item_renderer.sections[static_cast<std::size_t>(item_renderer.selected_index)].section,
                  sh.current_section());
    }
} // namespace maui::controls::shell_tests
