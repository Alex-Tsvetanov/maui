#pragma once
// maui::samples::dispatcher_page — ports DispatcherPage.xaml (+ .xaml.cs).
//
// The MAUI page is a ScrollView over a StackLayout of "do X on the dispatcher, then read the result
// in a label" rows (DispatcherPage.xaml.cs):
//   - OnFailAccessClicked  — Task.Run touches the UI off-thread (the unhappy path; "Oops!").
//   - OnAccessClicked      — Task.Run then DispatchAsync back onto the UI thread ("This was a success!").
//   - OnLaterClicked       — Dispatcher.DispatchDelayed(3s, …) ("Started!" now, then a delayed message).
//   - OnTimerClicked       — Dispatcher.CreateTimer() (IsRepeating, 3s) ticking a counter label.
//   - OnObsoleteClicked    — the legacy Device.StartTimer(3s) repeating timer.
//
// Port mapping (headless-safe, code-first): a C# view's `.Dispatcher` is the UI-thread message pump
// (maui::core::i_dispatcher). The framework supplies it from the window in a real backend; here the page
// OWNS a maui::core::manual_dispatcher — the headless, virtual-clock i_dispatcher the port ships exactly
// for deterministic, backend-free testing (manual_dispatcher.hpp). Buttons connect their `clicked` event
// to the handlers; the demo then drives the work DETERMINISTICALLY so the static capture shows a result:
// dispatch()/dispatch_delayed()/timer ticks are pumped via run_pending() + advance(), with no wall clock
// and no threads. Each row's readout label is updated from the dispatched work, mirroring the C# labels.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/ios
// test trees exercise the same controls directly.
//
// note: the C# OnFailAccessClicked demonstrates the EXCEPTION you get touching the UI off the UI thread;
// the headless manual_dispatcher has no separate UI thread to violate, so this port shows the *intent*
// (set off-dispatcher vs marshalled-on-dispatcher) rather than reproducing a platform thread-affinity
// throw — there is no such seam to invent at this layer.

#include <chrono>
#include <cstdio>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/core/dispatcher.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class dispatcher_page
    {
    public:
        dispatcher_page()
        {
            using namespace std::chrono_literals;

            page_.set_title("Dispatcher");
            stack_.set_spacing(10);
            stack_.set_padding(maui::core::thickness(20));

            // --- Row 1: "Fail Access" — touch the UI without the dispatcher (the unhappy path). ---
            fail_prompt_.set_text("Watch the machines complain about accessing the UI thread from the "
                                  "background:");
            fail_button_.set_text("Fail Access");
            fail_label_.set_text("...");
            fail_button_.clicked.connect([this] { on_fail_access_clicked(); });

            // --- Row 2: "Access" — marshal the UI work back onto the dispatcher (the happy path). ---
            happy_prompt_.set_text("Now observe the happy machines when using a dispatcher:");
            access_button_.set_text("Access");
            happy_label_.set_text("...");
            access_button_.clicked.connect([this] { on_access_clicked(); });

            // --- Row 3: "3 Seconds Later" — DispatchDelayed (set "Started!" now, message after 3s). ---
            later_prompt_.set_text("Maybe you want something to happen in a few seconds:");
            later_button_.set_text("3 Seconds Later");
            later_label_.set_text("...");
            later_button_.clicked.connect([this] { on_later_clicked(); });

            // --- Row 4: "3 Second Timer (Start/Stop)" — CreateTimer, IsRepeating, ticking a counter. ---
            timer_prompt_.set_text("Or, you might want something to repeat like a timer:");
            timer_button_.set_text("3 Second Timer (Start/Stop)");
            timer_label_.set_text("...");
            timer_button_.clicked.connect([this] { on_timer_clicked(); });

            // --- Row 5: the OBSOLETE Device.StartTimer(3s) repeating timer (same effect, legacy API). ---
            obsolete_prompt_a_.set_text("OBSOLETE ZONE ALERT!");
            obsolete_prompt_b_.set_text("Some old code still works:");
            obsolete_button_.set_text("Device.StartTimer(3s)  (Start/Stop)");
            obsolete_label_.set_text("...");
            obsolete_button_.clicked.connect([this] { on_obsolete_clicked(); });

            stack_.add(fail_prompt_);
            stack_.add(fail_button_);
            stack_.add(fail_label_);
            stack_.add(happy_prompt_);
            stack_.add(access_button_);
            stack_.add(happy_label_);
            stack_.add(later_prompt_);
            stack_.add(later_button_);
            stack_.add(later_label_);
            stack_.add(timer_prompt_);
            stack_.add(timer_button_);
            stack_.add(timer_label_);
            stack_.add(obsolete_prompt_a_);
            stack_.add(obsolete_prompt_b_);
            stack_.add(obsolete_button_);
            stack_.add(obsolete_label_);

            scroller_.set_content(stack_);
            page_.set_content(scroller_);

            // Drive the demonstrated behavior deterministically so the static capture shows results
            // (no backend, no wall clock — the headless manual_dispatcher's whole point).
            seed_demo();
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last) so each parent can
        // host its child's native view, then re-host the tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, fail_prompt_, "fail_prompt_");
            gallery_attach_one(app, fail_button_, "fail_button_");
            gallery_attach_one(app, fail_label_, "fail_label_");
            gallery_attach_one(app, happy_prompt_, "happy_prompt_");
            gallery_attach_one(app, access_button_, "access_button_");
            gallery_attach_one(app, happy_label_, "happy_label_");
            gallery_attach_one(app, later_prompt_, "later_prompt_");
            gallery_attach_one(app, later_button_, "later_button_");
            gallery_attach_one(app, later_label_, "later_label_");
            gallery_attach_one(app, timer_prompt_, "timer_prompt_");
            gallery_attach_one(app, timer_button_, "timer_button_");
            gallery_attach_one(app, timer_label_, "timer_label_");
            gallery_attach_one(app, obsolete_prompt_a_, "obsolete_prompt_a_");
            gallery_attach_one(app, obsolete_prompt_b_, "obsolete_prompt_b_");
            gallery_attach_one(app, obsolete_button_, "obsolete_button_");
            gallery_attach_one(app, obsolete_label_, "obsolete_label_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, scroller_, "scroller_");
            gallery_attach_one(app, page_, "page_");

            // The tree was built in the ctor before any handler existed, so replay the host commands now.
            gallery_rehost_layout(stack_);     // stack hosts the prompt/button/label rows
            gallery_rehost_content(scroller_); // scroll_view hosts the stack
            gallery_rehost_content(page_);     // page hosts the scroll_view
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / inspection.
        [[nodiscard]] maui::controls::scroll_view& scroller()
        {
            return scroller_;
        }
        [[nodiscard]] maui::controls::stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::label& fail_label()
        {
            return fail_label_;
        }
        [[nodiscard]] maui::controls::label& happy_label()
        {
            return happy_label_;
        }
        [[nodiscard]] maui::controls::label& later_label()
        {
            return later_label_;
        }
        [[nodiscard]] maui::controls::label& timer_label()
        {
            return timer_label_;
        }
        [[nodiscard]] maui::controls::label& obsolete_label()
        {
            return obsolete_label_;
        }
        [[nodiscard]] maui::core::manual_dispatcher& dispatcher()
        {
            return dispatcher_;
        }

    private:
        // OnFailAccessClicked: the off-dispatcher write (no marshalling). At this layer there is no
        // separate UI thread to violate, so we show the intent rather than fabricate a thread-affinity
        // throw (see header note).
        void on_fail_access_clicked()
        {
            fail_label_.set_text("Oops!");
        }

        // OnAccessClicked: Task.Run(… DispatchAsync(…)) — marshal the UI write onto the dispatcher, then
        // pump it. dispatch() returns true when scheduled; run_pending() runs it on the dispatcher thread.
        void on_access_clicked()
        {
            dispatcher_.dispatch([this] { happy_label_.set_text("This was a success!"); });
            dispatcher_.run_pending();
        }

        // OnLaterClicked: set "Started!" immediately, schedule the delayed message 3s out via
        // DispatchDelayed, then advance the virtual clock past 3s so the delayed work fires.
        void on_later_clicked()
        {
            using namespace std::chrono_literals;
            later_label_.set_text("Started!");
            dispatcher_.dispatch_delayed(3000ms, [this] { later_label_.set_text("I happened 3 seconds later!"); });
            dispatcher_.advance(3000ms);
        }

        // OnTimerClicked: a repeating 3s timer that ticks a counter into the label (Start on first click,
        // Stop on the next). CreateTimer + Interval + IsRepeating + Start mirror the C# exactly; advancing
        // the virtual clock by N intervals fires the Tick N times (deterministic counter).
        void on_timer_clicked()
        {
            using namespace std::chrono_literals;
            if (timer_)
            {
                timer_->stop();
                timer_.reset();
                timer_label_.set_text("Stopped!");
                return;
            }

            timer_counter_ = 0;
            timer_ = dispatcher_.create_timer();
            timer_->set_interval(3000ms);
            timer_->set_is_repeating(true);
            timer_->tick().connect([this] {
                ++timer_counter_;
                char text[64];
                std::snprintf(text, sizeof(text), "I am on a 3 second timer! %d ticks", timer_counter_);
                timer_label_.set_text(text);
            });
            timer_->start();
            timer_label_.set_text("Started!");
        }

        // OnObsoleteClicked: the legacy Device.StartTimer(3s) repeating timer — same observable effect as
        // the modern timer above (the page's whole point is "old code still works"), driven by the same
        // dispatcher timer seam since the port has no separate obsolete Device pump.
        void on_obsolete_clicked()
        {
            using namespace std::chrono_literals;
            if (obsolete_timer_)
            {
                obsolete_timer_->stop();
                obsolete_timer_.reset();
                obsolete_label_.set_text("Stopped!");
                return;
            }

            obsolete_counter_ = 0;
            obsolete_timer_ = dispatcher_.create_timer();
            obsolete_timer_->set_interval(3000ms);
            obsolete_timer_->set_is_repeating(true);
            obsolete_timer_->tick().connect([this] {
                ++obsolete_counter_;
                char text[64];
                std::snprintf(text, sizeof(text), "I am on a 3 second timer! %d ticks", obsolete_counter_);
                obsolete_label_.set_text(text);
            });
            obsolete_timer_->start();
            obsolete_label_.set_text("Started!");
        }

        // Drive a representative slice of each row at construction so a static capture shows live results:
        // the happy DispatchAsync row, the delayed row, and three ticks of the repeating timer.
        void seed_demo()
        {
            using namespace std::chrono_literals;
            on_access_clicked();         // happy_label_ -> "This was a success!"
            on_later_clicked();          // later_label_ -> "I happened 3 seconds later!"
            on_timer_clicked();          // start the repeating timer
            dispatcher_.advance(9000ms); // three 3s ticks: timer_label_ -> "... 3 ticks"
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::stack_layout stack_;

        maui::controls::label fail_prompt_;
        maui::controls::button fail_button_;
        maui::controls::label fail_label_;

        maui::controls::label happy_prompt_;
        maui::controls::button access_button_;
        maui::controls::label happy_label_;

        maui::controls::label later_prompt_;
        maui::controls::button later_button_;
        maui::controls::label later_label_;

        maui::controls::label timer_prompt_;
        maui::controls::button timer_button_;
        maui::controls::label timer_label_;

        maui::controls::label obsolete_prompt_a_;
        maui::controls::label obsolete_prompt_b_;
        maui::controls::button obsolete_button_;
        maui::controls::label obsolete_label_;

        // The page-owned UI-thread pump (a C# view's .Dispatcher; headless virtual-clock impl).
        maui::core::manual_dispatcher dispatcher_;
        std::unique_ptr<maui::core::i_dispatcher_timer> timer_;          // the modern repeating timer
        std::unique_ptr<maui::core::i_dispatcher_timer> obsolete_timer_; // the legacy Device.StartTimer
        int timer_counter_ = 0;
        int obsolete_counter_ = 0;
    };
} // namespace maui::samples
