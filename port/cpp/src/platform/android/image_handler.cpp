// image_handler — Android (JNI) platform partial, the M-android per-control fan-out replayed over JNI
// for the image control (the iOS Rosetta Stone — src/platform/ios/image_handler.mm — mapped onto a real
// dev.mauicpp.MauiImageView, an android.widget.ImageView subclass). The managed platform view is held as a
// JNI global reference in image_platform::native; map_aspect pushes the scaling mode through the jni_cache'd
// method ids, the file fast-path decodes the bundled bytes into an android.graphics.Bitmap (BitmapFactory)
// and pushes it via setImageBitmap, and get_desired_size reports the decoded bitmap's intrinsic size
// (aspect-fit), exactly like the iOS/apple twins.
//
// Ported DIRECTLY from ImageHandler.Android.cs + Platform/Android/{ImageViewExtensions.cs,
// AspectExtensions.cs} + ViewExtensions.cs + ContextExtensions.cs (ToPixels). The cross-platform source
// routing (file fast-path vs the async loader) lives once in image_handler.cpp::map_source and dispatches
// into the per-backend source primitives at the bottom of this file.
//
// DOCUMENTED DEVIATIONS from the C# oracle (each is an infrastructure gap, not a behavior guess — mirrors
// exactly how the android button partial documents its plain-widget deviations):
//
//   - The widget is dev.mauicpp.MauiImageView (extends the stock android.widget.ImageView), not the
//     AppCompatImageView that ImageHandler.Android.cs creates: AndroidX.AppCompat is a gradle/AAR dependency
//     this APK-less backend does not carry (the same reason the button partial uses a plain Button, not
//     MauiMaterialButton). AppCompatImageView's only behavioral addition over ImageView that this cut
//     touches — SetAdjustViewBounds — exists on plain ImageView too (a stock View API since API 16), so the
//     create + UpdateAspect adjust-view-bounds toggle is ported faithfully. MauiImageView additionally
//     carries a setClipPath hook (the WrapperView.SetClip analog) the handler does not yet drive — the clip
//     family is a follow-up wave; the path stays null, so the subclass renders identically to ImageView.
//
//   - The generic IView property pushes (Visibility / Opacity / IsEnabled / AutomationId / Transform /
//     FlowDirection / Background / Semantics) are NOT overridden for android on image_platform: the
//     image_platform struct in include/maui/core/image_handler.hpp declares android overrides for none of
//     them (only MAUI_PLATFORM_APPLE / MAUI_PLATFORM_IOS blocks exist), and that header is out of scope for
//     this slice. So those properties keep ONLY the view_platform_base mirrors (the VM-less cross-platform
//     suite observes them), and the real ImageView does not yet receive them over JNI. The button partial
//     pushes them because button_handler.hpp DOES carry a MAUI_PLATFORM_ANDROID override block; wiring the
//     same block + shared android view/visual/semantics ops for the image is the header-side follow-up
//     (see port/STATUS.md). // TODO: verify against src/Core/src/Platform/Android/ViewExtensions.cs once the
//     image_platform android override block lands.
//
//   - FILE sources DECODE; uri/stream/font sources are still mirrored. The C# pipeline routes every source
//     through Glide (ImageImageSourcePartSetter.SetImageSource → ImageView.SetImageDrawable(platformImage)).
//     This cut decodes the FILE fast-path natively: load_file_source_sync resolves the bundled bytes
//     (assets/<name> via the Context AssetManager — the apphost packages the gallery resources into the APK;
//     then a disk read for an absolute path / the test host), BitmapFactory.decodeByteArray → Bitmap, and
//     setImageBitmap pushes it. The async apply (apply_loaded_result, uri/stream/font) stays a mirror: no
//     bytes flow through image_source_result on android (no Glide AAR / android image-source services yet),
//     so a remote-photo or font-glyph decode awaits that deferred wiring — only the file bitmaps the gallery
//     image / image_button pages show render (dotnet_bot.png; animated_heart.gif's first frame — BitmapFactory
//     decodes a GIF to its first frame, the animation cycling being a separate IsAnimationPlaying concern).
//     A from_file naming an asset that exists only as SVG (cog.png — never rasterized) decodes to null and
//     leaves the view image-less, matching the existing parity note. The headless-style mirrors (source_kind
//     / source_file / source_loaded) are ALWAYS set so the VM-less cross-platform suite still observes the
//     load. // TODO: verify against src/Core/src/Platform/Android/ImageViewExtensions.cs + the android
//     FileImageSourceService once the async android image-source services land.
//
//   - IsOpaque and IsAnimationPlaying are mirrored only. UpdateIsAnimationPlaying operates on the LOADED
//     Drawable (Drawable.UpdateIsAnimationPlaying → IAnimatable.Start/Stop), which does not exist until the
//     deferred Glide decode lands; IsOpaque has no plain-ImageView analog on android (C#'s ImageHandler does
//     not map it natively on android at all — the port's is_opaque mirror is the documented cross-platform
//     extension, faithful as a flag). Both keep the headless mirror and re-apply once the drawable decode
//     arrives. // TODO: verify against src/Core/src/Platform/Android/ImageViewExtensions.cs UpdateIsAnimationPlaying.
//
//   - PlatformArrange's center-crop clip (ImageHandler.Android.cs: PlatformInterop.IsImageViewCenterCrop →
//     SetClipBounds for AspectFill) is deferred: SetClipBounds is a PlatformInterop helper class this
//     backend's java/ does not yet provide, and clipping is a WrapperView concern in the same family the
//     button partial defers. The measure-Exactly + layout body is ported. // TODO: verify against
//     src/Core/src/Handlers/Image/ImageHandler.Android.cs PlatformArrange.
//
// VM-less degradation: the android preset also runs the PURE-NATIVE cross-platform suite on the emulator
// (tools/android-emu-run.sh) where no Java VM exists. Every JNI path here checks scoped_env / app_context()
// and quietly skips (the decode included — no Context means no AssetManager and no widget), while the
// headless mirrors (image_aspect / source_* / opaque / animation_playing + intrinsic size) are ALWAYS
// maintained — so that suite observes exactly the headless partial's behavior, and the widget test host
// (tools/android-testhost-run.sh) additionally observes the real MauiImageView with its decoded bitmap.

#include "maui/core/image_handler.hpp"

#include <jni.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp" // image_bytes
#include "maui/core/image_source_loader.hpp"   // configure_loader parameter type
#include "maui/core/image_source_result.hpp"
#include "maui/core/uri_bytes.hpp" // read_uri_bytes (disk fallback)
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    // The native widget is dev.mauicpp.MauiImageView (an android.widget.ImageView subclass that can clip
    // its draw to a native Path — the WrapperView.SetClip analog; the clip wiring is a later wave, the
    // path stays null here). Because it extends ImageView, every method the handler drives — setImageBitmap,
    // setScaleType, setAdjustViewBounds, measure/layout, the View surface — resolves through the subclass
    // (GetMethodID walks the superclasses). The class is dexed by both hosts from src/platform/android/java.
    constexpr const char* k_image_view_class = "dev/mauicpp/MauiImageView";
    constexpr const char* k_scale_type_class = "android/widget/ImageView$ScaleType";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";
    constexpr const char* k_bitmap_factory_class = "android/graphics/BitmapFactory";
    constexpr const char* k_bitmap_class = "android/graphics/Bitmap";

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler).
    constexpr jint k_measure_spec_unspecified = 0;
    constexpr auto k_measure_spec_at_most = static_cast<jint>(0x80000000U);
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject widget_of(const maui::core::image_platform& platform) noexcept
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
    // (the JNI walk is four calls). 1.0 when any step fails (failures are NOT memoized, so a transient
    // failure does not pin the fallback). The mirror of the button partial's display_density.
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

    // AspectExtensions.ToScaleType: the field NAME of the ImageView.ScaleType constant for an aspect.
    //   AspectFit → FIT_CENTER, AspectFill → CENTER_CROP, Fill → FIT_XY, Center → CENTER (default FIT_CENTER).
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
        return "FIT_CENTER"; // AspectExtensions' default arm
    }

    // Read one ImageView.ScaleType static object field (the enum-like constants are static fields of
    // android/widget/ImageView$ScaleType — GetStaticFieldID + GetStaticObjectField, the same shape C#'s
    // ImageView.ScaleType.FitCenter resolves to). Local ref (empty on any JNI failure). The jni_cache only
    // memoizes instance fields, so the static-field id is resolved directly through the pinned class.
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

    // ---- source bytes: APK asset → on-disk file ----
    // The gallery's from_file("dotnet_bot.png") names a BUNDLED asset. On the real app host the resources
    // are packaged into the APK under assets/ (build_android_apphost.sh), so the robust, SELinux-/uid-/
    // reinstall-immune fetch is the AssetManager: context.getAssets().open(name) → an InputStream we drain.
    // This is the android twin of the apple file fast-path resolving from_file() against the flat .app
    // bundle. A bare name first tries assets/<name>; if no asset (or no Context — the VM-less / test host),
    // it falls back to the cross-platform read_uri_bytes (a file:// / absolute path on disk).

    // Drain a java.io.InputStream fully into image_bytes (empty on any JNI failure / empty stream). Reads in
    // 64 KiB chunks via InputStream.read(byte[]) — the same buffered drain the testhost font loader uses.
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
                break; // -1 = EOF (and 0 only for a zero-length request, which we never make)
            }
            const std::size_t old = bytes.size();
            bytes.resize(old + static_cast<std::size_t>(n));
            // GetByteArrayRegion copies into a temporary jbyte buffer; reinterpret to std::byte is forbidden
            // (NOLINT would be a suppression), so go through a small jbyte staging span.
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

    // Read assets/<name> from the process Context's AssetManager (empty if no Context, no AssetManager, or
    // the asset is absent — the caller then falls back to a disk read). context.getAssets() :
    // android.content.res.AssetManager; assets.open(name) : java.io.InputStream (throws if absent — cleared).
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

    // Resolve the bytes for a from_file() name: assets/<name> first (the packaged APK resource), then the
    // cross-platform on-disk reader (an absolute path / file:// uri — and the test host, which has assets
    // neither). Empty when neither resolves (the caller clears the view, exactly like a nil apple decode).
    [[nodiscard]] maui::core::image_bytes resolve_file_bytes(JNIEnv* env, std::string_view file)
    {
        maui::core::image_bytes bytes = read_asset_bytes(env, file);
        if (!bytes.empty())
        {
            return bytes;
        }
        return maui::core::read_uri_bytes(file); // file:// + absolute/relative disk path
    }

    // BitmapFactory.decodeByteArray(bytes, 0, len) → android.graphics.Bitmap (null on a failed decode). The
    // returned local ref owns the decoded bitmap; the caller installs it via setImageBitmap (the ImageView
    // takes its own retain through its drawable) and reads getWidth/getHeight for the intrinsic measure.
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
        // Copy via a jbyte staging span (no reinterpret of std::byte* to jbyte*).
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

    // The decoded bitmap's intrinsic pixel size via getWidth()/getHeight() ({0,0} on failure). The gallery
    // assets are 1× rasters, so the pixel size IS the framework-point intrinsic size (the apple twin divides
    // an @2x NSImage by the screen scale; our packaged PNGs carry no @Nx sibling).
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

    // Push a decoded Bitmap (or null to clear) into the ImageView via setImageBitmap(Bitmap).
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
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the MauiImageView (the JNI shape of the pimpl-owned-native-view
    // doctrine: the ios twin CFReleases its UIImageView here; the headless twin just clears the slot). The
    // decoded Bitmap is owned by the ImageView's drawable, reclaimed when the View is GC'd.
    image_platform::~image_platform()
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

    std::unique_ptr<image_platform> image_handler::create_platform_view()
    {
        auto platform = std::make_unique<image_platform>();
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
        // ImageHandler.CreatePlatformView: new AppCompatImageView(Context) { SetAdjustViewBounds(true) }
        // — the AppCompat subtype is a Material/AppCompat AAR dependency (header deviations); MauiImageView
        // (extends the stock ImageView) carries the same SetAdjustViewBounds API and a theme-independent
        // (Context) ctor that constructs in both the testhost (no Activity theme) and the app host.
        const local_ref<jobject> widget{env.get(), env->NewObject(image_view_class, ctor, context)};
        if (clear_pending(env.get()) || !widget)
        {
            return platform;
        }
        // "Enable view bounds adjustment on measure" (the CreatePlatformView comment) — lets OnMeasure
        // account for the image's intrinsic aspect ratio during constrained measurement.
        call_void_bool(env.get(), widget.get(), "setAdjustViewBounds", JNI_TRUE);
        // Wrap-content LayoutParams up front (the same parentless-View guard the button partial documents:
        // a null-LayoutParams View NPEs in checkForRelayout once a parent measure pass runs; the android
        // container fan-out that would supply them on attach has not arrived).
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
        platform->native = env->NewGlobalRef(widget.get()); // released in ~image_platform
        return platform;
    }

    // Headless/android: leave the loader on its defaults — the synchronous read_uri_bytes fetch (file:// +
    // local paths) and the disk layer off (in-memory cache only). The real android loader wiring (Glide /
    // the android image services + a disk cache directory) is part of the deferred android image pipeline
    // (header deviations). Tests inject a dispatcher / disk dir / uri fetch via the loader's seams directly.
    void image_handler::configure_loader(image_source_loader& /*loader*/)
    {
    }

    void image_handler::map_aspect(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->image_aspect = view.aspect(); // headless mirror FIRST (the VM-less suite observes it)
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
        // then SetScaleType(aspect.ToScaleType()).
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

    // IsOpaque (headless mirror only on android — header deviations: no plain-ImageView analog, and C#'s
    // android handler does not map it natively).
    void image_handler::map_is_opaque(image_handler& handler, i_image& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->opaque = view.is_opaque();
        }
    }

    // IsAnimationPlaying (headless mirror only this cut — header deviations: UpdateIsAnimationPlaying acts
    // on the loaded Drawable, which awaits the deferred Glide decode; the flag is faithful + re-applied
    // once the drawable lands).
    void image_handler::map_is_animation_playing(image_handler& handler, i_image& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->animation_playing = view.is_animation_playing();
        }
    }

    // ---- per-backend source primitives (the cross-platform map_source in image_handler.cpp routes here) ----
    // The file fast-path decodes the bundled bytes (assets/<name>, then disk) into an android.graphics.Bitmap
    // via BitmapFactory and pushes it to the MauiImageView with setImageBitmap (the iOS twin's
    // [UIImage imageNamed:] / NSImageView.image analog). The headless-style mirrors (kind / file / loaded +
    // intrinsic size) are ALWAYS maintained so the android preset's pure-native cross-platform suite still
    // observes the load even with no JavaVM (tools/android-emu-run.sh). The async (uri/stream/font) apply is
    // still a mirror this cut — it carries no bytes through image_source_result on android (no Glide AAR), so
    // a remote photo decode awaits the deferred android image-source services; the FILE bitmaps the gallery's
    // image / image_button pages show (dotnet_bot.png, animated_heart.gif first frame) now render.

    // File fast-path: resolve the bundled bytes (assets/<name> → disk), decode to a Bitmap, push it into the
    // ImageView, and record the intrinsic size for get_desired_size. The mirror (kind/file/loaded) is set
    // FIRST so the VM-less suite observes the load; a failed decode clears the view (the nil-decode analog).
    void image_handler::load_file_source_sync(image_platform& platform, const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
        platform.intrinsic_width = 0.0;
        platform.intrinsic_height = 0.0;
        if (platform.native == nullptr)
        {
            return; // VM-less / context-less: mirror only (no native widget to push into)
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
            // A from_file naming an asset that is not packaged (e.g. cog.png — SVG-only, never rasterized) or
            // a failed decode leaves the view image-less, mirroring the iOS nil-decode: no pixels, no crash.
            set_image_bitmap(env.get(), widget_of(platform), nullptr);
            return;
        }
        set_image_bitmap(env.get(), widget_of(platform), bitmap.get());
        const bitmap_size size = bitmap_intrinsic_size(env.get(), bitmap.get());
        platform.intrinsic_width = size.width;
        platform.intrinsic_height = size.height;
    }

    // The async loader's apply (mirror this cut): copy the result's kind + detail. A !loaded() result clears
    // it, mirroring SetImageSource(null) / ImageViewExtensions.Clear. The real uri/stream bitmap decode +
    // setImageBitmap awaits the deferred android image-source services (no bytes flow through the result on
    // android), so no native push here — only the FILE fast-path above decodes.
    void image_handler::apply_loaded_result(image_platform& platform, const image_source_result& result)
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

    // Clear the loaded image (ImageViewExtensions.Clear: stop the animation + drop the drawable). Clears the
    // mirror + the intrinsic size AND drops the native bitmap (setImageBitmap(null)) so a source→null swap
    // actually empties the view, not just the mirror.
    void image_handler::clear_source_native(image_platform& platform)
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

    maui::graphics::size image_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        // Intrinsic-bitmap fast path (the iOS SizeThatFitsImage analog): when a file source decoded into a
        // bitmap, report its intrinsic size aspect-fit to any finite constraint. The old path measured the
        // native ImageView, which — with adjustViewBounds + a wrap-content drawable — returns the right
        // height under a finite width but {0,0} when BOTH constraints are infinite (an unconstrained stack
        // child), collapsing the auto-sized Image to nothing. Reporting the decoded size directly closes
        // that gap exactly as the apple/iOS twins do.
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
            // VM-less degradation: no native widget and no decoded bitmap, so there is no intrinsic content
            // size to report — the headless partial's {0,0}.
            return {0, 0};
        }
        const scoped_env env;
        if (!env)
        {
            return {0, 0};
        }
        jobject widget = widget_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandlerExtensions.GetDesiredSizeFromHandler (Android): finite constraints become AtMost specs
        // in pixels, infinite become Unspecified; View.measure, then the measured pixels come back as dp
        // (Context.FromPixels). With no decoded drawable the ImageView measures to its padding (wrap-content
        // with no intrinsic content), which is the faithful "no image loaded" measurement.
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

    void image_handler::platform_arrange(const maui::graphics::rect& frame)
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
        // ImageHandler.Android.PlatformArrange → ViewHandler.PlatformArrange: the dp frame becomes pixels,
        // the view measures Exactly at the final size (Android requires a measure pass before layout) and
        // lays out. The AspectFill center-crop ClipBounds branch is deferred (header deviations).
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

    // No display-density seam on android this cut (the loader's defaults run inline): pass the loader's
    // currently-set scale back through, so map_source's refresh_display_scale is a no-op that preserves
    // whatever a test set via source_loader().set_scale(). The headless twin does the same; ios reads the
    // real screen DPI instead. // TODO: verify against ContextExtensions.GetDisplayDensity once the android
    // density seam lands.
    float image_handler::query_display_scale() const
    {
        return source_loader_.scale();
    }
} // namespace maui::core
