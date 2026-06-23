// hello_world — the cross-platform entry-point proof (Stage 1, headless).
//
// 100% PORTABLE C++: no Objective-C, no .mm, no platform headers. The framework user writes ONLY this —
// an `application` subclass + the one `use_shared_maui_app` configurator — `#include "maui/maui_main.hpp"`,
// and gets a working main() that boots, mounts the window/page/content tree, and lays it out on whichever
// backend is linked (headless here). This is the C++ analog of a MAUI MauiProgram + App.
//
// Build + run (headless preset):
//   cmake --build --preset headless --target maui_hello_world
//   ./build/headless/maui_hello_world      # boots, mounts, settles one layout pass, exits 0

#include "maui/maui_main.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/window.hpp"

// The user's application: owns the window/page/content tree and returns its window from create_window()
// (IApplication.CreateWindow). Built entirely in the constructor — the framework attaches handlers + hosts
// it afterward (the generic mount, app_host.hpp), exactly as MAUI's platform startup does.
class my_app : public maui::controls::application
{
public:
    my_app()
    {
        page_.set_content(label_);
        label_.set_text("Hello, MAUI C++!");
        window_.set_content(page_);
        window_.set_title("Hello");
    }

    maui::core::i_window* create_window() override
    {
        return &window_;
    }

private:
    // Declared so the window outlives nothing it back-references improperly: the window holds a non-owning
    // back-ref to the page (set_content) and the page to the label, so each must outlive its referrer —
    // members destruct in reverse declaration order, so the window (the outermost referrer) dies first.
    maui::controls::window window_;
    maui::controls::content_page page_;
    maui::controls::label label_;
};

// The ONE function the user defines (maui_main.hpp declares it; the generated main() calls it): take a fresh
// builder, register the application, return it. The MauiProgram.CreateMauiApp shape.
maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<my_app>();
    return builder;
}
