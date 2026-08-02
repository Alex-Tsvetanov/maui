// gallery/main.cpp — the runnable demo gallery as a PURE C++ maui app (no Objective-C, no .mm, no platform
// headers). The same source builds + runs on headless, macOS, and iOS: a plain executable on headless/macOS,
// an installable .app (bundle id dev.maui-cpp.ios-gallery) on iOS. It replaces the former in-tree gallery
// hosts (src/samples/{ios,macos}_gallery.mm), which hand-rolled UIApplicationMain / NSApplication glue.
//
// What it does, all through the framework's generic hosting seam:
//   - MAUI_SAMPLE_PAGE (default "value_controls") selects ONE of the curated demo pages (gallery/pages/*.hpp).
//     The page list is single-sourced in gallery_host.hpp's MAUI_GALLERY_PAGES X-macro; the ctor expands it
//     to a runtime string -> compile-time page dispatch, owning the chosen page behind a small type-erased
//     holder (so a single non-templated gallery_app can host any page the env var names).
//   - MAUI_APPEARANCE (dark|light) sets the CROSS-PLATFORM application theme via set_platform_app_theme — NO
//     UIKit/AppKit call here. The framework's iOS/macOS run_app reads application::requested_theme() after
//     boot and forces the NATIVE window interface style from it, so the parity-capture dark/light shots work
//     without any platform code leaking into this example (src/platform/{ios,apple}/host_run.mm).
//   - The two optional per-page mount hooks (a page's register_handlers / on_mounted) are forwarded through
//     gallery_app::on_pre_mount / on_post_mount — the application-level generalization of the gallery's former
//     per-page plumbing, which the generic mount_window now calls around the window mount (app_host.hpp).
//
// Build + run: see examples/README.md (headless/macOS plain exe; iOS .app via maui_add_app) and the build
// command documented at the top of tools/parity/capture_all.py.

#include "maui/maui_main.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/i_window.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_host.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

// The debug-gated DevFlow test/automation agent (absent in release builds). Opt-in at RUNTIME via the
// MAUI_DEVFLOW_PORT env var so normal gallery/parity-capture runs are undisturbed — the agent is never
// auto-started (mirrors DevFlow's explicit AddMauiDevFlowAgent()).
#if defined(MAUI_DEVFLOW)
    #include "maui/devflow/agent.hpp"

    #include <cstdint>
    #include <functional>
    #include <future>
#endif

namespace
{
    // A type-erased owner of one demo page: the single non-templated gallery_app picks the concrete page from
    // the runtime MAUI_SAMPLE_PAGE string, but the rest of the app only needs the page's root element + the
    // two optional mount hooks. This base hides the concrete Page type behind those three operations.
    struct gallery_page_holder
    {
        gallery_page_holder() = default;
        gallery_page_holder(const gallery_page_holder&) = delete;
        gallery_page_holder& operator=(const gallery_page_holder&) = delete;
        virtual ~gallery_page_holder() = default;

        // The page's root element (a content_page, or flyout_page for tabbed_flyout) — what the window hosts.
        [[nodiscard]] virtual maui::controls::element& root() = 0;
        // Forward the page's optional pre-/post-mount hooks (no-ops unless the page declares them). Reuses the
        // gallery_host.hpp requires-detected free functions so the opt-in test lives in exactly one place.
        virtual void pre_mount(maui::hosting::maui_app& app) = 0;
        virtual void post_mount(maui::hosting::maui_app& app) = 0;
    };

    template <class Page> struct gallery_page_holder_impl final : gallery_page_holder
    {
        [[nodiscard]] maui::controls::element& root() override
        {
            return page_.page();
        }
        void pre_mount(maui::hosting::maui_app& app) override
        {
            maui::samples::gallery_pre_mount(app, page_);
        }
        void post_mount(maui::hosting::maui_app& app) override
        {
            maui::samples::gallery_post_mount(app, page_);
        }

        Page page_;
    };

    // Construct the holder for the page MAUI_SAMPLE_PAGE names (default value_controls), via the single-sourced
    // X-macro dispatch. An unknown name logs nothing here (pure C++ — no os_log) and falls back to the default.
    // (The compile-time-XAML twin of this gallery is the separate examples/gallery_xaml app.)
    std::unique_ptr<gallery_page_holder> make_selected_page()
    {
        const char* const env = std::getenv("MAUI_SAMPLE_PAGE");
        const std::string selected = (env != nullptr && std::strlen(env) > 0) ? env : "value_controls";

#define MAUI_GALLERY_DISPATCH(name, page_type)                                                                         \
    if (selected == (name))                                                                                            \
    {                                                                                                                  \
        return std::make_unique<gallery_page_holder_impl<maui::samples::page_type>>();                                 \
    }
        MAUI_GALLERY_PAGES(MAUI_GALLERY_DISPATCH)
#undef MAUI_GALLERY_DISPATCH

        return std::make_unique<gallery_page_holder_impl<maui::samples::value_controls_page>>();
    }
} // namespace

// The single gallery application the builder mints. It owns the selected page (behind the holder) + the
// window, hosts the page in the window, and seeds the cross-platform theme from MAUI_APPEARANCE — all pure
// C++. The framework's native run_app forces the native interface style from requested_theme() after boot.
class gallery_app final : public maui::controls::application
{
public:
    gallery_app() : page_(make_selected_page())
    {
        // Appearance: MAUI_APPEARANCE OVERRIDES the OS theme; unset means FOLLOW IT. UserAppTheme is exactly
        // MAUI's knob for "the app forces a theme" (RequestedTheme = UserAppTheme when set, else
        // PlatformAppTheme), and the application ctor already seeded PlatformAppTheme from
        // AppInfo.RequestedTheme — so this needs to do nothing at all in the unset case.
        //
        // It used to call set_platform_app_theme(dark ? dark : light), which forced LIGHT whenever the env
        // var was absent. That impersonated the platform, and it is why a board capture could never catch the
        // framework bug fixed in application.cpp: every capture sets MAUI_APPEARANCE, so the unseeded path
        // this harness is supposed to exercise was the one path it never took.
        if (const char* const appearance = std::getenv("MAUI_APPEARANCE"); appearance != nullptr)
        {
            set_user_app_theme(std::strcmp(appearance, "dark") == 0 ? maui::core::app_theme::dark
                                                                    : maui::core::app_theme::light);
        }

        window_.set_title("MAUI C++ — gallery");
        window_.set_content(page_->root()); // window holds a non-owning back-pointer to the page root
    }

    [[nodiscard]] maui::core::i_window* create_window() override
    {
        return &window_;
    }

    // Forward the selected page's optional mount hooks (the application-level generalization the generic
    // mount_window calls around the window mount — app_host.hpp).
    void on_pre_mount(maui::hosting::maui_app& app) override
    {
        page_->pre_mount(app);
    }
    void on_post_mount(maui::hosting::maui_app& app) override
    {
        page_->post_mount(app);
#if defined(MAUI_DEVFLOW)
        start_devflow_if_requested(app);
#endif
    }

private:
#if defined(MAUI_DEVFLOW)
    // Start the DevFlow agent iff MAUI_DEVFLOW_PORT is set (explicit runtime opt-in — never auto-started).
    // The root provider hands back the current page root; the UI executor marshals each request onto the
    // app dispatcher's thread (PROFILE §8) via a promise/future so tree reads/taps run on the UI thread.
    void start_devflow_if_requested(maui::hosting::maui_app& app)
    {
        const char* const env = std::getenv("MAUI_DEVFLOW_PORT");
        if (env == nullptr || std::strlen(env) == 0)
        {
            return;
        }
        const auto port = static_cast<std::uint16_t>(std::atoi(env));

        auto* dispatcher = &app.dispatcher();
        maui::devflow::agent::ui_executor run_on_ui = [dispatcher](const std::function<void()>& work) {
            if (!dispatcher->is_dispatch_required())
            {
                work();
                return;
            }
            std::promise<void> done;
            auto future = done.get_future();
            dispatcher->dispatch([&work, &done] {
                work();
                done.set_value();
            });
            future.wait();
        };

        maui::devflow::start_agent([this] { return &page_->root(); }, port,
                                   {.app = "maui-cpp gallery", .version = "1.0", .commit = ""}, std::move(run_on_ui));
    }
#endif

    // The page holder is declared BEFORE the window: the window keeps a non-owning back-pointer to the page
    // root (set_content), so the page must outlive the window (members destruct in reverse declaration order).
    std::unique_ptr<gallery_page_holder> page_;
    maui::controls::window window_;
};

// The MauiProgram.CreateMauiApp shape: register the gallery app on a fresh builder and return it.
maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<gallery_app>();
    return builder;
}
