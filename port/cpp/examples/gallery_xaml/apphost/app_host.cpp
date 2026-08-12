// examples/gallery_xaml/apphost/app_host.cpp — the native side of the C++&XAML gallery APK host.
//
// This is the XAML TWIN of src/platform/android/apphost/app_host.cpp: same JNI boot, same generic mount,
// but the page comes from a COMPILE-TIME-XAML factory (examples::Views::<name>_page(), built from the
// Views/<name>.xaml markup) instead of a hand-written C++ page type. It backs the C++&XAML Android parity
// column — the on-emulator render of the gallery_xaml twin, alongside the C++ builder column
// (src/platform/android/apphost) and the (external) real-.NET-MAUI column.
//
// Why a SEPARATE host (vs reusing the C++ one): the two galleries dispatch differently. The C++ host maps
// a page key to a maui::samples::<name>_page TYPE (MAUI_GALLERY_PAGES); this host maps it to a
// examples::Views::<name>_page() FACTORY FUNCTION that returns a fully-built content_page
// (MAUI_XAML_GALLERY_PAGES, single-sourced from Views/gallery_pages.hpp — the same X-macro the gallery_xaml
// main.cpp uses). So the holder here owns a unique_ptr<content_page> directly, with no per-page pre/post
// mount hooks (the XAML twins are purely structural — no code-behind).
//
// It coexists with the C++ host on the emulator via a DIFFERENT package id (dev.mauicpp.apphost.xaml) and
// its own Activity/manifest, so both APKs can be installed at once and captured back to back.
//
// Sources this is modelled on (port/CLAUDE.md: derive, never invent):
//   - src/platform/android/apphost/app_host.cpp — the JNI mount SEQUENCE (build the app from a fresh
//     builder, create_window, mount_window, drive_layout over the display size, return the content-view
//     FrameLayout) + the process-lifetime host_app_slot(). This host reuses that machinery verbatim except
//     for the page source; the window-bounds JNI walk itself is now SHARED, in
//     src/platform/android/jni/host_layout_rects.hpp (the two hosts' near-duplicate copies had drifted —
//     the fallback chrome height summed both bars in one and only the status bar in the other).
//   - examples/gallery_xaml/main.cpp — the XAML page dispatch: make_selected_page() expands
//     MAUI_XAML_GALLERY_PAGES to a `selected == "<name>" -> examples::Views::<name>_page()` switch, default
//     value_controls. We replicate that dispatch here (main.cpp's is in an anonymous namespace, not reusable).
//   - examples/gallery_xaml/Views/gallery_pages.hpp — the MAUI_XAML_GALLERY_PAGES(X) X-macro + every
//     per-page factory declaration. Included here so examples::Views::<name>_page() resolves.
//
// Lifetime (PROFILE §8): identical to the C++ host — the built maui_app owns application -> window ->
// page -> control tree; it is parked in a process-lifetime slot (never freed) so the tree the Activity is
// displaying is never torn down under it. Single-page capture host: one page per process, re-launched per
// key by the capture pipeline.

#include <android/log.h> // the skipped-markup warning (see loader_options)
#include <jni.h>

#include <cstring>
#include <memory>
#include <string>

#include "jni/app_context.hpp"
#include "jni/host_layout_rects.hpp" // the FULL + SAFE window rects the two-rect drive_layout takes
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "jni/relayout.hpp" // set_relayout_hook — replay this host's layout pass on a late resize

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"
#include "maui/xaml/xaml_loader.hpp" // xaml_load_options — the exception_handler knob (see loader_options)

// The compile-time-XAML page factories + the MAUI_XAML_GALLERY_PAGES(X) dispatch macro (single-sourced).
// The gallery_xaml example root is on the include path so this "Views/..." include resolves.
#include "Views/gallery_pages.hpp"

namespace
{
    using maui::platform::android::to_utf8;

    // The loader knobs every page load on this host runs under. The ONLY one set is C#'s doNotThrow knob
    // (HydrationContext.ExceptionHandler, wired from ResourceLoader.ExceptionHandler2 in
    // XamlLoader.cs:101-107): without it the loader RE-THROWS (hydration_context::handle) and ONE
    // unsupported attribute — e.g. gap_event_attribute's Clicked="OnClicked", which needs the reflection
    // the port does not have — escapes to std::terminate and SIGABRTs this process. The capture pipeline
    // then photographs whatever came forward (the launcher), so the crash reads as a capture bug. With the
    // handler installed the offending property is skipped, the rest of the tree still renders, and the
    // error is logged — MAUI's own collect-and-continue branch.
    //
    // `application` is deliberately left unset: the loader falls back to maui::controls::application::
    // current(), which is what this host has always relied on (apphost_app seeds the theme in its page_
    // mem-initializer, BEFORE the application object it would pass is fully constructed).
    [[nodiscard]] maui::xaml::xaml_load_options loader_options()
    {
        maui::xaml::xaml_load_options options;
        options.exception_handler = [](const maui::xaml::xaml_parse_exception& error) {
            __android_log_print(ANDROID_LOG_WARN, "maui-xaml", "unsupported markup skipped: %s", error.what());
        };
        return options;
    }

    // Build the selected XAML page (a fully-hydrated content_page) for `key`, via the single-sourced
    // MAUI_XAML_GALLERY_PAGES dispatch — the same table gallery_xaml/main.cpp uses. An unknown key falls
    // back to value_controls (the gallery's own default), so a typo never aborts the capture.
    std::unique_ptr<maui::controls::content_page> make_selected_page(const std::string& key)
    {
        const maui::xaml::xaml_load_options options = loader_options();
#define MAUI_XAML_APPHOST_DISPATCH(name)                                                                               \
    if (key == #name)                                                                                                  \
    {                                                                                                                  \
        return examples::Views::name##_page(options);                                                                  \
    }
        MAUI_XAML_GALLERY_PAGES(MAUI_XAML_APPHOST_DISPATCH)
#undef MAUI_XAML_APPHOST_DISPATCH
        return examples::Views::value_controls_page(options);
    }

    // The single application the builder mints: owns the selected XAML page + the window, hosts the page in
    // the window, and seeds the cross-platform theme from MAUI_APPEARANCE. The XAML twin of the C++ host's
    // apphost_app — but the page is a fully-built content_page (from the factory), not a holder with hooks.
    class apphost_app final : public maui::controls::application
    {
    public:
        explicit apphost_app(const std::string& page_key, bool dark) : page_(make_page_themed(page_key, dark))
        {
            window_.set_title("MAUI C++ — gallery (XAML)");
            window_.set_content(*page_); // window holds a non-owning back-pointer to the page (an element&)
        }

        [[nodiscard]] maui::core::i_window* create_window() override
        {
            return &window_;
        }

    private:
        // Seed the CROSS-PLATFORM theme BEFORE hydrating the XAML page, then build it. make_selected_page
        // HYDRATES the tree and {AppThemeBinding}.pick reads application::requested_theme() AT hydration — so
        // the theme MUST be set first. Doing it here (inside the page_ initializer) rather than in the ctor
        // BODY is the fix: the ctor body runs AFTER the page_ member-init-list construction, so seeding there
        // was too late and EVERY {AppThemeBinding} resolved to the Light branch in dark mode (shape_app_theme
        // dark rendered Green-on-White instead of Red-on-Black). NON-static: set_platform_app_theme is an
        // application member, and the base application subobject is fully constructed before this runs in the
        // page_ mem-initializer, so this->set_platform_app_theme is valid; only base + free functions are
        // touched (no derived members), so calling it before page_/window_ init is safe. The C++ host seeds in
        // the ctor BODY instead — fine there because its code-first pages resolve AppThemeBinding at APPLY
        // time (post-seed), whereas the XAML loader resolves it at HYDRATION time (make_selected_page).
        [[nodiscard]] std::unique_ptr<maui::controls::content_page> make_page_themed(const std::string& key, bool dark)
        {
            set_platform_app_theme(dark ? maui::core::app_theme::dark : maui::core::app_theme::light);
            return make_selected_page(key);
        }

        // Page declared BEFORE the window: the window keeps a non-owning back-pointer to the page
        // (set_content), so the page must outlive the window (members destruct in reverse declaration order).
        std::unique_ptr<maui::controls::content_page> page_;
        maui::controls::window window_;
    };

    // ---- process-lifetime owner (see the Lifetime note in the file header) ----
    std::unique_ptr<maui::hosting::maui_app>& host_app_slot() noexcept
    {
        static std::unique_ptr<maui::hosting::maui_app> slot;
        return slot;
    }

    // Read pageKey / appearance, build + mount the app, and return the window's content-view FrameLayout (a
    // JNI local ref to hand back to the Activity), or nullptr on any failure. Identical shape to the C++
    // host's mount_gallery — only make_selected_page differs (XAML factory vs C++ page type).
    jobject mount_gallery(JNIEnv* env, const std::string& page_key, bool dark)
    {
        // `dark` is forwarded from the JNI export (the MAUI_APPEARANCE intent extra the Activity reads — am
        // start cannot set the process env, so getenv is empty under a normal launch).

        // (1) Build from a FRESH builder (create_builder() already seeded the controls handler table).
        host_app_slot() =
            maui::hosting::maui_app::create_builder()
                .use_maui_app<apphost_app>([page_key, dark] { return std::make_shared<apphost_app>(page_key, dark); })
                .build();
        maui::hosting::maui_app& app = *host_app_slot();

        // (2) Ask the application for its window.
        const std::shared_ptr<maui::controls::application>& application = app.application();
        if (application == nullptr)
        {
            return nullptr;
        }
        auto* const window = dynamic_cast<maui::controls::window*>(application->create_window());
        if (window == nullptr)
        {
            return nullptr;
        }

        // (3) Generic mount: attach handlers, re-host containers, open the window (creates the real
        //     android.widget.FrameLayout content view + hosts the page's native view in it).
        maui::hosting::mount_window(app, *window);

        // (4) One layout pass over the window's bounds, through the TWO-RECT drive_layout: the FULL
        //     (edge-to-edge) window plus the safe-area rect inside it, so each view applies the system-bar
        //     inset for itself exactly as MAUI's android host does. Twin of the non-XAML host's step 4; the
        //     derivation lives in src/platform/android/jni/host_layout_rects.hpp, which both hosts share.
        const auto [full_bounds, safe_bounds] = maui::platform::android::layout_rects(env);
        maui::hosting::drive_layout(*window, full_bounds, safe_bounds);

        // (4b) Register that pass as the process' RELAYOUT hook (jni/relayout.hpp) — the twin of the
        //      non-XAML host's step 4b. A view whose desired size changes AFTER this one-shot mount (the
        //      image handler's async uri decode) replays it; `app` is leaked for the process lifetime, so
        //      the captured window outlives every callback.
        maui::platform::android::set_relayout_hook(
            [window, full_bounds, safe_bounds] { maui::hosting::drive_layout(*window, full_bounds, safe_bounds); });

        // (4c) ALSO install the general window::request_relayout hook (window.hpp) — the twin of the
        //      non-XAML host's step 4c. view<>::invalidate_measure routes through containing_window(), not
        //      through the Android-only slot above, so without this the now-live invalidate_measure() call
        //      sites would stay no-ops on this example host.
        window->set_relayout_hook(
            [window, full_bounds, safe_bounds] { maui::hosting::drive_layout(*window, full_bounds, safe_bounds); });

        // (5) Reach the native FrameLayout through the window's handler and return a fresh local ref.
        const auto handler = std::dynamic_pointer_cast<maui::core::window_handler>(window->handler());
        if (handler == nullptr || handler->typed_platform_view() == nullptr)
        {
            return nullptr;
        }
        auto* const native = static_cast<jobject>(handler->typed_platform_view()->native);
        if (native == nullptr)
        {
            return nullptr;
        }
        return env->NewLocalRef(native);
    }
} // namespace

// MauiHostActivity.nativeMount(String pageKey) -> android.view.View. The JNI export name encodes the
// fully-qualified class dev.mauicpp.apphost.xaml.MauiHostActivity (underscores escaped per JNI mangling).
extern "C" JNIEXPORT jobject JNICALL Java_dev_mauicpp_apphost_xaml_MauiHostActivity_nativeMount(JNIEnv* env,
                                                                                                jobject activity,
                                                                                                jstring page_key,
                                                                                                jstring appearance)
{
    namespace android = maui::platform::android;

    JavaVM* vm = nullptr;
    if (env->GetJavaVM(&vm) == JNI_OK)
    {
        android::set_java_vm(vm);
    }
    android::set_app_context(env->NewGlobalRef(activity));

    const std::string key = to_utf8(env, page_key);
    // Dark from the DEVICE theme the Activity read (Configuration.uiMode — see MauiHostActivity.onCreate).
    // The MAUI_APPEARANCE fallback below is only reachable when Java hands over an empty string, which the
    // Activity no longer does; kept for a VM-less/direct-JNI caller rather than as the normal path.
    std::string appear = to_utf8(env, appearance);
    if (appear.empty())
    {
        const char* const env_appear = std::getenv("MAUI_APPEARANCE");
        if (env_appear != nullptr)
        {
            appear = env_appear;
        }
    }
    const bool dark = appear == "dark";
    return mount_gallery(env, key.empty() ? std::string{"value_controls"} : key, dark);
}
