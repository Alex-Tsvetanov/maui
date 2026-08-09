// chrome.xaml.cpp — implements examples::Views::chrome_page() (declared in chrome.xaml.hpp).
//
// This TU OWNS the compile-time XAML: the COMPILER embeds the CANONICAL SHARED page
// port/maui-reference/pages/chrome.xaml — the exact same bytes real .NET MAUI compiles via XamlC +
// code-behind (MauiReference.Pages.ChromePage) — and build_page hydrates it through the runtime
// loader into a fresh page_impl<no_view_model>. One file, two frameworks, zero divergence.
//
// HAND-WRITTEN code-behind — the generator-owned marker line is deliberately ABSENT so the generator
// leaves this file alone (same mechanism as gestures.xaml.cpp / header_footer_template.xaml.cpp, and the
// same one the MAUI side's ChromePage.xaml.cs uses).
//
// WHY. The shared twin declares a Button and a "Ready" readout but wired nothing to them, while the
// code-first builder (examples/gallery/pages/chrome_page.hpp:75) connects the button to
// `stamp("Button pressed")`. At rest all three columns read "Ready", so the STILL comparison never saw
// the gap — but `chrome` is one of the board's ANIMATED pages, and on a driven frame only the code-first
// column would move. That is a MOTION MISMATCH blamed on the port for this file's omission, which is
// exactly the failure gestures.xaml.cpp was written to stop.
//
// THE STRING IS A CONTRACT and the code-first page is its ORIGIN: chrome_page.hpp:126-129 defines
// `stamp(what) => "Last: " + what`, so this and ChromePage.xaml.cs must both produce exactly
// "Last: Button pressed" or the driven frame diverges for a reason unrelated to either renderer.

#include "Views/chrome.xaml.hpp"

#include <cstddef>
#include <string_view>

#include "maui/controls/button.hpp"
#include "maui/controls/label.hpp"
#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file, like a quoted
    // #include). The two-line embed is the canonical form (a #embed directive cannot live inside a
    // macro body — see fixed_string.hpp).
    constexpr unsigned char chrome_xaml_bytes[] = {
#embed "../../../../maui-reference/pages/chrome.xaml"
    };
    constexpr maui::fixed_string chrome_xaml{chrome_xaml_bytes};

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
    static_assert(bytes_contain(chrome_xaml_bytes, "<ContentPage"));
    static_assert(bytes_contain(chrome_xaml_bytes, "x:Class=\"MauiReference.Pages.ChromePage\""));
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> chrome_page(const maui::xaml::xaml_load_options& options)
    {
        auto page = maui::build_page<maui::no_view_model, chrome_xaml>(options);

        // The two x:Name anchors the shared twin carries so BOTH frameworks find the same elements. A
        // missing one leaves the page inert rather than crashing — the same posture gestures.xaml.cpp
        // takes, and the same one the loader takes for a stray recognizer.
        const auto button = page->find<maui::controls::button>("ActionButton");
        const auto readout = page->find<maui::controls::label>("Readout");
        if (button != nullptr && readout != nullptr)
        {
            // Parked on the page via retain(), so this factory can RETURN the page by value and the
            // subscription still dies with it — retained_tokens_ is the page's LAST member, so every
            // token disconnects while its control is still alive (xaml_build.hpp:100-107).
            page->retain(maui::core::scoped_connection(
                button->clicked, button->clicked.connect([readout] { readout->set_text("Last: Button pressed"); })));
        }
        // unique_ptr<page_impl<no_view_model>> upcasts to unique_ptr<content_page> on return.
        return page;
    }
} // namespace examples::Views
