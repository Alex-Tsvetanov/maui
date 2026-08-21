// switch_handler — Android (JNI) platform partial, the M-android per-control fan-out replayed for the
// two-state toggle. The managed platform view is a REAL android.widget.Switch (held as a JNI global
// reference in switch_platform::native): IsOn maps onto setChecked (isChecked read-back), TrackColor
// onto the track drawable's color filter, ThumbColor onto setThumbTintList. Like the iOS twin, IsOn is
// also the INBOUND channel — a native checked-change writes i_switch::set_is_on back — but that
// listener is DEFERRED with the gesture/event fan-out (see the deviation below), so on_value_changed
// stays an invokable C++ callback the cross-platform suite drives.
//
// Ported DIRECTLY from SwitchHandler.Android.cs + Platform/Android/{SwitchExtensions.cs,
// ColorStateListExtensions.cs, ViewExtensions.cs (UpdateVisibility/UpdateOpacity/UpdateIsEnabled/
// UpdateAutomationId), ContextExtensions.cs (ToPixels)}.
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each a library or infrastructure gap, not a
// behavior guess):
//   - The widget IS SwitchHandler.Android.cs's `AndroidX.AppCompat.Widget.SwitchCompat` (the `ASwitch`
//     alias) as of 2026-08-10. It was a plain android.widget.Switch, justified by "AppCompat is a
//     gradle/AAR dependency this APK-less backend does not carry" — which was stale: the AAR closure
//     is staged and dexed in both app hosts (probed in the shipped APK). The Material `MSwitch`
//     (MaterialSwitch) overloads of
//     UpdateTrackColor/UpdateThumbColor are likewise Material-only and unreachable; this partial always
//     takes the plain-ASwitch branch (UpdateTrackColor via TrackDrawable.SetColorFilter, UpdateThumbColor
//     via ThumbTintList) — exactly the branch C# takes for a non-Material SwitchCompat.
//   - ThumbColor: C# is `ThumbTintList = ColorStateListExtensions.CreateDefault(color.ToPlatform())`,
//     and CreateDefault → PlatformInterop.GetDefaultColorStateList — a MAUI runtime Java helper this
//     backend does not ship. ColorStateList.valueOf(int) is the faithful framework equivalent: a
//     single-default-state list carrying that one color (what GetDefaultColorStateList builds), and the
//     same constructor progress_bar_handler.cpp uses for setProgressTintList. The thumb shadow the C#
//     comment preserves by avoiding SetColorFilter is preserved here for the same reason (tint list, not
//     color filter).
//   - The port's colors are non-nullable value types, so SwitchExtensions' `color is not null` branches
//     collapse exactly as in the apple/ios partials: an UNSET TrackColor (the default-black sentinel) and
//     an unset ThumbColor keep the widget's DEFAULT track/thumb — UpdateThumbColor(ASwitch)'s C# body only
//     sets a tint when a custom color is present (no else branch).
//   - NO TINT SEEDING (removed 2026-08-10, together with the framework-Switch deviation above). This
//     partial used to pin hardcoded Material grays (#B2B2B2 track / #ECECEC thumb) as the UNSET baseline,
//     because a framework android.widget.Switch on Theme.DeviceDefault.Light drew its checked thumb in the
//     DeviceDefault indigo. BOTH premises died with the SwitchCompat swap: the widget now resolves its own
//     tints, and the host theme parents Theme.MaterialComponents.DayNight
//     (apphost/res/values/styles.xml:48). The seed had become an invention MAUI never performs, and it was
//     ACTIVELY WRONG in three measured ways — it froze the LIGHT greys into DARK (MAUI thumb 185, port
//     231-236), rendered IsEnabled="False" identically to the enabled rows (the opaque filter masking the
//     disabled state of abc_tint_switch_track), and swallowed an explicit BackgroundColor (MAUI track 77,
//     port 178). Dark whole-frame diff was 0.872% against light's 0.193%.
//   - FIRST-INSTANCE TRACK, OPEN AND NOT A TINT PROBLEM (measured 2026-08-21, android emulator API 34).
//     The FIRST Switch constructed per process renders its thumb correctly and NO TRACK: on switch.xaml the
//     grey #B2B2B2 track bands are y=267/799/1263/1495 in MAUI and y=799/1263/1495 in the port -- the first
//     is missing, while the SECOND bare `<Switch />` (same markup, y=799) is byte-correct. context_flyout
//     has exactly one Switch, so it always loses it (1087 track px in MAUI vs 154 in the port).
//     SEVEN hypotheses were instrumented on-device and ALL REFUTED -- every one of these is identical
//     across all six switches on the page, so none of them is the cause:
//         1. ctor fallback chain      all six log `plain-first -> OK`; the styled fallbacks never run
//         2. null track drawable      all six report the drawable PRESENT at construction
//         3. capture artifact         reproduces on every fresh build + fresh capture
//         4. measure                  all six: wc=392.7 hc=inf density=2.75 -> 132x132 px -> 48x48 dp
//         5. arrange frame            all six: x=0.0 w=392.7 h=48.0 (only y differs)
//         6. drawable identity/alpha  all six: alpha=255, android.graphics.drawable.NinePatchDrawable
//         7. post-layout geometry     all six: view w=1080 l=0 r=1080 padR=0 minW=132
//     The rendered ink is 90px wide for a correct switch and 60px for the first one -- exactly the track's
//     width -- and the pixels where the track belongs are PURE WHITE (255), not a faint or mis-tinted grey.
//     So the track is not drawn at all, by a widget whose every JNI-observable property matches its
//     working siblings. Whatever remains is inside SwitchCompat's own draw path.
//     DO NOT "fix" this by re-seeding a track tint: an opaque filter would paint the first switch AND
//     re-break dark theme, IsEnabled=false and explicit BackgroundColor, which is precisely why the seeding
//     was removed.
//     SOLVED 2026-08-22 — AND NOT IN THIS FILE. An eighth probe read getTrackTintList/getThumbTintList/the
//     drawable class/alpha/colour filter for all six switches on switch.xaml: byte-identical on every one
//     (null tint list, NinePatchDrawable, alpha 255, no filter), so the tint branch is refuted too. The
//     difference was never a property of the widget — it was WHO BUILT THE FIRST ONE. AppCompat tints a
//     widget's style drawables through ResourceManagerInternal's hooks, and those hooks are installed when
//     AppCompatDrawableManager is first created; a widget constructed before that gets the UNTINTED asset,
//     and abc_switch_track_mtrl_alpha untinted is a WHITE nine-patch — exactly the "pure white where the
//     track belongs" the probes measured. MAUI never sees this because MauiAppCompatActivity IS an
//     AppCompatActivity; the port's host is a plain android.app.Activity. The fix is one
//     AppCompatDrawableManager.preload() in apphost/MauiHostActivity.onCreate, before any page widget is
//     built. Verified on device: the port's switch.xaml now has ink counts identical to MAUI's on all three
//     switch rows (2127 / 1951 / 2476).
//     map_track_color's unset branch now ClearColorFilters exactly as SwitchExtensions.cs:33 does, and
//     map_thumb_color's unset branch leaves the tint alone exactly as :89-99 does (no else branch).
//   - The OnCheckedChangeListener (CheckedChangeListener → OnCheckedChanged → VirtualView.IsOn write,
//     guarded against echo) is DEFERRED with the gesture/event fan-out, exactly like button_handler.cpp
//     defers the OnTouchListener. on_value_changed stays a wired, invokable C++ callback carrying that
//     exact guarded write-back body (the cross-platform suite drives it); the real native checked-change
//     channel arrives with the rest of the android event wiring.
//   - GetDesiredSize's SwitchMinWidth theme-attribute fallback (Context.GetThemeAttributeDp on a
//     zero-width measure) is a bare-Context-hostile theme read; the partial uses the shared
//     measure-via-View.measure seam (the same ViewHandlerExtensions.GetDesiredSizeFromHandler path the
//     other android partials use), which yields a non-zero width for a real Switch, so the C# `Width == 0`
//     fallback branch is not reached. The headless mirror keeps the UISwitch-shaped 51x31 placeholder.
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the emulator
// where no Java VM exists. Every JNI path here checks scoped_env/app_context() and quietly skips, while
// the headless mirrors (is_on/track_color/thumb_color + the base IView mirrors) are ALWAYS maintained —
// so that suite observes exactly the headless partial's behavior, and the widget test host additionally
// observes the real widget.

#include "maui/core/switch_handler.hpp"

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
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_switch.hpp"
#include "maui/core/semantics.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
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
    // superclasses, so the CompoundButton/TextView/View surface resolves through android/widget/Switch).
    // SwitchHandler.Android.cs:6 aliases ASwitch to AndroidX.AppCompat.Widget.SwitchCompat and :15
    // constructs `new ASwitch(Context)` — so MAUI's Switch is an APPCOMPAT widget on the plain (Context)
    // ctor, not android.widget.Switch. The AAR carrying it is ALREADY staged and dexed in both app hosts
    // (verified by probing the shipped APK's classes.dex for Landroidx/appcompat/widget/SwitchCompat;),
    // so the long-standing "no gradle, no maven egress" note that justified the framework widget is stale.
    constexpr const char* k_switch_class = "androidx/appcompat/widget/SwitchCompat";
    constexpr const char* k_style_class = "android/R$style";
    // The concrete platform style that carries the Switch's thumb + track drawables, resolved
    // theme-independently as a defStyleRes so the bare app_process testhost (and the app host) construct a
    // Switch that actually HAS its thumb/track (defStyleAttr=0 with no defStyleRes resolves no thumb/track
    // drawable → an invisible, drawable-less, size-0 toggle — empirically the missing-glyph bug here).
    //
    // UNLIKE CheckBox/RadioButton there is NO `Widget_CompoundButton_Switch` field in android.R.style
    // (verified against API-34 android.jar: the CompoundButton family ships CheckBox/RadioButton/Star but
    // the Switch style lives ONLY under the Material namespace). So the Switch's concrete style is
    // android.R.style.Widget_Material_Light_CompoundButton_Switch (the light-theme variant the gallery's
    // light Activity theme uses; the dark twin is Widget_Material_CompoundButton_Switch). The k_*_alt field
    // is the non-Light Material fallback tried if the Light id is absent on some API level. (Read with
    // GetStaticFieldID — these are static fields.)
    constexpr const char* k_switch_style_field = "Widget_Material_Light_CompoundButton_Switch";
    constexpr const char* k_switch_style_field_alt = "Widget_Material_CompoundButton_Switch";
    constexpr const char* k_drawable_class = "android/graphics/drawable/Drawable";
    constexpr const char* k_color_state_list_class = "android/content/res/ColorStateList";
    constexpr const char* k_porter_duff_mode_class = "android/graphics/PorterDuff$Mode";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // The Material light-theme Switch default track + thumb colors (PARITY, the Switch twin of the slider's
    // seed_default_material_tints fix, commit bbb632f301). Real .NET MAUI renders its toggle as a
    // SwitchCompat under Theme.MaterialComponents.DayNight, whose UNSET (native-default) track + thumb
    // resolve through colorControlNormal / colorControlActivated to the Material light GRAYS. Measured off
    // the maui-compare Android baseline (docs/comparison/android/{switch,controls_stack}.png): the OFF track
    // is #B2B2B2 with an #ECECEC thumb, and the ON track lightens to #DCDCDC with a white thumb — i.e. a
    // neutral-gray Material toggle in BOTH states. This AAR-less app host has no AndroidX/Material theme;
    // its Activity uses the framework Theme.DeviceDefault.Light (see apphost/res/values/styles.xml), whose
    // colorAccent / colorControlActivated is the DeviceDefault INDIGO — so a bare framework Switch draws its
    // CHECKED thumb (and a faint blue tint on the track) in that dark navy (#485B8F measured), NOT MAUI's
    // gray. That accent mismatch is the sole cause of the "thick dark-blue thumb/track vs MAUI's thin gray
    // Material" parity diff on controls_stack (and any page with a checked, unset-color Switch). The fix
    // seeds these Material grays as the Switch's UNSET baseline (seed_default_material_tints) so the
    // DeviceDefault indigo is replaced by MAUI's gray. This is NOT a deviation from SwitchExtensions' plain-
    // ASwitch semantics — MAUI leaves the native default *because that default is already gray*; the port
    // pins the equivalent gray because its host theme's default is not. An explicitly-set TrackColor /
    // ThumbColor still overrides through the mappers below (the BackgroundColor/OnColor/ThumbColor rows on
    // the switch page verify the override path). A single filter/tint cannot reproduce Material's per-state
    // track transition (#B2B2B2 off → #DCDCDC on); the neutral off-state gray is the faithful "unset
    // baseline" (the same single-value choice the slider made for its track), and it removes the offending
    // blue entirely.

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


    [[nodiscard]] jobject widget_of(const maui::core::switch_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the handler must never leak JNI pending-exception state into the
    // cross-platform layer); true when one was pending — call sites skip the read-back.
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
        if (jmethodID method = default_jni_cache().method(env, k_switch_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_switch_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_switch_class, name, "(Z)V"))
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
    // s_displayDensity cache; the measure/arrange seam reads it directly here, the same walk —
    // identical to progress_bar_handler.cpp.)
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_switch_class, "getContext", "()Landroid/content/Context;");
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

    // Push a SrcAtop color filter onto the Switch's track drawable (SwitchExtensions.UpdateTrackColor's
    // set branch). A null track drawable no-ops (C#'s `TrackDrawable?.`). Shared by map_track_color (an
    // explicit TrackColor) and seed_default_material_tints (the Material-gray baseline). Returns false only
    // on a JNI resolution failure (the caller then leaves the track as-is).
    bool apply_track_filter(JNIEnv* env, jobject widget, const maui::graphics::color& color)
    {
        auto& cache = default_jni_cache();
        jmethodID get_track_drawable =
            cache.method(env, k_switch_class, "getTrackDrawable", "()Landroid/graphics/drawable/Drawable;");
        if (get_track_drawable == nullptr)
        {
            return false;
        }
        const local_ref<jobject> drawable{env, env->CallObjectMethod(widget, get_track_drawable)};
        if (clear_pending(env) || !drawable)
        {
            return true; // no track drawable yet — C#'s `TrackDrawable?.` null-conditional no-op
        }
        jmethodID set_color_filter =
            cache.method(env, k_drawable_class, "setColorFilter", "(ILandroid/graphics/PorterDuff$Mode;)V");
        jclass mode_class = cache.find_class(env, k_porter_duff_mode_class);
        // SRC_ATOP is a STATIC field — jni_cache::field() is GetFieldID (instance) and returns null for it,
        // so resolve it directly with GetStaticFieldID (lesson 1).
        jfieldID src_atop_field =
            mode_class != nullptr ? env->GetStaticFieldID(mode_class, "SRC_ATOP", "Landroid/graphics/PorterDuff$Mode;")
                                  : nullptr;
        clear_pending(env);
        if (set_color_filter == nullptr || src_atop_field == nullptr || mode_class == nullptr)
        {
            return false;
        }
        const local_ref<jobject> src_atop{env, env->GetStaticObjectField(mode_class, src_atop_field)};
        if (clear_pending(env) || !src_atop)
        {
            return false;
        }
        env->CallVoidMethod(drawable.get(), set_color_filter, static_cast<jint>(color.to_int()), src_atop.get());
        clear_pending(env);
        return true;
    }

    // Push a single-color ColorStateList onto the Switch's thumb (SwitchExtensions.UpdateThumbColor's set
    // branch — ThumbTintList, NOT SetColorFilter, to preserve the thumb shadow, as the C# comment notes).
    // Shared by map_thumb_color (an explicit ThumbColor) and seed_default_material_tints (the Material-gray
    // baseline).
    void apply_thumb_tint(JNIEnv* env, jobject widget, const maui::graphics::color& color)
    {
        auto& cache = default_jni_cache();
        jmethodID value_of =
            cache.static_method(env, k_color_state_list_class, "valueOf", "(I)Landroid/content/res/ColorStateList;");
        jmethodID set_thumb_tint =
            cache.method(env, k_switch_class, "setThumbTintList", "(Landroid/content/res/ColorStateList;)V");
        jclass color_state_list_class = cache.find_class(env, k_color_state_list_class);
        if (value_of == nullptr || set_thumb_tint == nullptr || color_state_list_class == nullptr)
        {
            return;
        }
        const auto argb = static_cast<jint>(color.to_int());
        const local_ref<jobject> tint_list{env, env->CallStaticObjectMethod(color_state_list_class, value_of, argb)};
        if (clear_pending(env) || !tint_list)
        {
            return;
        }
        env->CallVoidMethod(widget, set_thumb_tint, tint_list.get());
        clear_pending(env);
    }

    // STALE-COMMENT CORRECTION (2026-08-21): the paragraph that used to sit here described
    // `seed_default_material_tints` -- pinning hardcoded Material grays at construction -- as if this
    // partial still did it. IT DOES NOT. That seeding was REMOVED on 2026-08-10 (see the header's "NO TINT
    // SEEDING" bullet) because it was measured wrong in three ways at once. The function it described does
    // not exist in this file; only slider_handler.cpp still has one. Anyone reading the old text would
    // reasonably "restore" a fix that was deliberately deleted, so it is replaced rather than trimmed.
    //
    // What this function actually is: SwitchExtensions.UpdateTrackColor's ELSE branch --
    // `aSwitch.TrackDrawable?.ClearColorFilter()` (SwitchExtensions.cs:33). Set-or-clear, no third state.
    bool clear_track_filter(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        jmethodID get_track_drawable =
            cache.method(env, k_switch_class, "getTrackDrawable", "()Landroid/graphics/drawable/Drawable;");
        if (get_track_drawable == nullptr)
        {
            return false;
        }
        const local_ref<jobject> drawable{env, env->CallObjectMethod(widget, get_track_drawable)};
        if (clear_pending(env) || !drawable)
        {
            return true; // C#'s `TrackDrawable?.` null-conditional no-op
        }
        jmethodID clear_color_filter = cache.method(env, k_drawable_class, "clearColorFilter", "()V");
        if (clear_color_filter == nullptr)
        {
            return false;
        }
        env->CallVoidMethod(drawable.get(), clear_color_filter);
        clear_pending(env);
        return true;
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.Switch (the JNI shape of the
    // pimpl-owned-native-view doctrine: the ios twin CFReleases its UISwitch here). The container
    // wrapper is an iOS/Apple NeedsContainer concept (UIView/NSView) — the android backend has no
    // native wrapper here (the container fan-out has not arrived), so there is nothing to release.
    switch_platform::~switch_platform()
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

    void switch_platform::update_visibility(maui::core::visibility value)
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

    void switch_platform::update_opacity(double value)
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

    void switch_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // ViewExtensions.UpdateIsEnabled: platformView.Enabled = view.IsEnabled (a Switch is an
            // interactive CompoundButton — unlike the progress bar, it has a real enabled state).
            call_void_bool(env.get(), widget_of(*this), "setEnabled", static_cast<jboolean>(value));
        }
    }

    void switch_platform::update_automation_id(std::string_view value)
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
        jmethodID get_important = cache.method(env.get(), k_switch_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_switch_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
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

    // Render transform + flow direction + background + semantics pushed to the real widget via the shared
    // android ops. Each calls the view_platform_base body FIRST — the VM-less cross-platform suite
    // observes the headless mirror — then the shared op (itself VM-less safe) pushes to the View.
    void switch_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void switch_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void switch_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        // VisualElement.Background paints the Switch's View background via the shared android op
        // (ViewExtensions.UpdateBackground — the band behind the track). VM-less safe.
        maui::platform::android::apply_background(native, value);
    }

    void switch_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<switch_platform> switch_handler::create_platform_view()
    {
        auto platform = std::make_unique<switch_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass switch_class = cache.find_class(env.get(), k_switch_class);
        if (switch_class == nullptr)
        {
            return platform;
        }
        // SwitchHandler.CreatePlatformView: `new ASwitch(Context)`. The plain (Context) ctor of
        // android.widget.Switch resolves the `switchStyle` theme attribute against the Context's THEME —
        // which the app_process widget test host (a bare, Activity-less Context) does NOT carry, so that
        // ctor THROWS there → null widget (the same trap progress_bar_handler.cpp's header documents).
        //
        // The 3-arg (Context, AttributeSet, int defStyleAttr) ctor with defStyleAttr=0 constructs fine, but
        // with NO defStyleRes it resolves NO thumb/track drawable — so the Switch renders as an invisible,
        // drawable-less toggle (the exact missing-glyph bug the slider's thumb/track hit, fixed there with
        // defStyleRes=Widget_SeekBar). So construct THEME-INDEPENDENTLY via the 4-arg (Context, AttributeSet,
        // int defStyleAttr, int defStyleRes) ctor with defStyleAttr=0 and defStyleRes =
        // android.R.style.Widget_CompoundButton_Switch (a concrete style resource that CARRIES the thumb +
        // track drawables — read with GetStaticFieldID since it is a static field). Then fall back to the
        // 3-arg defStyleAttr=0 form, and finally the plain (Context) ctor, so the widget is never null.
        // APPCOMPAT INVERTS THE CHAIN, and getting this backwards fails SILENTLY. The order below was built
        // for a FRAMEWORK widget, whose plain (Context) ctor resolves `switchStyle` off the theme and so
        // throws on the Activity-less widget-test Context — hence "styled 4-arg first, plain last".
        // SwitchCompat has NO 4-arg (Context, AttributeSet, int, int) ctor at all, and its 3-arg form with
        // defStyleAttr=0 CONSTRUCTS FINE while resolving no style — so the old order would have skipped
        // straight past the correct ctor to a working-but-unstyled toggle, and reported success.
        //
        // MAUI's form is the plain (Context) ctor (SwitchHandler.Android.cs:15), which resolves
        // ?attr/switchStyle against the app theme — and this app host's theme already parents on
        // Theme.MaterialComponents.DayNight (apphost/res/values/styles.xml:48). So try plain FIRST here and
        // keep the styled forms only as the Activity-less fallback the widget tests need.
        jobject created = nullptr;
        if (jmethodID ctor_plain_first =
                cache.method(env.get(), k_switch_class, "<init>", "(Landroid/content/Context;)V"))
        {
            created = env->NewObject(switch_class, ctor_plain_first, context);
            if (clear_pending(env.get()))
            {
                created = nullptr; // Activity-less context (widget test host) — fall through to the pins
            }
        }
        jmethodID ctor_styled = created != nullptr
                                    ? nullptr
                                    : cache.method(env.get(), k_switch_class, "<init>",
                                                   "(Landroid/content/Context;Landroid/util/AttributeSet;II)V");
        jclass style_class = cache.find_class(env.get(), k_style_class);
        jfieldID style_field =
            style_class != nullptr ? env->GetStaticFieldID(style_class, k_switch_style_field, "I") : nullptr;
        clear_pending(env.get()); // a missing-field lookup raises NoSuchFieldError — clear it, then try the alt
        if (style_class != nullptr && style_field == nullptr)
        {
            style_field = env->GetStaticFieldID(style_class, k_switch_style_field_alt, "I");
            clear_pending(env.get());
        }
        if (ctor_styled != nullptr && style_class != nullptr && style_field != nullptr)
        {
            const jint style_res = env->GetStaticIntField(style_class, style_field);
            if (!clear_pending(env.get()))
            {
                created = env->NewObject(switch_class, ctor_styled, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0), style_res);
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_styleable = cache.method(env.get(), k_switch_class, "<init>",
                                                    "(Landroid/content/Context;Landroid/util/AttributeSet;I)V");
            if (ctor_styleable != nullptr)
            {
                created = env->NewObject(switch_class, ctor_styleable, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0));
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_plain = cache.method(env.get(), k_switch_class, "<init>", "(Landroid/content/Context;)V");
            if (ctor_plain != nullptr)
            {
                created = env->NewObject(switch_class, ctor_plain, context);
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
        // Wrap-content LayoutParams up front (parentless View measure/layout safety — the android
        // container fan-out has not arrived; the partial stands in for the parent ViewGroup attach,
        // exactly like button_handler.cpp / progress_bar_handler.cpp do).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_switch_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
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
        // NO TINT SEEDING. This used to pin hardcoded Material grays as the UNSET baseline, because a
        // framework android.widget.Switch on Theme.DeviceDefault.Light drew its checked thumb in the
        // DeviceDefault indigo. BOTH premises are now false: the widget is SwitchCompat and the host theme
        // parents Theme.MaterialComponents.DayNight (apphost/res/values/styles.xml:48), so the widget
        // resolves its own theme-correct tints — and MAUI seeds nothing at all.
        //
        // MEASURED on the post-swap captures, which is what turned this from tidy-up into a defect: the
        // seed froze the LIGHT grays into DARK (MAUI thumb 185, port 231-236), rendered IsEnabled="False"
        // identically to the enabled rows, and its opaque SRC_ATOP filter obliterated an explicit
        // BackgroundColor (MAUI track 77, port 178). Dark whole-frame diff 0.872% against light's 0.193%.
        platform->native = env->NewGlobalRef(widget.get()); // released in ~switch_platform
        return platform;
    }

    void switch_handler::on_connect_handler(switch_platform& platform)
    {
        // SwitchHandler.Android's CheckedChangeListener → OnCheckedChanged → VirtualView.IsOn write-back,
        // guarded against echo (C#: `if (VirtualView is null || VirtualView.IsOn == isOn) return`). The
        // REAL native OnCheckedChangeListener wiring (a RegisterNatives trampoline, like the shared
        // dev.mauicpp.MauiDialogBridge) is DEFERRED with the gesture/event fan-out; the callback stays wired even
        // VM-less so the cross-platform suite can drive it (it mirrors the headless partial's body).
        platform.on_value_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr && view->is_on() != platform_view->is_on)
            {
                view->set_is_on(platform_view->is_on);
            }
        };
    }

    void switch_handler::on_disconnect_handler(switch_platform& platform)
    {
        // DisconnectHandler: SetOnCheckedChangeListener(null) — the native channel is deferred (see
        // on_connect_handler), so disconnect just drops the C++ callback.
        platform.on_value_changed = nullptr;
    }

    void switch_handler::map_is_on(switch_handler& handler, i_switch& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // C# MapIsOn → UpdateIsOn(handler) re-runs the TrackColor mapper (the effective track color
        // depends on the toggle state — UpdateIsOn calls UpdateTrackColor), then SwitchExtensions.UpdateIsOn
        // pushes the native checked state. Mirror first (the headless mirror + VM-less suite).
        platform->is_on = view.is_on();
        handler.update_value("track_color");
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // SwitchExtensions.UpdateIsOn: aSwitch.Checked = view.IsOn → setChecked(Z).
            call_void_bool(env.get(), widget_of(*platform), "setChecked", static_cast<jboolean>(view.is_on()));
        }
    }

    void switch_handler::map_track_color(switch_handler& handler, i_switch& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->track_color = view.track_color(); // headless mirror first (VM-less suite)
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
        // SwitchExtensions.UpdateTrackColor (the plain-ASwitch overload): TrackDrawable?.SetColorFilter
        // (trackColor, FilterMode.SrcAtop) when set, else TrackDrawable?.ClearColorFilter(). The color is
        // non-nullable in the port, so `trackColor is not null` becomes `!= color{}` (the default-black
        // sentinel = unset). The unset branch now CLEARS the filter, exactly as C# does — it used to
        // re-apply a hardcoded Material gray to avoid the DeviceDefault indigo a framework Switch on
        // Theme.DeviceDefault.Light would show. On SwitchCompat under Theme.MaterialComponents.DayNight
        // there is no indigo to avoid, and holding an opaque filter over the track is what froze the light
        // greys into dark and swallowed IsEnabled=false and BackgroundColor (see create_platform_view).
        if (view.track_color() != maui::graphics::color{})
        {
            apply_track_filter(env.get(), widget, view.track_color());
        }
        else
        {
            clear_track_filter(env.get(), widget);
        }
    }

    void switch_handler::map_thumb_color(switch_handler& handler, i_switch& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->thumb_color = view.thumb_color(); // headless mirror first (VM-less suite)
        if (platform->native == nullptr)
        {
            return;
        }
        // SwitchExtensions.UpdateThumbColor (the plain-ASwitch overload): only sets a tint when a custom
        // color is present (C#: `if (thumbColor is not null) ThumbTintList = CreateDefault(...)`, NO else
        // branch — the default thumb keeps the theme tint). The port's color is non-nullable, so the gate
        // is `!= color{}` (the default-black sentinel = unset). PARITY: an UNSET thumb early-returns and so
        // leaves the widget's own theme-resolved thumb tint alone, matching SwitchExtensions.cs:89-99, which has
        // no else branch. It formerly kept a hardcoded gray seeded at
        // construction — that is the port's "theme default" here, matching MAUI's light thumb rather than
        // the DeviceDefault indigo. An explicit ThumbColor overrides it via the shared apply_thumb_tint
        // (ThumbColor row on the switch page verifies the orange override).
        if (view.thumb_color() == maui::graphics::color{})
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // ColorStateListExtensions.CreateDefault(color) → ColorStateList.valueOf(int) (the framework
        // equivalent of PlatformInterop.GetDefaultColorStateList — see the header deviation). setThumbTintList
        // (not SetColorFilter) preserves the thumb shadow, as the C# comment notes.
        apply_thumb_tint(env.get(), widget_of(*platform), view.thumb_color());
    }

    maui::graphics::size switch_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's UISwitch-shaped placeholder (51x31), so the
            // backend-agnostic size-request suites see consistent numbers in the pure-native run.
            return {51.0, 31.0};
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
        // as dp (Context.FromPixels). The SwitchMinWidth theme-attr fallback is not reached (header note).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_switch_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_switch_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_switch_class, "getMeasuredHeight", "()I");
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
        // DOCUMENTED DEVIATION (the standing doctrine + rule 4 (RENDER-BREAKS-TIES) — match the shipped RENDER): real MAUI renders the Switch as a
        // Material SwitchCompat (~48dp measured touch-target row), but this AAR-less host's plain
        // android.widget.Switch under Theme.DeviceDefault measures shorter, so a page of stacked switches
        // drifts up vs MAUI. Floor the measured height to the Material switch row height (the thumb/track
        // center in the taller band as MAUI does) — same fix as slider's k_material_seekbar_height_dp.
        constexpr double k_material_switch_height_dp = 48.0;
        const double height_dp = std::max(static_cast<double>(measured_height) / density, k_material_switch_height_dp);
        return {static_cast<double>(measured_width) / density, height_dp};
    }

    void switch_handler::platform_arrange(const maui::graphics::rect& frame)
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
        jmethodID measure = cache.method(env.get(), k_switch_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_switch_class, "layout", "(IIII)V");
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
