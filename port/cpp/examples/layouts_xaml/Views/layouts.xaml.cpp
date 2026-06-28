// layouts.xaml.cpp — implements examples::layouts_page() (declared in layouts.xaml.hpp).
//
// This TU owns the compile-time XAML (#embed + build_page). layouts.xaml round-trips the full Grid through
// the runtime loader: ColumnDefinitions/RowDefinitions parse into the grid's definition vectors and each
// child's Grid.Row/Grid.Column attached property is placed by the loader's deferred-attached pass. Purely
// structural, so no view-model. No host tool, so this cross-compiles to the iOS bundle.

#include "Views/layouts.xaml.hpp"

#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file).
    constexpr unsigned char layouts_xaml_bytes[] = {
#embed "layouts.xaml"
    };
    constexpr maui::fixed_string layouts_xaml{layouts_xaml_bytes};
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> layouts_page()
    {
        return maui::build_page<maui::no_view_model, layouts_xaml>();
    }
} // namespace examples::Views
