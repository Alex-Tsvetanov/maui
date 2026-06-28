// hello_world_xaml — the WITH-XAML twin of examples/hello_world, structured like a MAUI .xaml/.xaml.cs app.
//
// The page is authored as markup (hello_world.xaml) and built by examples::hello_world_page(), declared in
// hello_world.xaml.hpp and implemented in the separate TU hello_world.xaml.cpp (which owns the #embed +
// build_page). main.cpp is just the app shell: include the page header, construct the page, host it. There
// is NO #embed/build_page here — markup mechanics stay in the .xaml.cpp.
//
// 100% PORTABLE C++: no platform headers.

#include "maui/maui_main.hpp"

#include "maui/ui.hpp"

#include "maui/controls/content_page.hpp"

#include "Views/hello_world.xaml.hpp"

#include <memory>

namespace ui = maui::ui;

class my_app : public ui::app
{
public:
    my_app()
    {
        set_content(ui::view_ref<maui::controls::content_page>{
            std::shared_ptr<maui::controls::content_page>{examples::Views::hello_world_page()}});
        set_title("Hello");
    }
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<my_app>();
    return builder;
}
