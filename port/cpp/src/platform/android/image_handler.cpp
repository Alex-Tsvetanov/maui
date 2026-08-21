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
//     carries a setClipPath hook (the WrapperView.SetClip analog) the handler DRIVES (wave 11): the image's
//     VisualElement.Clip is masked by clipping the view's onDraw to a native android.graphics.Path. C#'s
//     ViewExtensions.UpdateClip masks an unwrapped View's WrapperView; the port has no per-control
//     WrapperView, so the custom MauiImageView clips itself instead (the iOS CAShapeLayer-mask analog —
//     update_clip below builds the Path + platform_arrange re-frames it, the bounds-dependent reapply_clip
//     twin). The generic-IView pushes other than Clip still keep ONLY the view_platform_base mirrors (next
//     bullet). Clip is wired for images only this wave (the gallery's clip pages clip images); a shared
//     android native-Path clip for any view's VisualElement.Clip is future work (image_handler.hpp note).
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
//     SetClipBounds for AspectFill) is now PORTED (set_center_crop_clip_bounds below): rather than the C#
//     PlatformInterop helper, the port calls the stock View.setClipBounds(Rect) directly (a plain View API
//     since API 18). It clips a CENTER_CROP (AspectFill) ImageView's over-scaled draw to the view's own
//     pixel bounds so the drawable does not spill over its siblings — the header/footer StackLayout Images
//     that were rendering OVER their captions/buttons (the header_footer_grid parity red). Every non-fill
//     aspect gets ClipBounds cleared to null, exactly as the C# arrange does.
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
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "android_image_decode.hpp" // shared byte-fetch + density-variant (@2x/@3x) resolve
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "jni/relayout.hpp" // request_relayout — re-run the host's layout pass after a late uri decode
#include "maui/core/aspect.hpp"
#include "maui/core/cancellation_token.hpp"  // the uri fetch seam's token parameter
#include "maui/core/i_font_image_source.hpp" // font-source band sizing in get_desired_size
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp" // image_bytes
#include "maui/core/image_source_loader.hpp"   // configure_loader parameter type
#include "maui/core/image_source_result.hpp"
#include "maui/core/uri_bytes.hpp" // read_uri_bytes — the non-http branch of the fetch seam
#include "maui/graphics/color.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    namespace image_decode = maui::platform::android::image_decode; // shared byte-fetch + @2x/@3x resolve

    // The native widget is dev.mauicpp.MauiImageView (an android.widget.ImageView subclass that can clip
    // its draw to a native android.graphics.Path — the WrapperView.SetClip analog; the handler now DRIVES
    // it via setClipPath, see install_clip). Because it extends ImageView, every method the handler drives —
    // setImageBitmap, setScaleType, setAdjustViewBounds, measure/layout, the View surface — resolves through
    // the subclass (GetMethodID walks the superclasses). The class is dexed by both hosts from
    // src/platform/android/java.
    constexpr const char* k_image_view_class = "dev/mauicpp/MauiImageView";
    constexpr const char* k_scale_type_class = "android/widget/ImageView$ScaleType";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";
    constexpr const char* k_bitmap_factory_class = "android/graphics/BitmapFactory";
    constexpr const char* k_bitmap_class = "android/graphics/Bitmap";
    // The android.graphics.Path class the clip mask is built into (the WrapperView.SetClip → CAShapeLayer
    // analog, expressed as a Path the view's onDraw clips to). Same class android_canvas builds its shape
    // paths from — duplicated here (not shared) so the two TUs stay independently buildable, exactly as the
    // box/border android partials each carry their own corner_radii_of. The Path keeps its DEFAULT WINDING
    // (non-zero) fill type: WrapperView.SetClip installs a plain CAShapeLayer mask whose fill rule is the
    // CoreAnimation default kCAFillRuleNonZero (the iOS apply_clip sets no even-odd), so a GeometryGroup's
    // EvenOdd rule — which lives on the concrete geometry_group, NOT on the i_shape seam path_for_bounds
    // exposes — is not conveyed on either backend. Matching the iOS reference (the ground truth) means the
    // non-zero union, not an even-odd hollow. // TODO: thread a fill rule through i_shape if a future page
    // depends on the even-odd hollow centre (it would diverge from the current iOS render).
    constexpr const char* k_path_class = "android/graphics/Path";
    // android.graphics.Rect — the clip-bounds rectangle View.setClipBounds takes (the AspectFill
    // center-crop clip, ImageHandler.Android.PlatformArrange → PlatformInterop.SetClipBounds).
    constexpr const char* k_rect_class = "android/graphics/Rect";

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

    void call_void_float(JNIEnv* env, jobject widget, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_image_view_class, name, "(F)V"))
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
    // The gallery's from_file("dotnet_bot.png") names a BUNDLED asset packaged into the APK under assets/
    // (build_android_apphost.sh). The AssetManager fetch (context.getAssets().open(name) → drained InputStream)
    // plus its disk fallback AND the @2x/@3x density-variant probe all live in the shared android_image_decode.hpp
    // now (image_decode::resolve_scaled_file_bytes) — the same helper the button/image-button icon paths use, so
    // there is one asset-fetch. decode_bitmap / bitmap_intrinsic_size stay file-local (image_handler owns them).

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

    // ARGB transparent — the "no background" fill (View.setBackgroundColor(TRANSPARENT)) used to clear a
    // previously-painted background when the paint becomes null / non-solid.
    constexpr jint k_transparent_argb = 0;

    // VisualElement.Background (a solid color) → View.setBackgroundColor(argb). The C# ViewHandler.
    // UpdateBackground sets the View's background to the paint's drawable; for a solid_paint the faithful
    // plain-View analog is setBackgroundColor(int) — the same primitive the collection_view cell background
    // uses. A null / non-solid paint clears the background to transparent (a gradient/tiled brush awaits a
    // shared android paint→drawable bridge; see the header note). color::to_int() already packs 0xAARRGGBB,
    // exactly what setBackgroundColor(int) expects (mirrors android_canvas::to_argb).
    void set_view_background(JNIEnv* env, jobject widget, const maui::graphics::paint* paint)
    {
        jmethodID set_background_color =
            default_jni_cache().method(env, k_image_view_class, "setBackgroundColor", "(I)V");
        if (set_background_color == nullptr)
        {
            return;
        }
        jint argb = k_transparent_argb;
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(paint))
        {
            argb = static_cast<jint>(solid->color().to_int());
        }
        env->CallVoidMethod(widget, set_background_color, argb);
        clear_pending(env);
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

    // ---- clip: i_shape → android.graphics.Path (the WrapperView.SetClip / CAShapeLayer-mask analog) ----
    // The view's onDraw (MauiImageView.onDraw) clips the bitmap draw to a native Path the handler installs
    // via setClipPath. The Path is built here from the clip geometry resolved against the view's CURRENT
    // bounds, in the View's PIXEL coordinate space — onDraw applies no density scale (super.onDraw blits the
    // bitmap in raw view pixels), unlike android_canvas which scales the Canvas by density and feeds points.
    // So this builder emits pixel coordinates: it resolves path_for_bounds against the bounds in POINTS, then
    // multiplies every coordinate by `density` as it walks the path into the android.graphics.Path.

    // Build `path` (a path_f in POINT coordinates) into a fresh android.graphics.Path scaled to PIXELS by
    // `density`. Empty local ref on any JNI failure. The walk mirrors android_canvas::build_path 1:1
    // (move/line/quad/cubic/arc/close) — duplicated, not shared, so the image partial stays independently
    // buildable (the same doctrine the box/border partials' corner_radii_of follow). Arc mapping matches
    // android_canvas exactly: android's arcTo angles are clockwise from +x and the framework's are
    // counter-clockwise, so the start is negated and the sweep sign-adjusted.
    [[nodiscard]] local_ref<jobject> build_clip_path(JNIEnv* env, const maui::graphics::path_f& path, float density)
    {
        auto& cache = default_jni_cache();
        jclass path_class = cache.find_class(env, k_path_class);
        jmethodID path_ctor = cache.method(env, k_path_class, "<init>", "()V");
        jmethodID move_to = cache.method(env, k_path_class, "moveTo", "(FF)V");
        jmethodID line_to = cache.method(env, k_path_class, "lineTo", "(FF)V");
        jmethodID quad_to = cache.method(env, k_path_class, "quadTo", "(FFFF)V");
        jmethodID cubic_to = cache.method(env, k_path_class, "cubicTo", "(FFFFFF)V");
        jmethodID close = cache.method(env, k_path_class, "close", "()V");
        jmethodID arc_to = cache.method(env, k_path_class, "arcTo", "(FFFFFFZ)V");
        if (path_class == nullptr || path_ctor == nullptr || move_to == nullptr || line_to == nullptr ||
            quad_to == nullptr || cubic_to == nullptr || close == nullptr)
        {
            return {};
        }
        local_ref<jobject> path_obj{env, env->NewObject(path_class, path_ctor)};
        if (clear_pending(env) || !path_obj)
        {
            return {};
        }
        const auto s = static_cast<jfloat>(density);
        int point_index = 0;
        int arc_angle_index = 0;
        int arc_clockwise_index = 0;
        const auto& operations = path.segment_types();
        for (const auto type : operations)
        {
            switch (type)
            {
                case maui::graphics::path_operation::move: {
                    const maui::graphics::point_f p = path[point_index++];
                    env->CallVoidMethod(path_obj.get(), move_to, p.x * s, p.y * s);
                    break;
                }
                case maui::graphics::path_operation::line: {
                    const maui::graphics::point_f p = path[point_index++];
                    env->CallVoidMethod(path_obj.get(), line_to, p.x * s, p.y * s);
                    break;
                }
                case maui::graphics::path_operation::quad: {
                    const maui::graphics::point_f control = path[point_index++];
                    const maui::graphics::point_f end = path[point_index++];
                    env->CallVoidMethod(path_obj.get(), quad_to, control.x * s, control.y * s, end.x * s, end.y * s);
                    break;
                }
                case maui::graphics::path_operation::cubic: {
                    const maui::graphics::point_f c1 = path[point_index++];
                    const maui::graphics::point_f c2 = path[point_index++];
                    const maui::graphics::point_f end = path[point_index++];
                    env->CallVoidMethod(path_obj.get(), cubic_to, c1.x * s, c1.y * s, c2.x * s, c2.y * s, end.x * s,
                                        end.y * s);
                    break;
                }
                case maui::graphics::path_operation::arc: {
                    const maui::graphics::point_f top_left = path[point_index++];
                    const maui::graphics::point_f bottom_right = path[point_index++];
                    const float start_angle = path.get_arc_angle(arc_angle_index++);
                    const float end_angle = path.get_arc_angle(arc_angle_index++);
                    const bool clockwise = path.get_arc_clockwise(arc_clockwise_index++);
                    // Path.arcTo(left,top,right,bottom,startAngle,sweepAngle,forceMoveTo) — the RectF-less
                    // overload (a Path method since API 21); the oval is the arc's bounding box in pixels.
                    float sweep = -(end_angle - start_angle);
                    if (!clockwise && sweep > 0)
                    {
                        sweep -= 360.0F;
                    }
                    else if (clockwise && sweep < 0)
                    {
                        sweep += 360.0F;
                    }
                    env->CallVoidMethod(path_obj.get(), arc_to, top_left.x * s, top_left.y * s, bottom_right.x * s,
                                        bottom_right.y * s, static_cast<jfloat>(-start_angle),
                                        static_cast<jfloat>(sweep), static_cast<jboolean>(false));
                    break;
                }
                case maui::graphics::path_operation::close:
                    env->CallVoidMethod(path_obj.get(), close);
                    break;
            }
            clear_pending(env);
        }
        return path_obj;
    }

    // Resolve the image's CURRENT pixel size (getWidth/getHeight). {0,0} before the first layout (the view
    // has not been measured/laid out yet) — install_clip skips building a path against a 0-sized view, the
    // same 0×0-at-map-time guard the iOS twin handles by re-framing from platform_arrange.
    struct pixel_size
    {
        jint width;
        jint height;
    };
    [[nodiscard]] pixel_size view_pixel_size(JNIEnv* env, jobject widget)
    {
        auto& cache = default_jni_cache();
        jmethodID get_width = cache.method(env, k_image_view_class, "getWidth", "()I");
        jmethodID get_height = cache.method(env, k_image_view_class, "getHeight", "()I");
        if (get_width == nullptr || get_height == nullptr)
        {
            return {.width = 0, .height = 0};
        }
        const jint w = env->CallIntMethod(widget, get_width);
        const jint h = env->CallIntMethod(widget, get_height);
        if (clear_pending(env))
        {
            return {.width = 0, .height = 0};
        }
        return {.width = w, .height = h};
    }

    // Install (or clear, when shape is null) the clip path on the MauiImageView. Resolves the geometry
    // against the view's live bounds (in points = pixels / density), builds the pixel-space Path, and calls
    // setClipPath(path) — or setClipPath(null) to remove the mask (the WrapperView.SetClip(null) analog).
    // Called from update_clip (the map-time push) AND platform_arrange (the bounds-dependent reapply — the
    // iOS reapply_clip twin: the geometry resolves against the live frame, so a resize must rebuild it).
    void install_clip(JNIEnv* env, jobject widget, const maui::graphics::i_shape* shape, float density)
    {
        jmethodID set_clip_path =
            default_jni_cache().method(env, k_image_view_class, "setClipPath", "(Landroid/graphics/Path;)V");
        if (set_clip_path == nullptr)
        {
            return;
        }
        if (shape == nullptr)
        {
            env->CallVoidMethod(widget, set_clip_path, static_cast<jobject>(nullptr)); // remove the mask
            clear_pending(env);
            return;
        }
        const pixel_size size = view_pixel_size(env, widget);
        if (size.width <= 0 || size.height <= 0 || !(density > 0))
        {
            return; // not laid out yet (0×0) — platform_arrange re-installs once the view has bounds
        }
        // WrapperView.SetClip resolves the geometry against RectF(0, 0, frame.Width, frame.Height) in POINTS;
        // the view's point-size is its pixel-size / density.
        const auto width_pt = static_cast<float>(size.width) / density;
        const auto height_pt = static_cast<float>(size.height) / density;
        const maui::graphics::path_f path = shape->path_for_bounds(maui::graphics::rect{0, 0, width_pt, height_pt});
        const local_ref<jobject> path_obj = build_clip_path(env, path, density);
        if (!path_obj)
        {
            return;
        }
        env->CallVoidMethod(widget, set_clip_path, path_obj.get());
        clear_pending(env);
    }

    // ImageHandler.Android.PlatformArrange's AspectFill center-crop clip
    // (PlatformInterop.IsImageViewCenterCrop → SetClipBounds). A CENTER_CROP (AspectFill) ImageView scales
    // the bitmap to COVER the view, so the drawable exceeds the view size in one dimension and — since a
    // MauiLayout host does NOT clip its children (setClipChildren(false), the ClipBounds=null default) — the
    // overflowing image draws OVER its siblings (a header StackLayout's caption/button rendered UNDER the
    // over-scaled Image; the header_footer_grid parity red). C# clips the draw to the view's OWN bounds via
    // View.setClipBounds(new Rect(0,0,w,h)) for a center-crop view, and clears it (setClipBounds(null)) for
    // every other aspect. `is_center_crop` is AspectFill (the ONLY aspect ImageViewExtensions maps to
    // CENTER_CROP). Sizes are the just-laid-out PIXEL bounds. No-op on any JNI failure.
    void set_center_crop_clip_bounds(JNIEnv* env, jobject widget, bool is_center_crop, jint width_px, jint height_px)
    {
        auto& cache = default_jni_cache();
        jmethodID set_clip_bounds =
            cache.method(env, k_image_view_class, "setClipBounds", "(Landroid/graphics/Rect;)V");
        if (set_clip_bounds == nullptr)
        {
            return;
        }
        if (!is_center_crop)
        {
            env->CallVoidMethod(widget, set_clip_bounds, static_cast<jobject>(nullptr)); // ClipBounds = null
            clear_pending(env);
            return;
        }
        jclass rect_class = cache.find_class(env, k_rect_class);
        jmethodID rect_ctor = cache.method(env, k_rect_class, "<init>", "(IIII)V");
        if (rect_class == nullptr || rect_ctor == nullptr)
        {
            return;
        }
        const local_ref<jobject> rect{env, env->NewObject(rect_class, rect_ctor, 0, 0, width_px, height_px)};
        if (clear_pending(env) || !rect)
        {
            return;
        }
        env->CallVoidMethod(widget, set_clip_bounds, rect.get());
        clear_pending(env);
    }

    // Quantize a measured size to WHOLE DEVICE PIXELS — what the native ImageView measure the intrinsic
    // fast-path replaced did implicitly, and the reason the port's auto-sized images ran up to 1px taller
    // than MAUI's.
    //
    // ONLY for the AdjustViewBounds case (a free axis derived from a fixed one, the other UNCONSTRAINED).
    // With both axes finite the image is fit into a bounded slot the parent already sized, and the
    // unquantized value is exactly right — the call sites gate on that.
    //
    // MAUI measures an Image on Android by asking the platform view: ViewHandlerExtensions
    // .GetDesiredSizeFromHandler → ImageView.onMeasure, which with AdjustViewBounds derives the free axis
    // in PIXELS and ROUNDS ((int)(size / aspect + 0.5f), View.resolveAdjustedSize), then FromPixels() takes
    // that whole-pixel value back to dp. The port's fast path computes the aspect fit in dp and never
    // quantizes, so the fractional remainder survives to platform_arrange — whose to_pixels CEILS
    // (ContextExtensions.ToPixels) and turns any fraction into a whole extra pixel.
    //
    // Measured on the image page at density 2.75 with a 368.727dp content width:
    //   campus.jpg    788x399  → 1014*399/788 = 513.42px → MAUI round 513, port ceil 514
    //   dotnet_bot.png 1200x694 → 1014*694/1200 = 586.43px → MAUI round 586, port ceil 587
    // The extra pixel both mis-sized the image and pushed every row below it 1px down.
    //
    // Rounding here (before the dp value ever reaches to_pixels) reproduces MAUI's whole-pixel measure
    // exactly: round(dp * density) / density lands on a dp value to_pixels maps back to the same pixel.
    // Degrades to the identity when the density cannot be read (no VM / no widget → display_density's 1.0
    // fallback would quantize to whole dp, which is NOT the native behaviour), so the VM-less cross-platform
    // suite is untouched.
    [[nodiscard]] maui::graphics::size snap_to_device_pixels(maui::graphics::size value,
                                                             const maui::core::image_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return value; // VM-less mirror: no display to quantize against
        }
        const scoped_env env;
        if (!env)
        {
            return value;
        }
        const float density = display_density(env.get(), widget_of(platform));
        if (!(density > 0.0F))
        {
            return value;
        }
        const auto snap = [density](double dp) {
            return std::round(dp * static_cast<double>(density)) / static_cast<double>(density);
        };
        return {snap(value.width), snap(value.height)};
    }

    // ---- the async uri fetch (the loader's uri_fetch seam; apple's fetch_uri_async twin) ----------------
    // C# UriImageSourceService.Android hands the download to Glide (PlatformInterop.LoadImageFromUri), which
    // fetches OFF the UI thread and delivers back ON it. With no Glide AAR the port does the same shape over
    // dev.mauicpp.MauiUriFetch: HttpURLConnection on a worker thread, then a main-looper post into
    // native_uri_fetched — so the loader's decode + setImageBitmap run on the UI thread and the NETWORK
    // never touches it.
    constexpr const char* k_uri_fetch_class = "dev/mauicpp/MauiUriFetch";

    // The loader's byte sink plus its own copy of the cancellation token, heap-owned across the round trip
    // to Java (the token is copied, not referenced — the loader's const& parameter does not outlive the
    // call). Handed over as the opaque `peer` jlong; native_uri_fetched takes ownership back and frees it.
    struct uri_fetch_state
    {
        maui::core::image_source_loader::uri_bytes_sink sink;
        maui::core::cancellation_token token;
    };

    // The native half of dev.mauicpp.MauiUriFetch.nativeUriFetched(long, byte[]), running on the MAIN
    // looper. Takes the state back (unique_ptr = delivered exactly once, freed however this returns) and
    // reports the downloaded bytes to the loader's sink — empty on a failed download OR a superseded load
    // (the token check mirrors the apple completion handler), which the loader reads as "nothing loaded".
    void JNICALL native_uri_fetched(JNIEnv* env, jclass /*fetch_class*/, jlong peer, jbyteArray data)
    {
        // The canonical JNI peer pattern: the Java side stores the pointer the C++ side handed it.
        const std::unique_ptr<uri_fetch_state> state{reinterpret_cast<uri_fetch_state*>(peer)};
        if (!state || !state->sink)
        {
            return;
        }
        maui::core::image_bytes bytes;
        if (data != nullptr && !state->token.is_cancelled())
        {
            const jsize len = env->GetArrayLength(data);
            if (!clear_pending(env) && len > 0)
            {
                std::vector<jbyte> staging(static_cast<std::size_t>(len));
                env->GetByteArrayRegion(data, 0, len, staging.data());
                if (!clear_pending(env))
                {
                    bytes.resize(static_cast<std::size_t>(len));
                    for (std::size_t i = 0; i < bytes.size(); ++i)
                    {
                        bytes[i] = static_cast<std::byte>(static_cast<unsigned char>(staging[i]));
                    }
                }
            }
        }
        state->sink(std::move(bytes));
    }

    // Binds nativeUriFetched to MauiUriFetch (RegisterNatives — no Java_* export needed). Idempotent, so
    // every fetch can call it without once-flag coordination (the MauiDialogBridge recipe).
    [[nodiscard]] bool register_uri_fetch_natives(JNIEnv* env, jclass fetch_class)
    {
        // JNINativeMethod's name/signature members are non-const char* and fnPtr is a void* for historical
        // JNI-spec reasons — the const_casts/reinterpret_cast are the API's own shape.
        static const std::array<JNINativeMethod, 1> k_methods{
            JNINativeMethod{.name = const_cast<char*>("nativeUriFetched"),
                            .signature = const_cast<char*>("(J[B)V"),
                            .fnPtr = reinterpret_cast<void*>(&native_uri_fetched)},
        };
        const jint status = env->RegisterNatives(fetch_class, k_methods.data(), static_cast<jint>(k_methods.size()));
        if (status != JNI_OK)
        {
            clear_pending(env);
            return false;
        }
        return true;
    }

    // Context.getCacheDir().getAbsolutePath() — the android twin of apple's platform_cache_directory()
    // (NSCachesDirectory). Empty string when there is no Context (the VM-less cross-platform suite) or any
    // JNI step fails; configure_loader then leaves the loader's disk layer off rather than guessing a path.
    // Memoized: the walk is four JNI calls and the answer cannot change within a process.
    [[nodiscard]] std::string platform_cache_directory()
    {
        static const std::string memoized = [] {
            const scoped_env env;
            jobject context = maui::platform::android::app_context();
            if (!env || context == nullptr)
            {
                return std::string{};
            }
            auto& cache = default_jni_cache();
            jmethodID get_cache_dir =
                cache.method(env.get(), "android/content/Context", "getCacheDir", "()Ljava/io/File;");
            jmethodID get_absolute_path =
                cache.method(env.get(), "java/io/File", "getAbsolutePath", "()Ljava/lang/String;");
            if (get_cache_dir == nullptr || get_absolute_path == nullptr)
            {
                return std::string{};
            }
            const local_ref<jobject> dir{env.get(), env->CallObjectMethod(context, get_cache_dir)};
            if (clear_pending(env.get()) || !dir)
            {
                return std::string{};
            }
            const local_ref<jobject> path{env.get(), env->CallObjectMethod(dir.get(), get_absolute_path)};
            if (clear_pending(env.get()) || !path)
            {
                return std::string{};
            }
            return maui::platform::android::to_utf8(env.get(), static_cast<jstring>(path.get()));
        }();
        return memoized;
    }

    // The image_source_loader::uri_fetch seam installed in configure_loader. A non-http scheme (file:// or a
    // bare path) reads synchronously — the loader's own default. An http(s) uri is dispatched to the Java
    // worker; the sink then fires later, on the main looper. EVERY path delivers to the sink exactly once:
    // when there is no VM / the Java class is missing (the widget test host dexes it, but a VM-less or
    // Activity-less host may not reach it) the sink gets empty bytes immediately, so a load never hangs.
    void fetch_uri_async(const std::string& uri, const maui::core::cancellation_token& token,
                         maui::core::image_source_loader::uri_bytes_sink sink)
    {
        if (!uri.starts_with("http://") && !uri.starts_with("https://"))
        {
            sink(maui::core::read_uri_bytes(uri));
            return;
        }
        const scoped_env env;
        if (!env)
        {
            sink(maui::core::image_bytes{}); // VM-less: nothing to fetch with
            return;
        }
        auto& cache = default_jni_cache();
        jclass fetch_class = cache.find_class(env.get(), k_uri_fetch_class);
        jmethodID fetch = cache.static_method(env.get(), k_uri_fetch_class, "fetch", "(JLjava/lang/String;)V");
        const local_ref<jstring> jurl = maui::platform::android::to_jstring(env.get(), uri);
        if (fetch_class == nullptr || fetch == nullptr || !jurl || !register_uri_fetch_natives(env.get(), fetch_class))
        {
            sink(maui::core::image_bytes{});
            return;
        }
        auto state = std::make_unique<uri_fetch_state>(uri_fetch_state{.sink = std::move(sink), .token = token});
        env->CallStaticVoidMethod(fetch_class, fetch, reinterpret_cast<jlong>(state.get()), jurl.get());
        if (clear_pending(env.get()))
        {
            state->sink(maui::core::image_bytes{}); // the dispatch threw — deliver here, then free
            return;
        }
        // Ownership crossed into Java: release the local owner so native_uri_fetched — which adopts the
        // peer back into a unique_ptr — is the single deleter. The released pointer IS the peer Java holds;
        // nothing more to do with it here.
        [[maybe_unused]] const uri_fetch_state* const owned_by_java = state.release();
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

    // ViewMapper map_clip → WrapperView.SetClip: mask the MauiImageView's onDraw to the clip geometry. The
    // shared view_mapper drives this through view_platform_base::update_clip (view_mapper.cpp). The base body
    // runs FIRST (the headless mirror — view_platform_base::clip — must stay live for the VM-less
    // cross-platform suite), then the native push installs the android.graphics.Path. The geometry resolves
    // against the view's LIVE bounds (0×0 before the first layout — platform_arrange re-installs it then, the
    // iOS reapply_clip analog), so the borrow is stashed in clip_shape for that bounds-dependent reapply.
    // Wave 11: this is the one generic-IView push wired for the image on android (header note); the rest keep
    // only the base mirror.
    void image_platform::update_clip(const maui::graphics::i_shape* value)
    {
        view_platform_base::update_clip(value); // headless mirror first (the VM-less suite observes it)
        clip_shape = value;                     // the borrow platform_arrange re-resolves against live bounds
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
        install_clip(env.get(), widget, value, display_density(env.get(), widget));
    }

    // ViewMapper map_background → ViewHandler.UpdateBackground: paint the MauiImageView's background with the
    // VisualElement.Background paint (the clip/clip_gallery pages' LightGray frame). The shared view_mapper
    // drives this through view_platform_base::update_background. The base body runs FIRST (the headless mirror
    // must stay live for the VM-less cross-platform suite), then the native push sets the View's background
    // color for a solid_paint (a gradient/tiled brush is deferred — header note). Mirrors the button partial's
    // update_background structure (base mirror first, then the native push).
    void image_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value); // headless mirror first (the VM-less suite observes it)
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        set_view_background(env.get(), widget_of(*this), value);
    }

    // ViewMapper map_opacity → ViewExtensions.UpdateOpacity: platformView.Alpha = (float)opacity. The base
    // body runs FIRST (the headless mirror must stay live for the VM-less cross-platform suite), then the
    // native push sets the MauiImageView's alpha. Mirrors the button partial's update_opacity structure —
    // the image page fades its Opacity=0.5 rows, so this is the largest light-mode parity contributor.
    void image_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value); // headless mirror first (the VM-less suite observes it)
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

    // Install the async http(s) fetch seam (the android twin of apple's NSURLSession dataTask install).
    // No i_dispatcher is set: fetch_uri_async already marshals its completion onto the MAIN looper, so the
    // loader's apply runs inline on the UI thread where the ImageView lives — the main-looper hop IS the
    // dispatcher hand-off, exactly as the apple partial documents.
    //
    // THE ON-DISK CACHE IS ON, and its absence was a real parity gap rather than a refinement. MAUI reaches
    // Glide, which caches decoded uri bytes to disk and therefore renders a remote photo on the FIRST frame
    // of every launch after the first. Without a disk layer the port re-downloaded on every launch, and the
    // in-memory CacheValidity layer cannot help because it dies with the process. Measured on the image
    // page (Source="https://aka.ms/campus.jpg", 152307 bytes): the fetch itself always succeeded, but on a
    // COLD network it landed after the parity capture's 4s settle, so the board banked a frame with no
    // photo and everything below it shifted up — 70.00% differing pixels in light, scored as a port defect.
    // Warm, the same binary renders it inside 4s. A race the reference does not have is a gap, not noise.
    //
    // The directory is the Context's own cache dir (Context.getCacheDir()), the closest analog of the
    // FileSystem.CacheDirectory path C# hands Glide; uri_image_disk_cache appends its own "MauiUriImages"
    // subdirectory, exactly as on apple. An unavailable Context leaves the directory empty, which is the
    // loader's documented "disk layer off" state — the same degradation as before this change.
    void image_handler::configure_loader(image_source_loader& loader)
    {
        if (std::string directory = platform_cache_directory(); !directory.empty())
        {
            loader.set_disk_cache_directory(std::move(directory));
        }
        loader.set_uri_fetch(&fetch_uri_async);
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
        // Prefer the density-appropriate @2x/@3x variant when packaged (iOS parity) so the bitmap isn't
        // upscale-blurred at the device density; resolved.scale divides the pixel intrinsic back to logical dp.
        const float density = display_density(env.get(), widget_of(platform));
        const image_decode::scaled_bytes resolved =
            image_decode::resolve_scaled_file_bytes(env.get(), file_src.file(), density);
        const local_ref<jobject> bitmap = decode_bitmap(env.get(), resolved.bytes);
        if (!bitmap)
        {
            // A from_file naming an asset that is not packaged (e.g. cog.png — SVG-only, never rasterized) or
            // a failed decode leaves the view image-less, mirroring the iOS nil-decode: no pixels, no crash.
            set_image_bitmap(env.get(), widget_of(platform), nullptr);
            return;
        }
        set_image_bitmap(env.get(), widget_of(platform), bitmap.get());
        const bitmap_size size = bitmap_intrinsic_size(env.get(), bitmap.get());
        // A @2x/@3x asset carries N× the pixels for the same logical size — divide so the intrinsic stays dp.
        platform.intrinsic_width = size.width / resolved.scale;
        platform.intrinsic_height = size.height / resolved.scale;
    }

    // The async loader's apply: copy the result's kind + detail mirror, then — when the result carries a
    // native image (image_source_services' decode_image_bytes hands back a global-ref
    // android.graphics.Bitmap for a uri download / an in-memory stream, and the FONT rasterizer one for a
    // glyph) — push it into the ImageView via setImageBitmap (the android twin of apple's
    // imageView.image = result.image()). A !loaded() result clears the view, mirroring SetImageSource(null)
    // / ImageViewExtensions.Clear. The loader runs INLINE on the UI thread — the uri fetch marshals its
    // completion through the main looper before this apply, so setImageBitmap is always UI-thread-safe.
    //
    // INTRINSIC SIZE: recorded exactly as the FILE fast-path does (get_desired_size then aspect-fits the
    // real content size, the iOS SizeThatFitsImage analog) — a remote photo has no @Nx density variant, so
    // its decoded pixels ARE the intrinsic. NOT for a FONT result: those keep {0,0} so the FONT branch of
    // get_desired_size (returns {size,size}) stays authoritative and the ImageView AspectFit-centers the
    // glyph at MAUI's measured band height. is_resolution_dependent() is the font marker (C#
    // FontImageSourceService passes true; every other service false).
    //
    // RELAYOUT: an async result lands AFTER the host's one-shot layout pass, so growing the intrinsic from
    // {0,0} changes this view's desired size with nothing to re-measure the tree — MAUI gets that for free
    // because LayoutViewGroup.OnMeasure re-runs CrossPlatformMeasure on the traversal Glide's
    // SetImageDrawable triggers. request_relayout() is the port's equivalent seam: it replays the host's
    // own layout pass (see jni/relayout.hpp), and is a NO-OP when no host installed one (the widget test
    // host, the VM-less cross-platform suite). Gated on the intrinsic actually CHANGING, so a synchronous
    // cache-hit apply during the mount — and every font/mirror result — costs nothing.
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
        if (platform.native == nullptr || result.image() == nullptr)
        {
            return; // VM-less / a mirror-only result (an undecodable payload, or the font fallback)
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        auto* const bitmap = static_cast<jobject>(result.image());
        set_image_bitmap(env.get(), widget_of(platform), bitmap);
        const bitmap_size size = bitmap_intrinsic_size(env.get(), bitmap);
        if (result.is_resolution_dependent())
        {
            // FONT: the glyph IS rasterized here (image_source_services.cpp's rasterize_glyph ports
            // FontModelResourceDecoder.decode 1:1), so its bitmap is the intrinsic exactly as it is for
            // MAUI — FontImageSourceService.Android hands Glide a bitmap and ImageView.onMeasure sizes to
            // the drawable, with nothing anywhere reading the font Size as a measurement. The band is
            // TALLER than the font size because the decoder's box is round(-ascent) + round(descent), not
            // textSize: measured on device for context_flyout's Size=50 🆒 glyph at density 2.75,
            // textPx=137.50 asc=-127.56 desc=33.57 -> a 171x162 px bitmap, and MAUI renders that Image
            // 162 px tall while the port's font-Size shortcut rendered it 138 (= textPx). Rasterized at
            // the DEVICE density, so divide by it to reach the dp the intrinsic is expressed in — the
            // same "px / asset scale" the file fast path applies above.
            const float density = display_density(env.get(), widget_of(platform));
            if (density > 0.0F)
            {
                platform.intrinsic_width = size.width / static_cast<double>(density);
                platform.intrinsic_height = size.height / static_cast<double>(density);
                maui::platform::android::request_relayout();
            }
            return;
        }
        if (size.width == platform.intrinsic_width && size.height == platform.intrinsic_height)
        {
            return; // same content size — the existing layout already fits it
        }
        platform.intrinsic_width = size.width;
        platform.intrinsic_height = size.height;
        maui::platform::android::request_relayout();
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
        // FONT image source, WITH NO RASTERIZED GLYPH (an empty/unresolvable glyph, or a VM-less host): fall
        // back to the font Size as the band height so the Image still occupies MAUI's approximate row instead
        // of collapsing. When the glyph DID rasterize, the intrinsic fast path below owns the measurement —
        // that is what MAUI does (the ImageView measures the rasterized drawable; nothing in
        // FontImageSourceService.Android reads Size back as a size), and the two disagree: the decoder's box
        // is round(-ascent)+round(descent), which for context_flyout's Size=50 glyph is 162 px against a
        // textPx of 137.5. This branch used to run unconditionally and reported {50,50} dp, rendering that
        // Image 138 px tall where MAUI renders 162. Read straight off the virtual view: map_source clears only
        // the platform mirror when the glyph is empty, NOT the view's Source, so font().size() is reachable.
        if (platform->intrinsic_width <= 0.0 || platform->intrinsic_height <= 0.0)
        {
            if (const auto* view = virtual_view())
            {
                if (const auto* font_src = dynamic_cast<const maui::core::i_font_image_source*>(view->source()))
                {
                    if (const double sz = font_src->font().size(); sz > 0.0)
                    {
                        return {sz, sz};
                    }
                }
            }
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
            // Single-axis aspect-fit (MAUI's Android ImageView AdjustViewBounds): when ONE of WidthRequest/
            // HeightRequest is set (EXACTLY) and the other is auto, derive the free axis from the fixed one,
            // preserving the decoded aspect. The shrink-to-constraint below never re-derives the free axis
            // from an EXACT request, so e.g. the WidthRequest=200 400x300 animated_heart.gif measured 112dp
            // tall instead of MAUI's 200*(300/400)=150dp, shifting every image row below it. (Mirrors the
            // image_button_handler fix; a plain Image has no padding.)
            // Single-axis aspect-fit ONLY when the free axis is UNCONSTRAINED (infinite) — the exact
            // WidthRequest-image-in-a-VerticalStackLayout case (e.g. the WidthRequest=200 400x300
            // animated_heart.gif, whose parent passes infinite height): derive the free axis from the exact
            // request, preserving the decoded aspect. When the free axis HAS a finite constraint (a bounded
            // parent such as a Grid cell), the shrink-to-constraint logic below already fits within both axes
            // and matches MAUI — do NOT override it (that regressed header_footer_template's photo_cell).
            if (platform->image_aspect == aspect::aspect_fit)
            {
                if (const auto* view = virtual_view())
                {
                    const double req_w = view->width();  // WidthRequest (NaN when unset)
                    const double req_h = view->height(); // HeightRequest (NaN when unset)
                    if (!std::isnan(req_w) && std::isnan(req_h) && !std::isfinite(height_constraint))
                    {
                        return snap_to_device_pixels({req_w, req_w * (h / w)}, *platform);
                    }
                    if (!std::isnan(req_h) && std::isnan(req_w) && !std::isfinite(width_constraint))
                    {
                        return snap_to_device_pixels({req_h * (w / h), req_h}, *platform);
                    }
                }
            }
            double scale = 1.0;
            if (std::isfinite(width_constraint) && width_constraint < w)
            {
                scale = std::min(scale, width_constraint / w);
            }
            if (std::isfinite(height_constraint) && height_constraint < h * scale)
            {
                scale = std::min(scale, height_constraint / h);
            }
            // Whole-pixel quantization ONLY when an axis is UNCONSTRAINED — see snap_to_device_pixels. That
            // is the case this fast path stands in for: ImageView.onMeasure deriving the free axis from the
            // fixed one with AdjustViewBounds, in pixels, rounded. With BOTH axes finite the image sits in a
            // bounded slot (a Grid cell) whose geometry — not the ImageView's own rounding — fixes the frame,
            // and the unquantized fit already matches MAUI exactly; snapping there moved header_footer_
            // template's photo cells off a pixel-perfect match (measured 0.00% -> 0.25%).
            if (std::isfinite(width_constraint) && std::isfinite(height_constraint))
            {
                return {w * scale, h * scale};
            }
            return snap_to_device_pixels({w * scale, h * scale}, *platform);
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
        // AspectFill center-crop clip (ImageHandler.Android.PlatformArrange → IsImageViewCenterCrop /
        // SetClipBounds): a CENTER_CROP ImageView over-scales its bitmap to COVER the view, so the drawable
        // exceeds the view bounds in one axis; clip the draw to the view's own pixel rect so it does not spill
        // over its siblings (the header_footer_grid stack's caption/button were rendered UNDER the over-scaled
        // 60dp header Image). Cleared (ClipBounds = null) for every other aspect. Mirrors the C# order (clip
        // set/cleared as part of arrange).
        set_center_crop_clip_bounds(env.get(), widget, platform->image_aspect == aspect::aspect_fill, width, height);
        // Re-install the clip mask against the just-laid-out bounds (the iOS reapply_clip analog): the clip
        // geometry resolves against the live frame, and update_clip may have run before the first layout when
        // the view was 0×0 (install_clip skipped it then). A resize likewise lands here, so the Path tracks
        // the new size. Only when a clip is stashed — an unclipped image never needs a re-mask (reapply_clip
        // likewise early-returns when nothing is stashed); the null-clear path is owned by update_clip.
        if (platform->clip_shape != nullptr)
        {
            install_clip(env.get(), widget, platform->clip_shape, density);
        }
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
