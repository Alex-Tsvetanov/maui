// progress_bar_handler — Android (JNI) platform partial, the M-android per-control fan-out replayed
// for the determinate progress bar. The managed platform view is a REAL android.widget.ProgressBar
// (horizontal style, determinate, held as a JNI global reference in progress_bar_platform::native):
// Progress maps onto setProgress over a fixed Max=10000 range, ProgressColor onto setProgressTintList
// (ColorStateList.valueOf(argb)). Display-only — a progress bar has no inbound event channel, so there
// is no connect/disconnect listener wiring (unlike button_handler.cpp).
//
// Ported DIRECTLY from ProgressBarHandler.Android.cs + Platform/Android/{ProgressBarExtensions.cs,
// ViewExtensions.cs (UpdateVisibility/UpdateOpacity/UpdateAutomationId), ContextExtensions.cs (ToPixels)}.
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each a Material-library or infrastructure gap, not
// a behavior guess):
//   - The widget is a plain android.widget.ProgressBar, constructed via the (Context, AttributeSet,
//     defStyleAttr) ctor with android.R.attr.progressBarStyleHorizontal — exactly C#'s
//     `new AndroidProgressBar(Context, null, Attribute.ProgressBarStyleHorizontal)`. C# additionally
//     uses Google.Android.Material's LinearProgressIndicator in some skins (UpdateProgressColor's
//     `is LinearProgressIndicator` branch); the Material Components library is a gradle/AAR dependency
//     this APK-less backend does not carry, so the plain-ProgressBar path (UpdateProgressBarColor) is
//     always taken — the same branch C# takes for a non-Material ProgressBar.
//   - The port's colors are non-nullable value types (default color{} = opaque BLACK), so C#'s
//     ProgressBarExtensions.UpdateProgressBarColor `color == null` branch (ClearColorFilter to restore the
//     theme default) has no value-type analog. A `!= color{}` compare cannot stand in for `!= null` (it
//     misreads an explicit ProgressColor=Black as unset AND tinted every unset bar black), so map_progress_color
//     discriminates on BindableObject.IsSet (is_property_set("progress_color")) and, on the UNSET branch,
//     positively asserts the measured native default (#E0E0E0 light gray) rather than pushing black — the
//     same reproduce-the-ClearColorFilter-result convention activity_indicator_handler::map_color uses. An
//     explicit ProgressColor still overrides. Because Indeterminate is fixed false, only the ProgressTintList
//     branch is reachable (never IndeterminateTintList). The TRACK (progressBackground) tint is seeded once
//     at creation to the neutral native-default gray (#D7D7D7) so the force-styled Material defStyleRes's
//     lavender secondary baseline matches MAUI's neutral track for every row (see create_platform_view).
//   - Shadow / Clip / InputTransparent have NO plain-android.view.View analog (WrapperView-only in C#),
//     so those generic-IView properties keep ONLY the headless mirror — matching the unwrapped-View
//     behavior the shared android ops document (android_visual_ops.hpp). is_enabled likewise keeps the
//     base mirror: a ProgressBar is not interactive and C# has no MapIsEnabled for it (parallel to the
//     apple/iOS twins, where NSProgressIndicator/UIProgressView are not controls).
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the
// emulator where no Java VM exists. Every JNI path here checks scoped_env/app_context() and quietly
// skips, while the headless mirrors (progress/progress_color/resolved_flow_direction + the base IView
// mirrors) are ALWAYS maintained — so that suite observes exactly the headless partial's behavior, and
// the widget test host additionally observes the real widget.

#include "maui/core/progress_bar_handler.hpp"

#include <jni.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_progress.hpp"
#include "maui/core/semantics.hpp"
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
    // superclasses, so the View surface resolves through android/widget/ProgressBar too).
    constexpr const char* k_progress_bar_class = "android/widget/ProgressBar";
    constexpr const char* k_color_state_list_class = "android/content/res/ColorStateList";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";
    constexpr const char* k_style_class = "android/R$style";

    // The horizontal ProgressBar's defStyleRes (see create_platform_view). PARITY P2: real .NET MAUI renders
    // its determinate ProgressBar through the AppCompat/Material theme → a THIN flat track (a ~2dp hairline
    // with a flat colored fill, no shadow). The classic android.R.style.Widget_ProgressBar_Horizontal (used
    // before this fix) is the pre-Holo Gingerbread style → a THICK rounded bar with a heavy dark/gray
    // gradient and drop shadow, which does NOT match MAUI. This AAR-less backend has no AndroidX (no
    // AppCompat ProgressBar), but the framework carries the concrete Material horizontal style resource, and
    // a concrete defStyleRes needs no theme attribute (so it constructs on the bare, Activity-less testhost
    // Context too). Widget_Material_Light_ProgressBar_Horizontal is the light-theme thin variant the
    // gallery's default light Material theme renders under (matching MAUI's light capture);
    // Widget_Material_ProgressBar_Horizontal is the generic/dark twin, tried if the light field is absent —
    // the same primary→alt GetStaticFieldID pattern slider_handler.cpp/switch_handler.cpp use. Both are
    // static fields (GetStaticFieldID, not the instance field() helper). The ProgressColor tint push in
    // map_progress_color is unchanged and still tints on the new style.
    constexpr const char* k_horizontal_style_field = "Widget_Material_Light_ProgressBar_Horizontal";
    constexpr const char* k_horizontal_style_field_alt = "Widget_Material_ProgressBar_Horizontal";

    // ProgressBarExtensions.Maximum — MAUI scales the [0,1] fraction onto this fixed integer range.
    constexpr jint k_progress_maximum = 10000;

    // PARITY (the track twin of the ProgressColor unset-default fix): C#'s UpdateProgressColor touches
    // ONLY the progress (fill) tint — it NEVER sets the track (progressBackground) tint, so the track
    // keeps the native default. On MAUI's native-default determinate ProgressBar that track is a neutral
    // light gray — the maui-compare reference samples #D7D7D7 for every row's track (Default, the explicit
    // ProgressColor rows, and ProgressTo alike). This AAR-less backend force-styles a concrete Material
    // defStyleRes onto the bare Context (see create_platform_view), whose baseline progressBackground tint
    // is the theme SECONDARY (a lavender #D0D5E3, sampled in the pre-fix capture), NOT MAUI's neutral gray
    // — the "track color also differs" half of the parity review. Seeding the measured neutral gray as the
    // track baseline at creation (below, once) reproduces MAUI's native-default track for ALL rows without
    // touching any per-row mapper, exactly the seed-the-measured-default technique switch_handler.cpp uses
    // (seed_default_material_tints). The ProgressColor mapper only ever sets the FILL tint, so this track
    // baseline is never disturbed by an explicit or unset ProgressColor.
    //
    // NOTE on the seed value: setProgressBackgroundTintList tints the secondaryProgress drawable, which the
    // Material defStyleRes gives a ~0.25 alpha, so the seed is composited over the (white) field before it
    // renders — a solid #D7D7D7 seed lands ~#F5F5F5 (too light). Pre-compensating for that alpha, seeding
    // the darker #5F5F5F composites to MAUI's measured neutral #D7D7D7 (0.25*0x5F + 0.75*0xFF ≈ 0xD7). A
    // neutral gray either way (the lavender is gone with either); this pins it to MAUI's exact tone.
    constexpr jint k_native_default_track_color = static_cast<jint>(0xFF5F5F5FU);

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_AUTO (PlatformInterop restores it after
    // setContentDescription auto-flips the view to YES).
    constexpr jint k_important_for_accessibility_auto = 0;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject widget_of(const maui::core::progress_bar_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the handler must never leak JNI pending-exception state into
    // the cross-platform layer); true when one was pending — call sites skip the read-back.
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
        if (jmethodID method = default_jni_cache().method(env, k_progress_bar_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_progress_bar_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_progress_bar_class, name, "(Z)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon), then C#'s (int) truncation at the call
    // sites — the ceil already produced an integral value, so truncation is exact.
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The widget's display density (Context.getResources().getDisplayMetrics().density). 1.0 when any
    // step fails. (The shared android view ops memoize this process-wide via ContextExtensions'
    // s_displayDensity cache; the measure/arrange seam reads it directly here, the same walk.)
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_progress_bar_class, "getContext", "()Landroid/content/Context;");
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
        return density;
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.ProgressBar (the JNI shape of the
    // pimpl-owned-native-view doctrine: the apple twin CFReleases its NSProgressIndicator here).
    progress_bar_platform::~progress_bar_platform()
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

    void progress_bar_platform::update_visibility(maui::core::visibility value)
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

    void progress_bar_platform::update_opacity(double value)
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

    void progress_bar_platform::update_automation_id(std::string_view value)
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
        jmethodID get_important = cache.method(env.get(), k_progress_bar_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_progress_bar_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
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

    // Render transform + flow direction + background + semantics pushed to the real widget via the
    // shared android ops. Each calls the view_platform_base body FIRST — the VM-less cross-platform
    // suite observes the headless mirror — then the shared op (itself VM-less safe) pushes to the View.
    void progress_bar_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void progress_bar_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        // The struct push applies the raw FlowDirection; the handler's map_flow_direction additionally
        // runs the resolved recipe (MatchParent → parent-IView fallback) and re-applies it — the same
        // split as the apple twin (struct update_flow_direction vs handler map_flow_direction).
        maui::platform::android::apply_flow_direction(native, value);
    }

    void progress_bar_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        // VisualElement.Background paints the ProgressBar's View background via the shared android op
        // (ViewExtensions.UpdateBackground — behind the track). VM-less safe.
        maui::platform::android::apply_background(native, value);
    }

    void progress_bar_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<progress_bar_platform> progress_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<progress_bar_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass progress_bar_class = cache.find_class(env.get(), k_progress_bar_class);
        if (progress_bar_class == nullptr)
        {
            return platform;
        }
        // ProgressBarHandler.CreatePlatformView wants the HORIZONTAL determinate style. MAUI uses the
        // 3-arg theme-attr ctor `new ProgressBar(Context, null, android.R.attr.progressBarStyleHorizontal)`,
        // but that resolves the style attribute against the Context's THEME — which the app_process widget
        // test host (a bare, Activity-less Context) does not carry, so that ctor throws (the agent flagged
        // this). Use instead the theme-INDEPENDENT 4-arg ctor (Context, AttributeSet, int defStyleAttr,
        // int defStyleRes) with a concrete horizontal style resource as defStyleRes (applies the horizontal
        // look without a theme). PARITY P2: that defStyleRes is the THIN Material style
        // Widget_Material_Light_ProgressBar_Horizontal (see the k_horizontal_style_field note) — NOT the
        // classic Widget_ProgressBar_Horizontal, whose thick pre-Holo bar + heavy gradient/shadow did not
        // match real MAUI's thin flat AppCompat/Material track. Try the light field, then the generic/dark
        // alt, then fall back to the plain (Context) ctor so the widget is never null (a valid determinate
        // bar; only the horizontal *look* is lost).
        jobject created = nullptr;
        jmethodID ctor_styled = cache.method(env.get(), k_progress_bar_class, "<init>",
                                             "(Landroid/content/Context;Landroid/util/AttributeSet;II)V");
        jclass style_class = cache.find_class(env.get(), k_style_class);
        // The Material horizontal style is a STATIC field — the jni_cache's field() is GetFieldID
        // (instance) and returns null for it, so resolve it directly with GetStaticFieldID.
        jfieldID horizontal_style_field =
            style_class != nullptr ? env->GetStaticFieldID(style_class, k_horizontal_style_field, "I") : nullptr;
        clear_pending(env.get()); // a missing-field lookup raises NoSuchFieldError — clear it, then try the alt
        if (style_class != nullptr && horizontal_style_field == nullptr)
        {
            horizontal_style_field = env->GetStaticFieldID(style_class, k_horizontal_style_field_alt, "I");
            clear_pending(env.get());
        }
        if (ctor_styled != nullptr && style_class != nullptr && horizontal_style_field != nullptr)
        {
            const jint style_res = env->GetStaticIntField(style_class, horizontal_style_field);
            if (!clear_pending(env.get()))
            {
                created = env->NewObject(progress_bar_class, ctor_styled, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0), style_res);
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_plain =
                cache.method(env.get(), k_progress_bar_class, "<init>", "(Landroid/content/Context;)V");
            if (ctor_plain != nullptr)
            {
                created = env->NewObject(progress_bar_class, ctor_plain, context);
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            return platform;
        }
        const local_ref<jobject> widget{env.get(), created};
        // The object-initializer of CreatePlatformView: { Indeterminate = false, Max = Maximum }.
        call_void_bool(env.get(), widget.get(), "setIndeterminate", JNI_FALSE);
        call_void_int(env.get(), widget.get(), "setMax", k_progress_maximum);
        // PARITY: pin the neutral Material-default track (progressBackground) gray so the force-styled
        // defStyleRes's lavender secondary-color baseline is replaced by MAUI's native-default #D7D7D7 for
        // every row (see the k_native_default_track_color note). setProgressBackgroundTintList takes a
        // ColorStateList; build it via ColorStateList.valueOf, exactly like map_progress_color's fill tint.
        // Best-effort — a null id / failed alloc leaves the styled baseline (never crashes).
        {
            jmethodID track_value_of = cache.static_method(env.get(), k_color_state_list_class, "valueOf",
                                                           "(I)Landroid/content/res/ColorStateList;");
            jmethodID set_background_tint =
                cache.method(env.get(), k_progress_bar_class, "setProgressBackgroundTintList",
                             "(Landroid/content/res/ColorStateList;)V");
            jclass color_state_list_class = cache.find_class(env.get(), k_color_state_list_class);
            if (track_value_of != nullptr && set_background_tint != nullptr && color_state_list_class != nullptr)
            {
                const local_ref<jobject> track_tint{
                    env.get(),
                    env->CallStaticObjectMethod(color_state_list_class, track_value_of, k_native_default_track_color)};
                if (!clear_pending(env.get()) && track_tint)
                {
                    env->CallVoidMethod(widget.get(), set_background_tint, track_tint.get());
                    clear_pending(env.get());
                }
            }
        }
        // Wrap-content LayoutParams up front (parentless View measure/layout safety — the android
        // container fan-out has not arrived; the partial stands in for the parent ViewGroup attach,
        // exactly like button_handler.cpp does).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params = cache.method(env.get(), k_progress_bar_class, "setLayoutParams",
                                                   "(Landroid/view/ViewGroup$LayoutParams;)V");
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
        platform->native = env->NewGlobalRef(widget.get()); // released in ~progress_bar_platform
        return platform;
    }

    void progress_bar_handler::map_progress(progress_bar_handler& handler, i_progress& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->progress = view.progress(); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // ProgressBarExtensions.UpdateProgress: Progress = (int)(progress.Progress * Maximum).
            const auto scaled = static_cast<jint>(view.progress() * static_cast<double>(k_progress_maximum));
            call_void_int(env.get(), widget_of(*platform), "setProgress", scaled);
        }
    }

    void progress_bar_handler::map_progress_color(progress_bar_handler& handler, i_progress& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->progress_color = view.progress_color(); // headless mirror first (VM-less suite)
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
        // ProgressBarExtensions.UpdateProgressColor → UpdateProgressBarColor (the plain-ProgressBar
        // branch; the Material LinearProgressIndicator branch is a documented deviation). C# discriminates
        // on `color == null`: SET → ColorStateList.ValueOf(color) on ProgressTintList; UNSET → the
        // (Indeterminate?…:ProgressDrawable)?.ClearColorFilter() that restores the platform default. The
        // port's Color is a NON-nullable value type whose default-constructed value (color{}) is opaque
        // BLACK, so a `!= color{}` value compare cannot stand in for C#'s `!= null` — it BOTH misreads an
        // explicit ProgressColor=Black as unset AND (the bug this fix closes) tinted every UNSET progress
        // bar solid BLACK. Discriminate instead on whether ProgressColor was explicitly SET
        // (BindableObject.IsSet) — the faithful stand-in for `!= null`, exactly the sentinel
        // activity_indicator_handler::map_color / button_handler::map_text_color use.
        //
        // Native-default deviation (this AAR-less backend only, the activity_indicator twin): C#'s unset
        // path is ClearColorFilter, which on MAUI's native-default determinate ProgressBar reveals the
        // platform's pale fill — the maui-compare reference samples #E0E0E0 (light gray) for both the
        // Default and Disabled bars. This backend force-styles a concrete Material defStyleRes onto the
        // bare Context (see create_platform_view), whose baseline progress tint is the theme accent, so a
        // plain ClearColorFilter would NOT reveal MAUI's gray. Positively assert the measured native
        // default (#E0E0E0) on the unset branch instead — reproducing MAUI's ClearColorFilter *result* via
        // the same ColorStateList.ValueOf path an explicit color takes. An explicitly-set ProgressColor
        // still overrides through the SET branch.
        constexpr jint k_native_default_progress_color = static_cast<jint>(0xFFE0E0E0U);
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("progress_color");
        jmethodID value_of = cache.static_method(env.get(), k_color_state_list_class, "valueOf",
                                                 "(I)Landroid/content/res/ColorStateList;");
        jmethodID set_progress_tint = cache.method(env.get(), k_progress_bar_class, "setProgressTintList",
                                                   "(Landroid/content/res/ColorStateList;)V");
        jclass color_state_list_class = cache.find_class(env.get(), k_color_state_list_class);
        if (value_of == nullptr || set_progress_tint == nullptr || color_state_list_class == nullptr)
        {
            return;
        }
        const jint argb =
            color_is_set ? static_cast<jint>(view.progress_color().to_int()) : k_native_default_progress_color;
        const local_ref<jobject> tint_list{env.get(),
                                           env->CallStaticObjectMethod(color_state_list_class, value_of, argb)};
        if (clear_pending(env.get()) || !tint_list)
        {
            return;
        }
        env->CallVoidMethod(widget, set_progress_tint, tint_list.get());
        clear_pending(env.get());
    }

    void progress_bar_handler::map_flow_direction(progress_bar_handler& handler, i_progress& view)
    {
        // ProgressBarHandler.MapFlowDirection: apply the RESOLVED direction (the MatchParent →
        // parent-IView fallback) via the View's LayoutDirection and mirror it. The android analog of the
        // apple twin's userInterfaceLayoutDirection / the iOS UISemanticContentAttribute recipe.
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const maui::core::flow_direction resolved = resolved_flow_direction(view);
        platform->resolved_flow_direction = resolved; // headless mirror first (VM-less suite)
        if (platform->native != nullptr)
        {
            maui::platform::android::apply_flow_direction(platform->native, resolved);
        }
    }

    maui::graphics::size progress_bar_handler::get_desired_size(double width_constraint, double height_constraint) const
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
            return {100.0, 4.0};
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandlerExtensions.GetDesiredSizeFromHandler (Android): finite constraints become AtMost
        // specs in pixels, infinite become Unspecified; View.measure, then the measured pixels come back
        // as dp (Context.FromPixels).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_progress_bar_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_progress_bar_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_progress_bar_class, "getMeasuredHeight", "()I");
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

    void progress_bar_handler::platform_arrange(const maui::graphics::rect& frame)
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
        jmethodID measure = cache.method(env.get(), k_progress_bar_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_progress_bar_class, "layout", "(IIII)V");
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
    }
} // namespace maui::core
