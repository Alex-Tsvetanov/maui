// layouts_xaml — the WITH-XAML twin of examples/layouts, structured like a MAUI .xaml/.xaml.cs app.
//
// The page is authored as markup (layouts.xaml: a vertical stack above a 2x2 Grid) and built by
// examples::layouts_page(), declared in layouts.xaml.hpp and implemented in the separate TU layouts.xaml.cpp
// (which owns the #embed + build_page). main.cpp is just the app shell. No #embed/build_page here.
//
// 100% PORTABLE C++: no platform headers.

#include "maui/maui_main.hpp"

#include "maui/ui.hpp"

#include "maui/controls/content_page.hpp"

#include "Views/layouts.xaml.hpp"

#include <memory>

namespace ui = maui::ui;

class layouts_app : public ui::app
{
public:
    layouts_app()
    {
        set_content(ui::view_ref<maui::controls::content_page>{
            std::shared_ptr<maui::controls::content_page>{examples::Views::layouts_page()}});
        set_title("Layouts");
    }
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<layouts_app>();
    return builder;
}
