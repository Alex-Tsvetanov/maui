// image_button_handler — Android (JNI) platform partial (wave 19): a tappable image, built as a
// dev.mauicpp.MauiImageView (the same android.widget.ImageView subclass the image_handler decodes onto)
// with a button-style chrome background. The image rides setImageBitmap + setScaleType (the image_handler
// decode/aspect path verbatim); the chrome — BackgroundColor/brush + BorderColor/Width + CornerRadius —
// rides ONE shared android.graphics.drawable.GradientDrawable installed as the view's background (the
// button_handler chrome recipe: setColor + setStroke + setCornerRadius live together on one drawable);
// Padding rides setPadding; and a native click flows back through the dev.mauicpp.NativeOnClickListener
// trampoline (RegisterNatives — reflection-free) into image_button_platform::on_click → send_clicked.
//
// Ported from ImageButtonHandler.Android.cs (CreatePlatformView = AppCompatImageView; the touch proxy →
// Pressed/Released/Clicked; SetImageSource; MapPadding/MapStrokeColor/MapStrokeThickness/MapCornerRadius)
// + the shared Platform/Android extensions (ImageViewExtensions.UpdateAspect, ButtonExtensions stroke,
// ContextExtensions.ToPixels), mirroring the structure of src/platform/android/{image_handler.cpp,
// button_handler.cpp} (the two foundations this partial reuses) and the apple/ios image_button twins.
//
// DOCUMENTED DEVIATIONS from the C# oracle (each an infrastructure gap, not a behavior guess — they are
// exactly the deviations the android image + button partials already document, reused here):
//
//   - The widget is dev.mauicpp.MauiImageView (extends the stock android.widget.ImageView), not the
//     AppCompatImageView ImageButtonHandler.Android.cs creates: AndroidX.AppCompat is a gradle/AAR
//     dependency this APK-less backend does not carry (the image partial's exact deviation). SetScaleType
//     / SetAdjustViewBounds / setImageBitmap / the View surface all resolve through the subclass.
//
//   - The chrome (background fill + stroke + corner radius) lands in ONE GradientDrawable installed as the
//     ImageView's background — the plain-widget stand-in for C#'s MauiMaterialButton.StrokeColor/Width/
//     CornerRadius over a RippleDrawable (the button partial's deviation). C# routes ImageButton's stroke
//     through the SAME ButtonExtensions.UpdateButtonStroke the Button uses; this cut mirrors that, writing
//     setStroke + setCornerRadius + setColor onto one drawable. The drawable is installed LAZILY (only once
//     a fill, a visible stroke, or a positive corner radius appears) so an untouched ImageButton keeps its
//     default (transparent) background. The page's green/purple-filled buttons become visible through this
//     fill exactly as on iOS — cog.png is SVG-only and never rasterizes, so on both platforms the chrome
//     IS the visible rectangle; only dotnet_bot.png (the Custom-Size button) shows a real decoded bitmap.
//
//   - FILE sources DECODE (assets/<name> → disk via BitmapFactory, the image_handler path); uri/font/stream
//     sources stay a mirror (no Glide AAR / android image-source services on this backend — the image
//     partial's deferral). So the gallery's "Use Online Source" remote URI swap and the animated-GIF frame
//     animation are OUT OF SCOPE this wave (BitmapFactory decodes a GIF's first frame only; the cycling is
//     a separate IsAnimationPlaying concern ImageButton pins false). The static FileSource bot image renders.
//
//   - PlatformArrange's AspectFill center-crop ClipBounds (PlatformInterop.IsImageViewCenterCrop) is
//     deferred with the image partial (SetClipBounds is a PlatformInterop helper this java/ does not carry).
//     The OnTouch Pressed/Released pair is deferred with the button partial's gesture fan-out — on_press/
//     on_release stay invokable C++ callbacks (the cross-platform suite drives them) and the real
//     OnClickListener carries Clicked.
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the emulator
// (tools/android-emu-run.sh) where no Java VM exists. Every JNI path here checks scoped_env / app_context()
// and quietly skips, while the headless mirrors (image_aspect / source_* / padding / stroke / corner +
// the on_* hooks) are ALWAYS maintained — so that suite observes exactly the headless partial's behavior,
// and the widget test host / the gallery app host additionally observe the real MauiImageView.

#include "maui/core/image_button_handler.hpp"

#include <jni.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/i_image_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp" // image_bytes
#include "maui/core/image_source_loader.hpp"   // configure_loader parameter type
#include "maui/core/image_source_result.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/uri_bytes.hpp" // read_uri_bytes (disk fallback)
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    // The native widget is dev.mauicpp.MauiImageView (an android.widget.ImageView subclass — the image
    // partial's widget; reused here so the decode/scale-type/measure surface resolves identically). Because
    // it extends ImageView (which extends View), the chrome (setBackground/getBackground) and the click
    // (setOnClickListener) surface resolve through GetMethodID's superclass walk too.
    constexpr const char* k_image_view_class = "dev/mauicpp/MauiImageView";
    constexpr const char* k_scale_type_class = "android/widget/ImageView$ScaleType";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";
    constexpr const char* k_bitmap_factory_class = "android/graphics/BitmapFactory";
    constexpr const char* k_bitmap_class = "android/graphics/Bitmap";
    constexpr const char* k_gradient_drawable_class = "android/graphics/drawable/GradientDrawable";
    constexpr const char* k_click_listener_class = "dev/mauicpp/NativeOnClickListener";

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_AUTO (restored after setContentDescription auto-flips).
    constexpr jint k_important_for_accessibility_auto = 0;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject widget_of(const maui::core::image_button_platform& platform) noexcept
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
        if (jmethodID method = default_jni_cache().method(env, k_image_view_class, name, "(I)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_image_view_class, name, "(F)V"))
        {
            env->CallVoidMethod(widget, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject widget, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_image_view_class, name, "(Z)V"))
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

    // The widget's display density (Context.getResources().getDisplayMetrics().density). Memoized
    // process-wide after the first successful read, exactly like ContextExtensions' s_displayDensity cache
    // (the JNI walk is four calls). 1.0 when any step fails (failures are NOT memoized). The mirror of the
    // image/button partial's display_density.
    [[nodiscard]] float display_density(JNIEnv* env, jobject widget)
    {
        static std::atomic<float> memoized{0.0F}; // 0 = not read yet (a real density is never 0)
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_image_view_class, "getContext", "()Landroid/content/Context;");
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

    // AspectExtensions.ToScaleType: the field NAME of the ImageView.ScaleType constant for an aspect
    // (the image partial's map verbatim).
    [[nodiscard]] const char* scale_type_field(maui::core::aspect value) noexcept
    {
        switch (value)
        {
            case maui::core::aspect::aspect_fit:
                return "FIT_CENTER";
            case maui::core::aspect::aspect_fill:
                return "CENTER_CROP";
            case maui::core::aspect::fill:
                return "FIT_XY";
            case maui::core::aspect::center:
                return "CENTER";
        }
        return "FIT_CENTER";
    }

    [[nodiscard]] local_ref<jobject> scale_type_constant(JNIEnv* env, const char* field_name)
    {
        jclass scale_type_class = default_jni_cache().find_class(env, k_scale_type_class);
        if (scale_type_class == nullptr)
        {
            return {};
        }
        const jfieldID field =
            env->GetStaticFieldID(scale_type_class, field_name, "Landroid/widget/ImageView$ScaleType;");
        if (clear_pending(env) || field == nullptr)
        {
            return {};
        }
        local_ref<jobject> value{env, env->GetStaticObjectField(scale_type_class, field)};
        if (clear_pending(env))
        {
            return {};
        }
        return value;
    }

    // ---- the chrome drawable: ONE GradientDrawable for fill + stroke + corner radius ----
    // The button partial's maui_background_drawable verbatim, retargeted at the ImageView. Returns OUR
    // installed GradientDrawable (an ImageView has no default background, so getBackground() is null until
    // we install one — the instanceof check still identifies ours), installing a fresh one only when
    // `install` is set. An empty ref means "not installed and not asked to install" (or a JNI failure).
    [[nodiscard]] local_ref<jobject> maui_background_drawable(JNIEnv* env, jobject widget, bool install)
    {
        auto& cache = default_jni_cache();
        jclass gradient_class = cache.find_class(env, k_gradient_drawable_class);
        jmethodID get_background =
            cache.method(env, k_image_view_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
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
            cache.method(env, k_image_view_class, "setBackground", "(Landroid/graphics/drawable/Drawable;)V");
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

    // Push setStroke(widthPx, color) + setCornerRadius(radiusPx) onto OUR GradientDrawable (the button
    // partial's update_button_stroke, retargeted). C# routes ImageButton's MapStrokeColor/Thickness/
    // CornerRadius through the SAME intertwined ButtonExtensions.UpdateButtonStroke, recomputing all three
    // each time. Installs the drawable only once a stroke or radius is actually visible; the fill (set by
    // update_background) and these can each trigger the install independently — whichever runs first
    // creates the shared drawable, the others find it via the instanceof check.
    void apply_stroke_and_corner(JNIEnv* env, jobject widget, const maui::core::image_button_platform& platform)
    {
        const double thickness = platform.stroke_thickness >= 0 ? platform.stroke_thickness : 0;
        const int radius = platform.corner_radius >= 0 ? platform.corner_radius : 0;
        const bool visible = thickness > 0 || radius > 0;
        const local_ref<jobject> drawable = maui_background_drawable(env, widget, /*install=*/visible);
        if (!drawable)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_stroke = cache.method(env, k_gradient_drawable_class, "setStroke", "(II)V");
        jmethodID set_corner_radius = cache.method(env, k_gradient_drawable_class, "setCornerRadius", "(F)V");
        if (set_stroke == nullptr || set_corner_radius == nullptr)
        {
            return;
        }
        const float density = display_density(env, widget);
        env->CallVoidMethod(drawable.get(), set_stroke, to_pixels(thickness, density),
                            static_cast<jint>(platform.stroke_color.to_int()));
        if (clear_pending(env))
        {
            return;
        }
        env->CallVoidMethod(drawable.get(), set_corner_radius,
                            static_cast<jfloat>(to_pixels(static_cast<double>(radius), density)));
        clear_pending(env);
    }

    // Push the effective content padding onto the ImageView (the button partial has no analog — this is
    // ImageButtonExtensions.UpdatePadding, unique to the ImageButton). C# adds the STROKE WIDTH to the
    // developer's Padding as "additional padding" (SetContentPadding = padding + strokeWidth on every side),
    // then zeroes View.setPadding — because ShapeableImageView's content-padding and the stroke overlap. On
    // this bare-ImageView backend there is no content-padding channel, so the combined value rides the plain
    // setPadding directly. The stroke contribution is exactly GetStrokeProperties' strokeWidth: the set
    // StrokeThickness when a border is present (thickness > 0), else 0 — so a BorderWidth-only ImageButton
    // (no Padding, no size request, an undecodable cog.png) still measures to 2×strokeWidth and renders as
    // the thin red bar MAUI shows, instead of collapsing to 0×0 and vanishing. (C#'s "StrokeColor set but
    // thickness unset → 1dp default" sub-case has no page witness and the port carries no stroke-color-set
    // flag, so thickness>0 is the faithful, contained rule — documented deviation.) A NaN Padding keeps the
    // View default (0) with no stroke add, exactly as MapPadding's NaN gate does.
    void apply_padding(JNIEnv* env, jobject widget, const maui::core::image_button_platform& platform)
    {
        if (platform.padding.is_nan())
        {
            return;
        }
        const double stroke_extra = platform.stroke_thickness > 0 ? platform.stroke_thickness : 0.0;
        const float density = display_density(env, widget);
        jmethodID set_padding = default_jni_cache().method(env, k_image_view_class, "setPadding", "(IIII)V");
        if (set_padding == nullptr)
        {
            return;
        }
        env->CallVoidMethod(widget, set_padding, to_pixels(platform.padding.left + stroke_extra, density),
                            to_pixels(platform.padding.top + stroke_extra, density),
                            to_pixels(platform.padding.right + stroke_extra, density),
                            to_pixels(platform.padding.bottom + stroke_extra, density));
        clear_pending(env);
    }

    // ---- source bytes: APK asset → on-disk file (the image partial's resolver verbatim) ----
    [[nodiscard]] maui::core::image_bytes drain_input_stream(JNIEnv* env, jobject stream)
    {
        auto& cache = default_jni_cache();
        jmethodID read = cache.method(env, "java/io/InputStream", "read", "([B)I");
        jmethodID close = cache.method(env, "java/io/InputStream", "close", "()V");
        if (read == nullptr)
        {
            return {};
        }
        constexpr jsize k_chunk = 64 * 1024;
        const local_ref<jbyteArray> buffer{env, env->NewByteArray(k_chunk)};
        if (clear_pending(env) || !buffer)
        {
            return {};
        }
        maui::core::image_bytes bytes;
        for (;;)
        {
            const jint n = env->CallIntMethod(stream, read, buffer.get());
            if (clear_pending(env) || n <= 0)
            {
                break; // -1 = EOF
            }
            const std::size_t old = bytes.size();
            bytes.resize(old + static_cast<std::size_t>(n));
            std::vector<jbyte> staging(static_cast<std::size_t>(n));
            env->GetByteArrayRegion(buffer.get(), 0, n, staging.data());
            if (clear_pending(env))
            {
                break;
            }
            for (jint i = 0; i < n; ++i)
            {
                bytes[old + static_cast<std::size_t>(i)] =
                    static_cast<std::byte>(static_cast<unsigned char>(staging[static_cast<std::size_t>(i)]));
            }
        }
        if (close != nullptr)
        {
            env->CallVoidMethod(stream, close);
            clear_pending(env);
        }
        return bytes;
    }

    [[nodiscard]] maui::core::image_bytes read_asset_bytes(JNIEnv* env, std::string_view name)
    {
        jobject context = app_context();
        if (env == nullptr || context == nullptr)
        {
            return {};
        }
        auto& cache = default_jni_cache();
        jmethodID get_assets =
            cache.method(env, "android/content/Context", "getAssets", "()Landroid/content/res/AssetManager;");
        jmethodID open =
            cache.method(env, "android/content/res/AssetManager", "open", "(Ljava/lang/String;)Ljava/io/InputStream;");
        if (get_assets == nullptr || open == nullptr)
        {
            return {};
        }
        const local_ref<jobject> assets{env, env->CallObjectMethod(context, get_assets)};
        if (clear_pending(env) || !assets)
        {
            return {};
        }
        const local_ref<jstring> jname = maui::platform::android::to_jstring(env, name);
        if (!jname)
        {
            return {};
        }
        const local_ref<jobject> stream{env, env->CallObjectMethod(assets.get(), open, jname.get())};
        if (clear_pending(env) || !stream) // a missing asset throws FileNotFoundException → cleared, empty
        {
            return {};
        }
        return drain_input_stream(env, stream.get());
    }

    [[nodiscard]] maui::core::image_bytes resolve_file_bytes(JNIEnv* env, std::string_view file)
    {
        maui::core::image_bytes bytes = read_asset_bytes(env, file);
        if (!bytes.empty())
        {
            return bytes;
        }
        return maui::core::read_uri_bytes(file); // file:// + absolute/relative disk path
    }

    [[nodiscard]] local_ref<jobject> decode_bitmap(JNIEnv* env, const maui::core::image_bytes& bytes)
    {
        if (env == nullptr || bytes.empty())
        {
            return {};
        }
        auto& cache = default_jni_cache();
        jmethodID decode =
            cache.static_method(env, k_bitmap_factory_class, "decodeByteArray", "([BII)Landroid/graphics/Bitmap;");
        jclass factory_class = cache.find_class(env, k_bitmap_factory_class);
        if (decode == nullptr || factory_class == nullptr)
        {
            return {};
        }
        const auto len = static_cast<jsize>(bytes.size());
        const local_ref<jbyteArray> array{env, env->NewByteArray(len)};
        if (clear_pending(env) || !array)
        {
            return {};
        }
        std::vector<jbyte> staging(bytes.size());
        for (std::size_t i = 0; i < bytes.size(); ++i)
        {
            staging[i] = static_cast<jbyte>(std::to_integer<unsigned char>(bytes[i]));
        }
        env->SetByteArrayRegion(array.get(), 0, len, staging.data());
        if (clear_pending(env))
        {
            return {};
        }
        local_ref<jobject> bitmap{env, env->CallStaticObjectMethod(factory_class, decode, array.get(), 0, len)};
        if (clear_pending(env))
        {
            return {};
        }
        return bitmap;
    }

    struct bitmap_size
    {
        double width;
        double height;
    };
    [[nodiscard]] bitmap_size bitmap_intrinsic_size(JNIEnv* env, jobject bitmap)
    {
        auto& cache = default_jni_cache();
        jmethodID get_width = cache.method(env, k_bitmap_class, "getWidth", "()I");
        jmethodID get_height = cache.method(env, k_bitmap_class, "getHeight", "()I");
        if (bitmap == nullptr || get_width == nullptr || get_height == nullptr)
        {
            return {.width = 0.0, .height = 0.0};
        }
        const jint w = env->CallIntMethod(bitmap, get_width);
        const jint h = env->CallIntMethod(bitmap, get_height);
        if (clear_pending(env))
        {
            return {.width = 0.0, .height = 0.0};
        }
        return {.width = static_cast<double>(w), .height = static_cast<double>(h)};
    }

    void set_image_bitmap(JNIEnv* env, jobject widget, jobject bitmap)
    {
        jmethodID set_bitmap =
            default_jni_cache().method(env, k_image_view_class, "setImageBitmap", "(Landroid/graphics/Bitmap;)V");
        if (set_bitmap == nullptr)
        {
            return;
        }
        env->CallVoidMethod(widget, set_bitmap, bitmap);
        clear_pending(env);
    }

    // The native half of dev.mauicpp.NativeOnClickListener.nativeOnClick(long): the peer is the handler's
    // image_button_platform; the wired on_click callback carries OnClick's body (VirtualView.Clicked()).
    // Bound via RegisterNatives (no Java_* export needed) — the button partial's trampoline retargeted.
    void JNICALL native_on_click(JNIEnv* /*env*/, jclass /*listener_class*/, jlong peer)
    {
        auto* platform = reinterpret_cast<maui::core::image_button_platform*>(peer);
        if (platform != nullptr && platform->on_click)
        {
            platform->on_click();
        }
    }

    [[nodiscard]] bool register_click_natives(JNIEnv* env, jclass listener_class)
    {
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
    // Releases the global reference pinning the MauiImageView (the JNI shape of the pimpl-owned-native-view
    // doctrine: the ios twin CFReleases its UIButton here). The decoded Bitmap + the chrome GradientDrawable
    // are owned by the View, reclaimed when it is GC'd.
    image_button_platform::~image_button_platform()
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

    // ---- the generic-IView pushes (the shared view_mapper calls these through view_platform_base) ----
    // Each calls the base body FIRST — the headless mirrors must stay live for the VM-less cross-platform
    // suite — then pushes to the real widget when one exists. Mirrors the android button partial.

    void image_button_platform::update_visibility(maui::core::visibility value)
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

    void image_button_platform::update_opacity(double value)
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

    void image_button_platform::update_is_enabled(bool value)
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

    void image_button_platform::update_automation_id(std::string_view value)
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
        jmethodID get_important = cache.method(env.get(), k_image_view_class, "getImportantForAccessibility", "()I");
        jmethodID set_description =
            cache.method(env.get(), k_image_view_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (get_important == nullptr || set_description == nullptr)
        {
            return;
        }
        const jint important_before = env->CallIntMethod(widget, get_important);
        if (clear_pending(env.get()))
        {
            return;
        }
        const local_ref<jstring> description = maui::platform::android::to_jstring(env.get(), value);
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

    void image_button_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void image_button_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    void image_button_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    // Background → the SHARED chrome GradientDrawable's fill. A SolidPaint (the page's green/purple brush)
    // sets setColor(argb); a gradient paint installs the GradientDrawable color ramp via the shared
    // apply_background helper (the android_visual_ops gradient path); a null paint clears OUR drawable's
    // fill. The fill can be the FIRST thing that installs the shared drawable (before any stroke/corner),
    // so after setting the color we re-assert stroke + corner onto the same drawable (they no-op when
    // unset). The base mirror runs first (the VM-less suite observes background()).
    void image_button_platform::update_background(const maui::graphics::paint* value)
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
        // A gradient/radial paint goes through the shared android_visual_ops apply_background, which
        // installs its OWN background ramp; the page uses only solid brushes (the gradient demo collapses
        // to a deterministic green/purple SOLID toggle), so this branch is the faithful best-effort tail.
        // A solid (or null) paint stays on the maui shared GradientDrawable so fill + stroke + corner share
        // one drawable (otherwise apply_background's setBackgroundColor would clobber the chrome drawable).
        if (value != nullptr && dynamic_cast<const maui::graphics::gradient_paint*>(value) != nullptr)
        {
            maui::platform::android::apply_background(native, value);
            return;
        }
        // Solid or null: write the fill color onto OUR shared maui drawable, installing it for a non-null
        // fill (so the green/purple rectangle appears even with cog.png undecodable). A null paint clears
        // the fill to transparent when our drawable exists; an untouched (never-filled, never-stroked)
        // button keeps no background.
        const local_ref<jobject> drawable = maui_background_drawable(env.get(), widget, /*install=*/value != nullptr);
        if (drawable)
        {
            jmethodID set_color = default_jni_cache().method(env.get(), k_gradient_drawable_class, "setColor", "(I)V");
            if (set_color != nullptr)
            {
                const jint argb = value != nullptr ? static_cast<jint>(value->background_color().to_int()) : 0;
                env->CallVoidMethod(drawable.get(), set_color, argb);
                clear_pending(env.get());
            }
        }
        // The fill may have just created the shared drawable — re-assert stroke + corner so a button that
        // set its border BEFORE its background still shows both on the one drawable.
        apply_stroke_and_corner(env.get(), widget, *this);
    }

    std::unique_ptr<image_button_platform> image_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_button_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        jclass image_view_class = cache.find_class(env.get(), k_image_view_class);
        jmethodID ctor = cache.method(env.get(), k_image_view_class, "<init>", "(Landroid/content/Context;)V");
        if (image_view_class == nullptr || ctor == nullptr)
        {
            return platform;
        }
        // CreatePlatformView: new AppCompatImageView(Context) { SetAdjustViewBounds(true) } — the
        // AppCompat subtype is the AAR deviation; MauiImageView (extends the stock ImageView) carries the
        // same SetAdjustViewBounds API + a theme-independent (Context) ctor (the image partial's recipe).
        const local_ref<jobject> widget{env.get(), env->NewObject(image_view_class, ctor, context)};
        if (clear_pending(env.get()) || !widget)
        {
            return platform;
        }
        call_void_bool(env.get(), widget.get(), "setAdjustViewBounds", JNI_TRUE);
        // The button is tappable: make it clickable + focusable (ImageView defaults to non-clickable, so a
        // raw ImageView would never fire OnClickListener). new MauiMaterialButton is clickable by default;
        // an ImageButton must opt in.
        call_void_bool(env.get(), widget.get(), "setClickable", JNI_TRUE);
        call_void_bool(env.get(), widget.get(), "setFocusable", JNI_TRUE);
        // Wrap-content LayoutParams up front (the parentless-View guard the image/button partials document:
        // a null-LayoutParams View NPEs in checkForRelayout once a parent measure pass runs).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_image_view_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
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
        platform->native = env->NewGlobalRef(widget.get()); // released in ~image_button_platform
        return platform;
    }

    // Android loader wiring: leave the loader on its defaults (the image partial's deviation — the async
    // android image-source services / Glide AAR are deferred; only the FILE fast-path decodes).
    void image_button_handler::configure_loader(image_source_loader& /*loader*/)
    {
    }

    void image_button_handler::on_connect_handler(image_button_platform& platform)
    {
        // The Pressed/Released pair belongs to the deferred touch listener (the button partial's gesture
        // deviation); the callbacks stay wired even VM-less so the cross-platform suite can drive them.
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
                view->send_released();
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
            cache.method(env.get(), k_image_view_class, "setOnClickListener", "(Landroid/view/View$OnClickListener;)V");
        if (listener_class == nullptr || listener_ctor == nullptr || set_on_click_listener == nullptr ||
            !register_click_natives(env.get(), listener_class))
        {
            return; // host-provided listener class absent → the click channel stays C++-only (VM-less analog)
        }
        const local_ref<jobject> listener{
            env.get(), env->NewObject(listener_class, listener_ctor, reinterpret_cast<jlong>(&platform))};
        if (clear_pending(env.get()) || !listener)
        {
            return;
        }
        env->CallVoidMethod(widget_of(platform), set_on_click_listener, listener.get());
        clear_pending(env.get());
    }

    void image_button_handler::on_disconnect_handler(image_button_platform& platform)
    {
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
        jmethodID set_on_click_listener = default_jni_cache().method(
            env.get(), k_image_view_class, "setOnClickListener", "(Landroid/view/View$OnClickListener;)V");
        if (set_on_click_listener != nullptr)
        {
            env->CallVoidMethod(widget_of(platform), set_on_click_listener, static_cast<jobject>(nullptr));
            clear_pending(env.get());
        }
    }

    void image_button_handler::map_aspect(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->image_aspect = view.aspect(); // headless mirror FIRST
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
        // ImageViewExtensions.UpdateAspect: AspectFill turns adjust-view-bounds OFF, every other aspect ON,
        // then SetScaleType (the image partial's map verbatim).
        call_void_bool(env.get(), widget, "setAdjustViewBounds",
                       view.aspect() == aspect::aspect_fill ? JNI_FALSE : JNI_TRUE);
        const local_ref<jobject> scale_type = scale_type_constant(env.get(), scale_type_field(view.aspect()));
        jmethodID set_scale_type =
            cache.method(env.get(), k_image_view_class, "setScaleType", "(Landroid/widget/ImageView$ScaleType;)V");
        if (!scale_type || set_scale_type == nullptr)
        {
            return;
        }
        env->CallVoidMethod(widget, set_scale_type, scale_type.get());
        clear_pending(env.get());
    }

    void image_button_handler::map_is_opaque(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->opaque = view.is_opaque(); // headless mirror only (no plain-ImageView analog)
        }
    }

    void image_button_handler::map_is_animation_playing(image_button_handler& handler, i_image_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            // ImageButton pins IsAnimationPlaying false; the mirror stays false (the deferred GIF cycling
            // is OUT OF SCOPE — header note). No native push.
            platform->animation_playing = view.is_animation_playing();
        }
    }

    void image_button_handler::map_padding(image_button_handler& handler, i_image_button& view)
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
        // ImageButtonExtensions.UpdatePadding → SetContentPadding(padding + strokeWidth) (the ImageButton's
        // MapPadding shape, distinct from the Button's plain SetPadding): the stroke width is folded into the
        // content padding so a bordered button reserves room for its stroke. NaN keeps the View default (0).
        apply_padding(env.get(), widget_of(*platform), *platform);
    }

    // MapStrokeColor / MapStrokeThickness / MapCornerRadius all push onto the SAME shared chrome
    // GradientDrawable (C# routes the three through the one intertwined ButtonExtensions.UpdateButtonStroke).
    void image_button_handler::map_stroke_color(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->stroke_color = view.stroke_color();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            apply_stroke_and_corner(env.get(), widget_of(*platform), *platform);
        }
    }

    void image_button_handler::map_stroke_thickness(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->stroke_thickness = view.stroke_thickness();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            apply_stroke_and_corner(env.get(), widget_of(*platform), *platform);
            // C#'s MapStrokeThickness re-runs MapPadding (handler.UpdateValue(nameof(IImageButton.Padding)))
            // because the stroke width folds into content padding — so a new border thickness re-reserves its
            // room, and a border-only button gains a non-zero measured size.
            apply_padding(env.get(), widget_of(*platform), *platform);
        }
    }

    void image_button_handler::map_corner_radius(image_button_handler& handler, i_image_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->corner_radius = view.corner_radius();
        if (platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            apply_stroke_and_corner(env.get(), widget_of(*platform), *platform);
            // C#'s MapCornerRadius also re-runs MapPadding (the stroke/corner/padding recompute is
            // intertwined); harmless when the stroke is unchanged, and keeps the content padding coherent.
            apply_padding(env.get(), widget_of(*platform), *platform);
        }
    }

    // ---- per-backend source primitives (the cross-platform map_source routes here) ----

    void image_button_handler::load_file_source_sync(image_button_platform& platform,
                                                     const i_file_image_source& file_src)
    {
        platform.source_kind = "file"; // mirror FIRST (the VM-less suite observes the load)
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
        platform.intrinsic_width = 0.0;
        platform.intrinsic_height = 0.0;
        if (platform.native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        const maui::core::image_bytes bytes = resolve_file_bytes(env.get(), file_src.file());
        const local_ref<jobject> bitmap = decode_bitmap(env.get(), bytes);
        if (!bitmap)
        {
            // cog.png (SVG-only) decodes to null → no pixels (the chrome fill is the visible rectangle,
            // matching iOS); a real PNG (dotnet_bot.png) decodes and shows.
            set_image_bitmap(env.get(), widget_of(platform), nullptr);
            return;
        }
        set_image_bitmap(env.get(), widget_of(platform), bitmap.get());
        const bitmap_size size = bitmap_intrinsic_size(env.get(), bitmap.get());
        platform.intrinsic_width = size.width;
        platform.intrinsic_height = size.height;
    }

    void image_button_handler::apply_loaded_result(image_button_platform& platform, const image_source_result& result)
    {
        // Mirror only this cut: uri/stream/font carry no bytes through the result on android (no Glide AAR
        // / android image-source services — OUT OF SCOPE: the "Use Online Source" remote swap). A !loaded()
        // result clears.
        if (!result.loaded())
        {
            clear_source_native(platform);
            return;
        }
        platform.source_kind = result.kind();
        platform.source_file = result.detail();
        platform.source_loaded = true;
    }

    void image_button_handler::clear_source_native(image_button_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
        platform.intrinsic_width = 0.0;
        platform.intrinsic_height = 0.0;
        if (platform.native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            set_image_bitmap(env.get(), widget_of(platform), nullptr);
        }
    }

    maui::graphics::size image_button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        // Intrinsic-bitmap fast path (the image partial's SizeThatFitsImage analog): a decoded file source
        // reports its intrinsic size aspect-fit to any finite constraint. This is what the Custom-Size
        // dotnet_bot.png button needs (with no WidthRequest it must auto-size to the boat, not collapse).
        if (platform->intrinsic_width > 0.0 && platform->intrinsic_height > 0.0)
        {
            const double w = platform->intrinsic_width;
            const double h = platform->intrinsic_height;
            double scale = 1.0;
            if (std::isfinite(width_constraint) && width_constraint < w)
            {
                scale = std::min(scale, width_constraint / w);
            }
            if (std::isfinite(height_constraint) && height_constraint < h * scale)
            {
                scale = std::min(scale, height_constraint / h);
            }
            return {w * scale, h * scale};
        }
        if (platform->native == nullptr)
        {
            return {0, 0}; // VM-less + no decoded bitmap: no intrinsic content (the headless partial's {0,0})
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandlerExtensions.GetDesiredSizeFromHandler (Android): finite constraints become AtMost specs
        // in pixels, infinite become Unspecified; View.measure, then measured pixels come back as dp. With
        // no decoded drawable the ImageView measures to its padding (the faithful "no image" measurement) —
        // an explicit WidthRequest/HeightRequest still wins via the cross-platform ResolveConstraints clamp.
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_image_view_class, "measure", "(II)V");
        jmethodID get_measured_width = cache.method(env.get(), k_image_view_class, "getMeasuredWidth", "()I");
        jmethodID get_measured_height = cache.method(env.get(), k_image_view_class, "getMeasuredHeight", "()I");
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

    void image_button_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the view measures Exactly at the final
        // size (Android requires a measure pass before layout) and lays out (the image/button partial path).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_image_view_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_image_view_class, "layout", "(IIII)V");
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
