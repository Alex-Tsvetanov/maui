// ios_blur_effect.xaml.cpp — implements examples::Views::ios_blur_effect_page() (declared in ios_blur_effect.xaml.hpp).
//
// This TU OWNS the compile-time XAML: the COMPILER embeds the CANONICAL SHARED page
// port/maui-reference/pages/ios_blur_effect.xaml — the exact same bytes real .NET MAUI compiles via XamlC +
// code-behind (MauiReference.Pages.IosBlurEffectPage) — and build_page hydrates it through the runtime
// loader into a fresh page_impl<no_view_model>. One file, two frameworks, zero divergence.
//
// HAND-WRITTEN code-behind — the generator-owned marker line is deliberately ABSENT so the generator
// leaves this file alone (same mechanism as chrome.xaml.cpp / gestures.xaml.cpp, and the same one the
// MAUI side's IosBlurEffectPage.xaml.cs uses).
//
// WHY. The twin declared four buttons and a readout and wired nothing, while the code-first builder
// (examples/gallery/pages/ios_blur_effect_page.hpp:54-60) connects all four. At rest every column reads
// "BlurEffect: ExtraLight", so the still comparison never saw it — but on a driven frame only the
// code-first column would move, and the board would call that the port's defect.
//
// THE READOUT IS READ BACK, NOT ASSUMED. update_readout() (hpp:141) calls get_blur_effect and formats
// whatever it finds. On this backend — and on Android/Windows — the blur itself is INERT (a stored
// platform-spec with no headless analog, see the hpp header), so the readout is the only evidence the
// tap did anything. Echoing the requested style instead of the stored one would make it claim a state
// nothing verified, which is the exact failure mode the whole motion pass exists to remove.
//
// THE STRINGS ARE A CONTRACT and the code-first page is their ORIGIN: hpp:146-161 is
// `"BlurEffect: " + name_of(style)` over exactly {None, ExtraLight, Light, Dark}.

#include "Views/ios_blur_effect.xaml.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <string_view>

#include "maui/controls/button.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/ios_specific/blur_effect_style.hpp"
#include "maui/controls/platform_configuration/ios_specific/visual_element.hpp"
#include "maui/fixed_string.hpp"
#include "maui/xaml_build.hpp"

namespace
{
    // The raw markup, embedded by the compiler (resolved relative to this source file, like a quoted
    // #include). The two-line embed is the canonical form (a #embed directive cannot live inside a
    // macro body — see fixed_string.hpp).
    constexpr unsigned char ios_blur_effect_xaml_bytes[] = {
#embed "../../../../maui-reference/pages/ios_blur_effect.xaml"
    };
    constexpr maui::fixed_string ios_blur_effect_xaml{ios_blur_effect_xaml_bytes};

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
    static_assert(bytes_contain(ios_blur_effect_xaml_bytes, "<ContentPage"));
    static_assert(bytes_contain(ios_blur_effect_xaml_bytes, "x:Class=\"MauiReference.Pages.IosBlurEffectPage\""));

    using blur_style = maui::controls::platform_configuration::ios_specific::blur_effect_style;

    // A verbatim copy of ios_blur_effect_page.hpp:149-162. Copied rather than shared because the
    // code-first page owns it as a private static; the two must agree or a driven frame diverges.
    const char* name_of(blur_style style)
    {
        switch (style)
        {
            case blur_style::extra_light: return "ExtraLight";
            case blur_style::light: return "Light";
            case blur_style::dark: return "Dark";
            case blur_style::none:
            default: return "None";
        }
    }
} // namespace

namespace examples::Views
{
    std::unique_ptr<maui::controls::content_page> ios_blur_effect_page(const maui::xaml::xaml_load_options& options)
    {
        auto page = maui::build_page<maui::no_view_model, ios_blur_effect_xaml>(options);

        const auto target = page->find<maui::controls::image>("BlurTarget");
        const auto readout = page->find<maui::controls::label>("Readout");
        if (target == nullptr || readout == nullptr)
        {
            return page;   // inert rather than crashing, the same posture chrome.xaml.cpp takes
        }

        // Set, then READ BACK and format what is actually stored — never the value just requested.
        const auto apply = [target, readout](blur_style style) {
            namespace ios_ve = maui::controls::platform_configuration::ios_specific::visual_element;
            ios_ve::use_blur_effect(target->on<maui::controls::platform_configuration::ios>(), style);
            readout->set_text(std::string("BlurEffect: ") +
                              name_of(ios_ve::get_blur_effect(
                                  target->on<maui::controls::platform_configuration::ios>())));
        };

        for (const auto& [name, style] : {std::pair{"NoBlurButton", blur_style::none},
                                          std::pair{"ExtraLightButton", blur_style::extra_light},
                                          std::pair{"LightButton", blur_style::light},
                                          std::pair{"DarkButton", blur_style::dark}})
        {
            if (const auto button = page->find<maui::controls::button>(name))
            {
                page->retain(maui::core::scoped_connection(
                    button->clicked, button->clicked.connect([apply, style] { apply(style); })));
            }
        }
        // unique_ptr<page_impl<no_view_model>> upcasts to unique_ptr<content_page> on return.
        return page;
    }
} // namespace examples::Views
