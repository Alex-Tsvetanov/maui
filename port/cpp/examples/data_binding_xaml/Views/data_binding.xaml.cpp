// data_binding.xaml.cpp — implements examples::data_binding_page() (declared in data_binding.xaml.hpp).
//
// FULLY MARKUP-BOUND: data_binding.xaml carries BOTH the layout AND the bindings ({Binding Message,
// Mode=TwoWay} / {Binding Message}). This TU owns the #embed + build_page; bind_to sets the page's
// BindingContext to the view-model, which makes those markup bindings resolve LIVE — no code-behind wiring,
// no reflection. The view-model type is sealed here; the header exposes only unique_ptr<content_page>.

#include "Views/data_binding.xaml.hpp"

#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file).
    constexpr unsigned char data_binding_xaml_bytes[] = {
#embed "data_binding.xaml"
    };
    constexpr maui::fixed_string data_binding_xaml{data_binding_xaml_bytes};
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> data_binding_page(std::unique_ptr<ViewModels::greeting_view_model> vm)
    {
        auto page = maui::build_page<ViewModels::greeting_view_model, data_binding_xaml>();
        page->bind_to(std::move(vm)); // BindingContext -> the markup {Binding}s go live
        return page;                  // unique_ptr<page_impl<greeting_view_model>> upcasts to content_page
    }
} // namespace examples::Views
