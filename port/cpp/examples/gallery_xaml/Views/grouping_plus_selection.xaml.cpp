// grouping_plus_selection.xaml.cpp — implements examples::Views::grouping_plus_selection_page() (declared in
// grouping_plus_selection.xaml.hpp).
//
// This TU OWNS the compile-time XAML: the COMPILER embeds the CANONICAL SHARED page
// port/maui-reference/pages/grouping_plus_selection.xaml — the exact same bytes real .NET MAUI compiles via XamlC +
// code-behind (MauiReference.Pages.GroupingPlusSelectionPage) — and build_page hydrates it through the runtime
// loader into a fresh page_impl<no_view_model>. One file, two frameworks, zero divergence.
//
// HAND-WRITTEN port-side code-behind (NOT generated — the GENERATED marker is intentionally absent so
// `e2e.py gen` leaves this TU alone). The shared markup sets NO ItemsSource; real MAUI assigns it in the
// page's .xaml.cs code-behind (`CollectionView.ItemsSource = ...`). gallery_xaml has no C# code-behind,
// so we do the reflection-free equivalent here: hydrate, find the x:Name'd CollectionView, give it the
// bindable grouped source so the group-header/footer/item DataTemplates' {Binding Name}/{Binding Count}
// paths resolve. See ViewModels/super_teams.hpp.

#include "Views/grouping_plus_selection.xaml.hpp"

#include "ViewModels/super_teams.hpp"
#include "maui/controls/items/collection_view.hpp"

#include <cstddef>
#include <string_view>

#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file, like a quoted
    // #include). The two-line embed is the canonical form (a #embed directive cannot live inside a
    // macro body — see fixed_string.hpp).
    constexpr unsigned char grouping_plus_selection_xaml_bytes[] = {
#embed "../../../../maui-reference/pages/grouping_plus_selection.xaml"
    };
    constexpr maui::fixed_string grouping_plus_selection_xaml{grouping_plus_selection_xaml_bytes};

    // Compile-time naming-triple lock: the embedded bytes must be a ContentPage whose x:Class matches
    // this page's key-derived MAUI partial class (the lint's runtime check, enforced by the compiler).
    template <std::size_t N> constexpr bool bytes_contain(const unsigned char (&hay)[N], std::string_view needle)
    {
        if (needle.empty() || N < needle.size())
        {
            return needle.empty();
        }
        for (std::size_t i = 0; i + needle.size() <= N; i++)
        {
            bool match = true;
            for (std::size_t j = 0; j < needle.size(); j++)
            {
                if (static_cast<char>(hay[i + j]) != needle[j])
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                return true;
            }
        }
        return false;
    }
    static_assert(bytes_contain(grouping_plus_selection_xaml_bytes, "<ContentPage"));
    static_assert(bytes_contain(grouping_plus_selection_xaml_bytes,
                                "x:Class=\"MauiReference.Pages.GroupingPlusSelectionPage\""));
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> grouping_plus_selection_page(
        const maui::xaml::xaml_load_options& options)
    {
        auto page = maui::build_page<maui::no_view_model, grouping_plus_selection_xaml>(options);
        // Code-behind data assignment (the reflection-free analog of the page's .xaml.cs).
        if (auto list = page->find<maui::controls::collection_view>("CollectionView"))
        {
            list->set_items_source(examples::ViewModels::super_teams());
        }
        // unique_ptr<page_impl<no_view_model>> upcasts to unique_ptr<content_page> on return.
        return page;
    }
} // namespace examples::Views
