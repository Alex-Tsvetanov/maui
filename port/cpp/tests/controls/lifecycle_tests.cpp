// Tests for the M5c element lifecycle: typed inherited BindingContext (bindable_object + controls::element
// propagation) — and, below, the Window/Application lifecycle + Loaded/Unloaded. A plain view-model
// (shared_ptr-managed) stands in for the data context; real controls form the element tree.
#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"

#include <memory>
#include <string>

#include <gtest/gtest.h>

namespace
{
    struct person
    {
        std::string name;
    };

    TEST(binding_context, inherits_from_parent_to_content)
    {
        maui::controls::content_page page;
        maui::controls::button child;
        page.set_content(child);

        auto context = std::make_shared<person>(person{.name = "Ada"});
        page.set_binding_context(context);
        EXPECT_EQ(child.binding_context<person>(), context); // inherited down to the content
    }

    TEST(binding_context, propagates_to_a_child_added_after_the_context_is_set)
    {
        maui::controls::content_page page;
        auto context = std::make_shared<person>();
        page.set_binding_context(context);

        maui::controls::button child;
        page.set_content(child); // attached AFTER the context is set -> attach_logical_child propagates
        EXPECT_EQ(child.binding_context<person>(), context);
    }

    TEST(binding_context, an_explicit_child_context_is_not_overridden_by_inheritance)
    {
        maui::controls::content_page page;
        maui::controls::button child;
        auto child_context = std::make_shared<person>(person{.name = "child"});
        child.set_binding_context(child_context); // explicitly set on the child
        page.set_content(child);

        page.set_binding_context(std::make_shared<person>(person{.name = "page"}));
        EXPECT_EQ(child.binding_context<person>(), child_context); // keeps its explicit context
    }

    TEST(binding_context, inherits_through_a_layout_to_grandchildren)
    {
        maui::controls::content_page page;
        maui::controls::vertical_stack_layout stack;
        maui::controls::button leaf;
        stack.add(leaf);
        page.set_content(stack);

        auto context = std::make_shared<person>();
        page.set_binding_context(context);
        EXPECT_EQ(stack.binding_context<person>(), context); // page -> layout
        EXPECT_EQ(leaf.binding_context<person>(), context);  // layout -> leaf (two levels deep)
    }

    TEST(binding_context, detaching_a_child_clears_its_inherited_window_but_keeps_context)
    {
        // Removing a child from its parent stops further inheritance; the already-inherited context value
        // remains until something replaces it (matching C#, where SetInheritedBindingContext is one-way).
        maui::controls::content_page page;
        maui::controls::button child;
        page.set_content(child);
        auto context = std::make_shared<person>();
        page.set_binding_context(context);
        EXPECT_EQ(child.binding_context<person>(), context);

        page.set_content(nullptr); // detach
        page.set_binding_context(std::make_shared<person>(person{.name = "new"}));
        EXPECT_EQ(child.binding_context<person>(), context); // no longer a child -> not re-propagated
    }

    TEST(binding_context, typed_getter_returns_null_for_a_mismatched_type)
    {
        maui::controls::button control;
        auto context = std::make_shared<person>();
        control.set_binding_context(context);
        EXPECT_EQ(control.binding_context<person>(), context);
        EXPECT_EQ(control.binding_context<int>(), nullptr); // the type_tag guards the unchecked cast
    }

    // ---- Window + Application lifecycle, Loaded/Unloaded ----

    TEST(window, activating_appears_and_loads_the_page)
    {
        maui::controls::content_page page;
        maui::controls::window win;
        win.set_content(page);

        int loaded = 0;
        page.loaded.connect([&loaded] { ++loaded; });

        EXPECT_FALSE(page.has_appeared());
        win.send_created();
        win.send_activated();

        EXPECT_TRUE(win.is_activated());
        EXPECT_TRUE(page.has_appeared());          // window-rooted Appearing fired
        EXPECT_EQ(loaded, 1);                      // Loaded fired as the page entered the window
        EXPECT_EQ(page.containing_window(), &win); // the page now knows its window
    }

    TEST(window, deactivating_disappears_and_unloads_the_page)
    {
        maui::controls::content_page page;
        maui::controls::window win;
        win.set_content(page);
        int unloaded = 0;
        page.unloaded.connect([&unloaded] { ++unloaded; });

        win.send_created();
        win.send_activated();
        ASSERT_TRUE(page.has_appeared());

        win.send_deactivated();
        EXPECT_FALSE(win.is_activated());
        EXPECT_FALSE(page.has_appeared()); // Disappearing fired
        EXPECT_EQ(unloaded, 1);            // Unloaded fired as the page left the window
        EXPECT_EQ(page.containing_window(), nullptr);
    }

    TEST(window, propagates_loaded_and_the_window_down_the_subtree)
    {
        maui::controls::content_page page;
        maui::controls::button child;
        page.set_content(child);
        maui::controls::window win;
        win.set_content(page);

        int child_loaded = 0;
        child.loaded.connect([&child_loaded] { ++child_loaded; });

        win.send_created();
        win.send_activated();
        EXPECT_EQ(child.containing_window(), &win); // the window reached the grandchild
        EXPECT_EQ(child_loaded, 1);                 // Loaded propagated down the tree
    }

    TEST(window, content_inherits_the_window_binding_context)
    {
        maui::controls::content_page page;
        maui::controls::window win;
        win.set_content(page);

        auto context = std::make_shared<person>();
        win.set_binding_context(context);
        EXPECT_EQ(page.binding_context<person>(), context); // window -> page
    }

    TEST(application, open_window_starts_activates_and_appears)
    {
        maui::controls::application app;
        maui::controls::content_page page;
        maui::controls::window win;
        win.set_content(page);

        int started = 0;
        app.started.connect([&started] { ++started; });

        app.open_window(win);
        EXPECT_EQ(started, 1);
        EXPECT_TRUE(win.is_created());
        EXPECT_TRUE(win.is_activated());
        EXPECT_TRUE(page.has_appeared());
        EXPECT_EQ(app.main_window(), &win);
        EXPECT_EQ(page.containing_window(), &win);

        app.open_window(win); // already open -> no second start
        EXPECT_EQ(started, 1);
    }

    TEST(application, binding_context_inherits_through_window_to_page)
    {
        maui::controls::application app;
        maui::controls::content_page page;
        maui::controls::window win;
        win.set_content(page);

        auto context = std::make_shared<person>();
        app.set_binding_context(context);
        app.open_window(win);

        EXPECT_EQ(win.binding_context<person>(), context);  // app -> window
        EXPECT_EQ(page.binding_context<person>(), context); // window -> page
    }

    TEST(application, close_window_destroys_and_drops_it)
    {
        maui::controls::application app;
        maui::controls::content_page page;
        maui::controls::window win;
        win.set_content(page);
        app.open_window(win);
        ASSERT_TRUE(page.has_appeared());

        app.close_window(win);
        EXPECT_FALSE(page.has_appeared());            // deactivation Disappeared the page
        EXPECT_EQ(page.containing_window(), nullptr); // and Unloaded it
        EXPECT_EQ(app.main_window(), nullptr);        // the window was dropped
    }
} // namespace
