// label_handler — Android (JNI) platform partial: the display-only text control of the M-android
// per-control fan-out (button's Rosetta Stone replayed for a TextView). The managed platform view is a
// REAL android.widget.TextView (held as a JNI global reference in label_platform::native); every map_*
// pushes its property through the jni_cache'd method ids. A label has no inbound event channel, so
// there is no listener/trampoline (unlike button).
//
// Ported DIRECTLY from LabelHandler.Android.cs + Platform/Android/{TextViewExtensions.cs (UpdateText*,
// UpdateTextColor, UpdateFont, UpdateCharacterSpacing, UpdateHorizontalTextAlignment,
// UpdateVerticalTextAlignment, UpdatePadding, UpdateTextDecorations, UpdateLineHeight), ViewExtensions.cs}
// + Fonts/FontManager.Android.cs (the CreateTypeface / GetFontSize tail shared verbatim with button).
//
// DOCUMENTED DEVIATIONS from the C# oracle (each an infrastructure gap, not a behavior guess — the
// button partial documents the same set):
//   - The widget is a plain android.widget.TextView, not MauiTextView/AppCompatTextView: the AppCompat
//     library is a gradle/AAR dependency this APK-less backend does not carry. MauiTextView's HTML-text
//     and AppCompat tinting extras have no plain-widget analog and are skipped.
//   - FontManager's registrar/asset/file lookups are skipped (no font registrar yet, on any backend):
//     family → Typeface.create(family, style), then the API-28+ Typeface.create(base, weight, italic)
//     refinement — the exact CreateTypeface tail button documents.
//   - update_background pushes a SOLID fill via View.setBackgroundColor; gradient/image paints (the
//     GradientDrawable layering) are deferred. // TODO: verify against ViewExtensions.UpdateBackground.
//   - MaxLines IS pushed (TextView.setMaxLines); the LineBreakMode → Ellipsize/SingleLine resolution is
//     deferred (the headless mirror is kept live). // TODO: verify against the Android SetLineBreakMode.
//   - FormattedText (SpannableString) is deferred — the headless run mirror is kept so the cross-platform
//     suite still observes the runs. The GetDesiredSize multi-line width-narrowing refinement
//     (LabelHandler.Android.GetDesiredSize) and PrepareForTextViewArrange are deferred; the base
//     measure/arrange (button's pattern) stands in.
//
// VM-less degradation: identical to button — every JNI path checks scoped_env/app_context() and quietly
// skips, while the headless mirrors are ALWAYS maintained (the pure-native cross-platform suite on the
// emulator observes exactly the headless partial; the widget test host additionally observes the View).

#include "maui/core/label_handler.hpp"

#include <jni.h>

#include <atomic>
#include <cmath>
#include <cstddef>
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
#include "maui/core/font.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
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

    // GetMethodID walks superclasses, so the View/TextView surface resolves through the widget class.
    constexpr const char* k_text_view_class = "android/widget/TextView";
    constexpr const char* k_typeface_class = "android/graphics/Typeface";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    constexpr float k_default_font_size = 14.0F;         // FontManager.DefaultFontSize (14sp)
    constexpr float k_em_coefficient = 0.0624F;          // UnitExtensions.EmCoefficient
    constexpr double k_to_pixels_epsilon = 0.0000000001; // GeometryUtil.Epsilon (ContextExtensions.ToPixels)

    constexpr jint k_view_visible = 0; // android.view.View VISIBLE/INVISIBLE/GONE
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;
    constexpr jint k_important_for_accessibility_auto = 0;

    constexpr jint k_typeface_normal = 0; // android.graphics.Typeface styles
    constexpr jint k_typeface_bold = 1;
    constexpr jint k_typeface_italic = 2;
    constexpr jint k_complex_unit_dip = 1; // android.util.TypedValue complex units
    constexpr jint k_complex_unit_sp = 2;

    constexpr jint k_measure_spec_unspecified = 0; // android.view.View.MeasureSpec modes
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    // android.view.View text-alignment constants (TextAlignmentExtensions.ToTextAlignment targets).
    constexpr jint k_text_alignment_center = 4;
    constexpr jint k_text_alignment_view_start = 5;
    constexpr jint k_text_alignment_view_end = 6;
    // android.view.View JustificationMode (API 26): InterWord for Justify, else None.
    constexpr jint k_justification_none = 0;
    constexpr jint k_justification_inter_word = 1;
    // android.view.Gravity vertical bits (UpdateVerticalAlignment preserves the horizontal bits).
    constexpr jint k_gravity_top = 0x30;
    constexpr jint k_gravity_center_vertical = 0x10;
    constexpr jint k_gravity_bottom = 0x50;
    constexpr jint k_gravity_vertical_mask = 0x70;
    // android.graphics.Paint flags (UpdateTextDecorations toggles these).
    constexpr jint k_paint_underline = 0x08;
    constexpr jint k_paint_strike_thru = 0x10;
    // TextView's "no maximum" line count (Integer.MAX_VALUE) — MapMaxLines' unset (-1) fallback.
    constexpr jint k_max_lines_unbounded = 0x7fffffff;

    [[nodiscard]] jobject widget_of(const maui::core::label_platform& platform) noexcept
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
        if (jmethodID method = default_jni_cache().method(env, k_text_view_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_text_view_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_text_view_class, name, "(Z)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon).
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // Context display density, memoized process-wide (ContextExtensions' s_displayDensity cache). 1.0 on
    // any failure (failures are not memoized, so a transient failure does not pin the fallback).
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        static std::atomic<float> memoized{0.0F}; // 0 = not read yet (a real density is never 0)
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_text_view_class, "getContext", "()Landroid/content/Context;");
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

    // MapMaxLines → TextView.SetMaxLines (unset -1 → Integer.MAX_VALUE). The LineBreakMode resolution
    // (Ellipsize / SingleLine) is the deferred half (see header). Shared by map_line_break_mode +
    // map_max_lines, which both touch the MaxLines/LineBreakMode pair in C#.
    void push_max_lines(const maui::core::label_platform& platform, maui::core::i_label& view)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        const jint max = view.max_lines() > 0 ? view.max_lines() : k_max_lines_unbounded;
        call_void_int(env.get(), widget_of(platform), "setMaxLines", max);
        // TODO: verify against src/Core/src/Platform/Android (LabelExtensions.SetLineBreakMode) — the
        // Ellipsize / SingleLine half of LineBreakMode is not yet pushed.
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.widget.TextView (the JNI shape of the
    // pimpl-owned-native-view doctrine; the ios twin CFReleases its UILabel here).
    label_platform::~label_platform()
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

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each calls
    // the base body FIRST — the headless mirrors must stay live for the VM-less cross-platform suite —
    // then pushes to the real widget when one exists. Mirrors button_platform's set exactly.

    void label_platform::update_visibility(maui::core::visibility value)
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

    void label_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            call_void_float(env.get(), widget_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void label_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            call_void_bool(env.get(), widget_of(*this), "setEnabled", static_cast<jboolean>(value));
        }
    }

    void label_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
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
        // ImportantForAccessibility to YES; restore AUTO when that is what the view had.
        jmethodID get_important = cache.method(env.get(), k_text_view_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_text_view_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
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

    void label_platform::update_background(const maui::graphics::paint* value)
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
        // ViewExtensions.UpdateBackground: a solid paint paints a flat fill (ColorDrawable). The
        // gradient/image paints + GradientDrawable layering are deferred (header). A null paint clears to
        // transparent.
        const jint argb = value != nullptr ? static_cast<jint>(value->background_color().to_int()) : 0;
        call_void_int(env.get(), widget_of(*this), "setBackgroundColor", argb);
    }

    void label_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void label_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void label_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    std::unique_ptr<label_platform> label_handler::create_platform_view()
    {
        auto platform = std::make_unique<label_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass text_view_class = cache.find_class(env.get(), k_text_view_class);
        jmethodID ctor = cache.method(env.get(), k_text_view_class, "<init>", "(Landroid/content/Context;)V");
        if (text_view_class == nullptr || ctor == nullptr)
        {
            return platform;
        }
        // LabelHandler.CreatePlatformView: new MauiTextView(Context) (plain TextView here — header).
        const local_ref<jobject> widget{env.get(), env->NewObject(text_view_class, ctor, context)};
        if (clear_pending(env.get()) || !widget)
        {
            return platform;
        }
        // Wrap-content LayoutParams up front: a parentless TextView with null LayoutParams NPEs in
        // checkForRelayout on any setText after the first measure (button documents the same guard).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_text_view_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
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
        platform->native = env->NewGlobalRef(widget.get()); // released in ~label_platform
        return platform;
    }

    void label_handler::map_text(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text = std::string(view.text());
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // TextViewExtensions.UpdateTextPlainText: textView.Text = label.Text.
        jmethodID set_text =
            default_jni_cache().method(env.get(), k_text_view_class, "setText", "(Ljava/lang/CharSequence;)V");
        if (set_text != nullptr)
        {
            const local_ref<jstring> text = to_jstring(env.get(), view.text());
            env->CallVoidMethod(widget_of(*platform), set_text, text.get());
            clear_pending(env.get());
        }
    }

    void label_handler::map_text_color(label_handler& handler, i_label& view)
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
            // TextViewExtensions.UpdateTextColor: SetTextColor(textColor.ToPlatform()). C#'s null guard
            // collapses (non-nullable color), exactly as in the button/apple/ios partials.
            call_void_int(env.get(), widget_of(*platform), "setTextColor",
                          static_cast<jint>(view.text_color().to_int()));
        }
    }

    void label_handler::map_font(label_handler& handler, i_label& view)
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

        // FontManager.GetTypeface → CreateTypeface (the non-registered-family tail; header): base =
        // Typeface.create(family, ToTypefaceStyle(weight, italic)), then the API-28+ refinement
        // Typeface.create(base, weight, italic). Identical to the button partial.
        const bool italic = value.slant() != font_slant::normal;
        const bool bold = value.weight() >= font_weight::bold;
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
            cache.method(env.get(), k_text_view_class, "setTypeface", "(Landroid/graphics/Typeface;)V");
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

        // FontManager.GetFontSize: size ≤ 0 / NaN → DefaultFontSize; AutoScalingEnabled picks Sp vs Dip.
        auto size = static_cast<float>(value.size());
        if (!(size > 0) || std::isnan(size))
        {
            size = k_default_font_size;
        }
        const jint unit = value.auto_scaling_enabled() ? k_complex_unit_sp : k_complex_unit_dip;
        jmethodID set_text_size = cache.method(env.get(), k_text_view_class, "setTextSize", "(IF)V");
        if (set_text_size != nullptr)
        {
            env->CallVoidMethod(widget, set_text_size, unit, static_cast<jfloat>(size));
            clear_pending(env.get());
        }
    }

    void label_handler::map_horizontal_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment();
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
        // TextViewExtensions.UpdateHorizontalTextAlignment (Rtl supported on min API): View.TextAlignment =
        // ToTextAlignment(h); Center→CENTER, End→VIEW_END, else VIEW_START.
        jint alignment = k_text_alignment_view_start;
        if (view.horizontal_text_alignment() == text_alignment::center)
        {
            alignment = k_text_alignment_center;
        }
        else if (view.horizontal_text_alignment() == text_alignment::end)
        {
            alignment = k_text_alignment_view_end;
        }
        call_void_int(env.get(), widget, "setTextAlignment", alignment);
        // API 26+: JustificationMode = InterWord for Justify, else None.
        const jint justification = view.horizontal_text_alignment() == text_alignment::justify
                                       ? k_justification_inter_word
                                       : k_justification_none;
        call_void_int(env.get(), widget, "setJustificationMode", justification);
    }

    void label_handler::map_vertical_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->vertical_alignment = view.vertical_text_alignment();
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
        // ViewExtensions.UpdateVerticalAlignment: Gravity = (Gravity & ~VERTICAL_MASK) | verticalBits;
        // Start→Top, Center→CenterVertical, End→Bottom (the horizontal bits, set by TextAlignment, survive).
        jmethodID get_gravity = cache.method(env.get(), k_text_view_class, "getGravity", "()I");
        jmethodID set_gravity = cache.method(env.get(), k_text_view_class, "setGravity", "(I)V");
        if (get_gravity == nullptr || set_gravity == nullptr)
        {
            return;
        }
        const jint current = env->CallIntMethod(widget, get_gravity);
        if (clear_pending(env.get()))
        {
            return;
        }
        jint vertical = k_gravity_top;
        if (view.vertical_text_alignment() == text_alignment::center)
        {
            vertical = k_gravity_center_vertical;
        }
        else if (view.vertical_text_alignment() == text_alignment::end)
        {
            vertical = k_gravity_bottom;
        }
        env->CallVoidMethod(widget, set_gravity, (current & ~k_gravity_vertical_mask) | vertical);
        clear_pending(env.get());
    }

    void label_handler::map_character_spacing(label_handler& handler, i_label& view)
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

    void label_handler::map_text_decorations(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->decorations = view.text_decorations();
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
        // TextViewExtensions.UpdateTextDecorations: toggle the StrikeThru / Underline PaintFlags bits.
        jmethodID get_paint_flags = cache.method(env.get(), k_text_view_class, "getPaintFlags", "()I");
        jmethodID set_paint_flags = cache.method(env.get(), k_text_view_class, "setPaintFlags", "(I)V");
        if (get_paint_flags == nullptr || set_paint_flags == nullptr)
        {
            return;
        }
        const jint current = env->CallIntMethod(widget, get_paint_flags);
        if (clear_pending(env.get()))
        {
            return;
        }
        const auto decorations = static_cast<std::uint8_t>(view.text_decorations());
        jint flags = current;
        if ((decorations & static_cast<std::uint8_t>(text_decorations::strikethrough)) == 0)
        {
            flags &= ~k_paint_strike_thru;
        }
        else
        {
            flags |= k_paint_strike_thru;
        }
        if ((decorations & static_cast<std::uint8_t>(text_decorations::underline)) == 0)
        {
            flags &= ~k_paint_underline;
        }
        else
        {
            flags |= k_paint_underline;
        }
        env->CallVoidMethod(widget, set_paint_flags, flags);
        clear_pending(env.get());
    }

    void label_handler::map_line_height(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->line_height = view.line_height();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env || !(view.line_height() >= 0))
        {
            return; // TextViewExtensions.UpdateLineHeight gates on LineHeight >= 0
        }
        jmethodID set_line_spacing =
            default_jni_cache().method(env.get(), k_text_view_class, "setLineSpacing", "(FF)V");
        if (set_line_spacing != nullptr)
        {
            // SetLineSpacing(add: 0, mult: LineHeight).
            env->CallVoidMethod(widget_of(*platform), set_line_spacing, 0.0F, static_cast<jfloat>(view.line_height()));
            clear_pending(env.get());
        }
    }

    void label_handler::map_padding(label_handler& handler, i_label& view)
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
        // TextViewExtensions.UpdatePadding: SetPaddingRelative(left, top, right, bottom) in pixels.
        const maui::core::thickness padding = view.padding();
        const float density = display_density(env.get(), widget);
        jmethodID set_padding =
            default_jni_cache().method(env.get(), k_text_view_class, "setPaddingRelative", "(IIII)V");
        if (set_padding != nullptr)
        {
            env->CallVoidMethod(widget, set_padding, to_pixels(padding.left, density), to_pixels(padding.top, density),
                                to_pixels(padding.right, density), to_pixels(padding.bottom, density));
            clear_pending(env.get());
        }
    }

    void label_handler::map_formatted_text(label_handler& handler, i_label& view)
    {
        // Keep the headless run mirror live; the SpannableString build is deferred (header). The plain
        // text mirror remains for the Text path.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->formatted_text_runs = view.formatted_text_runs();
            // TODO: verify against LabelExtensions (Android) — build a SpannableString from the runs.
        }
    }

    void label_handler::map_line_break_mode(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->line_break_mode_value = view.line_break_mode();
        platform->max_lines = view.max_lines();
        push_max_lines(*platform, view);
    }

    void label_handler::map_max_lines(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->line_break_mode_value = view.line_break_mode();
        platform->max_lines = view.max_lines();
        push_max_lines(*platform, view);
    }

    maui::graphics::size label_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // VM-less degradation: the headless partial's placeholder metric (~7pt/char, 16pt line) so
            // the backend-agnostic size-request suites see consistent numbers in the pure-native run.
            return {static_cast<double>(platform->text.size()) * 7.0, 16.0};
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandlerExtensions.GetDesiredSizeFromHandler: finite constraints → AtMost specs (px),
        // infinite → Unspecified; View.measure, then measured px back to dp. (The multi-line
        // width-narrowing refinement in LabelHandler.Android.GetDesiredSize is deferred — header.)
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_text_view_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_text_view_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_text_view_class, "getMeasuredHeight", "()I");
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

    void label_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, measure Exactly, then layout. (The
        // PrepareForTextViewArrange vertical-gravity adjustment is deferred — header.)
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_text_view_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_text_view_class, "layout", "(IIII)V");
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
