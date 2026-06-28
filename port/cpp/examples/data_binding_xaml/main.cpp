// data_binding_xaml — the WITH-XAML twin of examples/data_binding, structured like a MAUI .xaml/.xaml.cs app.
//
// The page is authored as markup (data_binding.xaml, fully markup-bound) and built by
// examples::data_binding_page(vm), declared in data_binding.xaml.hpp and implemented in the separate TU
// data_binding.xaml.cpp (which owns the #embed + build_page + bind_to). The view-model lives in its own
// header (greeting_view_model.hpp). main.cpp is just the app shell. No #embed/build_page here.
//
// 100% PORTABLE C++: no platform headers.

#include "maui/maui_main.hpp"

#include "maui/ui.hpp"

#include "maui/controls/content_page.hpp"

#include "ViewModels/greeting_view_model.hpp"
#include "Views/data_binding.xaml.hpp"

#include <memory>

namespace ui = maui::ui;

class data_binding_app : public ui::app
{
public:
    data_binding_app()
    {
        set_content(ui::view_ref<maui::controls::content_page>{std::shared_ptr<maui::controls::content_page>{
            examples::Views::data_binding_page(std::make_unique<examples::ViewModels::greeting_view_model>())}});
        set_title("Data Binding");
    }
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<data_binding_app>();
    return builder;
}
