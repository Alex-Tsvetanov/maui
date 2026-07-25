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
#include "android_image_decode.hpp"
#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/bindable_object.hpp"
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
    constexpr const char* k_style_class = "android/R$style";

    // ButtonHandler.CreatePlatformView constructs a MauiMaterialButton (Google.Android.Material's
    // MaterialButton, wrapped in MauiMaterialContextThemeWrapper). Real .NET MAUI thus renders a FILLED
    // Material button: a light-gray fill, rounded corners, compact Material insets, and — the dominant
    // Android parity diff this fixes — WHITE label text (the Material theme's onSurface/onPrimary
    // default that ButtonHandler captures as _defaultTextColors). This AAR-less backend cannot link the
    // Material Components library (it packages against android.jar only — no gradle, no maven egress), so
    // the port constructs a plain android.widget.Button through the theme-INDEPENDENT 4-arg ctor (Context,
    // AttributeSet=null, defStyleAttr=0, defStyleRes) with the framework's concrete Material button style
    // as defStyleRes — the same pattern progress_bar_handler.cpp / slider_handler.cpp use for their thin
    // Material look. A concrete defStyleRes applies the filled Material button appearance (fill + rounded
    // background + insets) WITHOUT needing a theme attribute, so it constructs on the bare, Activity-less
    // widget test host too. Widget_Material_Light_Button is the light-theme filled variant matching MAUI's
    // light capture; Widget_Material_Button is the generic/dark twin, tried if the light field is absent
    // (the primary→alt GetStaticFieldID fallback the progress-bar/slider handlers share). Both are STATIC
    // fields (GetStaticFieldID, not the instance field() helper). The WHITE text itself is asserted in
    // map_text_color's unset branch (mirroring C#'s _defaultTextColors restore — see there).
    //
    // CHROME CAVEAT: that concrete framework style also ships chrome the native-default MAUI button does NOT
    // have — (a) a default stateListAnimator that raises translationZ on rest (a DROP SHADOW), (b) an
    // InsetDrawable background (inset fill → small GAPS between adjacent docked buttons), and (c) rounded
    // corners. Real .NET MAUI here renders FLAT / edge-to-edge / near-square (its MauiMaterialButton inherits
    // a non-MaterialComponents base theme; verified against docs/comparison/android/maui/{button,
    // custom_layout}.png — pure white below every button, #E0E0E0 contiguous fill). create_platform_view
    // fixes (a) by calling strip_elevation on the constructed widget → shadowless, which clears the dominant
    // custom_layout parity RED (the drop shadow + the perceived card separation the shadow created).
    // DEFERRED — (b)+(c) the inset + rounded corners: they live in the framework background's
    // InsetDrawable/GradientDrawable, which ALSO carries the button's intrinsic min-height + content padding.
    // Swapping it for a bare flat GradientDrawable was tried and collapsed the default (no-size-request)
    // buttons to zero size (they lost their intrinsic sizing) → a blank page. Removing the inset/corners
    // safely needs the android container/measure fan-out to supply the button sizing independently of the
    // background; until then the port keeps the framework fill (correct #E0E0E0 color, minor extra corner
    // radius + ~4dp inset). See strip_elevation's tail comment.
    constexpr const char* k_button_style_field = "Widget_Material_Light_Button";
    constexpr const char* k_button_style_field_alt = "Widget_Material_Button";

    // ButtonHandler.DefaultPadding (Android) — "the Material Components minimum size" derivation:
    // horizontal 16dp, vertical 8.5dp; substituted when the cross-platform Padding is NaN.
    constexpr double k_default_padding_horizontal = 16;
    constexpr double k_default_padding_vertical = 8.5;

    // MauiMaterialButton's flat filled look (pixel-verified vs docs/comparison/android/maui/{button,
    // custom_layout}.png): a solid opaque #E0E0E0 fill with near-square ~4dp Material-default corners,
    // installed edge-to-edge in place of the framework InsetDrawable — whose inset gap, over-rounded
    // corners, and near-invisible disabled fill were the dominant Android button parity diff. A disabled
    // MauiMaterialButton dims its container to colorOnSurface@12% (translucent black) and its label to
    // colorOnSurface@38% — TRANSLUCENT overlays, so a colored parent panel bleeds through (layout_is_enabled:
    // a LightBlue panel behind a disabled button reads (152,190,202) = panel x 0.88, not opaque #E0E0E0).
    // Over a white parent both composite to the historical #E0E0E0 fill / #8B8B8B label, so white-bg pages are
    // unchanged. The ~36dp button height is content-driven (the 8.5dp vertical padding), NOT a min-height floor.
    constexpr auto k_material_default_button_color = static_cast<jint>(0xFFE0E0E0U);
    // Disabled-state overlays (colorOnSurface = black in the light Material theme): container @~12%, label @~38%.
    constexpr auto k_material_disabled_button_color = static_cast<jint>(0x1F000000U); // black @ ~12% alpha
    constexpr double k_material_button_corner_radius = 4;
    // MauiMaterialButton REMOVES the framework's 88x48dp Material minimum (styles.xml "MAUI manages it"), so a
    // one-line button realizes ~36dp = 2x8.5dp vertical padding + ~19dp text line (pixel-verified vs the MAUI
    // android captures). The plain android.widget.Button we build off Widget_Material_Light_Button KEEPS that
    // 48dp minHeight, rendering ~11dp too tall; override it down to the content-driven ~36dp floor so buttons
    // match MAUI without collapsing (0 would let TextView.onMeasure drop to text-only at map time).
    constexpr double k_material_button_min_height = 36;
    constexpr auto k_material_disabled_text_color =
        static_cast<jint>(0x61000000U); // black @ ~38% alpha (composites over the panel, not a baked #8B8B8B)

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

    // Strip the framework Material button style's default ELEVATION so the widget renders FLAT, matching the
    // native-default MAUI reference (parity policy §4). The port constructs the button through a framework
    // Material defStyleRes (see create_platform_view / k_button_style_field) because this AAR-less backend
    // can't link Material Components; that concrete style carries a default stateListAnimator that animates
    // translationZ (elevation) on rest → a DROP SHADOW under the button. Real .NET MAUI here renders under a
    // non-MaterialComponents base theme, so its MauiMaterialButton falls back to a flat, shadowless fill
    // (verified in docs/comparison/android/maui/{button,custom_layout}.png: pure white below every button,
    // no elevation). MauiMaterialButton.cs itself never sets elevation — the flatness is the theme's. To
    // reproduce that flat look on the styled widget, drop the animator and zero the resting elevation:
    //   setStateListAnimator(null); setElevation(0); setTranslationZ(0)
    // All three are android.view.View methods (GetMethodID walks the superclasses, so they resolve through
    // android/widget/Button). VM-less/failure-safe: each lookup is guarded and pending exceptions cleared.
    //
    // NOT flattened here — the framework background's INSET + rounded CORNERS (see the DEFERRED note in the
    // header block): those live in the style's InsetDrawable/GradientDrawable background, which ALSO carries
    // the button's intrinsic min-height + content padding. Swapping it for a bare flat GradientDrawable was
    // tried and COLLAPSED the default (no-size-request) buttons to nothing (they lost their intrinsic size) —
    // so the inset/corner cleanup is deferred to the android container/measure fan-out that will supply the
    // sizing independently. The shadow strip below is the safe, self-contained part.
    void strip_elevation(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        // setStateListAnimator(android.animation.StateListAnimator) — pass null to remove the default
        // Material animator that drives the elevation-on-rest shadow.
        if (jmethodID set_animator =
                cache.method(env, k_button_class, "setStateListAnimator", "(Landroid/animation/StateListAnimator;)V"))
        {
            env->CallVoidMethod(widget, set_animator, static_cast<jobject>(nullptr));
            clear_pending(env);
        }
        // Zero the resting elevation + translationZ (View.setElevation / View.setTranslationZ) so no shadow
        // is cast even if a residual Z remains after the animator is gone.
        if (jmethodID set_elevation = cache.method(env, k_button_class, "setElevation", "(F)V"))
        {
            env->CallVoidMethod(widget, set_elevation, static_cast<jfloat>(0));
            clear_pending(env);
        }
        if (jmethodID set_translation_z = cache.method(env, k_button_class, "setTranslationZ", "(F)V"))
        {
            env->CallVoidMethod(widget, set_translation_z, static_cast<jfloat>(0));
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

    // Forward decl: the stateful-fill helper is defined below (next to set_text_color_state_list) but used here.
    [[nodiscard]] bool set_fill_color_state_list(JNIEnv* env, jobject drawable, jint default_argb, jint disabled_argb);

    // THEME-AWARE disabled overlays. A disabled MauiMaterialButton dims its container to colorOnSurface@12%
    // and its label to colorOnSurface@38%, and Material flips colorOnSurface with the theme: BLACK in the
    // light theme, WHITE in the dark theme. The overlays were hardcoded to the LIGHT (black) values, so a
    // disabled button in DARK mode dimmed toward black over the dark surface — the WRONG direction (the 38%
    // layout_is_enabled dark diff + the modal disabled-button gap). Flip the RGB with the theme while keeping
    // the alpha bytes (0x1F ≈ 12%, 0x61 ≈ 38%) unchanged: light reuses the pixel-exact k_material_disabled_*
    // constants (already byte-exact), dark uses the same-alpha WHITE overlay. Over the button's #E0E0E0 fill /
    // a colored parent, white@12% / white@38% reproduce MAUI's measured dark LightBlue/LightPink/Teal/modal
    // disabled values to the byte. is_night_mode is the same Configuration.uiMode probe map_text_color reads
    // for the default label color (~line 1137); it is a cold-path property-map call, so re-reading it here
    // costs nothing (the codebase reads it inline per-site throughout the android handlers).
    [[nodiscard]] jint disabled_button_overlay(JNIEnv* env)
    {
        return maui::platform::android::detail::is_night_mode(env) ? static_cast<jint>(0x1FFFFFFFU)    // white @ ~12%
                                                                   : k_material_disabled_button_color; // black @ ~12%
    }
    [[nodiscard]] jint disabled_text_overlay(JNIEnv* env)
    {
        return maui::platform::android::detail::is_night_mode(env) ? static_cast<jint>(0x61FFFFFFU)  // white @ ~38%
                                                                   : k_material_disabled_text_color; // black @ ~38%
    }

    // Replace the framework Material InsetDrawable background with MAUI's flat MauiMaterialButton fill: a
    // solid #E0E0E0 GradientDrawable with ~4dp corners, installed edge-to-edge (no inset gap between docked
    // buttons, no near-invisible disabled state). setBackground zeroes the view's padding to the drawable's
    // own 0, so the Material content padding (16dp h / 8.5dp v) is re-supplied afterward — that padding, not
    // a hard min-height, drives the button's ~36dp content-driven height (matching MAUI, not a 48dp floor).
    // All JNI guarded and VM-less safe; a clean no-op when the GradientDrawable class can't be resolved.
    void install_flat_material_background(JNIEnv* env, jobject widget)
    {
        const local_ref<jobject> drawable = maui_background_drawable(env, widget, /*install=*/true);
        if (!drawable)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_color = cache.method(env, k_gradient_drawable_class, "setColor", "(I)V");
        jmethodID set_corner_radius = cache.method(env, k_gradient_drawable_class, "setCornerRadius", "(F)V");
        if (set_color == nullptr || set_corner_radius == nullptr)
        {
            return;
        }
        const float density = display_density(env, widget);
        // Stateful fill so a disabled button dims to colorOnSurface@12% (bleeding a colored parent through)
        // rather than staying opaque #E0E0E0; fall back to the plain opaque fill if the CSL can't be built.
        if (!set_fill_color_state_list(env, drawable.get(), k_material_default_button_color,
                                       disabled_button_overlay(env)))
        {
            env->CallVoidMethod(drawable.get(), set_color, k_material_default_button_color);
        }
        if (clear_pending(env))
        {
            return;
        }
        env->CallVoidMethod(drawable.get(), set_corner_radius,
                            static_cast<jfloat>(to_pixels(k_material_button_corner_radius, density)));
        if (clear_pending(env))
        {
            return;
        }
        // Re-supply the Material content padding setBackground just cleared (ButtonHandler.DefaultPadding).
        jmethodID set_padding = cache.method(env, k_button_class, "setPadding", "(IIII)V");
        if (set_padding != nullptr)
        {
            const jint px_h = to_pixels(k_default_padding_horizontal, density);
            const jint px_v = to_pixels(k_default_padding_vertical, density);
            env->CallVoidMethod(widget, set_padding, px_h, px_v, px_h, px_v);
            clear_pending(env);
        }
        // Override the framework style's 48dp Material minimum down to MAUI's content-driven ~36dp so the flat
        // button matches MAUI's height (TextView.setMinHeight AND View.setMinimumHeight — onMeasure enforces the
        // TextView pixel minimum separately from getSuggestedMinimumHeight).
        const jint min_px = to_pixels(k_material_button_min_height, density);
        jmethodID set_min_height = cache.method(env, k_button_class, "setMinHeight", "(I)V");
        if (set_min_height != nullptr)
        {
            env->CallVoidMethod(widget, set_min_height, min_px);
            clear_pending(env);
        }
        jmethodID set_minimum_height = cache.method(env, k_button_class, "setMinimumHeight", "(I)V");
        if (set_minimum_height != nullptr)
        {
            env->CallVoidMethod(widget, set_minimum_height, min_px);
            clear_pending(env);
        }
        // Zero the framework style's 88dp Material min-WIDTH (MauiMaterialButton's android:minWidth=0dp) so a
        // no-WidthRequest button sizes to its content + padding, not the ~242px Material floor — otherwise every
        // unsized button renders far too wide (custom_layout, clipping, header_footer_view, …). Width can't
        // collapse like height here: WRAP_CONTENT + the re-supplied 16dp horizontal padding give the intrinsic
        // content width.
        jmethodID set_min_width = cache.method(env, k_button_class, "setMinWidth", "(I)V");
        if (set_min_width != nullptr)
        {
            env->CallVoidMethod(widget, set_min_width, 0);
            clear_pending(env);
        }
        jmethodID set_minimum_width = cache.method(env, k_button_class, "setMinimumWidth", "(I)V");
        if (set_minimum_width != nullptr)
        {
            env->CallVoidMethod(widget, set_minimum_width, 0);
            clear_pending(env);
        }
    }

    // Install a two-state text ColorStateList {disabled → disabled_argb, default → default_argb} so the
    // framework dims the label to the Material disabled color when the button is disabled and restores the
    // resolved color when enabled — the stateful analog of MaterialButton's captured _defaultTextColors, with
    // no manual repaint on each setEnabled. Returns false (caller falls back to a plain setTextColor) on any
    // JNI failure or the VM-less path. Every ref is local; pending exceptions are cleared.
    [[nodiscard]] bool set_text_color_state_list(JNIEnv* env, jobject widget, jint default_argb, jint disabled_argb)
    {
        auto& cache = default_jni_cache();
        jclass color_state_list_class = cache.find_class(env, "android/content/res/ColorStateList");
        jmethodID ctor = cache.method(env, "android/content/res/ColorStateList", "<init>", "([[I[I)V");
        jmethodID set_text_color =
            cache.method(env, k_button_class, "setTextColor", "(Landroid/content/res/ColorStateList;)V");
        jclass int_array_class = cache.find_class(env, "[I");
        if (color_state_list_class == nullptr || ctor == nullptr || set_text_color == nullptr ||
            int_array_class == nullptr)
        {
            return false;
        }
        // states = { {-state_enabled}, {} }: the disabled spec first, then the empty catch-all (ColorStateList
        // returns the first matching spec's color). android.R.attr.state_enabled is the stable public framework
        // id 0x0101009e — hardcoded to avoid an android.R$attr lookup per button.
        constexpr jint k_state_enabled = 0x0101009e;
        const jint disabled_spec = -k_state_enabled;
        const local_ref<jintArray> disabled_state{env, env->NewIntArray(1)};
        const local_ref<jintArray> default_state{env, env->NewIntArray(0)};
        if (clear_pending(env) || !disabled_state || !default_state)
        {
            return false;
        }
        env->SetIntArrayRegion(disabled_state.get(), 0, 1, &disabled_spec);
        const local_ref<jobjectArray> states{env, env->NewObjectArray(2, int_array_class, nullptr)};
        if (clear_pending(env) || !states)
        {
            return false;
        }
        env->SetObjectArrayElement(states.get(), 0, disabled_state.get());
        env->SetObjectArrayElement(states.get(), 1, default_state.get());
        const std::array<jint, 2> color_values{disabled_argb, default_argb};
        const local_ref<jintArray> colors{env, env->NewIntArray(2)};
        if (clear_pending(env) || !colors)
        {
            return false;
        }
        env->SetIntArrayRegion(colors.get(), 0, 2, color_values.data());
        const local_ref<jobject> state_list{env,
                                            env->NewObject(color_state_list_class, ctor, states.get(), colors.get())};
        if (clear_pending(env) || !state_list)
        {
            return false;
        }
        env->CallVoidMethod(widget, set_text_color, state_list.get());
        return !clear_pending(env);
    }

    // Fill the flat GradientDrawable with a two-state ColorStateList {disabled → disabled_argb, default →
    // default_argb} via GradientDrawable.setColor(ColorStateList) (API 21+; a ColorStateList makes the drawable
    // stateful, so the View's drawableStateChanged on setEnabled auto-switches the fill — no manual repaint).
    // The disabled fill is a TRANSLUCENT overlay (colorOnSurface@12%) so a colored parent panel bleeds through,
    // matching MAUI; over white it composites to the same #E0E0E0. Returns false (caller falls back to a plain
    // setColor(I)) on any JNI failure or the VM-less path. Mirrors set_text_color_state_list's state array.
    [[nodiscard]] bool set_fill_color_state_list(JNIEnv* env, jobject drawable, jint default_argb, jint disabled_argb)
    {
        auto& cache = default_jni_cache();
        jclass color_state_list_class = cache.find_class(env, "android/content/res/ColorStateList");
        jmethodID ctor = cache.method(env, "android/content/res/ColorStateList", "<init>", "([[I[I)V");
        jmethodID set_color_csl =
            cache.method(env, k_gradient_drawable_class, "setColor", "(Landroid/content/res/ColorStateList;)V");
        jclass int_array_class = cache.find_class(env, "[I");
        if (color_state_list_class == nullptr || ctor == nullptr || set_color_csl == nullptr ||
            int_array_class == nullptr)
        {
            return false;
        }
        constexpr jint k_state_enabled = 0x0101009e; // android.R.attr.state_enabled (stable public id)
        const jint disabled_spec = -k_state_enabled;
        const local_ref<jintArray> disabled_state{env, env->NewIntArray(1)};
        const local_ref<jintArray> default_state{env, env->NewIntArray(0)};
        if (clear_pending(env) || !disabled_state || !default_state)
        {
            return false;
        }
        env->SetIntArrayRegion(disabled_state.get(), 0, 1, &disabled_spec);
        const local_ref<jobjectArray> states{env, env->NewObjectArray(2, int_array_class, nullptr)};
        if (clear_pending(env) || !states)
        {
            return false;
        }
        env->SetObjectArrayElement(states.get(), 0, disabled_state.get());
        env->SetObjectArrayElement(states.get(), 1, default_state.get());
        const std::array<jint, 2> color_values{disabled_argb, default_argb};
        const local_ref<jintArray> colors{env, env->NewIntArray(2)};
        if (clear_pending(env) || !colors)
        {
            return false;
        }
        env->SetIntArrayRegion(colors.get(), 0, 2, color_values.data());
        const local_ref<jobject> state_list{env,
                                            env->NewObject(color_state_list_class, ctor, states.get(), colors.get())};
        if (clear_pending(env) || !state_list)
        {
            return false;
        }
        env->CallVoidMethod(drawable, set_color_csl, state_list.get());
        return !clear_pending(env);
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
    // Forward decl: the icon+text composition helper is defined below (next to the source primitives) but
    // several mappers route through it — map_text (re-splice the icon next to new text), the source loaders
    // (compose on load), and clear_source_native (drop the icon). Static = internal linkage, one definition.
    static void apply_button_content(button_platform& platform);

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
        // gradient/ripple layers are part of the documented Material deviation). create_platform_view now
        // installs a flat #E0E0E0 GradientDrawable up front, so a null paint (the initial mapper sweep, or a
        // cleared Background) RESTORES that Material default fill rather than wiping it to transparent — the
        // faithful analog of C#'s "recreate the default ripple" restore.
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
        if (value == nullptr)
        {
            // Restore the flat default with the SAME stateful fill create_platform_view installs, so a null/
            // cleared paint doesn't clobber the disabled colorOnSurface@12% state back to an opaque #E0E0E0.
            if (!set_fill_color_state_list(env.get(), drawable.get(), k_material_default_button_color,
                                           disabled_button_overlay(env.get())))
            {
                env->CallVoidMethod(drawable.get(), set_color, k_material_default_button_color);
            }
            clear_pending(env.get());
            return;
        }
        // An explicit BackgroundColor is a single opaque fill (MAUI's ButtonExtensions.UpdateButtonBackground).
        env->CallVoidMethod(drawable.get(), set_color, static_cast<jint>(value->background_color().to_int()));
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
            // (DefaultStrokeThicknessNoColor). CornerRadius: a positive value wins; the port's default/unset 0
            // (MAUI's -1 collapsed to 0 in button.cpp) keeps the flat Material default ~4dp shape that
            // create_platform_view installs, so this connect-time sweep re-applies 4dp instead of squaring the
            // corners to 0. ponytail: default 0 is indistinguishable from an explicit CornerRadius=0, so an
            // explicit 0 also renders 4dp — restore the -1 sentinel in button.cpp if a square button matters.
            const double thickness = view.stroke_thickness() >= 0 ? view.stroke_thickness() : 0;
            const double radius =
                view.corner_radius() > 0 ? static_cast<double>(view.corner_radius()) : k_material_button_corner_radius;
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
            env->CallVoidMethod(drawable.get(), set_corner_radius, static_cast<jfloat>(to_pixels(radius, density)));
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
        if (button_class == nullptr)
        {
            return platform;
        }
        // ButtonHandler.CreatePlatformView: new MauiMaterialButton(Context) { SoundEffectsEnabled =
        // false, … } — the Icon* initializers are Material-only (header deviations). To reproduce the
        // filled Material button (light-gray fill + rounded corners + white text) that MauiMaterialButton
        // renders, construct through the theme-INDEPENDENT 4-arg ctor with the framework's concrete
        // Material button style as defStyleRes (see the k_button_style_field note). Try the light field,
        // then the generic/dark alt, then fall back to the plain (Context) ctor so the widget is never
        // null (a valid Button; only the filled Material *look* is lost on that fallback).
        jobject created = nullptr;
        jmethodID ctor_styled = cache.method(env.get(), k_button_class, "<init>",
                                             "(Landroid/content/Context;Landroid/util/AttributeSet;II)V");
        jclass style_class = cache.find_class(env.get(), k_style_class);
        // The Material button style is a STATIC field — the jni_cache's field() is GetFieldID (instance)
        // and returns null for it, so resolve it directly with GetStaticFieldID.
        jfieldID button_style_field =
            style_class != nullptr ? env->GetStaticFieldID(style_class, k_button_style_field, "I") : nullptr;
        clear_pending(env.get()); // a missing-field lookup raises NoSuchFieldError — clear it, then try the alt
        if (style_class != nullptr && button_style_field == nullptr)
        {
            button_style_field = env->GetStaticFieldID(style_class, k_button_style_field_alt, "I");
            clear_pending(env.get());
        }
        if (ctor_styled != nullptr && style_class != nullptr && button_style_field != nullptr)
        {
            const jint style_res = env->GetStaticIntField(style_class, button_style_field);
            if (!clear_pending(env.get()))
            {
                created = env->NewObject(button_class, ctor_styled, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0), style_res);
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_plain = cache.method(env.get(), k_button_class, "<init>", "(Landroid/content/Context;)V");
            if (ctor_plain != nullptr)
            {
                created = env->NewObject(button_class, ctor_plain, context);
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
        // Strip the framework Material style's default elevation so the button renders shadowless, matching
        // the native-default MAUI reference (see strip_elevation). Harmless on the plain-ctor fallback (that
        // widget has no animator/elevation, so the calls are no-ops).
        strip_elevation(env.get(), widget.get());
        // Replace the framework Material InsetDrawable with MAUI's flat edge-to-edge #E0E0E0 fill (~4dp
        // corners) and re-supply the content padding it carried — see install_flat_material_background. This
        // is the safe form of the header's DEFERRED inset/corner cleanup: the padding re-supply provides the
        // button's intrinsic sizing independently, so default (no-size-request) buttons no longer collapse.
        install_flat_material_background(env.get(), widget.get());
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
        // TextViewExtensions.UpdateTextPlainText sets textView.Text = label.Text, but on this backend the
        // button's CharSequence is COUPLED with the icon (an inline Left/Right icon is spliced INTO the text
        // as an ImageSpan — see apply_button_content), so a bare setText would drop the icon. Route through
        // the shared helper: it re-composes [icon + text] (or plain text when there's no icon) so a text
        // change re-splices the icon next to the new label. apply_button_content is a no-op when native is
        // null (VM-less) — the headless title mirror set above is what that suite observes.
        apply_button_content(*platform);
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
        if (!env)
        {
            return;
        }
        // ButtonHandler.MapTextColor: `if (button.TextColor is null) SetTextColor(_defaultTextColors)`
        // else `UpdateTextColor(button)`. C#'s ITextStyle.TextColor is a Color? defaulting to null → the
        // Material theme default the handler captured before any mapping (_defaultTextColors), which on a
        // filled MaterialButton is WHITE. The port models TextColor as a NON-nullable value type whose
        // default-constructed value (color{}) is opaque BLACK, so pushing view.text_color() unconditionally
        // set BLACK text on every unset (default) button — the dominant Android parity diff (MAUI renders
        // WHITE). Discriminate on whether the property was explicitly SET (BindableObject.IsSet), the
        // faithful stand-in for C#'s `!= null` — exactly as label_handler.mm does for the unset-color
        // sentinel collision. Unset → assert the Material default (white); a value compare can't be used
        // here because an explicit TextColor=White would equal that default and be misread. This AAR-less
        // backend has no captured _defaultTextColors ColorStateList, but the Material button's default
        // label IS white, so a white constant reproduces the same on-screen result MAUI's restore does.
        // LIGHT: MAUI's default filled-MaterialButton label is WHITE, so a white constant reproduces MAUI's
        // _defaultTextColors restore. DARK: MAUI's theme-dependent _defaultTextColors flips the default label
        // to BLACK on the same light-gray #E0E0E0 fill (measured off the shipped dark render —
        // toolbar/semantics/ios pages), so seed black when unset + night. Same DeviceDefault-vs-Material dark
        // gap the label default-text seed closed.
        const jint k_material_default_text_color = maui::platform::android::detail::is_night_mode(env.get())
                                                       ? static_cast<jint>(0xFF000000U)  // dark: black label
                                                       : static_cast<jint>(0xFFFFFFFFU); // light: white label
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
        // TextViewExtensions.UpdateTextColor: SetTextColor(textColor.ToPlatform()) — the ARGB int.
        const jint argb = color_is_set ? static_cast<jint>(view.text_color().to_int()) : k_material_default_text_color;
        // A disabled MauiMaterialButton dims its label to colorOnSurface@38% while the fill stays solid — black
        // over the light #E0E0E0 fill (#8B8B8B), WHITE in dark (disabled_text_overlay flips the RGB with the
        // theme; see its comment). Install a two-state text ColorStateList {disabled → that overlay, default →
        // the resolved color} so setEnabled dims/restores the label automatically (no per-toggle repaint). Fall
        // back to the plain single-color setTextColor if the ColorStateList can't be built (missing class / VM-less).
        jobject widget = widget_of(*platform);
        if (!set_text_color_state_list(env.get(), widget, argb, disabled_text_overlay(env.get())))
        {
            call_void_int(env.get(), widget, "setTextColor", argb);
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
        // Re-apply the render transform against the just-laid-out bounds (the same reapply the label handler
        // does). apply_transform's pivot is AnchorX/Y × the View's laid-out size; at map time the Button is 0×0,
        // so the pivot resolved to (0,0) and a rotated/scaled button pivoted about its top-left corner instead
        // of its center (the hit_testing transforms). Post-layout getWidth()/getHeight() are the real size, so
        // re-pushing the stored spec lands the pivot correctly. Only when the transform depends on the pivot
        // (any rotation, or a scale ≠ 1) — a pure translation/identity is already correct from the map-time push.
        {
            const auto& t = platform->transform;
            const bool pivot_matters = t.rotation != 0.0 || t.rotation_x != 0.0 || t.rotation_y != 0.0 ||
                                       t.scale != 1.0 || t.scale_x != 1.0 || t.scale_y != 1.0;
            if (pivot_matters)
            {
                maui::platform::android::apply_transform(platform->native, t);
            }
        }
    }

    // ---- per-backend image-source primitives (the cross-platform map_image_source routes here) ----
    // The android Button is a TextView-derived widget, so it has no MaterialButton .Icon/IconGravity; the icon
    // is composed onto the real widget by apply_button_content below (inline ImageSpan for the Left/Right
    // layouts, compound drawable for Top/Bottom). The primitives ALSO keep the shared headless-style mirrors
    // (kind/file/loaded) live so the android preset's pure-native cross-platform suite still observes the load
    // where no Java VM exists (apply_button_content is a no-op there).
    void button_handler::configure_loader(maui::core::image_source_loader& /*loader*/)
    {
    }

    // Compose the Button's CharSequence as [icon + spacing + text] — ONE group centered as a unit — matching
    // MAUI's MauiMaterialButton, which sets IconGravity = ICON_GRAVITY_TEXT_START: the icon is drawn adjacent
    // to the label and the whole [icon|text] group is centered by the Material style's gravity=center. A plain
    // android.widget.Button has no MaterialButton IconGravity; setCompoundDrawables (the previous cut) pins the
    // icon to the view's LEFT EDGE and then center-gravities only the leftover TEXT, so the icon can never sit
    // next to centered text (measured: cpp gear+text spanned x=165..795, the gear ~205px too far left of MAUI's
    // centered group). To reproduce IconGravity=TextStart on a bare TextView, SPLICE the icon INTO the text as
    // an android.text.style.ImageSpan(ALIGN_CENTER) over a U+FFFC object-replacement char, so the single line —
    // icon glyph + spacing + letters — is centered as one unit. api-34 honors ImageSpan.ALIGN_CENTER (=2) via
    // the stock ImageSpan(Drawable,int) ctor (min-api 24; degrades harmlessly to bottom-align below 29).
    //
    // Only the inline Left/Right layouts splice: a single-line TextView cannot STACK an icon, so Top/Bottom keep
    // the compound-drawable seam (already horizontally centered, the h-centered analog of IconGravity=Top). No
    // decoded source ⇒ plain title + cleared compound drawable. This helper OWNS the button's text now (icon and
    // label are coupled), so every caller that changes either — map_text / the source loaders / clear_source_native
    // — routes through it. The bundled bytes decode via the shared image_decode helper (the image control's
    // fast-path). Ported to stand in for ButtonExtensions.UpdateContentLayout (MaterialButton.Icon+IconGravity).
    static void apply_button_content(button_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return; // VM-less / headless: the title + content_layout mirrors (maintained by callers) suffice
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject widget = widget_of(platform);
        auto& cache = default_jni_cache();
        jmethodID set_text = cache.method(env.get(), k_button_class, "setText", "(Ljava/lang/CharSequence;)V");
        if (set_text == nullptr)
        {
            return;
        }
        using pos = maui::core::button_content_spec::image_position;
        const maui::core::button_content_spec& spec = platform.content_layout;
        const bool has_icon = platform.source_loaded && !platform.source_file.empty();

        // Set the plain title (to_jstring is supplementary-plane safe — see jni_string.hpp).
        const auto set_plain_title = [&] {
            const local_ref<jstring> text = to_jstring(env.get(), platform.title);
            env->CallVoidMethod(widget, set_text, text.get());
            clear_pending(env.get());
        };
        // Drop any Top/Bottom compound-drawable icon (null×4) — used by the no-icon and inline paths so a
        // Left↔Top cycle doesn't leave a stale stacked icon behind the spliced/plain text.
        const auto clear_compound = [&] {
            jmethodID clear = cache.method(
                env.get(), k_button_class, "setCompoundDrawablesWithIntrinsicBounds",
                "(Landroid/graphics/drawable/Drawable;Landroid/graphics/drawable/Drawable;Landroid/graphics/"
                "drawable/Drawable;Landroid/graphics/drawable/Drawable;)V");
            if (clear != nullptr)
            {
                jobject nul = nullptr;
                env->CallVoidMethod(widget, clear, nul, nul, nul, nul);
                clear_pending(env.get());
            }
        };

        if (!has_icon)
        {
            set_plain_title();
            clear_compound();
            return;
        }

        // Decode the bundled icon (shared by the inline Left/Right splice + the Top/Bottom compound path).
        namespace img = maui::platform::android::image_decode;
        const auto bytes = img::resolve_file_bytes(env.get(), platform.source_file);
        const local_ref<jobject> bitmap = img::decode_bitmap(env.get(), bytes);
        if (!bitmap)
        {
            // Undecodable (e.g. an SVG-only asset) → plain text, no icon (matches a nil apple decode).
            set_plain_title();
            clear_compound();
            return;
        }
        // new BitmapDrawable(context.getResources(), bitmap) — the density-aware ctor so the icon renders at
        // its natural dp size (like MAUI's ImageSource icon).
        jmethodID get_context = cache.method(env.get(), k_button_class, "getContext", "()Landroid/content/Context;");
        const local_ref<jobject> context{env.get(),
                                         get_context != nullptr ? env->CallObjectMethod(widget, get_context) : nullptr};
        if (clear_pending(env.get()) || !context)
        {
            set_plain_title();
            return;
        }
        jmethodID get_resources =
            cache.method(env.get(), "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
        const local_ref<jobject> resources{
            env.get(), get_resources != nullptr ? env->CallObjectMethod(context.get(), get_resources) : nullptr};
        if (clear_pending(env.get()) || !resources)
        {
            set_plain_title();
            return;
        }
        jclass drawable_class = cache.find_class(env.get(), "android/graphics/drawable/BitmapDrawable");
        jmethodID drawable_ctor = cache.method(env.get(), "android/graphics/drawable/BitmapDrawable", "<init>",
                                               "(Landroid/content/res/Resources;Landroid/graphics/Bitmap;)V");
        if (drawable_class == nullptr || drawable_ctor == nullptr)
        {
            set_plain_title();
            return;
        }
        const local_ref<jobject> drawable{env.get(),
                                          env->NewObject(drawable_class, drawable_ctor, resources.get(), bitmap.get())};
        if (clear_pending(env.get()) || !drawable)
        {
            set_plain_title();
            return;
        }
        // Size the icon to the bitmap's natural size treated as DP, scaled by display density — the mdpi(1x)→
        // device-density upscale MAUI's resizetizer performs by emitting settings.png into the density-less
        // res/drawable/ folder. setCompoundDrawables / the ImageSpan below honor these explicit bounds.
        const float density = display_density(env.get(), widget);
        jmethodID get_bw = cache.method(env.get(), "android/graphics/Bitmap", "getWidth", "()I");
        jmethodID get_bh = cache.method(env.get(), "android/graphics/Bitmap", "getHeight", "()I");
        const jint bw = get_bw != nullptr ? env->CallIntMethod(bitmap.get(), get_bw) : 0;
        const jint bh = get_bh != nullptr ? env->CallIntMethod(bitmap.get(), get_bh) : 0;
        clear_pending(env.get());
        const jint icon_w = bw > 0 ? to_pixels(static_cast<double>(bw), density) : 0;
        const jint icon_h = bh > 0 ? to_pixels(static_cast<double>(bh), density) : 0;
        if (icon_w > 0 && icon_h > 0)
        {
            jmethodID set_bounds =
                cache.method(env.get(), "android/graphics/drawable/BitmapDrawable", "setBounds", "(IIII)V");
            if (set_bounds != nullptr)
            {
                env->CallVoidMethod(drawable.get(), set_bounds, 0, 0, icon_w, icon_h);
                clear_pending(env.get());
            }
        }

        // Top/Bottom: a single-line TextView can't splice inline, so hang the icon above/below via the
        // compound-drawable seam (already h-centered). Set the plain title, then the drawable + spacing.
        if (spec.position == pos::top || spec.position == pos::bottom)
        {
            set_plain_title();
            jmethodID set_compound = cache.method(
                env.get(), k_button_class, "setCompoundDrawables",
                "(Landroid/graphics/drawable/Drawable;Landroid/graphics/drawable/Drawable;Landroid/graphics/"
                "drawable/Drawable;Landroid/graphics/drawable/Drawable;)V");
            if (set_compound != nullptr)
            {
                jobject d = drawable.get();
                jobject nul = nullptr;
                if (spec.position == pos::top)
                {
                    env->CallVoidMethod(widget, set_compound, nul, d, nul, nul);
                }
                else
                {
                    env->CallVoidMethod(widget, set_compound, nul, nul, nul, d);
                }
                clear_pending(env.get());
            }
            call_void_int(env.get(), widget, "setCompoundDrawablePadding", to_pixels(spec.spacing, density));
            return;
        }

        // Left/Right: wrap the icon in an InsetDrawable that reserves the ButtonContentLayout spacing on the
        // side facing the text, then splice it into the label via an ImageSpan(ALIGN_CENTER). The InsetDrawable's
        // OWN bounds are (0,0, icon_w+spacing, icon_h); its inset pushes the icon to one side, leaving `spacing`
        // px of gap toward the text — reproducing MauiButtonContentLayout's icon-title spacing inside the single
        // centered line. ponytail: clamp a negative spacing (the gallery's Decrease-Spacing can drive it < 0) to
        // 0 so the InsetDrawable never over-draws its bounds — a negative inline gap has no MAUI witness here.
        jint spacing_px = to_pixels(spec.spacing, density);
        if (spacing_px < 0)
        {
            spacing_px = 0;
        }
        jclass inset_class = cache.find_class(env.get(), "android/graphics/drawable/InsetDrawable");
        jmethodID inset_ctor = cache.method(env.get(), "android/graphics/drawable/InsetDrawable", "<init>",
                                            "(Landroid/graphics/drawable/Drawable;IIII)V");
        jclass spannable_class = cache.find_class(env.get(), "android/text/SpannableString");
        jmethodID spannable_ctor =
            cache.method(env.get(), "android/text/SpannableString", "<init>", "(Ljava/lang/CharSequence;)V");
        jmethodID set_span =
            cache.method(env.get(), "android/text/SpannableString", "setSpan", "(Ljava/lang/Object;III)V");
        // dev.mauicpp.MauiCenteredImageSpan CO-CENTERS the icon with the text (getSize/draw overrides) —
        // MAUI's IconGravity=TextStart shares one vertical centre. The stock ImageSpan(ALIGN_CENTER) left the
        // text on its baseline below a tall icon (button "settings" rows). Fall back to the stock span if the
        // app-dex class is unavailable (e.g. a host that didn't bundle it).
        jclass image_span_class = cache.find_class(env.get(), "dev/mauicpp/MauiCenteredImageSpan");
        jmethodID image_span_ctor = image_span_class != nullptr
                                        ? cache.method(env.get(), "dev/mauicpp/MauiCenteredImageSpan", "<init>",
                                                       "(Landroid/graphics/drawable/Drawable;)V")
                                        : nullptr;
        bool centered_span = image_span_class != nullptr && image_span_ctor != nullptr;
        if (!centered_span)
        {
            image_span_class = cache.find_class(env.get(), "android/text/style/ImageSpan");
            image_span_ctor = cache.method(env.get(), "android/text/style/ImageSpan", "<init>",
                                           "(Landroid/graphics/drawable/Drawable;I)V");
        }
        if (inset_class == nullptr || inset_ctor == nullptr || spannable_class == nullptr ||
            spannable_ctor == nullptr || set_span == nullptr || image_span_class == nullptr ||
            image_span_ctor == nullptr)
        {
            set_plain_title(); // old-API / missing-class fallback: keep the label, lose the inline icon
            return;
        }
        // left → inset the gap on the RIGHT (icon before text); right → inset it on the LEFT (icon after text).
        const jint inset_left = spec.position == pos::right ? spacing_px : 0;
        const jint inset_right = spec.position == pos::left ? spacing_px : 0;
        const local_ref<jobject> inset{
            env.get(), env->NewObject(inset_class, inset_ctor, drawable.get(), inset_left, 0, inset_right, 0)};
        if (clear_pending(env.get()) || !inset)
        {
            set_plain_title();
            return;
        }
        if (jmethodID set_bounds_inset =
                cache.method(env.get(), "android/graphics/drawable/InsetDrawable", "setBounds", "(IIII)V"))
        {
            env->CallVoidMethod(inset.get(), set_bounds_inset, 0, 0, icon_w + spacing_px, icon_h);
            clear_pending(env.get());
        }
        // U+FFFC OBJECT REPLACEMENT CHARACTER (UTF-8 EF BF BC) — the single glyph the ImageSpan covers; the
        // label is spliced before/after it so the [icon|text] group flows as one centered line.
        constexpr std::string_view k_object_replacement = "\xEF\xBF\xBC";
        const std::string combined = spec.position == pos::left ? std::string(k_object_replacement) + platform.title
                                                                : platform.title + std::string(k_object_replacement);
        const local_ref<jstring> combined_j = to_jstring(env.get(), combined);
        if (!combined_j)
        {
            set_plain_title();
            return;
        }
        constexpr jint k_align_center = 2; // DynamicDrawableSpan.ALIGN_CENTER (API 29+; degrades below)
        const local_ref<jobject> spannable{env.get(),
                                           env->NewObject(spannable_class, spannable_ctor, combined_j.get())};
        // MauiCenteredImageSpan ctor is (Drawable); the stock ImageSpan fallback is (Drawable, alignment).
        const local_ref<jobject> span{
            env.get(), centered_span ? env->NewObject(image_span_class, image_span_ctor, inset.get())
                                     : env->NewObject(image_span_class, image_span_ctor, inset.get(), k_align_center)};
        if (clear_pending(env.get()) || !spannable || !span)
        {
            set_plain_title();
            return;
        }
        // The object-replacement char is at index 0 (left) or the last UTF-16 unit (right); GetStringLength
        // returns the exact UTF-16 length SpannableString copies char-for-char.
        const jint total_len = env->GetStringLength(combined_j.get());
        const jint span_start = spec.position == pos::left ? 0 : total_len - 1;
        constexpr jint k_span_inclusive_exclusive = 33; // Spanned.SPAN_INCLUSIVE_EXCLUSIVE (0x21)
        env->CallVoidMethod(spannable.get(), set_span, span.get(), span_start, span_start + 1,
                            k_span_inclusive_exclusive);
        if (clear_pending(env.get()))
        {
            set_plain_title();
            return;
        }
        env->CallVoidMethod(widget, set_text, spannable.get());
        clear_pending(env.get());
        clear_compound(); // drop any stale Top/Bottom stacked icon now that the icon rides inline
    }

    void button_handler::load_file_source_sync(button_platform& platform, const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
        apply_button_content(platform);
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
        apply_button_content(platform);
    }

    void button_handler::clear_source_native(button_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
        // Re-compose through the shared helper: with no source it restores the plain title (dropping the
        // spliced inline ImageSpan icon via setText) and clears any Top/Bottom compound drawable. Guards
        // native == nullptr internally (VM-less no-op), so the source mirrors above are all that suite sees.
        apply_button_content(platform);
    }
} // namespace maui::core
