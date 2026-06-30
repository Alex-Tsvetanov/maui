// button_handler — Android (JNI) platform partial, the first control of the M-android per-control
// fan-out (M6's iOS Rosetta Stone replayed over JNI). The managed platform view is a REAL
// android.widget.Button (held as a JNI global reference in button_platform::native), every map_*
// pushes its property through the jni_cache'd method ids, and a native click flows back through the
// dev.mauicpp.NativeOnClickListener trampoline (RegisterNatives — reflection-free) into
// i_button::send_clicked.
//
// Ported DIRECTLY from ButtonHandler.Android.cs + Platform/Android/{ButtonExtensions.cs,
// TextViewExtensions.cs, ViewExtensions.cs, MauiRippleDrawableExtensions.cs (GetStrokeProperties),
// ContextExtensions.cs (ToPixels), UnitExtensions.cs (ToEm)} + Fonts/FontManager.Android.cs.
//
// DOCUMENTED DEVIATIONS from the C# oracle (first cut — each is a Material-library or
// infrastructure gap, not a behavior guess):
//   - The widget is a plain android.widget.Button, not MauiMaterialButton: the Material Components
//     library is a gradle/AAR dependency this APK-less backend does not carry. The Material-only
//     construction knobs (IconGravity / IconTintMode / IconTint) have no plain-widget analog and are
//     skipped; SoundEffectsEnabled=false is ported (it is a plain View property).
//   - Stroke + corner radius + background color land in ONE GradientDrawable installed as the
//     button's background (the shape MauiRippleDrawableExtensions builds layers of), instead of
//     MaterialButton.StrokeColor/StrokeWidth/CornerRadius over a RippleDrawable. The drawable is
//     installed LAZILY — only once the view actually carries a visible stroke, a positive corner
//     radius, or a background paint — so an untouched button keeps its default theme background.
//     Installing it replaces that default background (no ripple recreation), like C#'s fallback path.
//   - The port's colors are non-nullable value types, so C#'s null-color branches collapse exactly
//     as they did in the apple/ios partials: MapTextColor's "restore the Material defaults" branch
//     and GetStrokeProperties' DefaultStrokeColor/DefaultStrokeThicknessWithColor logic
//     (StrokeThickness < 0 maps to width 0 = DefaultStrokeThicknessNoColor).
//   - FontManager's registrar/asset/file lookups and the "-light"/"-medium" suffix map are skipped
//     (the port has no font registrar yet, on any backend): family goes straight to
//     Typeface.create(family, style), then the API-28+ Typeface.create(base, weight, italic)
//     refinement — the exact CreateTypeface tail for a non-registered family.
//   - The OnTouchListener (MotionEvent Down→Pressed, Up/Cancel→Released) and the focus/layout-change
//     wiring are deferred with the gesture fan-out; on_press/on_release stay invokable C++ callbacks
//     (the cross-platform suite drives them) and the real OnClickListener carries Clicked.
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the
// emulator (tools/android-emu-run.sh) where no Java VM exists. Every JNI path here checks
// scoped_env/app_context() and quietly skips, while the headless mirrors (title/text_color/…) are
// ALWAYS maintained — so that suite observes exactly the headless partial's behavior, and the
// widget test host (tools/android-testhost-run.sh) additionally observes the real widget.

#include "maui/core/button_handler.hpp"

#include <jni.h>

#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "android_clip_ops.hpp"
#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    // All instance methods are resolved through the widget's own class (GetMethodID walks the
    // superclasses, so the View/TextView surface resolves through android/widget/Button too).
    constexpr const char* k_button_class = "android/widget/Button";
    constexpr const char* k_gradient_drawable_class = "android/graphics/drawable/GradientDrawable";
    constexpr const char* k_typeface_class = "android/graphics/Typeface";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";
    constexpr const char* k_click_listener_class = "dev/mauicpp/NativeOnClickListener";

    // ButtonHandler.DefaultPadding (Android) — "the Material Components minimum size" derivation:
    // horizontal 16dp, vertical 8.5dp; substituted when the cross-platform Padding is NaN.
    constexpr double k_default_padding_horizontal = 16;
    constexpr double k_default_padding_vertical = 8.5;

    // FontManager.DefaultFontSize (Android) — 14sp.
    constexpr float k_default_font_size = 14.0F;

    // UnitExtensions.EmCoefficient — TextView.LetterSpacing speaks em, CharacterSpacing speaks pt.
    constexpr float k_em_coefficient = 0.0624F;

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_AUTO (PlatformInterop restores it after
    // setContentDescription auto-flips the view to YES).
    constexpr jint k_important_for_accessibility_auto = 0;

    // android.graphics.Typeface styles (FontManager.ToTypefaceStyle's targets).
    constexpr jint k_typeface_normal = 0;
    constexpr jint k_typeface_bold = 1;
    constexpr jint k_typeface_italic = 2;

    // android.util.TypedValue complex unit types (FontManager.GetFontSize's units).
    constexpr jint k_complex_unit_dip = 1;
    constexpr jint k_complex_unit_sp = 2;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject widget_of(const maui::core::button_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the handler must never leak JNI pending-exception state
    // into the cross-platform layer); true when one was pending — call sites skip the read-back.
    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe(); // logcat/stderr breadcrumb, same channel the test host uses
        env->ExceptionClear();
        return true;
    }

    void call_void_int(JNIEnv* env, jobject widget, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_button_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_button_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_button_class, name, "(Z)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon), then C#'s (int) truncation at the
    // call sites — the ceil already produced an integral value, so truncation is exact.
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The widget's display density (Context.getResources().getDisplayMetrics().density). Memoized
    // process-wide after the first successful read, exactly like ContextExtensions' s_displayDensity
    // cache (the JNI walk is four calls; C# caches for the same reason). 1.0 when any step fails
    // (failures are NOT memoized, so a transient failure does not pin the fallback).
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        static std::atomic<float> memoized{0.0F}; // 0 = not read yet (a real density is never 0)
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_button_class, "getContext", "()Landroid/content/Context;");
        jmethodID get_resources =
            cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
        jmethodID get_display_metrics =
            cache.method(env, "android/content/res/Resources", "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
        jfieldID density_field = cache.field(env, "android/util/DisplayMetrics", "density", "F");
        if (get_context == nullptr || get_resources == nullptr || get_display_metrics == nullptr ||
            density_field == nullptr)
        {
            return 1.0F;
        }
        const local_ref<jobject> context{env, env->CallObjectMethod(widget, get_context)};
        if (clear_pending(env) || !context)
        {
            return 1.0F;
        }
        const local_ref<jobject> resources{env, env->CallObjectMethod(context.get(), get_resources)};
        if (clear_pending(env) || !resources)
        {
            return 1.0F;
        }
        const local_ref<jobject> metrics{env, env->CallObjectMethod(resources.get(), get_display_metrics)};
        if (clear_pending(env) || !metrics)
        {
            return 1.0F;
        }
        const jfloat density = env->GetFloatField(metrics.get(), density_field);
        if (clear_pending(env) || density == 0.0F)
        {
            return 1.0F;
        }
        memoized.store(density, std::memory_order_relaxed);
        return density;
    }

    // The view's CURRENT size in POINTS (getWidth/getHeight pixels / density). {0,0} before the first
    // layout (apply_outline_clip skips a 0-sized view; platform_arrange re-resolves once laid out). The
    // generic-IView clip op resolves the geometry against rect{0,0,w,h} in points (the WrapperView.SetClip
    // convention), so the clip lands in the same point space the apple apply_clip twin uses.
    struct point_size
    {
        double width;
        double height;
    };
    [[nodiscard]] point_size view_point_size(JNIEnv* env, jobject widget, float density)
    {
        auto& cache = default_jni_cache();
        jmethodID get_width = cache.method(env, k_button_class, "getWidth", "()I");
        jmethodID get_height = cache.method(env, k_button_class, "getHeight", "()I");
        if (get_width == nullptr || get_height == nullptr || density == 0.0F)
        {
            return {.width = 0.0, .height = 0.0};
        }
        const jint w = env->CallIntMethod(widget, get_width);
        const jint h = env->CallIntMethod(widget, get_height);
        if (clear_pending(env))
        {
            return {.width = 0.0, .height = 0.0};
        }
        return {.width = static_cast<double>(w) / density, .height = static_cast<double>(h) / density};
    }

    // The maui-managed GradientDrawable carrying background color + stroke + corner radius (the
    // plain-widget stand-in for the MauiRippleDrawable layer stack — see the header deviations).
    // Returns the installed one (View.getBackground() instanceof GradientDrawable identifies ours:
    // the default theme background is a RippleDrawable), installing a fresh one only when `install`
    // is set. An empty ref means "not installed and not asked to install" (or a JNI failure).
    [[nodiscard]] local_ref<jobject> maui_background_drawable(JNIEnv* env, jobject widget, bool install)
    {
        auto& cache = default_jni_cache();
        jclass gradient_class = cache.find_class(env, k_gradient_drawable_class);
        jmethodID get_background =
            cache.method(env, k_button_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        if (gradient_class == nullptr || get_background == nullptr)
        {
            return {};
        }
        local_ref<jobject> current{env, env->CallObjectMethod(widget, get_background)};
        if (clear_pending(env))
        {
            return {};
        }
        if (current && env->IsInstanceOf(current.get(), gradient_class) == JNI_TRUE)
        {
            return current;
        }
        if (!install)
        {
            return {};
        }
        jmethodID ctor = cache.method(env, k_gradient_drawable_class, "<init>", "()V");
        jmethodID set_background =
            cache.method(env, k_button_class, "setBackground", "(Landroid/graphics/drawable/Drawable;)V");
        if (ctor == nullptr || set_background == nullptr)
        {
            return {};
        }
        local_ref<jobject> fresh{env, env->NewObject(gradient_class, ctor)};
        if (clear_pending(env) || !fresh)
        {
            return {};
        }
        env->CallVoidMethod(widget, set_background, fresh.get());
        if (clear_pending(env))
        {
            return {};
        }
        return fresh;
    }

    // The native half of dev.mauicpp.NativeOnClickListener.nativeOnClick(long): the peer is the
    // handler's button_platform, and the wired on_click callback carries ButtonHandler.OnClick's
    // body (VirtualView.Clicked()). Bound via RegisterNatives (no Java_* export needed).
    void JNICALL native_on_click(JNIEnv* /*env*/, jclass /*listener_class*/, jlong peer)
    {
        // The canonical JNI peer pattern: the Java side stores the pointer the C++ side handed it.
        auto* platform = reinterpret_cast<maui::core::button_platform*>(peer);
        if (platform != nullptr && platform->on_click)
        {
            platform->on_click();
        }
    }

    // Binds nativeOnClick to the listener class. Idempotent (RegisterNatives replaces an existing
    // binding), so connecting handlers need no once-flag coordination.
    [[nodiscard]] bool register_click_natives(JNIEnv* env, jclass listener_class)
    {
        // JNINativeMethod's name/signature members are non-const char* and fnPtr is a void* for
        // historical JNI-spec reasons — the const_casts/reinterpret_cast are the API's own shape.
        static const std::array<JNINativeMethod, 1> k_methods{
            JNINativeMethod{.name = const_cast<char*>("nativeOnClick"),
                            .signature = const_cast<char*>("(J)V"),
                            .fnPtr = reinterpret_cast<void*>(&native_on_click)},
        };
        const jint status = env->RegisterNatives(listener_class, k_methods.data(), static_cast<jint>(k_methods.size()));
        if (status != JNI_OK)
        {
            clear_pending(env);
            return false;
        }
        return true;
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.Button (the JNI shape of the
    // pimpl-owned-native-view doctrine: the ios twin CFReleases its UIButton here).
    button_platform::~button_platform()
    {
        if (native != nullptr)
        {
            const scoped_env env; // any-thread teardown, exactly like global_ref::reset
            if (env)
            {
                env->DeleteGlobalRef(static_cast<jobject>(native));
            }
            native = nullptr;
        }
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the VM-less cross-platform
    // suite (see the header comment) — then pushes to the real widget when one exists.

    void button_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // ViewExtensions.UpdateVisibility → ToPlatformVisibility: Visible/Hidden/Collapsed map to
        // View.VISIBLE/INVISIBLE/GONE.
        jint state = k_view_visible;
        if (value == maui::core::visibility::hidden)
        {
            state = k_view_invisible;
        }
        else if (value == maui::core::visibility::collapsed)
        {
            state = k_view_gone;
        }
        call_void_int(env.get(), widget_of(*this), "setVisibility", state);
    }

    void button_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // ViewExtensions.UpdateOpacity: platformView.Alpha = (float)opacity.
            call_void_float(env.get(), widget_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void button_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // ViewExtensions.UpdateIsEnabled: platformView.Enabled = view.IsEnabled.
            call_void_bool(env.get(), widget_of(*this), "setEnabled", static_cast<jboolean>(value));
        }
    }

    void button_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId's IsNullOrWhiteSpace gate (a blank id is never pushed).
        if (native == nullptr || value.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*this);
        auto& cache = default_jni_cache();
        // PlatformInterop.setContentDescriptionForAutomationId: setting a ContentDescription flips
        // ImportantForAccessibility to YES; restore AUTO when that is what the view had, so the
        // automation id does not change the view's accessibility exposure.
        jmethodID get_important = cache.method(env.get(), k_button_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_button_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (get_important == nullptr || set_description == nullptr)
        {
            return;
        }
        const jint important_before = env->CallIntMethod(widget, get_important);
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jstring> description = to_jstring(env.get(), value);
        env->CallVoidMethod(widget, set_description, description.get());
        if (clear_pending(env.get()))
        {
            return;
        }
        if (important_before == k_important_for_accessibility_auto)
        {
            call_void_int(env.get(), widget, "setImportantForAccessibility", k_important_for_accessibility_auto);
        }
    }

    void button_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*this);
        // ButtonExtensions.UpdateButtonBackground fills the maui drawable with the paint's color (the
        // gradient/ripple layers are part of the documented Material deviation). A null paint clears
        // OUR drawable's fill to transparent when one is installed; C#'s "recreate the default ripple"
        // restore has no plain-widget analog (deviation), and an untouched button never installs one.
        const local_ref<jobject> drawable = maui_background_drawable(env.get(), widget, /*install=*/value != nullptr);
        if (!drawable)
        {
            return;
        }
        jmethodID set_color = default_jni_cache().method(env.get(), k_gradient_drawable_class, "setColor", "(I)V");
        if (set_color == nullptr)
        {
            return;
        }
        const jint argb = value != nullptr ? static_cast<jint>(value->background_color().to_int()) : 0;
        env->CallVoidMethod(drawable.get(), set_color, argb);
        clear_pending(env.get());
    }

    // Render transform + flow direction + semantics pushed to the real widget via the shared android
    // ops (W4-34e). Each calls the view_platform_base body FIRST — the VM-less cross-platform suite
    // observes the headless mirror — then the shared op (itself VM-less safe) pushes to the View.
    void button_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void button_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void button_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    // VisualElement.Clip on a generic view (wave 24): install a ViewOutlineProvider + setClipToOutline so
    // the framework clips the whole Button to the convex shape (the clip_views EllipseGeometry). The base
    // mirror runs FIRST (the VM-less cross-platform suite observes it). The borrow is stashed so
    // platform_arrange can re-resolve the bounds-dependent geometry after layout (the view is 0×0 at map
    // time — apply_outline_clip clears the clip then, and the arrange pass rebuilds it at the live size).
    void button_platform::update_clip(const maui::graphics::i_shape* value)
    {
        view_platform_base::update_clip(value);
        clip_shape = value;
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*this);
        const float density = display_density(env.get(), widget);
        const point_size size = view_point_size(env.get(), widget, density);
        maui::platform::android::apply_outline_clip(native, value, density, size.width, size.height);
    }

    namespace
    {
        // ButtonExtensions.UpdateButtonStroke — C# routes MapStrokeColor / MapStrokeThickness /
        // MapCornerRadius through this ONE update (the three properties are intertwined in the
        // drawable), recomputing GetStrokeProperties each time. The plain-widget cut pushes
        // setStroke(width, color) + setCornerRadius onto the maui GradientDrawable, installing it
        // only once a stroke or radius is actually visible (see the header deviations).
        void update_button_stroke(button_handler& handler, i_button& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr || platform->native == nullptr)
            {
                return;
            }
            const scoped_env env;
            if (!env)
            {
                return;
            }
            jobject widget = widget_of(*platform);
            // GetStrokeProperties with the port's non-nullable color: thickness < 0 → width 0
            // (DefaultStrokeThicknessNoColor); radius < 0 cannot occur below the install gate.
            const double thickness = view.stroke_thickness() >= 0 ? view.stroke_thickness() : 0;
            const int radius = view.corner_radius() >= 0 ? view.corner_radius() : 0;
            const bool visible = thickness > 0 || radius > 0;
            const local_ref<jobject> drawable = maui_background_drawable(env.get(), widget, /*install=*/visible);
            if (!drawable)
            {
                return;
            }
            auto& cache = default_jni_cache();
            jmethodID set_stroke = cache.method(env.get(), k_gradient_drawable_class, "setStroke", "(II)V");
            jmethodID set_corner_radius = cache.method(env.get(), k_gradient_drawable_class, "setCornerRadius", "(F)V");
            if (set_stroke == nullptr || set_corner_radius == nullptr)
            {
                return;
            }
            const float density = display_density(env.get(), widget);
            env->CallVoidMethod(drawable.get(), set_stroke, to_pixels(thickness, density),
                                static_cast<jint>(view.stroke_color().to_int()));
            if (clear_pending(env.get()))
            {
                return;
            }
            env->CallVoidMethod(drawable.get(), set_corner_radius,
                                static_cast<jfloat>(to_pixels(static_cast<double>(radius), density)));
            clear_pending(env.get());
        }
    } // namespace

    std::unique_ptr<button_platform> button_handler::create_platform_view()
    {
        auto platform = std::make_unique<button_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass button_class = cache.find_class(env.get(), k_button_class);
        jmethodID ctor = cache.method(env.get(), k_button_class, "<init>", "(Landroid/content/Context;)V");
        if (button_class == nullptr || ctor == nullptr)
        {
            return platform;
        }
        // ButtonHandler.CreatePlatformView: new MauiMaterialButton(Context) { SoundEffectsEnabled =
        // false, … } — the Icon* initializers are Material-only (header deviations).
        const local_ref<jobject> widget{env.get(), env->NewObject(button_class, ctor, context)};
        if (clear_pending(env.get()) || !widget)
        {
            return platform;
        }
        call_void_bool(env.get(), widget.get(), "setSoundEffectsEnabled", JNI_FALSE);
        // Wrap-content LayoutParams up front: a parentless TextView with null LayoutParams NPEs in
        // checkForRelayout on any setText AFTER the first measure (TextView.java reads
        // getLayoutParams().width). In C# the parent ViewGroup attach supplies them; the port's
        // android container fan-out has not arrived, so the partial stands in for that attach.
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_button_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (layout_params_class != nullptr && layout_params_ctor != nullptr && set_layout_params != nullptr)
        {
            constexpr jint k_wrap_content = -2; // ViewGroup.LayoutParams.WRAP_CONTENT
            const local_ref<jobject> params{
                env.get(), env->NewObject(layout_params_class, layout_params_ctor, k_wrap_content, k_wrap_content)};
            if (!clear_pending(env.get()) && params)
            {
                env->CallVoidMethod(widget.get(), set_layout_params, params.get());
                clear_pending(env.get());
            }
        }
        platform->native = env->NewGlobalRef(widget.get()); // released in ~button_platform
        return platform;
    }

    void button_handler::on_connect_handler(button_platform& platform)
    {
        // ButtonHandler.Android's listener split: ButtonClickListener.OnClick → Clicked only (the
        // Pressed/Released pair belongs to the deferred ButtonTouchListener — header deviations).
        // The callbacks stay wired even VM-less so the cross-platform suite can drive them.
        platform.on_press = [this] {
            if (auto* view = virtual_view())
            {
                view->send_pressed();
            }
        };
        platform.on_release = [this] {
            if (auto* view = virtual_view())
            {
                view->send_released();
            }
        };
        platform.on_click = [this] {
            if (auto* view = virtual_view())
            {
                view->send_clicked();
            }
        };
        if (platform.native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jclass listener_class = cache.find_class(env.get(), k_click_listener_class);
        jmethodID listener_ctor = cache.method(env.get(), k_click_listener_class, "<init>", "(J)V");
        jmethodID set_on_click_listener =
            cache.method(env.get(), k_button_class, "setOnClickListener", "(Landroid/view/View$OnClickListener;)V");
        if (listener_class == nullptr || listener_ctor == nullptr || set_on_click_listener == nullptr ||
            !register_click_natives(env.get(), listener_class))
        {
            return; // the listener class is host-provided (see NativeOnClickListener.java); without
                    // it the click channel stays C++-only, exactly like the VM-less degradation
        }
        // ConnectHandler: ClickListener.Handler = this; platformView.SetOnClickListener(ClickListener).
        // The peer is the platform struct; the View holds the listener strongly, and disconnect
        // uninstalls it before the struct can die.
        const local_ref<jobject> listener{
            env.get(), env->NewObject(listener_class, listener_ctor, reinterpret_cast<jlong>(&platform))};
        if (clear_pending(env.get()) || !listener)
        {
            return;
        }
        env->CallVoidMethod(widget_of(platform), set_on_click_listener, listener.get());
        clear_pending(env.get());
    }

    void button_handler::on_disconnect_handler(button_platform& platform)
    {
        // DisconnectHandler: ClickListener.Handler = null; platformView.SetOnClickListener(null).
        platform.on_press = nullptr;
        platform.on_release = nullptr;
        platform.on_click = nullptr;
        if (platform.native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jmethodID set_on_click_listener = default_jni_cache().method(env.get(), k_button_class, "setOnClickListener",
                                                                     "(Landroid/view/View$OnClickListener;)V");
        if (set_on_click_listener != nullptr)
        {
            env->CallVoidMethod(widget_of(platform), set_on_click_listener, static_cast<jobject>(nullptr));
            clear_pending(env.get());
        }
    }

    void button_handler::map_text(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->title = std::string(view.text());
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // TextViewExtensions.UpdateTextPlainText: textView.Text = label.Text. to_jstring goes through
        // the real-UTF-8 path (supplementary-plane safe — see jni_string.hpp).
        jmethodID set_text =
            default_jni_cache().method(env.get(), k_button_class, "setText", "(Ljava/lang/CharSequence;)V");
        if (set_text != nullptr)
        {
            const local_ref<jstring> text = to_jstring(env.get(), view.text());
            env->CallVoidMethod(widget_of(*platform), set_text, text.get());
            clear_pending(env.get());
        }
    }

    void button_handler::map_text_color(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // TextViewExtensions.UpdateTextColor: SetTextColor(textColor.ToPlatform()) — the ARGB int.
            // MapTextColor's null branch (restore the Material defaults) collapses (header deviations).
            call_void_int(env.get(), widget_of(*platform), "setTextColor",
                          static_cast<jint>(view.text_color().to_int()));
        }
    }

    void button_handler::map_font(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        const font value = view.font();

        // FontManager.GetTypeface → CreateTypeface (the non-registered-family tail; header
        // deviations): base = family ? Typeface.create(family, ToTypefaceStyle(weight, italic))
        // : Typeface.DEFAULT, then the API-28+ refinement Typeface.create(base, weight, italic).
        const bool italic = value.slant() != font_slant::normal;
        const bool bold = value.weight() >= font_weight::bold; // ToTypefaceStyle: bold = weight >= Bold
        jint style = k_typeface_normal;
        if (bold && italic)
        {
            style = k_typeface_bold | k_typeface_italic;
        }
        else if (bold)
        {
            style = k_typeface_bold;
        }
        else if (italic)
        {
            style = k_typeface_italic;
        }
        jmethodID create_named = cache.static_method(env.get(), k_typeface_class, "create",
                                                     "(Ljava/lang/String;I)Landroid/graphics/Typeface;");
        jmethodID create_weighted = cache.static_method(env.get(), k_typeface_class, "create",
                                                        "(Landroid/graphics/Typeface;IZ)Landroid/graphics/Typeface;");
        jmethodID set_typeface =
            cache.method(env.get(), k_button_class, "setTypeface", "(Landroid/graphics/Typeface;)V");
        jclass typeface_class = cache.find_class(env.get(), k_typeface_class);
        if (create_named == nullptr || create_weighted == nullptr || set_typeface == nullptr ||
            typeface_class == nullptr)
        {
            return;
        }
        local_ref<jstring> family; // empty family() ⇒ C# null Family ⇒ Typeface.create(null, …) = default
        if (!value.family().empty())
        {
            family = to_jstring(env.get(), value.family());
        }
        const local_ref<jobject> base{env.get(),
                                      env->CallStaticObjectMethod(typeface_class, create_named, family.get(), style)};
        if (clear_pending(env.get()) || !base)
        {
            return;
        }
        const local_ref<jobject> typeface{
            env.get(), env->CallStaticObjectMethod(typeface_class, create_weighted, base.get(),
                                                   static_cast<jint>(value.weight()), static_cast<jboolean>(italic))};
        if (clear_pending(env.get()) || !typeface)
        {
            return;
        }
        env->CallVoidMethod(widget, set_typeface, typeface.get());
        if (clear_pending(env.get()))
        {
            return;
        }

        // FontManager.GetFontSize: size ≤ 0 / NaN → DefaultFontSize; AutoScalingEnabled picks Sp
        // (user-scaled) vs Dip. TextViewExtensions.UpdateFont: SetTextSize(unit, value).
        auto size = static_cast<float>(value.size());
        if (!(size > 0) || std::isnan(size))
        {
            size = k_default_font_size;
        }
        const jint unit = value.auto_scaling_enabled() ? k_complex_unit_sp : k_complex_unit_dip;
        jmethodID set_text_size = cache.method(env.get(), k_button_class, "setTextSize", "(IF)V");
        if (set_text_size != nullptr)
        {
            env->CallVoidMethod(widget, set_text_size, unit, static_cast<jfloat>(size));
            clear_pending(env.get());
        }
    }

    void button_handler::map_character_spacing(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // TextViewExtensions.UpdateCharacterSpacing: LetterSpacing = CharacterSpacing.ToEm().
            call_void_float(env.get(), widget_of(*platform), "setLetterSpacing",
                            static_cast<jfloat>(view.character_spacing()) * k_em_coefficient);
        }
    }

    void button_handler::map_padding(button_handler& handler, i_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->padding = view.padding();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*platform);
        // ButtonExtensions.UpdatePadding(button, DefaultPadding): NaN falls back to the default,
        // dp → px via ToPixels, then SetPadding's four ints.
        maui::core::thickness padding = view.padding();
        if (padding.is_nan())
        {
            padding = maui::core::thickness(k_default_padding_horizontal, k_default_padding_vertical);
        }
        const float density = display_density(env.get(), widget);
        jmethodID set_padding = default_jni_cache().method(env.get(), k_button_class, "setPadding", "(IIII)V");
        if (set_padding != nullptr)
        {
            env->CallVoidMethod(widget, set_padding, to_pixels(padding.left, density), to_pixels(padding.top, density),
                                to_pixels(padding.right, density), to_pixels(padding.bottom, density));
            clear_pending(env.get());
        }
    }

    void button_handler::map_stroke_color(button_handler& handler, i_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_color = view.stroke_color();
            update_button_stroke(handler, view); // ButtonExtensions.UpdateStrokeColor → UpdateButtonStroke
        }
    }

    void button_handler::map_stroke_thickness(button_handler& handler, i_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->stroke_thickness = view.stroke_thickness();
            update_button_stroke(handler, view); // ButtonExtensions.UpdateStrokeThickness → UpdateButtonStroke
        }
    }

    void button_handler::map_corner_radius(button_handler& handler, i_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->corner_radius = view.corner_radius();
            update_button_stroke(handler, view); // ButtonExtensions.UpdateCornerRadius → UpdateButtonStroke
        }
    }

    maui::graphics::size button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric, so the backend-agnostic
            // size-request suites see consistent numbers in the pure-native run.
            return {static_cast<double>(platform->title.size()) * 8.0, 20.0};
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandlerExtensions.GetDesiredSizeFromHandler (Android): finite constraints become
        // AtMost specs in pixels, infinite become Unspecified; View.measure, then the measured
        // pixels come back as dp (Context.FromPixels).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_button_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_button_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_button_class, "getMeasuredHeight", "()I");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || get_measured_width == nullptr ||
            get_measured_height == nullptr || measure_spec_class == nullptr)
        {
            return {0, 0};
        }
        const float density = display_density(env.get(), widget);
        const auto spec_for = [&](double constraint) -> jint {
            const jint size = std::isfinite(constraint) ? to_pixels(constraint, density) : 0;
            const jint mode = std::isfinite(constraint) ? k_measure_spec_at_most : k_measure_spec_unspecified;
            const jint spec = env->CallStaticIntMethod(measure_spec_class, make_measure_spec, size, mode);
            return clear_pending(env.get()) ? 0 : spec;
        };
        const jint width_spec = spec_for(width_constraint);
        const jint height_spec = spec_for(height_constraint);
        env->CallVoidMethod(widget, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return {0, 0};
        }
        const jint measured_width = env->CallIntMethod(widget, get_measured_width);
        const jint measured_height = env->CallIntMethod(widget, get_measured_height);
        if (clear_pending(env.get()))
        {
            return {0, 0};
        }
        return {static_cast<double>(measured_width) / density, static_cast<double>(measured_height) / density};
    }

    // Android keeps the cross-platform ResolveConstraints clamp (an explicit size request overrides the
    // measured size) — the intrinsic-content floor is an iOS/macOS native-button behavior. False here.
    bool button_handler::content_is_minimum_size() const
    {
        return false;
    }

    void button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // headless: no native layout to apply
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewExtensions/ViewHandler.PlatformArrange: the dp frame becomes pixels, the view measures
        // Exactly at the final size (Android requires a measure pass before layout) and lays out.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_button_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_button_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), widget);
        const jint left = to_pixels(frame.x, density);
        const jint top = to_pixels(frame.y, density);
        const jint width = to_pixels(frame.width, density);
        const jint height = to_pixels(frame.height, density);
        const jint width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, width, k_measure_spec_exactly);
        const jint height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, height, k_measure_spec_exactly);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(widget, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(widget, layout, left, top, left + width, top + height);
        clear_pending(env.get());

        // Re-resolve the clip against the just-laid-out bounds (the iOS reapply_clip analog): the outline
        // geometry is resolved against the live frame size in points, and update_clip may have run before
        // the first layout when the view was 0×0 (apply_outline_clip cleared it then). frame is in points.
        if (platform->clip_shape != nullptr)
        {
            maui::platform::android::apply_outline_clip(platform->native, platform->clip_shape, density, frame.width,
                                                        frame.height);
        }
    }

    // ---- per-backend image-source primitives (the cross-platform map_image_source routes here) ----
    // The android Button is a TextView-derived widget whose icon support (CompoundDrawables / MaterialButton
    // .Icon) is part of the deferred android backend (see port/STATUS.md — the trio is not yet built). The
    // primitives update the shared headless-style mirrors (kind/file/loaded) so the android preset's pure-
    // native cross-platform suite still observes the load; the real JNI drawable push is deferred.
    void button_handler::configure_loader(maui::core::image_source_loader& /*loader*/)
    {
    }

    void button_handler::load_file_source_sync(button_platform& platform, const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
    }

    void button_handler::apply_loaded_result(button_platform& platform, const image_source_result& result)
    {
        if (!result.loaded())
        {
            clear_source_native(platform);
            return;
        }
        platform.source_kind = result.kind();
        platform.source_file = result.detail();
        platform.source_loaded = true;
    }

    void button_handler::clear_source_native(button_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
    }
} // namespace maui::core
