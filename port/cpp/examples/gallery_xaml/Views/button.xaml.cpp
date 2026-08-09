// button.xaml.cpp — implements examples::Views::button_page() (declared in button.xaml.hpp).
//
// This TU OWNS the compile-time XAML: the COMPILER embeds the CANONICAL SHARED page
// port/maui-reference/pages/button.xaml — the exact same bytes real .NET MAUI compiles via XamlC +
// code-behind (MauiReference.Pages.ButtonPage) — and build_page hydrates it through the runtime
// loader into a fresh page_impl<no_view_model>. One file, two frameworks, zero divergence.
//
// HAND-WRITTEN code-behind — the generator-owned marker line is deliberately ABSENT so the generator
// leaves this file alone (same mechanism as chrome.xaml.cpp / ios_blur_effect.xaml.cpp / gestures.xaml
// .cpp, and the same one the MAUI side's ButtonPage.xaml.cs uses).
//
// WHY. The twin declared the "Clicked" button with its handler omitted while the code-first builder
// (examples/gallery/pages/button_page.hpp:63-66) connects it to `++tap_count_; update_readout()`. That
// asymmetry is the one motion_score's `twin_cannot_react` flag was invented for — a scorer exemption
// that button.toml set, whose own comment says "RETIRE THIS BY FIXING THE TWIN". This is that fix.
//
// THE FORMAT IS A CONTRACT and the code-first page owns it: button_page.hpp:228-231 is
// snprintf("Taps: %d", tap_count_), so one click must produce exactly "Taps: 1".

#include "Views/button.xaml.hpp"

#include <cstddef>
#include <memory>
#include <string_view>

#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/label.hpp"
#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file, like a quoted
    // #include). The two-line embed is the canonical form (a #embed directive cannot live inside a
    // macro body — see fixed_string.hpp).
    constexpr unsigned char button_xaml_bytes[] = {
#embed "../../../../maui-reference/pages/button.xaml"
    };
    constexpr maui::fixed_string button_xaml{button_xaml_bytes};

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
    static_assert(bytes_contain(button_xaml_bytes, "<ContentPage"));
    static_assert(bytes_contain(button_xaml_bytes, "x:Class=\"MauiReference.Pages.ButtonPage\""));
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> button_page(const maui::xaml::xaml_load_options& options)
    {
        auto page = maui::build_page<maui::no_view_model, button_xaml>(options);

        const auto clicked = page->find<maui::controls::button>("ClickedButton");
        const auto readout = page->find<maui::controls::label>("Readout");
        if (clicked != nullptr && readout != nullptr)
        {
            // The tap count lives in the closure rather than on the page: this factory owns the only
            // subscription that reads it, and page->retain keeps that subscription exactly as long as
            // the page. A shared_ptr<int> because the lambda must be copyable to be stored.
            const auto taps = std::make_shared<int>(0);
            page->retain(maui::core::scoped_connection(
                clicked->clicked, clicked->clicked.connect([readout, taps] {
                    readout->set_text("Taps: " + std::to_string(++*taps));
                })));
        }
        // unique_ptr<page_impl<no_view_model>> upcasts to unique_ptr<content_page> on return.
        return page;
    }
} // namespace examples::Views
