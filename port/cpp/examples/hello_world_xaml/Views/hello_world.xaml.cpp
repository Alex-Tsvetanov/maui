// hello_world.xaml.cpp — implements examples::hello_world_page() (declared in hello_world.xaml.hpp).
//
// This TU OWNS the compile-time XAML: the COMPILER embeds the raw markup with #embed and build_page hydrates
// it. Keeping the #embed + build_page here (a separate TU linked to main.cpp) mirrors MAUI's .xaml/.xaml.cs
// split — main.cpp stays free of markup mechanics. No host tool, so this cross-compiles to the iOS bundle.

#include "Views/hello_world.xaml.hpp"

#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file). The two-line embed
    // is the canonical form (a #embed directive cannot live inside a macro body — see fixed_string.hpp).
    constexpr unsigned char hello_world_xaml_bytes[] = {
#embed "hello_world.xaml"
    };
    constexpr maui::fixed_string hello_world_xaml{hello_world_xaml_bytes};
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> hello_world_page()
    {
        // unique_ptr<page_impl<no_view_model>> upcasts to unique_ptr<content_page> on return.
        return maui::build_page<maui::no_view_model, hello_world_xaml>();
    }
} // namespace examples::Views
