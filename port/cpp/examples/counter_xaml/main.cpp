// counter_xaml — the WITH-XAML twin of examples/counter, structured like a MAUI .xaml/.xaml.cs app.
//
// The page is authored as markup (counter.xaml) and built by examples::counter_page(vm), declared in
// counter.xaml.hpp and implemented in the separate TU counter.xaml.cpp (which owns the #embed + build_page +
// the x:Name code-behind). The view-model lives in its own header (counter_view_model.hpp). main.cpp is just
// the app shell: construct the view-model, hand it to the page factory, host the page. No #embed/build_page
// here.
//
// 100% PORTABLE C++: no platform headers.

#include "maui/maui_main.hpp"

#include "maui/ui.hpp"

#include "maui/controls/content_page.hpp"

#include "ViewModels/counter_view_model.hpp"
#include "Views/counter.xaml.hpp"

#include <memory>

namespace ui = maui::ui;

class counter_app : public ui::app
{
public:
    counter_app()
    {
        set_content(ui::view_ref<maui::controls::content_page>{std::shared_ptr<maui::controls::content_page>{
            examples::Views::counter_page(std::make_unique<examples::ViewModels::counter_view_model>())}});
        set_title("Counter");
    }
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<counter_app>();
    return builder;
}
