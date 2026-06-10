// Tests for the M5c element lifecycle: typed inherited BindingContext (bindable_object + controls::element
// propagation) — and, below, the Window/Application lifecycle + Loaded/Unloaded. A plain view-model
// (shared_ptr-managed) stands in for the data context; real controls form the element tree. Window
// geometry, resume/sleep, and Application themes are tested at the bottom. (The window_handler SEAM tests
// live in window_handler_tests.cpp — swapped per backend with the window partial, the M6 convention.)
#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/i_application.hpp"
#include "maui/core/i_window.hpp"
#include "maui/graphics/rect.hpp"

#include <vector>

#include <cmath>
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

    TEST(binding_context, detaching_a_child_clears_its_inherited_context)
    {
        // Element.SetParent(null) calls SetInheritedBindingContext(this, null): removing a child
        // CLEARS its inherited context (an explicitly-set one survives the guard). Corrected in W1-02
        // from an earlier mischaracterization — RelativeSourceBindingTests.cs pins this behavior.
        maui::controls::content_page page;
        maui::controls::button child;
        page.set_content(child);
        auto context = std::make_shared<person>();
        page.set_binding_context(context);
        EXPECT_EQ(child.binding_context<person>(), context);

        page.set_content(nullptr);                           // detach
        EXPECT_EQ(child.binding_context<person>(), nullptr); // inherited context cleared on detach
        page.set_binding_context(std::make_shared<person>(person{.name = "new"}));
        EXPECT_EQ(child.binding_context<person>(), nullptr); // no longer a child -> not re-propagated
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

    // ---- window geometry (Window.X/Y/Width/Height + FrameChanged + SizeChanged) ----

    TEST(window_geometry, defaults_are_unset)
    {
        const maui::controls::window win;
        EXPECT_TRUE(std::isnan(win.x()));
        EXPECT_TRUE(std::isnan(win.y()));
        EXPECT_TRUE(std::isnan(win.width()));
        EXPECT_TRUE(std::isnan(win.height()));
    }

    TEST(window_geometry, frame_changed_sets_all_four_and_raises_size_changed)
    {
        maui::controls::window win;
        int size_changed = 0;
        win.size_changed.connect([&size_changed] { ++size_changed; });

        win.frame_changed(maui::graphics::rect(10, 20, 300, 200));
        EXPECT_EQ(win.x(), 10);
        EXPECT_EQ(win.y(), 20);
        EXPECT_EQ(win.width(), 300);
        EXPECT_EQ(win.height(), 200);
        EXPECT_EQ(size_changed, 1); // the size changed from Unset -> 300x200
    }

    TEST(window_geometry, frame_changed_without_size_change_does_not_raise_size_changed)
    {
        maui::controls::window win;
        win.frame_changed(maui::graphics::rect(0, 0, 100, 100));
        int size_changed = 0;
        win.size_changed.connect([&size_changed] { ++size_changed; });

        win.frame_changed(maui::graphics::rect(50, 60, 100, 100)); // only the position moved
        EXPECT_EQ(win.x(), 50);
        EXPECT_EQ(win.y(), 60);
        EXPECT_EQ(size_changed, 0); // width/height unchanged -> no SizeChanged
    }

    TEST(window_geometry, a_manual_set_after_a_frame_changed_wins)
    {
        // BindableObject precedence: FrameChanged sets X/Y/Width/Height at the handler specificity; a later
        // developer set (Window.Width = …, at manual specificity) overrides AND removes the handler value
        // (bindable.handler_value_is_overridden_by_a_manual_set). So a developer can still pin the size after
        // the platform has reported a frame.
        maui::controls::window win;
        win.frame_changed(maui::graphics::rect(0, 0, 800, 480)); // platform frame report (handler)
        EXPECT_EQ(win.width(), 800);

        win.set_width(640); // manual set overrides the handler value
        EXPECT_EQ(win.width(), 640);
        EXPECT_EQ(win.height(), 480); // height kept the frame value (no manual override)
    }

    // ---- resume / sleep (IWindow.Resumed / Stopped / Backgrounding -> Application SendResume/SendSleep) ----

    TEST(window_lifecycle, resume_and_stop_route_to_the_application)
    {
        maui::controls::application app;
        maui::controls::window win;
        app.open_window(win);

        int app_resumed = 0;
        int app_slept = 0;
        app.resumed.connect([&app_resumed] { ++app_resumed; });
        app.stopped.connect([&app_slept] { ++app_slept; });
        int win_resumed = 0;
        int win_stopped = 0;
        int win_backgrounding = 0;
        win.resumed.connect([&win_resumed] { ++win_resumed; });
        win.stopped.connect([&win_stopped] { ++win_stopped; });
        win.backgrounding.connect([&win_backgrounding] { ++win_backgrounding; });

        win.send_resumed();
        win.send_stopped();
        win.send_backgrounding();

        EXPECT_EQ(win_resumed, 1);
        EXPECT_EQ(win_stopped, 1);
        EXPECT_EQ(win_backgrounding, 1);
        EXPECT_EQ(app_resumed, 1); // Window.SendResumed -> Application.SendResume
        EXPECT_EQ(app_slept, 1);   // Window.SendStopped -> Application.SendSleep
    }

    TEST(window_lifecycle, a_standalone_window_resume_is_a_no_op_without_an_application)
    {
        maui::controls::window win; // never opened by an application -> no resume hook
        int win_resumed = 0;
        win.resumed.connect([&win_resumed] { ++win_resumed; });
        win.send_resumed(); // must not crash (the hook is empty)
        EXPECT_EQ(win_resumed, 1);
    }

    // ---- Application themes (UserAppTheme / PlatformAppTheme / RequestedTheme / RequestedThemeChanged) ----

    TEST(application_theme, requested_theme_prefers_user_then_platform)
    {
        maui::controls::application app;
        EXPECT_EQ(app.requested_theme(), maui::core::app_theme::unspecified); // both unset

        app.set_platform_app_theme(maui::core::app_theme::dark);
        EXPECT_EQ(app.requested_theme(), maui::core::app_theme::dark); // falls back to platform

        app.set_user_app_theme(maui::core::app_theme::light);
        EXPECT_EQ(app.requested_theme(), maui::core::app_theme::light); // user overrides platform
    }

    TEST(application_theme, theme_changed_fires_once_on_a_real_change)
    {
        maui::controls::application app;
        int changed = 0;
        maui::core::app_theme observed = maui::core::app_theme::unspecified;
        app.requested_theme_changed.connect([&](maui::core::app_theme theme) {
            ++changed;
            observed = theme;
        });

        app.set_user_app_theme(maui::core::app_theme::dark);
        EXPECT_EQ(changed, 1);
        EXPECT_EQ(observed, maui::core::app_theme::dark);

        app.set_user_app_theme(maui::core::app_theme::dark); // same value -> no event
        EXPECT_EQ(changed, 1);

        app.set_user_app_theme(maui::core::app_theme::light); // a real change -> fires again
        EXPECT_EQ(changed, 2);
        EXPECT_EQ(observed, maui::core::app_theme::light);
    }

    // ---- i_application: the cross-platform Core contract (Microsoft.Maui.IApplication) an application
    // satisfies, used through the i_application/i_window faces (like a window through i_window). ----

    TEST(i_application_contract, windows_exposes_the_open_windows_as_i_window)
    {
        maui::controls::application app;
        maui::controls::window win;
        app.open_window(win);

        const maui::core::i_application& as_app = app; // reachable through the Core contract
        const std::vector<maui::core::i_window*> open = as_app.windows();
        ASSERT_EQ(open.size(), 1U);
        EXPECT_EQ(open.front(), static_cast<maui::core::i_window*>(&win));
        // The concrete face still works (the existing API is preserved).
        EXPECT_EQ(app.windows_typed().size(), 1U);
        EXPECT_EQ(app.main_window(), &win);
    }

    TEST(i_application_contract, open_and_close_through_the_i_window_face)
    {
        maui::controls::application app;
        maui::controls::content_page page;
        maui::controls::window win;
        win.set_content(page);

        maui::core::i_application& as_app = app;
        maui::core::i_window& as_window = win;
        as_app.open_window(as_window); // the i_application override down-casts to the concrete window
        EXPECT_TRUE(page.has_appeared());
        EXPECT_EQ(as_app.windows().size(), 1U);

        as_app.close_window(as_window);
        EXPECT_FALSE(page.has_appeared());
        EXPECT_TRUE(as_app.windows().empty());
    }

    TEST(i_application_contract, theme_changed_fires_requested_theme_changed_on_a_real_platform_change)
    {
        // C# IApplication.ThemeChanged() re-reads the OS theme into PlatformAppTheme. The port pushes the OS
        // theme via set_platform_app_theme first; theme_changed() then re-triggers (already covered by
        // set_platform_app_theme, so theme_changed() on top of an unchanged platform theme is a no-op).
        maui::controls::application app;
        int changed = 0;
        app.requested_theme_changed.connect([&](maui::core::app_theme) { ++changed; });

        app.set_platform_app_theme(maui::core::app_theme::dark); // a real change -> fires once
        EXPECT_EQ(changed, 1);

        maui::core::i_application& as_app = app;
        as_app.theme_changed(); // re-notify with no change -> guarded, no extra fire
        EXPECT_EQ(changed, 1);
        EXPECT_EQ(as_app.user_app_theme(), maui::core::app_theme::unspecified);
    }

    TEST(i_application_contract, base_create_window_returns_null_and_it_is_a_rootless_element)
    {
        maui::controls::application app;
        maui::core::i_application& as_app = app;
        EXPECT_EQ(as_app.create_window(), nullptr); // the base app manages no window of its own
        // An application is an i_element root: no handler, no parent (the windows are its children).
        EXPECT_EQ(as_app.handler(), nullptr);
        EXPECT_EQ(as_app.parent(), nullptr);
        as_app.set_handler(nullptr); // no-op, stays null
        EXPECT_EQ(as_app.handler(), nullptr);
    }
} // namespace
