// counter — the classic interactive example: a button that increments a label on each click.
//
// ONE concept: wiring a control's event to application state. Every MAUI control exposes its outbound
// events as `maui::core::event<...>` members; `button::clicked` is an `event<>` (no args). You subscribe
// by connecting a callable to it — exactly the std::function-style seam, no reflection. The handler runs
// whenever the native view reports a tap (the framework calls button::send_clicked, which raises the
// event after running any command).
//
// 100% PORTABLE C++: no platform headers. Same source builds + runs on headless, macOS, and iOS.

#include "maui/maui_main.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/event.hpp" // scoped_connection + connect_scoped

#include <string>

class counter_app : public maui::controls::application
{
public:
    counter_app()
    {
        count_label_.set_text("Count: 0");
        increment_button_.set_text("Increment");

        // Subscribe to the button's `clicked` event. `connect_scoped` registers the handler and returns a
        // scoped_connection (an RAII token) we stash in a member so the subscription lives as long as the
        // app — when the app dies, the member's destructor unsubscribes. The lambda captures `this` to
        // mutate the counter and re-render the label.
        click_token_ = maui::core::connect_scoped(increment_button_.clicked, [this] {
            ++count_;
            count_label_.set_text("Count: " + std::to_string(count_));
        });

        // Compose the visible tree: a vertical stack holding the label above the button.
        root_.set_spacing(12.0);
        root_.add(count_label_);
        root_.add(increment_button_);

        page_.set_content(root_);
        window_.set_content(page_);
        window_.set_title("Counter");
    }

    maui::core::i_window* create_window() override
    {
        return &window_;
    }

private:
    int count_ = 0;

    // Members destruct in reverse declaration order, so the window (outermost back-referrer) dies first —
    // every referrer outlives what it points at. The click token sits last so it tears the subscription
    // down before the button it subscribed to is destroyed.
    maui::controls::window window_;
    maui::controls::content_page page_;
    maui::controls::vertical_stack_layout root_;
    maui::controls::label count_label_;
    maui::controls::button increment_button_;
    maui::core::scoped_connection click_token_;
};

// The MauiProgram.CreateMauiApp shape: register the app on a fresh builder and return it.
maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<counter_app>();
    return builder;
}
