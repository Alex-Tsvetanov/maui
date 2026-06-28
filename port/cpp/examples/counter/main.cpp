// counter — a button click increments a label, authored with the idiomatic maui::ui surface.
//
// ONE concept: the verbosity-free owning builder + ui::app hosting. `ui::vstack(...)` builds the whole
// content subtree; `set_content(...)` hands it to ui::app, which OWNS the window and the tree in the correct
// teardown order — so this app declares only the ONE control it mutates later (a copyable `weak_ref`), no
// window/root members at all. The click handler captures `[this]` — never the button handle, which would not
// compile (the move-only handle makes §8's cycle footgun ill-formed). The `.on_click` token is parked inside
// the tree and tears down with it.
//
// 100% PORTABLE C++: no platform headers. Same source builds + runs on headless, macOS, and iOS.

#include "maui/maui_main.hpp"

#include "maui/ui.hpp"

#include "maui/controls/label.hpp"

#include <string>

namespace ui = maui::ui;

class counter_app : public ui::app
{
public:
    counter_app()
    {
        auto count_label = ui::label("Count: 0");
        count_label_ = count_label.weak(); // observe the one control we mutate later (mint BEFORE the move)

        set_content(ui::page(
            ui::vstack(std::move(count_label), ui::button("Increment").on_click([this] { bump(); })).spacing(12)));
        set_title("Counter");
    }

private:
    void bump()
    {
        ++count_;
        if (auto label = count_label_.lock())
        {
            label->set_text("Count: " + std::to_string(count_));
        }
    }

    int count_ = 0;
    ui::weak_ref<maui::controls::label> count_label_;
};

// The MauiProgram.CreateMauiApp shape: register the app on a fresh builder and return it.
maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<counter_app>();
    return builder;
}
