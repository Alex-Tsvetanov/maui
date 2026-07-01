// check_box_handler — Android (JNI) platform partial, the M-android per-control fan-out replayed for
// the binary-choice check box. The managed platform view is a REAL android.widget.CheckBox (held as a
// JNI global reference in check_box_platform::native): IsChecked maps onto setChecked(bool) (isChecked
// read-back), Foreground onto setButtonTintList (ColorStateList.valueOf(argb)) + setButtonTintMode
// (PorterDuff.Mode.SRC_IN), and Background onto the shared android background op. The native
// CheckedChange channel is DEFERRED — the C++ on_checked_changed callback stays invokable so the
// VM-less cross-platform suite drives it, but no real OnCheckedChangeListener is installed (see below).
//
// Ported DIRECTLY from CheckBoxHandler.Android.cs + Platform/Android/{CheckBoxExtensions.cs,
// ColorStateListExtensions.cs (CreateCheckBox), ViewExtensions.cs (UpdateVisibility/UpdateOpacity/
// UpdateIsEnabled/UpdateAutomationId/UpdateBackground), ContextExtensions.cs (ToPixels)}.
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each a Material-library or infrastructure gap, not a
// behavior guess):
//   - The widget is a plain android.widget.CheckBox, not Google.Android.Material's MaterialCheckBox
//     (the C# CreatePlatformView's `new MaterialCheckBox(MauiMaterialContextThemeWrapper.Create(Context))`):
//     the Material Components library is a gradle/AAR dependency this APK-less backend does not carry —
//     exactly the plain-widget deviation button_handler.cpp documents for the MauiMaterialButton. A plain
//     CheckBox is a CompoundButton, so it carries the same checkbox toggle, setChecked/isChecked, and the
//     CompoundButton setButtonTintList/setButtonTintMode that CheckBoxExtensions.UpdateForeground drives
//     through CompoundButtonCompat (the AppCompat shim is unnecessary on the plain widget — the methods
//     are on the View directly). C#'s SoundEffectsEnabled = false is ported (a plain View property);
//     SetClipToOutline(true) is ported (also a plain View property).
//   - CheckBoxExtensions.UpdateForeground builds the tint with ColorStateListExtensions.CreateCheckBox
//     (a FOUR-state ColorStateList: enabledChecked/enabledUnchecked/disabledChecked/disabledUnchecked).
//     For a developer-set SolidPaint that single overload is CreateCheckBox(all,all,all,all) — every
//     state the SAME color — which collapses exactly to ColorStateList.valueOf(argb), the single-color
//     CSL the progress_bar tint deviation already documents. So map_foreground applies valueOf(argb)
//     whenever the control carries an explicit Color (foreground() != null, the Color?.AsPaint() solid).
//     When Color is UNSET, foreground() is null and CheckBoxExtensions.GetColorStateList would return the
//     widget's ORIGINAL THEME buttonTint (Material3) / the theme accent (Material2) — a THEME-resolved
//     color the port cannot get from this AAR-less host without a mismatch: the app-host framework theme
//     is Theme.DeviceDefault.Light (navy accent), real MAUI's is Theme.MaterialComponents.DayNight (gray
//     accent). So create_platform_view SEEDS the unset baseline itself with valueOf(#E0E0E0) — the Material
//     gray real MAUI resolves — via seed_default_material_button_tint, and map_foreground's unset branch
//     leaves that seed untouched (returns on null). This is the CheckBox twin of the switch/radio/slider
//     theme-accent parity fix (commits 3bec64c267 / bbb632f301). See the k_material_check_box_gray note.
//   - The CheckedChange listener (CompoundButton.OnCheckedChangeListener → OnCheckedChange:
//     VirtualView.IsChecked = e.IsChecked) is DEFERRED with the gesture/event fan-out: there is no
//     host-provided listener Java class for it (the test host ships only dev.mauicpp.NativeOnClickListener
//     for the button). on_checked_changed stays a wired, invokable C++ callback carrying OnCheckedChange's
//     body — the cross-platform suite drives it directly, exactly as the headless mirror does — but the
//     real widget's setOnCheckedChangeListener is not installed. A programmatic setChecked still reaches
//     the widget (virtual→native) and reads back through isChecked.
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the emulator
// where no Java VM exists. Every JNI path here checks scoped_env/app_context() and quietly skips, while
// the headless mirrors (is_checked/foreground + the base IView mirrors) are ALWAYS maintained — so that
// suite observes exactly the headless partial's behavior, and the widget test host additionally observes
// the real widget.

#include "maui/core/check_box_handler.hpp"

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
#include "maui/core/i_check_box.hpp"
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
    // superclasses, so the View/CompoundButton surface resolves through android/widget/CheckBox too).
    constexpr const char* k_check_box_class = "android/widget/CheckBox";
    constexpr const char* k_style_class = "android/R$style";
    // android.R.style.Widget_CompoundButton_CheckBox — the concrete platform style that carries the
    // checkbox buttonDrawable (the checkmark-in-box glyph). Resolved theme-independently as a defStyleRes
    // so the bare app_process testhost (and the app host) construct a checkbox that actually HAS its glyph
    // (defStyleAttr=0 with no defStyleRes resolves no buttonDrawable → an invisible, drawable-less box).
    constexpr const char* k_check_box_style_field = "Widget_CompoundButton_CheckBox";
    constexpr const char* k_color_state_list_class = "android/content/res/ColorStateList";
    constexpr const char* k_porter_duff_mode_class = "android/graphics/PorterDuff$Mode";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // The Material light-theme CheckBox default buttonDrawable color (PARITY, the CheckBox twin of the
    // slider's seed_default_material_tints fix commit bbb632f301, and the exact analog of
    // radio_button_handler.cpp's seed_default_material_button_tint). CheckBoxExtensions.UpdateForeground
    // NEVER tints the buttonDrawable when Color is unset — GetColorStateList returns the widget's ORIGINAL
    // theme buttonTint (Material3 branch) or the theme accent (Material2) — so the checkmark/box color comes
    // ENTIRELY from the theme: real .NET MAUI renders an AppCompatCheckBox under
    // Theme.MaterialComponents.DayNight, whose buttonDrawable resolves through colorControlActivated (the
    // checked box fill) / colorControlNormal (the unchecked outline) to the SAME Material light gray
    // #E0E0E0 — measured off the maui-compare baselines: the CHECKED filled box (docs/comparison/android/
    // maui/controls_stack.png + entry.png) and the UNCHECKED outline box (check_box.png "Default") both
    // sample (224,224,224). This AAR-less app host uses the framework Theme.DeviceDefault.Light, whose
    // colorControlActivated is the DeviceDefault dark INDIGO — so a bare CheckBox draws its checked box in
    // dark navy (#495D92 measured on controls_stack/entry), NOT MAUI's neutral gray. That accent mismatch
    // is the sole cause of the "checkbox navy vs MAUI light Material gray" parity diff (controls_stack, and
    // the enabled+checked entry checkbox — which only LOOKS washed-out because Material's neutral gray box
    // is that pale, not because it is disabled: ~/maui-compare/Pages/EntryPage.cs is `new CheckBox {
    // IsChecked = true }`, enabled, unset color, exactly like entry_page.hpp). Seeding a single-color
    // ColorStateList.valueOf(#E0E0E0) on setButtonTintList reproduces MAUI's RENDERED default without any
    // semantic deviation — MAUI leaves the native default *because that default is already this gray*; the
    // port pins the equivalent gray because its host theme's default is not. An explicit CheckBox.Color
    // still OVERRIDES it: map_foreground re-tints with valueOf(argb) whenever foreground() is non-null (the
    // Colored/Disabled-Colored/Change-IsChecked purple+red boxes on check_box.png verify this).
    constexpr int k_material_check_box_gray = 0xE0; // #E0E0E0 — MAUI default box/checkmark tint (measured)

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

    [[nodiscard]] jobject widget_of(const maui::core::check_box_platform& platform) noexcept
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
        if (jmethodID method = default_jni_cache().method(env, k_check_box_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_check_box_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_check_box_class, name, "(Z)V"))
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
        jmethodID get_context = cache.method(env, k_check_box_class, "getContext", "()Landroid/content/Context;");
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

    // PorterDuff.Mode.SRC_IN — a STATIC enum field. The jni_cache's field() is GetFieldID (instance) and
    // returns null for a static, so resolve it directly with GetStaticFieldID + GetStaticObjectField
    // (the same lesson the progress_bar style-resource read records). Returned as a local ref ({} on any
    // failure), the caller owns its lifetime.
    [[nodiscard]] local_ref<jobject> porter_duff_src_in(JNIEnv* env)
    {
        jclass mode_class = default_jni_cache().find_class(env, k_porter_duff_mode_class);
        if (mode_class == nullptr)
        {
            return {};
        }
        jfieldID src_in_field = env->GetStaticFieldID(mode_class, "SRC_IN", "Landroid/graphics/PorterDuff$Mode;");
        if (clear_pending(env) || src_in_field == nullptr)
        {
            return {};
        }
        local_ref<jobject> mode{env, env->GetStaticObjectField(mode_class, src_in_field)};
        if (clear_pending(env))
        {
            return {};
        }
        return mode;
    }

    // Seed the just-created CheckBox's buttonDrawable (the box + checkmark glyph) with the Material light
    // gray #E0E0E0 via CompoundButton.setButtonTintList(ColorStateList.valueOf) + setButtonTintMode(SrcIn),
    // so its UNSET glyph matches real MAUI instead of the DeviceDefault navy accent (see the
    // k_material_check_box_gray note). A single-color valueOf CSL is the right shape here: MAUI's checked box
    // fill and unchecked outline BOTH measure #E0E0E0, so one color covers every state (checked/unchecked/
    // disabled), exactly the collapse CheckBoxExtensions.CreateCheckBox(all) already documents. The CheckBox
    // is a CompoundButton, so setButtonTintList/setButtonTintMode are on the widget directly (no
    // CompoundButtonCompat shim — the same plain-widget path map_foreground takes). Called once at
    // construction; an explicit CheckBox.Color re-tints via map_foreground's valueOf(argb) (foreground() !=
    // null), OVERRIDING this seed — the seed is only the unset baseline. Best-effort: any JNI resolution
    // failure leaves the theme default (the widget still renders — only the accent-vs-gray tint remains).
    void seed_default_material_button_tint(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        jmethodID value_of =
            cache.static_method(env, k_color_state_list_class, "valueOf", "(I)Landroid/content/res/ColorStateList;");
        jmethodID set_button_tint =
            cache.method(env, k_check_box_class, "setButtonTintList", "(Landroid/content/res/ColorStateList;)V");
        jmethodID set_button_tint_mode =
            cache.method(env, k_check_box_class, "setButtonTintMode", "(Landroid/graphics/PorterDuff$Mode;)V");
        jclass color_state_list_class = cache.find_class(env, k_color_state_list_class);
        if (value_of == nullptr || set_button_tint == nullptr || set_button_tint_mode == nullptr ||
            color_state_list_class == nullptr)
        {
            return;
        }
        const auto gray =
            static_cast<jint>(maui::graphics::color::from_rgb(k_material_check_box_gray, k_material_check_box_gray,
                                                              k_material_check_box_gray)
                                  .to_int());
        const local_ref<jobject> tint_list{env, env->CallStaticObjectMethod(color_state_list_class, value_of, gray)};
        if (clear_pending(env) || !tint_list)
        {
            return;
        }
        env->CallVoidMethod(widget, set_button_tint, tint_list.get());
        if (clear_pending(env))
        {
            return;
        }
        const local_ref<jobject> src_in = porter_duff_src_in(env);
        if (!src_in)
        {
            return;
        }
        env->CallVoidMethod(widget, set_button_tint_mode, src_in.get());
        clear_pending(env);
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.CheckBox (the JNI shape of the
    // pimpl-owned-native-view doctrine: the ios twin CFReleases its MauiCheckBox here).
    check_box_platform::~check_box_platform()
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

    void check_box_platform::update_visibility(maui::core::visibility value)
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

    void check_box_platform::update_opacity(double value)
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

    void check_box_platform::update_is_enabled(bool value)
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

    void check_box_platform::update_automation_id(std::string_view value)
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
        jmethodID get_important = cache.method(env.get(), k_check_box_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_check_box_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
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
    void check_box_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void check_box_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void check_box_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        // CheckBoxHandler.Android's MapBackground (the MONOANDROID-only mapping) → CheckBoxExtensions
        // .UpdateBackground: a null/empty paint sets a transparent background, else the standard
        // ViewExtensions.UpdateBackground. The shared android background op carries exactly that
        // null-vs-paint handling (the JNI twin of ViewExtensions.UpdateBackground). VM-less safe.
        maui::platform::android::apply_background(native, value);
    }

    void check_box_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<check_box_platform> check_box_handler::create_platform_view()
    {
        auto platform = std::make_unique<check_box_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass check_box_class = cache.find_class(env.get(), k_check_box_class);
        if (check_box_class == nullptr)
        {
            return platform;
        }
        // CheckBoxHandler.CreatePlatformView wants `new MaterialCheckBox(...)` — a plain CheckBox here
        // (Material deviation). `new CheckBox(Context)` resolves the `checkboxStyle` theme attr against
        // the Context's THEME, which the app_process widget test host (a bare, Activity-less Context)
        // does not carry, so that ctor may THROW → null widget (the agent flagged this; same shape as
        // progress_bar's styled-ctor throw).
        //
        // The 3-arg (Context, AttributeSet, int defStyleAttr) ctor with defStyleAttr=0 constructs fine on
        // the bare Context, but with NO defStyleRes it resolves NO buttonDrawable — so the checkbox renders
        // its label but the checkmark-in-box GLYPH is INVISIBLE (the exact missing-glyph bug the slider's
        // thumb/track hit, fixed there with defStyleRes=Widget_SeekBar). So construct THEME-INDEPENDENTLY
        // via the 4-arg (Context, AttributeSet, int defStyleAttr, int defStyleRes) ctor with defStyleAttr=0
        // and defStyleRes = android.R.style.Widget_CompoundButton_CheckBox (a concrete style resource that
        // CARRIES the buttonDrawable — read with GetStaticFieldID since it is a static field). Then fall
        // back to the 3-arg defStyleAttr=0 form, and finally the plain (Context) ctor, so the widget is
        // never null.
        jobject created = nullptr;
        jmethodID ctor_styled = cache.method(env.get(), k_check_box_class, "<init>",
                                             "(Landroid/content/Context;Landroid/util/AttributeSet;II)V");
        jclass style_class = cache.find_class(env.get(), k_style_class);
        jfieldID style_field =
            style_class != nullptr ? env->GetStaticFieldID(style_class, k_check_box_style_field, "I") : nullptr;
        clear_pending(env.get());
        if (ctor_styled != nullptr && style_class != nullptr && style_field != nullptr)
        {
            const jint style_res = env->GetStaticIntField(style_class, style_field);
            if (!clear_pending(env.get()))
            {
                created = env->NewObject(check_box_class, ctor_styled, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0), style_res);
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_attr = cache.method(env.get(), k_check_box_class, "<init>",
                                               "(Landroid/content/Context;Landroid/util/AttributeSet;I)V");
            if (ctor_attr != nullptr)
            {
                created = env->NewObject(check_box_class, ctor_attr, context, static_cast<jobject>(nullptr),
                                         static_cast<jint>(0));
                if (clear_pending(env.get()))
                {
                    created = nullptr;
                }
            }
        }
        if (created == nullptr)
        {
            jmethodID ctor_plain = cache.method(env.get(), k_check_box_class, "<init>", "(Landroid/content/Context;)V");
            if (ctor_plain != nullptr)
            {
                created = env->NewObject(check_box_class, ctor_plain, context);
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
        // CreatePlatformView's object-initializer { SoundEffectsEnabled = false } + SetClipToOutline(true)
        // (both plain-View properties — the Material wrapper is the only deviation).
        call_void_bool(env.get(), widget.get(), "setSoundEffectsEnabled", JNI_FALSE);
        call_void_bool(env.get(), widget.get(), "setClipToOutline", JNI_TRUE);
        // Seed the UNSET buttonDrawable tint to MAUI's Material gray #E0E0E0 (not the DeviceDefault navy
        // accent this host theme resolves) so a bare CheckBox matches real MAUI; an explicit CheckBox.Color
        // still overrides via map_foreground (see the k_material_check_box_gray + seed note). The radio /
        // switch / slider twin of the same theme-accent parity fix.
        seed_default_material_button_tint(env.get(), widget.get());
        // Wrap-content LayoutParams up front (parentless View measure/layout safety — the android
        // container fan-out has not arrived; the partial stands in for the parent ViewGroup attach,
        // exactly like button_handler.cpp / progress_bar_handler.cpp do).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_check_box_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
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
        platform->native = env->NewGlobalRef(widget.get()); // released in ~check_box_platform
        return platform;
    }

    void check_box_handler::on_connect_handler(check_box_platform& platform)
    {
        // CheckBoxHandler.OnCheckedChange: write the native checked state back to the virtual view. The
        // callback stays wired even VM-less so the cross-platform suite (and the headless mirror) can
        // drive it; the real CompoundButton.OnCheckedChangeListener install is DEFERRED with the event
        // fan-out (no host-provided listener Java class exists for it — header deviations).
        platform.on_checked_changed = [this] {
            auto* view = virtual_view();
            auto* platform_view = typed_platform_view();
            if (view != nullptr && platform_view != nullptr && view->is_checked() != platform_view->is_checked)
            {
                view->send_is_checked(platform_view->is_checked);
            }
        };
    }

    void check_box_handler::on_disconnect_handler(check_box_platform& platform)
    {
        platform.on_checked_changed = nullptr;
    }

    void check_box_handler::map_is_checked(check_box_handler& handler, i_check_box& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_checked = view.is_checked(); // headless mirror first (VM-less suite) + the read-back state
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // CheckBoxExtensions.UpdateIsChecked: platformCheckBox.Checked = check.IsChecked.
            call_void_bool(env.get(), widget_of(*platform), "setChecked", static_cast<jboolean>(view.is_checked()));
        }
    }

    void check_box_handler::map_foreground(check_box_handler& handler, i_check_box& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->foreground = view.foreground(); // headless mirror first (VM-less suite)
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
        // CheckBoxExtensions.UpdateForeground → GetColorStateList: a developer SolidPaint maps to
        // ColorStateListExtensions.CreateCheckBox(tint) = a four-state CSL all equal to `tint`, which
        // collapses to ColorStateList.valueOf(argb) (the single-color CSL — see header deviations). The
        // port's non-nullable color means the Material3-theme / accent-color fallbacks are unreachable.
        // The plain CheckBox is a CompoundButton, so SetButtonTintList/SetButtonTintMode are on the View
        // directly (no CompoundButtonCompat shim). Mode is PorterDuff.Mode.SrcIn.
        const maui::graphics::paint* foreground = view.foreground();
        if (foreground == nullptr)
        {
            // Unset Foreground: leave the create_platform_view seed (the Material gray #E0E0E0 baseline,
            // seed_default_material_button_tint) in place — MAUI's unset checkbox draws that theme gray, not
            // the DeviceDefault navy this host would otherwise resolve. Re-tinting here is unnecessary (and
            // must not clear the seed). The headless mirror above is enough for the VM-less suite.
            return;
        }
        jmethodID value_of = cache.static_method(env.get(), k_color_state_list_class, "valueOf",
                                                 "(I)Landroid/content/res/ColorStateList;");
        jmethodID set_button_tint =
            cache.method(env.get(), k_check_box_class, "setButtonTintList", "(Landroid/content/res/ColorStateList;)V");
        jmethodID set_button_tint_mode =
            cache.method(env.get(), k_check_box_class, "setButtonTintMode", "(Landroid/graphics/PorterDuff$Mode;)V");
        jclass color_state_list_class = cache.find_class(env.get(), k_color_state_list_class);
        if (value_of == nullptr || set_button_tint == nullptr || set_button_tint_mode == nullptr ||
            color_state_list_class == nullptr)
        {
            return;
        }
        const auto argb = static_cast<jint>(foreground->background_color().to_int());
        const local_ref<jobject> tint_list{env.get(),
                                           env->CallStaticObjectMethod(color_state_list_class, value_of, argb)};
        if (clear_pending(env.get()) || !tint_list)
        {
            return;
        }
        env->CallVoidMethod(widget, set_button_tint, tint_list.get());
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jobject> src_in = porter_duff_src_in(env.get());
        if (!src_in)
        {
            return;
        }
        env->CallVoidMethod(widget, set_button_tint_mode, src_in.get());
        clear_pending(env.get());
    }

    maui::graphics::size check_box_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric (the iOS MinimumSize
            // square), so the backend-agnostic size-request suites see consistent numbers.
            return {44.0, 44.0};
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
        jmethodID measure = cache.method(env.get(), k_check_box_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_check_box_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_check_box_class, "getMeasuredHeight", "()I");
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

    void check_box_handler::platform_arrange(const maui::graphics::rect& frame)
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
        jmethodID measure = cache.method(env.get(), k_check_box_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_check_box_class, "layout", "(IIII)V");
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
