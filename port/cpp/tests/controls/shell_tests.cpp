// Ported from ShellTests.cs — the shell MODEL suite: item-tree structure, implicit wrapping, the
// CurrentItem selection chain, BindingContext propagation, route validation, SimpleGoTo/RelativeGoTo,
// the trim rules, the animation flag, and the flyout model state. Out of scope here (native chrome /
// reflection / modal — STATUS.md): flyout header/template projection, toolbar/title, Visual,
// MenuShellItem, TabBarIsVisible, window-title, and the hot-reload location restore.

#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/flyout_behavior.hpp"
#include "maui/controls/shell/flyout_display_options.hpp"
#include "maui/controls/shell/shell.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>

#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/core/event.hpp"
#include "shell_test_base.hpp"
#include <gtest/gtest.h>

namespace
{
    using namespace maui::controls;
    using namespace maui::controls::shell_tests;

    class shell_test : public shell_test_base
    {
    };

    TEST_F(shell_test, default_state)
    {
        shell const sh;
        EXPECT_TRUE(sh.items().empty());
        EXPECT_EQ(sh.current_item(), nullptr);
        EXPECT_EQ(sh.current_state(), nullptr);
        EXPECT_EQ(flyout_behavior::flyout, sh.get_flyout_behavior());
        EXPECT_FALSE(sh.flyout_is_presented());
    }

    TEST_F(shell_test, current_item_auto_sets)
    {
        shell sh;
        auto item = std::make_shared<shell_item>();
        auto section = std::make_shared<shell_section>();
        auto content = std::make_shared<shell_content>();
        std::shared_ptr<content_page> const page = make_page();
        content->set_content(page.get());
        section->add(content);
        item->add(section);
        sh.add_item(item);

        EXPECT_EQ(sh.current_item(), item.get());
    }

    TEST_F(shell_test, set_current_item_with_implicitly_wrapped_shell_content)
    {
        for (const bool use_shell_content : {true, false})
        {
            routing::clear();
            shell sh;
            sh.add_item(create_shell_item());

            std::shared_ptr<shell_content> content;
            std::shared_ptr<shell_section> section;
            base_shell_item const* shell_element = nullptr;

            if (use_shell_content)
            {
                content = create_shell_content(nullptr, false, "TestMe");
                shell_element = content.get();
                sh.add_item(content);
            }
            else
            {
                section = create_shell_section(nullptr, false, "", "TestMe");
                shell_element = section.get();
                sh.add_item(section);
            }

            shell_item const* item2 = sh.items()[1].get();
            EXPECT_EQ(shell_element->find_parent_shell(), &sh);

            if (use_shell_content)
            {
                sh.set_current_item(content);
            }
            else
            {
                sh.set_current_item(section);
            }

            EXPECT_EQ(2U, sh.items().size());
            EXPECT_EQ(item2, sh.current_item());
        }
    }

    TEST_F(shell_test, setting_current_item_on_shell_via_content_page)
    {
        std::shared_ptr<content_page> const page1 = make_page();
        std::shared_ptr<content_page> const page2 = make_page();
        test_shell sh;
        auto bar = std::make_shared<tab_bar>();
        auto content1 = std::make_shared<shell_content>();
        content1->set_content(page1.get());
        auto content2 = std::make_shared<shell_content>();
        content2->set_content(page2.get());
        bar->add(shell_section::create_from_shell_content(content1));
        bar->add(shell_section::create_from_shell_content(content2));
        sh.add_item(bar);

        sh.set_current_item(*page2);
        EXPECT_EQ(1U, sh.items().size());
        EXPECT_EQ(2U, sh.items()[0]->items().size());
        EXPECT_EQ(1U, sh.items()[0]->items()[0]->items().size());
        EXPECT_EQ(1U, sh.items()[0]->items()[1]->items().size());
        EXPECT_EQ(sh.current_item()->current_item(), sh.items()[0]->items()[1].get());
    }

    TEST_F(shell_test, set_current_item_adds_to_shell_collection)
    {
        shell sh;
        auto item = create_shell_item();
        auto section = create_shell_section();
        auto content = create_shell_content();

        sh.set_current_item(item);
        EXPECT_NE(std::ranges::find(sh.items(), item), sh.items().end());
        EXPECT_EQ(sh.current_item(), item.get());

        sh.set_current_item(section);
        EXPECT_EQ(sh.current_item(), section->logical_parent());

        sh.set_current_item(content);
        EXPECT_EQ(sh.current_item(), content->logical_parent()->logical_parent());
    }

    TEST_F(shell_test, current_item_does_not_change_on_second_add)
    {
        shell sh;
        auto item = create_shell_item();
        sh.add_item(item);

        EXPECT_EQ(sh.current_item(), item.get());

        sh.add_item(create_shell_item());

        EXPECT_EQ(item.get(), sh.current_item());
    }

    TEST_F(shell_test, shell_children_binding_context)
    {
        shell sh;
        auto item = create_shell_item();
        sh.add_item(item);

        auto view_model = std::make_shared<std::string>("vm");
        sh.set_binding_context(view_model);

        EXPECT_EQ(view_model, sh.binding_context<std::string>());
        EXPECT_EQ(view_model, item->binding_context<std::string>());
        EXPECT_EQ(view_model, item->items()[0]->binding_context<std::string>());
        EXPECT_EQ(view_model, item->items()[0]->items()[0]->binding_context<std::string>());
        ASSERT_NE(item->items()[0]->items()[0]->content(), nullptr);
        EXPECT_EQ(view_model, item->items()[0]->items()[0]->content()->binding_context<std::string>());
    }

    TEST_F(shell_test, shell_propagate_binding_context_when_adding_new_shell_item)
    {
        shell sh;
        sh.add_item(create_shell_item());

        auto view_model = std::make_shared<std::string>("vm");
        sh.set_binding_context(view_model);
        auto item = create_shell_item();
        sh.add_item(item);

        EXPECT_EQ(view_model, item->binding_context<std::string>());
        EXPECT_EQ(view_model, item->items()[0]->binding_context<std::string>());
        EXPECT_EQ(view_model, item->items()[0]->items()[0]->binding_context<std::string>());
        EXPECT_EQ(view_model, item->items()[0]->items()[0]->content()->binding_context<std::string>());
    }

    TEST_F(shell_test, shell_propagate_binding_context_when_adding_new_shell_content)
    {
        shell sh;
        sh.add_item(create_shell_item());

        auto view_model = std::make_shared<std::string>("vm");
        sh.set_binding_context(view_model);
        auto content = create_shell_content();
        sh.items()[0]->items()[0]->add(content);

        EXPECT_EQ(view_model, content->binding_context<std::string>());
        EXPECT_EQ(view_model, content->content()->binding_context<std::string>());
    }

    TEST_F(shell_test, shell_propagate_binding_context_when_changing_content)
    {
        shell sh;
        sh.add_item(create_shell_item());

        auto view_model = std::make_shared<std::string>("vm");
        sh.set_binding_context(view_model);
        std::shared_ptr<content_page> const page = make_page();

        sh.items()[0]->items()[0]->items()[0]->set_content(page.get());
        EXPECT_EQ(view_model, page->binding_context<std::string>());
    }

    TEST_F(shell_test, shell_propagate_binding_context_when_pushing_content)
    {
        shell sh;
        sh.add_item(create_shell_item());

        auto view_model = std::make_shared<std::string>("vm");
        sh.set_binding_context(view_model);
        std::shared_ptr<content_page> const page = make_page();
        sh.navigation_push(*page);

        EXPECT_EQ(view_model, page->binding_context<std::string>());
    }

    TEST_F(shell_test, simple_go_to)
    {
        shell sh;

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

        EXPECT_EQ("//one/tabone/content", location_of(sh));

        sh.go_to_async(shell_navigation_state{"//two/tabfour/"});

        EXPECT_EQ("//two/tabfour/content", location_of(sh));
    }

    TEST_F(shell_test, fail_when_adding_duplicated_routing)
    {
        routing::register_route<test_page1>("dogs");
        EXPECT_THROW(routing::register_route<test_page2>("dogs"), std::invalid_argument);
    }

    TEST_F(shell_test, succeed_when_adding_duplicate_route_of_same_type)
    {
        routing::register_route<test_page1>("dogs");
        EXPECT_NO_THROW(routing::register_route<test_page1>("dogs"));
    }

    TEST_F(shell_test, relative_go_to)
    {
        routing::register_route<content_page>("RelativeGoTo_Page1");
        routing::register_route<content_page>("RelativeGoTo_Page2");

        shell sh;

        auto one = std::make_shared<shell_item>();
        one->set_route("one");
        auto two = std::make_shared<shell_item>();
        two->set_route("two");

        one->add(make_simple_shell_section("tab11", "content"));
        one->add(make_simple_shell_section("tab12", "content"));
        two->add(make_simple_shell_section("tab21", "content"));
        two->add(make_simple_shell_section("tab22", "content"));
        two->add(make_simple_shell_section("tab23", "content"));

        sh.add_item(one);
        sh.add_item(two);

        sh.go_to_async(shell_navigation_state{"//two/tab21/"});

        sh.navigation_manager().go_to(shell_navigation_state{"/tab22"}, false, true);
        EXPECT_EQ("//two/tab22/content", location_of(sh));

        sh.navigation_manager().go_to(shell_navigation_state{"tab21"}, false, true);
        EXPECT_EQ("//two/tab21/content", location_of(sh));

        sh.navigation_manager().go_to(shell_navigation_state{"/tab23"}, false, true);
        EXPECT_EQ("//two/tab23/content", location_of(sh));

        sh.go_to_async(shell_navigation_state{"RelativeGoTo_Page1"}, false);
        EXPECT_EQ("//two/tab23/content/RelativeGoTo_Page1", location_of(sh));

        sh.go_to_async(shell_navigation_state{"../RelativeGoTo_Page2"}, false);
        EXPECT_EQ("//two/tab23/content/RelativeGoTo_Page2", location_of(sh));

        sh.go_to_async(shell_navigation_state{".."}, false);
        EXPECT_EQ("//two/tab23/content", location_of(sh));
    }

    TEST_F(shell_test, dot_dot_adheres_to_animation_parameter)
    {
        routing::register_route<content_page>("DotDotAdheresToAnimationParameter");
        test_shell sh;
        auto section = std::make_shared<test_shell_section>();
        section->add(create_shell_content());
        auto flyout = std::make_shared<flyout_item>();
        flyout->add(section);
        sh.add_item(flyout);

        sh.go_to_async(shell_navigation_state{"DotDotAdheresToAnimationParameter"});
        sh.go_to_async(shell_navigation_state{".."}, true);
        ASSERT_TRUE(section->last_pop_was_animated.has_value());
        EXPECT_TRUE(section->last_pop_was_animated.value_or(false));
    }

    TEST_F(shell_test, back_navigation_defaults_to_animated_when_not_specified)
    {
        routing::register_route<content_page>("BackNavigationDefaultsToAnimatedWhenNotSpecified");
        test_shell sh;
        auto section = std::make_shared<test_shell_section>();
        section->add(create_shell_content());
        auto flyout = std::make_shared<flyout_item>();
        flyout->add(section);
        sh.add_item(flyout);

        sh.go_to_async(shell_navigation_state{"BackNavigationDefaultsToAnimatedWhenNotSpecified"});
        sh.go_to_async(shell_navigation_state{".."});
        ASSERT_TRUE(section->last_pop_was_animated.has_value());
        EXPECT_TRUE(section->last_pop_was_animated.value_or(false));
    }

    TEST_F(shell_test, default_routes_maintained_if_thats_all_there_is)
    {
        routing::register_route<content_page>("DefaultRoutesMaintainedIfThatsAllThereIs");
        shell sh;
        auto content = create_shell_content();
        auto flyout = std::make_shared<flyout_item>();
        flyout->add(content);
        sh.add_item(flyout);

        sh.go_to_async(shell_navigation_state{"DefaultRoutesMaintainedIfThatsAllThereIs"});
        EXPECT_EQ(location_of(sh), "//" + routing::get_route(*content) + "/DefaultRoutesMaintainedIfThatsAllThereIs");
        EXPECT_NO_THROW(sh.go_to_async(shell_navigation_state{".."}));
    }

    TEST_F(shell_test, route_path_default_removal_with_global_routes_keeps_one_default_route)
    {
        shell sh;
        sh.add_item(create_shell_item());

        routing::register_route<content_page>("RoutePathDefaultRemovalKeepsOneDefaultRoute");
        sh.go_to_async(shell_navigation_state{"RoutePathDefaultRemovalKeepsOneDefaultRoute"});

        // With an all-default chain the location still records WHERE in the shell structure you are.
        EXPECT_NE("//RoutePathDefaultRemovalKeepsOneDefaultRoute", location_of(sh));
    }

    TEST_F(shell_test, route_path_default_removal_with_global_routes_keeps_one_named_route)
    {
        shell sh;
        sh.add_item(create_shell_item(nullptr, false, "content"));

        routing::register_route<content_page>("RoutePathDefaultRemovalKeepsOneNamedRoute");
        sh.go_to_async(shell_navigation_state{"RoutePathDefaultRemovalKeepsOneNamedRoute"});

        EXPECT_EQ("//content/RoutePathDefaultRemovalKeepsOneNamedRoute", location_of(sh));
    }

    TEST_F(shell_test, duplicate_sibling_routes_should_throw_argument_exception)
    {
        shell sh;
        sh.add_item(create_shell_item(nullptr, false, "", "", "duplicate"));
        auto second = create_shell_item();
        sh.add_item(second);

        EXPECT_THROW(second->set_route("duplicate"), std::invalid_argument);
    }

    TEST_F(shell_test, tab_auto_creation_wraps_a_tab_into_a_tab_bar)
    {
        // C# TabBarAutoCreation: adding a Tab-backed section implicitly wraps into a TabBar.
        shell sh;
        auto my_tab = std::make_shared<tab>();
        my_tab->add(create_shell_content());
        sh.add_item(my_tab);

        ASSERT_EQ(1U, sh.items().size());
        EXPECT_NE(dynamic_cast<tab_bar*>(sh.items()[0].get()), nullptr);
        EXPECT_EQ(sh.items()[0]->items()[0], my_tab);
    }

    TEST_F(shell_test, implicit_wrapping_syncs_titles)
    {
        // The CreateFromShellContent/CreateFromShellSection Title bindings, as the live sync.
        shell sh;
        auto content = create_shell_content();
        content->set_title("Initial");
        auto item = sh.add_item(content);

        EXPECT_EQ("Initial", item->title());
        EXPECT_EQ("Initial", item->items()[0]->title());

        content->set_title("Updated");
        EXPECT_EQ("Updated", item->items()[0]->title());
        EXPECT_EQ("Updated", item->title());
    }

    TEST_F(shell_test, adopted_page_title_syncs_into_shell_content)
    {
        // The `implicit operator ShellContent(TemplatedPage)` Title binding.
        std::shared_ptr<content_page> const page = make_page();
        page->set_title("PageTitle");
        auto content = shell_content::adopt(*page);
        EXPECT_EQ("PageTitle", content->title());
        EXPECT_TRUE(routing::is_implicit(*content));

        page->set_title("Renamed");
        EXPECT_EQ("Renamed", content->title());
    }

    TEST_F(shell_test, shell_content_with_template_creates_page_lazily)
    {
        // IShellContentController.GetOrCreateContent over ContentTemplate (W1-09 templates).
        std::shared_ptr<content_page> const page = make_page();
        auto content = create_shell_content(page, false, "templated", true);

        EXPECT_EQ(content->page(), nullptr); // nothing created yet
        content_page const* created = content->get_or_create_content();
        EXPECT_EQ(page.get(), created);
        EXPECT_EQ(page.get(), content->page()); // cached
        EXPECT_EQ(created, content->get_or_create_content());
    }

    TEST_F(shell_test, shell_content_without_content_throws_on_create)
    {
        auto content = std::make_shared<shell_content>();
        EXPECT_THROW((void)content->get_or_create_content(), std::runtime_error);
    }

    TEST_F(shell_test, navigated_fires_after_content_is_created_when_using_template)
    {
        // NavigatedFiresAfterContentIsCreatedWhenUsingTemplate: with a templated content the
        // Navigated event waits (the OnAppearing gate) until the page exists.
        std::shared_ptr<content_page> const page = make_page();
        test_shell sh;
        auto content = create_shell_content(page, false, "templated", true);
        auto section = std::make_shared<shell_section>();
        section->set_route("section");
        section->add(content);
        auto item = std::make_shared<shell_item>();
        item->set_route("item");
        item->add(section);
        sh.add_item(item);

        EXPECT_EQ(0, sh.navigated_count); // gated — no page yet
        EXPECT_EQ("//item/section/templated", location_of(sh));

        (void)content->get_or_create_content();
        content->send_appearing();
        EXPECT_EQ(1, sh.navigated_count);
    }

    TEST_F(shell_test, flyout_model_state_round_trips)
    {
        shell sh;
        sh.set_flyout_behavior(flyout_behavior::locked);
        EXPECT_EQ(flyout_behavior::locked, sh.get_flyout_behavior());
        EXPECT_EQ(flyout_behavior::locked, sh.effective_flyout_behavior());

        sh.set_flyout_is_presented(true);
        EXPECT_TRUE(sh.flyout_is_presented());
        sh.set_flyout_is_presented(false);
        EXPECT_FALSE(sh.flyout_is_presented());
    }

    TEST_F(shell_test, flyout_display_options_change_notifies_shell)
    {
        shell sh;
        auto flyout = create_shell_item<flyout_item>();
        sh.add_item(flyout);

        int changed = 0;
        auto token = maui::core::connect_scoped(sh.flyout_items_changed, [&changed] { ++changed; });
        flyout->set_flyout_display_options(flyout_display_options::as_multiple_items);
        EXPECT_EQ(1, changed);
        EXPECT_EQ(flyout_display_options::as_multiple_items, flyout->get_flyout_display_options());
    }

    TEST_F(shell_test, is_checked_follows_the_current_chain)
    {
        shell sh;
        auto one = create_shell_item(nullptr, false, "", "", "one");
        auto two = create_shell_item(nullptr, false, "", "", "two");
        sh.add_item(one);
        sh.add_item(two);

        EXPECT_TRUE(one->is_checked());
        EXPECT_TRUE(one->items()[0]->is_checked());
        EXPECT_FALSE(two->is_checked());

        sh.set_current_item(two);
        EXPECT_FALSE(one->is_checked());
        EXPECT_TRUE(two->is_checked());
        EXPECT_TRUE(two->items()[0]->items()[0]->is_checked());
    }

    TEST_F(shell_test, icon_seeds_flyout_icon_unless_set)
    {
        auto content = create_shell_content();
        auto icon = maui::controls::image_source::from_file("icon.png");
        content->set_icon(icon);
        EXPECT_EQ(icon, content->flyout_icon()); // OnIconChanged mirror

        auto explicit_icon = maui::controls::image_source::from_file("other.png");
        content->set_flyout_icon(explicit_icon);
        auto replacement = maui::controls::image_source::from_file("new.png");
        content->set_icon(replacement);
        EXPECT_EQ(explicit_icon, content->flyout_icon()); // an explicitly-set FlyoutIcon wins
    }
} // namespace
