// gallery_xaml/main.cpp — the WITH-XAML twin of examples/gallery: the same demo gallery, but every page is
// authored as MAUI markup (Views/<name>.xaml) and built at COMPILE TIME (#embed + maui::build_page), exactly
// like the other _xaml examples (counter_xaml, hello_world_xaml). main.cpp is just the app shell — it picks
// ONE page at runtime and hosts it; all markup mechanics live in the per-page Views/<name>.xaml.cpp TUs.
//
// Page selection mirrors the builder gallery so the parity tooling is unchanged: MAUI_SAMPLE_PAGE (or its
// MAUI_XAML_TWIN alias) names one twin; the default is value_controls. The string -> compile-time factory
// dispatch is single-sourced from Views/gallery_pages.hpp's MAUI_XAML_GALLERY_PAGES X-macro (the analog of
// the builder gallery's MAUI_GALLERY_PAGES). Theme follows the SYSTEM appearance trait — like the other
// _xaml examples, no per-app env is needed (the parity harness sets it with `simctl ui appearance`).
//
// 100% PORTABLE C++: no platform headers. The same source builds + runs on headless, macOS, and iOS.

#include "maui/maui_main.hpp"

#include "maui/ui.hpp"

#include "maui/controls/content_page.hpp"
#include "maui/core/app_theme.hpp"   // MAUI_APPEARANCE -> set_platform_app_theme (parity column consistency)
#include "maui/xaml/xaml_loader.hpp" // xaml_load_options — thread the application into the page loads

#include "Views/gallery_pages.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

namespace ui = maui::ui;

namespace
{
    // Runtime page string -> compile-time page factory. MAUI_SAMPLE_PAGE matches the parity tooling's
    // SIMCTL_CHILD_MAUI_SAMPLE_PAGE; MAUI_XAML_TWIN is accepted as an alias (the name the old gallery used for
    // its runtime-loaded twin column). An unknown name falls back to value_controls — the gallery default.
    [[nodiscard]] std::unique_ptr<maui::controls::content_page> make_selected_page(
        const maui::xaml::xaml_load_options& options)
    {
        const char* env = std::getenv("MAUI_SAMPLE_PAGE");
        if (env == nullptr || std::strlen(env) == 0)
        {
            env = std::getenv("MAUI_XAML_TWIN");
        }
        const std::string selected = (env != nullptr && std::strlen(env) > 0) ? env : "value_controls";

#define MAUI_XAML_PAGE(name)                                                                                           \
    if (selected == #name)                                                                                             \
    {                                                                                                                  \
        return examples::Views::name##_page(options);                                                                  \
    }
        MAUI_XAML_GALLERY_PAGES(MAUI_XAML_PAGE)
#undef MAUI_XAML_PAGE

        return examples::Views::value_controls_page(options);
    }
} // namespace

// The single gallery application: it owns the selected XAML-built page and hosts it. Pure C++ via the ui::app
// facade — identical in shape to the other _xaml examples, only the page is chosen at runtime.
class gallery_xaml_app final : public ui::app
{
public:
    gallery_xaml_app()
    {
        // MAUI_APPEARANCE OVERRIDES the OS theme; unset means FOLLOW IT — same contract as the builder
        // gallery (examples/gallery/main.cpp), so the two C++ columns still share one appearance whether the
        // harness pins it or not. UserAppTheme is MAUI's own override knob; PlatformAppTheme was seeded from
        // AppInfo.RequestedTheme by the application ctor.
        if (const char* const appearance = std::getenv("MAUI_APPEARANCE"); appearance != nullptr)
        {
            set_user_app_theme(std::strcmp(appearance, "dark") == 0 ? maui::core::app_theme::dark
                                                                    : maui::core::app_theme::light);
        }

        // Thread THIS application into the loader (the port's explicit Application.Current): with it,
        // {AppThemeBinding} resolves against the requested theme seeded above AND re-applies on every
        // RequestedThemeChanged — the C# AppThemeBinding contract. The app outlives the page it owns,
        // so the load's theme subscriptions (owned by the page's load result) disconnect first.
        set_content(ui::view_ref<maui::controls::content_page>{std::shared_ptr<maui::controls::content_page>{
            make_selected_page(maui::xaml::xaml_load_options{.application = this})}});
        set_title("MAUI C++ — gallery (XAML)");
    }
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<gallery_xaml_app>();
    return builder;
}
