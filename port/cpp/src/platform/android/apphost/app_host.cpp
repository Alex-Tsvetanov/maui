// maui::platform::android::apphost — the native side of the real-Activity gallery APK host.
//
// This is the Android boot the framework does not yet have a run_app for: the apple/ios backends ship a
// host_run.mm whose run_app builds the app, mounts the window generically, and shows the native window;
// android has NO run_app linked (CMakeLists.txt's host_run generator-expression covers only headless/
// apple/ios — see the "windows/android remain later stages" note), so THIS JNI entry plays that role.
// MauiHostActivity.onCreate calls nativeMount(pageKey); we build the gallery page for that key, mount it
// through the SAME generic driver the other backends use, and RETURN the window's content view to the
// Activity instead of spinning a run loop — the Activity's setContentView puts it on screen.
//
// Sources this is modelled on (port/CLAUDE.md: derive, never invent):
//   - src/platform/android/testhost/test_host.cpp — the JNI bootstrap shape: GetJavaVM + set_java_vm,
//     NewGlobalRef(context) + set_app_context, the Java_dev_mauicpp_<pkg>_<Class>_native<Method> export,
//     scoped_env / to_utf8 usage. (That host runs gtest; this one mounts a UI and returns a View.)
//   - src/platform/headless/host_run.cpp — the cross-platform mount SEQUENCE: build the maui_app from a
//     fresh builder the configurator populates, ask the application for its window, mount_window, then
//     drive_layout. We replicate that body here (android has no headless run_app to call).
//   - src/platform/apple/host_run.mm — how a native backend reaches the window's native view AFTER the
//     mount: dynamic_pointer_cast<window_handler>(window->handler())->typed_platform_view()->native. On
//     apple that void* is an NSWindow shown key+front; on android it is the content-view FrameLayout
//     (src/platform/android/window_handler.cpp's window_platform::native), which we hand back to Java.
//   - examples/gallery/gallery_host.hpp — the MAUI_GALLERY_PAGES(X) X-macro (runtime key -> page type)
//     and the sample_app/gallery_app<Page> shape. examples/gallery/main.cpp's gallery_app +
//     make_selected_page live in an anonymous namespace (not reusable across TUs), so — exactly as the
//     resume doc instructs — we replicate the X-macro dispatch + a type-erased page holder + a single
//     application subclass here rather than #including that .cpp.
//
// Handler registration: maui_app_builder's constructor already seeds the default control -> handler table
// via add_maui_controls_handlers (src/hosting/maui_app_builder.cpp), so building from
// maui_app::create_builder() registers every control's handler. Which PARTIAL each handler uses (the
// android JNI widget vs the headless mirror) is a LINK-TIME choice: the android CMake preset swaps
// src/platform/android/<ctrl>_handler.cpp in for the headless twin (CMakeLists.txt MAUI_CORE_PLATFORM_
// SOURCES). So there is no per-platform handler list to pass here; we just build the app.
//
// VM-less degradation is irrelevant in this TU: nativeMount only ever runs WITH a JavaVM + a real
// Activity context (it is a JNI entry from a launched Activity), so the window_handler's real-FrameLayout
// path is always taken and typed_platform_view()->native is a live global ref.
//
// Lifetime (PROFILE §8): the built maui_app owns the application -> window -> page -> control tree, and the
// handlers keep raw context pointers into it; it must outlive every native view the Activity is showing.
// A real run loop holds it in a file-scope owner (apple host_run's host_state); here the Activity stays
// alive for the whole capture, so we leak the maui_app deliberately into a process-lifetime slot (released
// only at process exit, after the Activity is gone) rather than letting it destruct when nativeMount
// returns (which would tear down the tree the Activity is still displaying). Single-page capture host:
// one page per process, re-launched per key by the capture pipeline.

#include <jni.h>

#include <cstring>
#include <memory>
#include <string>

#include "jni/app_context.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"

// The curated gallery page set + the per-page mount hooks (gallery_pre_mount / gallery_post_mount). The
// page headers are pulled in transitively by gallery_host.hpp (every pages/*.hpp). The apphost CMake
// target adds examples/gallery to the include path so this resolves.
#include "gallery_host.hpp"

namespace
{
    using maui::platform::android::to_utf8;

    // The Activity's display size in framework POINTS (px / density), forward-declared so mount_gallery can
    // size the first layout pass with it; defined after mount_gallery for locality (the JNI walk is long).
    struct size2
    {
        double width;
        double height;
    };
    size2 display_size(JNIEnv* env);

    // ---- the selected-page holder (the examples/gallery/main.cpp shape, replicated per the resume doc) ----
    // A type-erased owner of one demo page: the X-macro picks the concrete page type from the runtime
    // pageKey string, but the application only needs the page's root element + the two optional mount hooks.
    struct gallery_page_holder
    {
        gallery_page_holder() = default;
        gallery_page_holder(const gallery_page_holder&) = delete;
        gallery_page_holder& operator=(const gallery_page_holder&) = delete;
        virtual ~gallery_page_holder() = default;

        [[nodiscard]] virtual maui::controls::element& root() = 0;
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

    // Build the holder for the page `key` names, via the single-sourced MAUI_GALLERY_PAGES dispatch
    // (gallery_host.hpp). An unknown key falls back to value_controls (the gallery's own default), so a
    // typo never aborts the capture — it shows a known page instead.
    std::unique_ptr<gallery_page_holder> make_selected_page(const std::string& key)
    {
#define MAUI_APPHOST_DISPATCH(name, page_type)                                                                         \
    if (key == (name))                                                                                                 \
    {                                                                                                                  \
        return std::make_unique<gallery_page_holder_impl<maui::samples::page_type>>();                                 \
    }
        MAUI_GALLERY_PAGES(MAUI_APPHOST_DISPATCH)
#undef MAUI_APPHOST_DISPATCH
        return std::make_unique<gallery_page_holder_impl<maui::samples::value_controls_page>>();
    }

    // ---- the single application the builder mints (the gallery_app shape, page chosen at runtime) ----
    // Owns the selected page (behind the holder) + the window, hosts the page in the window, seeds the
    // cross-platform theme from MAUI_APPEARANCE, and forwards the page's optional mount hooks — the exact
    // contract examples/gallery/main.cpp's gallery_app implements (kept in lock-step so the same pages
    // mount identically on android as on the other backends).
    class apphost_app final : public maui::controls::application
    {
    public:
        explicit apphost_app(const std::string& page_key, bool dark) : page_(make_selected_page(page_key))
        {
            // Seed the CROSS-PLATFORM theme BEFORE the tree mounts (theme-reactive pages read the right slot
            // at attach). A native backend's run_app then forces the native interface style from
            // requested_theme(); on android the dark/light system-bar styling is a future window_handler
            // concern, but the cross-platform theme still drives every theme-bound brush/color in the tree.
            set_platform_app_theme(dark ? maui::core::app_theme::dark : maui::core::app_theme::light);

            window_.set_title("MAUI C++ — gallery");
            window_.set_content(page_->root()); // window holds a non-owning back-pointer to the page root
        }

        [[nodiscard]] maui::core::i_window* create_window() override
        {
            return &window_;
        }

        void on_pre_mount(maui::hosting::maui_app& app) override
        {
            page_->pre_mount(app);
        }
        void on_post_mount(maui::hosting::maui_app& app) override
        {
            page_->post_mount(app);
        }

    private:
        // Page declared BEFORE the window: the window keeps a non-owning back-pointer to the page root
        // (set_content), so the page must outlive the window (members destruct in reverse declaration order).
        std::unique_ptr<gallery_page_holder> page_;
        maui::controls::window window_;
    };

    // ---- process-lifetime owners (see the Lifetime note in the file header) ----
    // The built maui_app + the page key/appearance it was built from, kept alive for the process so the
    // tree the Activity is displaying is never torn down under it. Never freed (process exit reclaims it).
    std::unique_ptr<maui::hosting::maui_app>& host_app_slot() noexcept
    {
        static std::unique_ptr<maui::hosting::maui_app> slot;
        return slot;
    }

    // Read pageKey / appearance, build + mount the app, and return the window's content-view FrameLayout
    // (a JNI local ref to hand back to the Activity), or nullptr on any failure. Separated from the JNI
    // export so the export stays a thin trampoline.
    jobject mount_gallery(JNIEnv* env, const std::string& page_key)
    {
        // Appearance from the same env var the gallery mains read (the capture pipeline sets it per shot).
        const char* const appearance = std::getenv("MAUI_APPEARANCE");
        const bool dark = appearance != nullptr && std::strcmp(appearance, "dark") == 0;

        // (1) Build from a FRESH builder. The configurator registers our application; create_builder() has
        //     already seeded the controls handler table (maui_app_builder ctor -> add_maui_controls_handlers).
        host_app_slot() =
            maui::hosting::maui_app::create_builder()
                .use_maui_app<apphost_app>([page_key, dark] { return std::make_shared<apphost_app>(page_key, dark); })
                .build();
        maui::hosting::maui_app& app = *host_app_slot();

        // (2) Ask the application for its window (IApplication.CreateWindow — apphost_app::create_window).
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

        // (3) Generic mount: attach handlers across the tree (children before parents), re-host each
        //     container, attach the window handler, open the window — the window_handler creates the real
        //     android.widget.FrameLayout content view and hosts the page's native view in it (the same
        //     mount_window the headless/apple lanes use; app_host.hpp).
        maui::hosting::mount_window(app, *window);

        // (4) One layout pass over the window's content bounds, sized to the device display (the android
        //     analog of the native run loop's first layout). The Activity is full-screen, so the display
        //     metrics width/height are the content size. drive_layout takes points; android widgets size in
        //     pixels, but the framework's layout is in density-independent points and the android handlers
        //     to_pixels at the seam, so we pass the metrics directly (the integrator should confirm the
        //     px/dp convention here — see the uncertainties list). Falls back to the headless default
        //     phone-ish size if the metrics could not be read.
        const auto [width, height] = display_size(env);
        maui::hosting::drive_layout(*window, width, height);

        // (5) Reach the native FrameLayout through the window's now-attached handler and return it. This is
        //     the apple host's typed_platform_view()->native step, except the void* is the content-view
        //     FrameLayout global ref (window_handler.cpp) instead of an NSWindow.
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
        // The pimpl owns `native` as a global ref for the process; hand a fresh LOCAL ref back to Java (the
        // JNI return-value contract — the Activity adds it as its content view, which retains it).
        return env->NewLocalRef(native);
    }

    // The Activity's display metrics (widthPixels x heightPixels) via JNI:
    // activity.getResources().getDisplayMetrics().{widthPixels,heightPixels}, divided by the metrics
    // `density` to yield framework POINTS. Falls back to a portrait phone viewport (the headless/ios
    // default) when any step fails, so the mount still settles. The Activity is reached through
    // app_context() — the JNI export below pinned the Activity as the process context (it IS-A Context).
    size2 display_size(JNIEnv* env)
    {
        constexpr size2 fallback{402.0, 874.0}; // the ios/headless gallery default (host_run.cpp)
        jobject activity = maui::platform::android::app_context();
        if (env == nullptr || activity == nullptr)
        {
            return fallback;
        }
        // activity.getResources() : android.content.res.Resources
        const maui::platform::android::local_ref<jclass> context_class{env, env->GetObjectClass(activity)};
        if (!context_class)
        {
            return fallback;
        }
        jmethodID get_resources =
            env->GetMethodID(context_class.get(), "getResources", "()Landroid/content/res/Resources;");
        if (get_resources == nullptr || env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionClear();
            return fallback;
        }
        const maui::platform::android::local_ref<jobject> resources{env,
                                                                    env->CallObjectMethod(activity, get_resources)};
        if (env->ExceptionCheck() == JNI_TRUE || !resources)
        {
            env->ExceptionClear();
            return fallback;
        }
        // resources.getDisplayMetrics() : android.util.DisplayMetrics
        const maui::platform::android::local_ref<jclass> resources_class{env, env->GetObjectClass(resources.get())};
        jmethodID get_metrics =
            env->GetMethodID(resources_class.get(), "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
        if (get_metrics == nullptr || env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionClear();
            return fallback;
        }
        const maui::platform::android::local_ref<jobject> metrics{env,
                                                                  env->CallObjectMethod(resources.get(), get_metrics)};
        if (env->ExceptionCheck() == JNI_TRUE || !metrics)
        {
            env->ExceptionClear();
            return fallback;
        }
        const maui::platform::android::local_ref<jclass> metrics_class{env, env->GetObjectClass(metrics.get())};
        jfieldID width_field = env->GetFieldID(metrics_class.get(), "widthPixels", "I");
        jfieldID height_field = env->GetFieldID(metrics_class.get(), "heightPixels", "I");
        jfieldID density_field = env->GetFieldID(metrics_class.get(), "density", "F");
        if (width_field == nullptr || height_field == nullptr)
        {
            env->ExceptionClear();
            return fallback;
        }
        const jint width_px = env->GetIntField(metrics.get(), width_field);
        const jint height_px = env->GetIntField(metrics.get(), height_field);
        // density (px per dp) converts the device PIXELS the metrics report into the density-independent
        // POINTS the framework lays out in (the android handlers to_pixels back at the seam). A metrics
        // object always carries density; guard anyway and treat <=0 as 1.0 (px == dp).
        jfloat density = density_field != nullptr ? env->GetFloatField(metrics.get(), density_field) : 1.0F;
        if (density <= 0.0F)
        {
            density = 1.0F;
        }
        if (width_px <= 0 || height_px <= 0)
        {
            return fallback;
        }
        return {static_cast<double>(width_px) / density, static_cast<double>(height_px) / density};
    }
} // namespace

// MauiHostActivity.nativeMount(String pageKey) -> android.view.View. The JNI export name encodes the
// fully-qualified class dev.mauicpp.apphost.MauiHostActivity (underscores escaped per the JNI mangling).
// `activity` is the MauiHostActivity instance (an android.content.Context) — we pin the JavaVM + a global
// ref to it as the process app context (every android handler partial creates its widgets from
// app_context()), then mount the gallery and return the root view.
extern "C" JNIEXPORT jobject JNICALL Java_dev_mauicpp_apphost_MauiHostActivity_nativeMount(JNIEnv* env,
                                                                                           jobject activity,
                                                                                           jstring page_key)
{
    namespace android = maui::platform::android;

    JavaVM* vm = nullptr;
    if (env->GetJavaVM(&vm) == JNI_OK)
    {
        android::set_java_vm(vm); // global_ref teardown + scoped_env need the VM (jni_env.hpp)
    }
    // Pin the Activity as the process-wide app context (it IS-A android.content.Context). A global ref the
    // process keeps for its lifetime — deliberately never released (the Activity outlives the capture, and
    // the android handler partials create their widgets from this context, exactly as the test host pins
    // its themed Context). NewGlobalRef so it survives past this native call.
    android::set_app_context(env->NewGlobalRef(activity));

    const std::string key = to_utf8(env, page_key);
    return mount_gallery(env, key.empty() ? std::string{"label"} : key);
}
