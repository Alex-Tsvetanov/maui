// Tests for the Window modal-navigation events (G4): ModalPushing/Pushed/Popping/Popped + PopCanceled.
// Ported from the C# ModalNavigationManager → Window.OnModal* call chain (the ordering oracle:
// ModalNavigationManager.PushModalAsync/PopModalAsync) + the Window.cs event surface. Backend-agnostic
// (the modal events are pure cross-platform), so this file compiles for every backend.
//
// The modal stack lives on the navigation_page in the port; pushing/popping a modal there fires the
// HOST window's events when the navigation_page is hosted in an activated window (containing_window).
//
// §8 teardown: the window (publisher of the modal events) is declared BEFORE the navigation_page and
// the event subscribers, so subscribers disconnect before the publisher dies.
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/modal_event_args.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/window.hpp"

#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::navigation_page;
    using maui::controls::window;

    // Host the navigation_page in an activated window so its modal push/pop reach the window's events.
    void host_in_active_window(window& win, navigation_page& nav)
    {
        win.set_content(nav);
        win.send_created();
        win.send_activated(); // attach_page -> nav.set_containing_window(win)
    }

    TEST(window_modal_events, push_fires_pushing_then_pushed_with_the_modal_page)
    {
        window win; // publisher first (§8)
        content_page root;
        content_page modal;
        navigation_page nav(root);
        host_in_active_window(win, nav);

        std::vector<std::string> transcript;
        const content_page* pushing_modal = nullptr;
        const content_page* pushed_modal = nullptr;
        auto t1 = win.modal_pushing.connect([&](const maui::controls::modal_pushing_event_args& args) {
            transcript.emplace_back("pushing");
            pushing_modal = args.modal;
        });
        auto t2 = win.modal_pushed.connect([&](const maui::controls::modal_pushed_event_args& args) {
            transcript.emplace_back("pushed");
            pushed_modal = args.modal;
        });

        nav.push_modal(modal);

        // C# PushModalAsync: OnModalPushing FIRST, OnModalPushed LAST.
        EXPECT_EQ(transcript, (std::vector<std::string>{"pushing", "pushed"}));
        EXPECT_EQ(pushing_modal, &modal); // the args carry the modal page
        EXPECT_EQ(pushed_modal, &modal);

        win.modal_pushing.disconnect(t1);
        win.modal_pushed.disconnect(t2);
    }

    TEST(window_modal_events, pop_fires_popping_then_popped_with_the_modal_page)
    {
        window win;
        content_page root;
        content_page modal;
        navigation_page nav(root);
        host_in_active_window(win, nav);
        nav.push_modal(modal);

        std::vector<std::string> transcript;
        const content_page* popping_modal = nullptr;
        const content_page* popped_modal = nullptr;
        auto t1 = win.modal_popping.connect([&](maui::controls::modal_popping_event_args& args) {
            transcript.emplace_back("popping");
            popping_modal = args.modal;
        });
        auto t2 = win.modal_popped.connect([&](const maui::controls::modal_popped_event_args& args) {
            transcript.emplace_back("popped");
            popped_modal = args.modal;
        });

        content_page* const popped = nav.pop_modal();

        EXPECT_EQ(popped, &modal);
        // C# PopModalAsync: OnModalPopping FIRST, OnModalPopped LAST.
        EXPECT_EQ(transcript, (std::vector<std::string>{"popping", "popped"}));
        EXPECT_EQ(popping_modal, &modal);
        EXPECT_EQ(popped_modal, &modal);

        win.modal_popping.disconnect(t1);
        win.modal_popped.disconnect(t2);
    }

    TEST(window_modal_events, popping_cancel_aborts_the_pop_and_fires_pop_canceled)
    {
        window win;
        content_page root;
        content_page modal;
        navigation_page nav(root);
        host_in_active_window(win, nav);
        nav.push_modal(modal);

        bool pop_canceled_fired = false;
        bool popped_fired = false;
        auto t1 = win.modal_popping.connect(
            [](maui::controls::modal_popping_event_args& args) { args.cancel = true; }); // veto the pop
        auto t2 = win.pop_canceled.connect([&] { pop_canceled_fired = true; });
        auto t3 =
            win.modal_popped.connect([&](const maui::controls::modal_popped_event_args&) { popped_fired = true; });

        content_page* const popped = nav.pop_modal();

        // C# OnModalPopping returns Cancel -> OnPopCanceled; the modal stays on the stack.
        EXPECT_EQ(popped, nullptr);
        EXPECT_TRUE(pop_canceled_fired);
        EXPECT_FALSE(popped_fired);              // modal_popped does NOT fire on a cancelled pop
        EXPECT_EQ(nav.modal_stack().size(), 1U); // the modal is still there
        EXPECT_EQ(nav.modal_stack().back(), &modal);

        win.modal_popping.disconnect(t1);
        win.pop_canceled.disconnect(t2);
        win.modal_popped.disconnect(t3);
    }

    TEST(window_modal_events, second_modal_push_carries_the_second_page)
    {
        window win;
        content_page root;
        content_page first;
        content_page second;
        navigation_page nav(root);
        host_in_active_window(win, nav);
        nav.push_modal(first);

        const content_page* last_pushed = nullptr;
        auto t = win.modal_pushed.connect(
            [&](const maui::controls::modal_pushed_event_args& args) { last_pushed = args.modal; });

        nav.push_modal(second);
        EXPECT_EQ(last_pushed, &second);

        win.modal_pushed.disconnect(t);
    }

    TEST(window_modal_events, no_host_window_does_not_fire_or_crash)
    {
        // A bare navigation_page (not hosted in a window) has no containing_window; modal push/pop must
        // simply not raise events (and not crash).
        content_page root;
        content_page modal;
        navigation_page nav(root);

        nav.push_modal(modal); // no window -> no events
        EXPECT_EQ(nav.modal_stack().size(), 1U);
        content_page* const popped = nav.pop_modal();
        EXPECT_EQ(popped, &modal);
        EXPECT_TRUE(nav.modal_stack().empty());
    }
} // namespace
