// hello_world — the minimal MAUI C++ app: a window hosting a page hosting a single label.
//
// 100% PORTABLE C++: no Objective-C, no .mm, no platform headers. The framework user writes ONLY this — a
// `maui::ui::app` subclass that calls set_content(...) in its constructor — `#include "maui/maui_main.hpp"`,
// and gets a working main() that boots, mounts the window/page/content tree, and lays it out on whichever
// backend is linked. ui::app OWNS the window and the content tree (in the correct teardown order), so this
// app declares no members at all — the cleanest expression of the C++ analog of a MAUI MauiProgram + App.
//
// Build + run (from the standalone examples project, headless):
//   cmake --build examples/build --target hello_world
//   ./examples/build/hello_world/hello_world   # boots, mounts, settles one layout pass, exits 0

#include "maui/maui_main.hpp"

#include "maui/ui.hpp"

namespace ui = maui::ui;

class my_app : public ui::app
{
public:
    my_app()
    {
        set_content(ui::page(ui::label("Hello, MAUI C++!")));
        set_title("Hello");
    }
};

// The ONE function the user defines (maui_main.hpp declares it; the generated main() calls it): take a fresh
// builder, register the application, return it. The MauiProgram.CreateMauiApp shape.
maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<my_app>();
    return builder;
}
