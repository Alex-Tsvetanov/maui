// selection_synchronization.xaml.cpp — implements examples::Views::selection_synchronization_page() (declared in
// selection_synchronization.xaml.hpp).
//
// This TU OWNS the compile-time XAML: the COMPILER embeds the CANONICAL SHARED page
// port/maui-reference/pages/selection_synchronization.xaml — the exact same bytes real .NET MAUI compiles via XamlC +
// code-behind (MauiReference.Pages.SelectionSynchronizationPage) — and build_page hydrates it through the runtime
// loader into a fresh page_impl<no_view_model>. One file, two frameworks, zero divergence.
//
// HAND-WRITTEN (the GENERATED marker is deliberately absent, so `e2e.py gen` leaves this TU alone).
// It supplies the SelectionSyncModel the shared markup binds — see ViewModels/selection_sync.hpp.

#include "Views/selection_synchronization.xaml.hpp"

#include "ViewModels/selection_sync.hpp"

#include <cstddef>
#include <string_view>

#include "maui/fixed_string.hpp"
#include "maui/xaml/xaml_static_check.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file, like a quoted
    // #include). The two-line embed is the canonical form (a #embed directive cannot live inside a
    // macro body — see fixed_string.hpp).
    constexpr unsigned char selection_synchronization_xaml_bytes[] = {
#embed "../../../../maui-reference/pages/selection_synchronization.xaml"
    };
    constexpr maui::fixed_string selection_synchronization_xaml{selection_synchronization_xaml_bytes};

    MAUI_XAML_REJECT_EVENT_ATTRIBUTES(selection_synchronization_xaml);

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
    static_assert(bytes_contain(selection_synchronization_xaml_bytes, "<ContentPage"));
    static_assert(bytes_contain(selection_synchronization_xaml_bytes,
                                "x:Class=\"MauiReference.Pages.SelectionSynchronizationPage\""));
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> selection_synchronization_page(
        const maui::xaml::xaml_load_options& options)
    {
        // unique_ptr<page_impl<no_view_model>> upcasts to unique_ptr<content_page> on return.
        // The markup binds every Selected* slot to the model (the original sample's form), so the page
        // needs a real BindingContext: build_page mints it, bind_to adopts it and RE-EVALUATES the
        // {Binding}s the loader attached during hydration. Without this the nine CollectionViews load
        // fine and render nothing selected — the exact failure the <x:Array> twin used to have.
        auto page = maui::build_page<examples::ViewModels::selection_sync_model, selection_synchronization_xaml>(
            options);
        page->bind_to(std::make_unique<examples::ViewModels::selection_sync_model>());
        return page;
    }
} // namespace examples::Views
