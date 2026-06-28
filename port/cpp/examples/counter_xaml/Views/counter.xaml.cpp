// counter.xaml.cpp — implements examples::counter_page() (declared in counter.xaml.hpp).
//
// This TU owns the compile-time XAML (#embed + build_page) AND the code-behind: it adopts the view-model,
// looks the controls up by x:Name, and wires button.clicked -> command / Count.changed -> label. The wiring
// tokens are parked on the page via page->retain(...), so the function can RETURN the page by value and the
// subscriptions travel with it (they disconnect first when the page is destroyed). The view-model type is
// sealed in this TU — the header exposes only unique_ptr<content_page>.

#include "Views/counter.xaml.hpp"

#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

#include "maui/controls/button.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/event.hpp"

#include <string>

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file).
    constexpr unsigned char counter_xaml_bytes[] = {
#embed "counter.xaml"
    };
    constexpr maui::fixed_string counter_xaml{counter_xaml_bytes};
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> counter_page(std::unique_ptr<ViewModels::counter_view_model> vm)
    {
        auto page = maui::build_page<ViewModels::counter_view_model, counter_xaml>();
        page->bind_to(std::move(vm));
        auto* view_model = page->view_model();

        // x:Name lookup — the only handles the code-behind needs.
        auto label = page->find<maui::controls::label>("count");
        if (auto button = page->find<maui::controls::button>("increment"))
        {
            page->retain(
                maui::core::connect_scoped(button->clicked, [view_model] { view_model->Increment.execute(); }));
        }
        // VM property -> label text (the one-way data flow XAML would express as {Binding Count}).
        page->retain(maui::core::connect_scoped(
            view_model->Count.changed, [weak = std::weak_ptr<maui::controls::label>(label)](int /*old*/, int now) {
                if (auto live = weak.lock())
                {
                    live->set_text("Count: " + std::to_string(now));
                }
            }));

        return page; // unique_ptr<page_impl<counter_view_model>> upcasts to unique_ptr<content_page>
    }
} // namespace examples::Views
