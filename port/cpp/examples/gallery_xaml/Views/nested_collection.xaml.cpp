// nested_collection.xaml.cpp — implements examples::Views::nested_collection_page() (declared in
// nested_collection.xaml.hpp).
//
// This TU OWNS the compile-time XAML: the COMPILER embeds the CANONICAL SHARED page
// port/maui-reference/pages/nested_collection.xaml — the exact same bytes real .NET MAUI compiles via XamlC +
// code-behind (MauiReference.Pages.NestedCollectionPage) — and build_page hydrates it through the runtime
// loader into a fresh page_impl<no_view_model>. One file, two frameworks, zero divergence.
//
// HAND-WRITTEN code-behind (the GENERATED marker was dropped so `e2e.py gen` leaves it alone, per
// docs/AUTHORING.md): the shared XAML sets no outer ItemsSource — assign it here, the reflection-free
// analog of NestedCollectionPage.xaml.cs. See nested_collection.xaml.cpp's history for the plain twin.

#include "Views/nested_collection.xaml.hpp"

#include <cstddef>
#include <string_view>

#include "ViewModels/nested_items.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file, like a quoted
    // #include). The two-line embed is the canonical form (a #embed directive cannot live inside a
    // macro body — see fixed_string.hpp).
    constexpr unsigned char nested_collection_xaml_bytes[] = {
#embed "../../../../maui-reference/pages/nested_collection.xaml"
    };
    constexpr maui::fixed_string nested_collection_xaml{nested_collection_xaml_bytes};

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
    static_assert(bytes_contain(nested_collection_xaml_bytes, "<ContentPage"));
    static_assert(bytes_contain(nested_collection_xaml_bytes, "x:Class=\"MauiReference.Pages.NestedCollectionPage\""));
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> nested_collection_page(const maui::xaml::xaml_load_options& options)
    {
        auto page = maui::build_page<maui::no_view_model, nested_collection_xaml>(options);
        // Code-behind data assignment (the reflection-free analog of NestedCollectionPage.xaml.cs): give
        // the x:Name'd OUTER CollectionView the 20 nested sources, so each realized outer cell is an inner
        // CollectionView whose ItemsSource="{Binding Items}" / Header="{Binding Title}" / item {Binding
        // Caption} resolve against a nested_source_item — matching the C++ builder column and original MAUI
        // ("CollectionViews all the way down"). Without this the outer list is empty (blank page).
        if (auto outer = page->find<maui::controls::collection_view>("CollectionView"))
        {
            outer->set_items_source(examples::ViewModels::nested_items());
        }
        // unique_ptr<page_impl<no_view_model>> upcasts to unique_ptr<content_page> on return.
        return page;
    }
} // namespace examples::Views
