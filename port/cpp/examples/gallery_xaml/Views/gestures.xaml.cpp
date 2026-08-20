// gestures.xaml.cpp — implements examples::Views::gestures_page() (declared in gestures.xaml.hpp).
//
// This TU OWNS the compile-time XAML: the COMPILER embeds the CANONICAL SHARED page
// port/maui-reference/pages/gestures.xaml — the exact same bytes real .NET MAUI compiles via XamlC +
// code-behind (MauiReference.Pages.GesturesPage) — and build_page hydrates it through the runtime
// loader into a fresh page_impl<no_view_model>. One file, two frameworks, zero divergence.
//
// HAND-WRITTEN code-behind — the generator-owned marker line is deliberately ABSENT so the
// generator leaves this file alone (same mechanism as header_footer_template.xaml.cpp, and the same one
// the MAUI side's GesturesPage.xaml.cs uses).
//
// WHY. The shared twin declares five GestureRecognizers, and 27fd12e283 taught the loader to attach them
// — proven by unit tests. But nothing was WIRED to them here, so this column sat still while MAUI's
// reacted, and the board read MOTION MISMATCH 3935 px vs 0 (f9dc430377). MAUI's column reacts through
// GesturesPage.xaml.cs, which is C# this port cannot execute; the reflection-free equivalent is exactly
// this file, per rule 5 (PORT-MUST-EXPRESS-IT)'s header_footer_template resolution (hand-write the code-behind rather than
// exempt the diff).
//
// THE STRINGS ARE A CONTRACT and this side is their ORIGIN: gestures_page.hpp:208-213 defines
// "Last gesture: " + the gesture name, and both the C# twin and this file must agree with it or a driven
// frame diverges for a reason that has nothing to do with either renderer.

#include "Views/gestures.xaml.hpp"

#include <cstddef>
#include <string>
#include <string_view>

#include "maui/controls/box_view.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/controls/label.hpp"

#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file, like a quoted
    // #include). The two-line embed is the canonical form (a #embed directive cannot live inside a
    // macro body — see fixed_string.hpp).
    constexpr unsigned char gestures_xaml_bytes[] = {
#embed "../../../../maui-reference/pages/gestures.xaml"
    };
    constexpr maui::fixed_string gestures_xaml{gestures_xaml_bytes};

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
    static_assert(bytes_contain(gestures_xaml_bytes, "<ContentPage"));
    static_assert(bytes_contain(gestures_xaml_bytes, "x:Class=\"MauiReference.Pages.GesturesPage\""));
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> gestures_page(const maui::xaml::xaml_load_options& options)
    {
        auto page = maui::build_page<maui::no_view_model, gestures_xaml>(options);

        // The two x:Name anchors the shared twin carries so BOTH frameworks can find the same elements
        // (added in 7007a8909a for precisely this). A missing one leaves the page inert rather than
        // crashing — the same posture the loader takes for a stray recognizer.
        const auto readout = page->find<maui::controls::label>("Readout");
        const auto target = page->find<maui::controls::box_view>("GestureTarget");
        if (readout == nullptr || target == nullptr)
        {
            return page;
        }
        const auto set_readout = [readout](std::string_view gesture) {
            std::string text = "Last gesture: ";
            text += gesture;
            readout->set_text(text);
        };

        // Walk the collection rather than naming each recognizer, so the twin needs no x:Name on a
        // non-view element — the same shape GesturesPage.xaml.cs uses on the MAUI side.
        auto& recognizers = target->gesture_recognizers();
        for (std::size_t i = 0; i < recognizers.count(); i++)
        {
            const auto& recognizer = recognizers.at(i);
            if (auto* tap = dynamic_cast<maui::controls::tap_gesture_recognizer*>(recognizer.get()))
            {
                page->retain(maui::core::scoped_connection(
                    tap->tapped,
                    tap->tapped.connect([set_readout](const maui::controls::tapped_event_args&) {
                        set_readout("Tapped");
                    })));
            }
            else if (auto* swipe = dynamic_cast<maui::controls::swipe_gesture_recognizer*>(recognizer.get()))
            {
                page->retain(maui::core::scoped_connection(
                    swipe->swiped,
                    swipe->swiped.connect([set_readout](const maui::controls::swiped_event_args& e) {
                        // Same table as gestures_page.hpp:216 — the C++ builder is the ORIGIN of these
                        // strings, so this is a copy, not a re-derivation.
                        std::string text = "Swiped ";
                        switch (e.direction)
                        {
                            case maui::core::swipe_direction::left: text += "Left"; break;
                            case maui::core::swipe_direction::right: text += "Right"; break;
                            case maui::core::swipe_direction::up: text += "Up"; break;
                            default: text += "Down"; break;
                        }
                        set_readout(text);
                    })));
            }
        }
        // unique_ptr<page_impl<no_view_model>> upcasts to unique_ptr<content_page> on return.
        return page;
    }
} // namespace examples::Views
