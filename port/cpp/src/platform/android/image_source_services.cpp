// Per-source image services + decode_image_bytes — ANDROID (JNI) backend. The real-native twin of
// src/platform/headless/image_source_services.cpp for the ONE source kind android decodes here: FONT.
// One TU implements all four services + the shared decode primitive (the per-backend partial-class split,
// PROFILE §5), exactly like the headless/apple twins.
//
//   - file / uri / stream — MIRROR ONLY, copied VERBATIM from the headless bodies. They carry no native
//     image handle on android: the FILE fast-path decodes into the ImageView in image_handler.cpp
//     (load_file_source_sync + BitmapFactory), and uri/stream still await the deferred android image
//     pipeline (no Glide AAR), so their result stays the headless kind/detail mirror the VM-less
//     cross-platform suite observes. A slip here (e.g. dropping the mirror) regresses every image page.
//
//   - font — the one real rasterizer, porting FontModelResourceDecoder.decode() +
//     FontImageSourceService.Android.cs over JNI (all stock android.graphics, ZERO AAR). Resolve the
//     Typeface (registrar-resolved asset → Typeface.createFromAsset, else the Typeface.create system-font
//     fallback), compute the sp/dip text size via TypedValue.applyDimension against the Context's
//     DisplayMetrics, then draw the glyph into an ARGB_8888 android.graphics.Bitmap with an anti-aliased
//     Paint (measureText/ascent/descent size the box, Canvas.drawText paints it). The Bitmap is handed
//     back as a process-wide global reference in the image_source_result; image_handler.cpp's
//     apply_loaded_result pushes it into the MauiImageView via setImageBitmap (the android twin of apple's
//     imageView.image = result.image()). Font results are RESOLUTION-DEPENDENT (C# passes true).
//
// VM-less degradation (the pure-native cross-platform suite runs on the emulator WITHOUT a JavaVM, and the
// image_seam tests assert a font source is loaded() + is_resolution_dependent()): when there is no VM /
// Context OR any JNI step fails, the font load returns the headless MIRROR result (image=null, kind="font",
// glyph, resolution_dependent=true) — the same body the headless partial produces — so that suite still
// observes the load and the app host degrades to a blank glyph rather than a crash. Only when a VM + a
// Context exist AND every JNI step succeeds does the real Bitmap result replace it.
//
// DOCUMENTED DEVIATION (color): C# uses (fontImageSource.Color ?? Colors.White). The port's color is a
// concrete value type with no null branch (the gallery's FontImageSource rows pass Colors.White
// explicitly), so — exactly like the ios/apple twins — the color is used directly; color.to_int() packs
// 0xAARRGGBB, which is what android.graphics.Paint.setColor(int) expects.

#include "maui/core/file_image_source_service.hpp"
#include "maui/core/font_image_source_service.hpp"
#include "maui/core/stream_image_source_service.hpp"
#include "maui/core/uri_image_source_service.hpp"

#include <jni.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/font.hpp"
#include "maui/core/font_registrar.hpp"
#include "maui/core/i_font_image_source.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/i_uri_image_source.hpp"
#include "maui/core/image_decode.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/uri_bytes.hpp"
#include "maui/graphics/color.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;
    using maui::platform::android::to_jstring;

    constexpr const char* k_typeface_class = "android/graphics/Typeface";
    constexpr const char* k_typed_value_class = "android/util/TypedValue";
    constexpr const char* k_paint_class = "android/graphics/Paint";
    constexpr const char* k_bitmap_class = "android/graphics/Bitmap";
    constexpr const char* k_bitmap_config_class = "android/graphics/Bitmap$Config";
    constexpr const char* k_canvas_class = "android/graphics/Canvas";

    // android.graphics.Typeface styles (FontManager.ToTypefaceStyle's targets).
    constexpr jint k_typeface_normal = 0;
    constexpr jint k_typeface_bold = 1;
    constexpr jint k_typeface_italic = 2;

    // android.util.TypedValue complex unit types (FontManager.GetFontSize's units).
    constexpr jint k_complex_unit_dip = 1;
    constexpr jint k_complex_unit_sp = 2;

    // FontManager.Android DefaultFontSize (14sp): the size used when Font.Size is <= 0 / NaN.
    constexpr float k_default_font_size = 14.0F;

    // Clears any pending Java exception (the service must never leak JNI pending-exception state into the
    // cross-platform layer); true when one was pending — call sites skip the read-back. Twin of the same
    // helper in image_handler.cpp (duplicated, not shared, so the two TUs stay independently buildable).
    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe(); // logcat/stderr breadcrumb, same channel the handler uses
        env->ExceptionClear();
        return true;
    }

    // ASCII lowercasing for the family→asset fallback (FontManager keys are ordinal-case-insensitive).
    [[nodiscard]] std::string to_lower(std::string_view value)
    {
        std::string out(value);
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    // Typeface.createFromAsset(context.getAssets(), asset), falling back to Typeface.create(family, style)
    // — a system-font family (e.g. "Arial") or the default typeface. Empty local ref only if even the
    // create fallback fails. `asset` is the registrar-resolved filename (else lowercase(family)+".ttf").
    [[nodiscard]] local_ref<jobject> create_typeface(JNIEnv* env, const std::string& family, const std::string& asset,
                                                     jint style)
    {
        auto& cache = default_jni_cache();
        jclass tf_class = cache.find_class(env, k_typeface_class);
        if (tf_class == nullptr)
        {
            return {};
        }
        // Preferred: the bundled/registered .ttf from assets/ (the Ionicons row → ionicons.ttf).
        jmethodID from_asset =
            cache.static_method(env, k_typeface_class, "createFromAsset",
                                "(Landroid/content/res/AssetManager;Ljava/lang/String;)Landroid/graphics/Typeface;");
        jobject context = app_context();
        jmethodID get_assets =
            cache.method(env, "android/content/Context", "getAssets", "()Landroid/content/res/AssetManager;");
        if (from_asset != nullptr && context != nullptr && get_assets != nullptr)
        {
            const local_ref<jobject> assets{env, env->CallObjectMethod(context, get_assets)};
            if (!clear_pending(env) && assets)
            {
                const local_ref<jstring> jasset = to_jstring(env, asset);
                local_ref<jobject> typeface{
                    env, env->CallStaticObjectMethod(tf_class, from_asset, assets.get(), jasset.get())};
                if (!clear_pending(env) && typeface) // a missing asset throws → cleared → fall through
                {
                    return typeface;
                }
            }
        }
        // Fallback: Typeface.create(family, style). A null family (empty) yields the default typeface.
        jmethodID create =
            cache.static_method(env, k_typeface_class, "create", "(Ljava/lang/String;I)Landroid/graphics/Typeface;");
        if (create == nullptr)
        {
            return {};
        }
        local_ref<jstring> jfamily; // empty family() ⇒ C# null Family ⇒ Typeface.create(null, …) = default
        if (!family.empty())
        {
            jfamily = to_jstring(env, family);
        }
        local_ref<jobject> typeface{env, env->CallStaticObjectMethod(tf_class, create, jfamily.get(), style)};
        if (clear_pending(env))
        {
            return {};
        }
        return typeface;
    }

    // Resolve + memoize the Typeface for a family/style. createFromAsset re-reads the whole .ttf, so the
    // resolved Typeface is pinned once per asset name in a process-lifetime global reference. The map is a
    // deliberate never-destroyed leak (the jni_cache doctrine): a static destructor DeleteGlobalRef'ing at
    // process exit would race VM shutdown. Returns a BORROWED jobject (owned by the map), or nullptr if even
    // the create fallback failed (then the rasterizer paints with the Paint's default typeface).
    [[nodiscard]] jobject typeface_for(JNIEnv* env, const std::string& family, jint style)
    {
        std::string asset = maui::core::default_font_registrar().get_font(family);
        if (asset.empty())
        {
            asset = to_lower(family) + ".ttf";
        }
        static std::mutex& mutex = *new std::mutex();                         // leaked on purpose (doctrine)
        static auto& cache = *new std::unordered_map<std::string, jobject>(); // leaked on purpose (doctrine)
        const std::scoped_lock lock(mutex);
        if (const auto memoized = cache.find(asset); memoized != cache.end())
        {
            return memoized->second;
        }
        const local_ref<jobject> resolved = create_typeface(env, family, asset, style);
        jobject pinned = resolved ? env->NewGlobalRef(resolved.get()) : nullptr;
        cache.emplace(asset, pinned); // cache the miss (nullptr) too — a bundled asset never appears later
        return pinned;
    }

    // TypedValue.applyDimension(unit, size, context.getResources().getDisplayMetrics()) → pixels. The same
    // getResources→getDisplayMetrics walk image_handler.cpp::display_density does, but off the process
    // Context directly (no widget here). 0 on any failure (the caller then degrades to the mirror).
    [[nodiscard]] float apply_dimension(JNIEnv* env, jint unit, float size)
    {
        jobject context = app_context();
        if (context == nullptr)
        {
            return 0.0F;
        }
        auto& cache = default_jni_cache();
        jmethodID get_resources =
            cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
        jmethodID get_metrics =
            cache.method(env, "android/content/res/Resources", "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
        jmethodID apply =
            cache.static_method(env, k_typed_value_class, "applyDimension", "(IFLandroid/util/DisplayMetrics;)F");
        jclass typed_value_class = cache.find_class(env, k_typed_value_class);
        if (get_resources == nullptr || get_metrics == nullptr || apply == nullptr || typed_value_class == nullptr)
        {
            return 0.0F;
        }
        const local_ref<jobject> resources{env, env->CallObjectMethod(context, get_resources)};
        if (clear_pending(env) || !resources)
        {
            return 0.0F;
        }
        const local_ref<jobject> metrics{env, env->CallObjectMethod(resources.get(), get_metrics)};
        if (clear_pending(env) || !metrics)
        {
            return 0.0F;
        }
        const jfloat pixels =
            env->CallStaticFloatMethod(typed_value_class, apply, unit, static_cast<jfloat>(size), metrics.get());
        if (clear_pending(env))
        {
            return 0.0F;
        }
        return pixels;
    }

    // Draw `glyph` at `text_px` in `typeface`/`color_argb` into a fresh ARGB_8888 Bitmap sized to the
    // glyph's measured box — ports FontModelResourceDecoder.decode 1:1: a new anti-aliased Paint, then
    // width = round(measureText), baseline = round(-ascent), height = round(baseline + descent), a
    // createBitmap(width, height, ARGB_8888), and Canvas.drawText(glyph, 0, baseline, paint). Empty local
    // ref on any JNI failure / a zero-size glyph (createBitmap throws on a 0 dimension).
    [[nodiscard]] local_ref<jobject> rasterize_glyph(JNIEnv* env, const std::string& glyph, jobject typeface,
                                                     float text_px, jint color_argb)
    {
        auto& cache = default_jni_cache();
        jclass paint_class = cache.find_class(env, k_paint_class);
        jmethodID paint_ctor = cache.method(env, k_paint_class, "<init>", "()V");
        jmethodID set_text_size = cache.method(env, k_paint_class, "setTextSize", "(F)V");
        jmethodID set_anti_alias = cache.method(env, k_paint_class, "setAntiAlias", "(Z)V");
        jmethodID set_color = cache.method(env, k_paint_class, "setColor", "(I)V");
        jmethodID set_typeface =
            cache.method(env, k_paint_class, "setTypeface", "(Landroid/graphics/Typeface;)Landroid/graphics/Typeface;");
        jmethodID measure_text = cache.method(env, k_paint_class, "measureText", "(Ljava/lang/String;)F");
        jmethodID ascent = cache.method(env, k_paint_class, "ascent", "()F");
        jmethodID descent = cache.method(env, k_paint_class, "descent", "()F");
        if (paint_class == nullptr || paint_ctor == nullptr || set_text_size == nullptr || set_anti_alias == nullptr ||
            set_color == nullptr || measure_text == nullptr || ascent == nullptr || descent == nullptr)
        {
            return {};
        }
        const local_ref<jobject> paint{env, env->NewObject(paint_class, paint_ctor)};
        if (clear_pending(env) || !paint)
        {
            return {};
        }
        env->CallVoidMethod(paint.get(), set_text_size, static_cast<jfloat>(text_px));
        env->CallVoidMethod(paint.get(), set_anti_alias, JNI_TRUE);
        env->CallVoidMethod(paint.get(), set_color, color_argb);
        // Paint's default Align IS LEFT, so the decoder's setTextAlign(Paint.Align.LEFT) is a no-op we skip
        // (avoids resolving the Paint$Align enum static field).
        if (typeface != nullptr && set_typeface != nullptr)
        {
            const local_ref<jobject> prev{env, env->CallObjectMethod(paint.get(), set_typeface, typeface)};
            clear_pending(env); // setTypeface returns the prior Typeface (a local ref) we discard
        }
        const local_ref<jstring> jglyph = to_jstring(env, glyph);
        if (!jglyph)
        {
            return {};
        }
        const jfloat measured = env->CallFloatMethod(paint.get(), measure_text, jglyph.get());
        if (clear_pending(env))
        {
            return {};
        }
        const jfloat asc = env->CallFloatMethod(paint.get(), ascent);
        if (clear_pending(env))
        {
            return {};
        }
        const jfloat desc = env->CallFloatMethod(paint.get(), descent);
        if (clear_pending(env))
        {
            return {};
        }
        // FontModelResourceDecoder: (int)(x + .5f) — round-half-up for these non-negative metrics.
        const jint width = static_cast<jint>(measured + 0.5F);
        const jint baseline = static_cast<jint>(-asc + 0.5F);
        const jint height = static_cast<jint>(static_cast<float>(baseline) + desc + 0.5F);
        if (width <= 0 || height <= 0)
        {
            return {}; // a 0-dimension bitmap throws IllegalArgumentException
        }
        jmethodID create_bitmap = cache.static_method(env, k_bitmap_class, "createBitmap",
                                                      "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        jclass bitmap_class = cache.find_class(env, k_bitmap_class);
        jclass config_class = cache.find_class(env, k_bitmap_config_class);
        if (create_bitmap == nullptr || bitmap_class == nullptr || config_class == nullptr)
        {
            return {};
        }
        // Bitmap.Config.ARGB_8888 (a static enum field; the jni_cache memoizes only instance fields, so the
        // static-field id is resolved directly through the pinned class).
        const jfieldID argb_field =
            env->GetStaticFieldID(config_class, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
        if (clear_pending(env) || argb_field == nullptr)
        {
            return {};
        }
        const local_ref<jobject> config{env, env->GetStaticObjectField(config_class, argb_field)};
        if (clear_pending(env) || !config)
        {
            return {};
        }
        local_ref<jobject> bitmap{
            env, env->CallStaticObjectMethod(bitmap_class, create_bitmap, width, height, config.get())};
        if (clear_pending(env) || !bitmap)
        {
            return {};
        }
        jclass canvas_class = cache.find_class(env, k_canvas_class);
        jmethodID canvas_ctor = cache.method(env, k_canvas_class, "<init>", "(Landroid/graphics/Bitmap;)V");
        jmethodID draw_text =
            cache.method(env, k_canvas_class, "drawText", "(Ljava/lang/String;FFLandroid/graphics/Paint;)V");
        if (canvas_class == nullptr || canvas_ctor == nullptr || draw_text == nullptr)
        {
            return {};
        }
        const local_ref<jobject> canvas{env, env->NewObject(canvas_class, canvas_ctor, bitmap.get())};
        if (clear_pending(env) || !canvas)
        {
            return {};
        }
        env->CallVoidMethod(canvas.get(), draw_text, jglyph.get(), static_cast<jfloat>(0),
                            static_cast<jfloat>(baseline), paint.get());
        if (clear_pending(env))
        {
            return {};
        }
        return bitmap;
    }
} // namespace

namespace maui::core
{
    // Android decode: no native image tree for uri/stream this cut — record the mirror (kind/detail),
    // loaded iff the byte buffer is non-empty. Copied VERBATIM from the headless twin (uri/stream stay
    // mirror-only on android until the deferred Glide pipeline lands).
    image_source_result decode_image_bytes(const image_bytes& bytes, std::string kind, std::string detail)
    {
        if (bytes.empty())
        {
            return {};
        }
        return image_source_result{nullptr, nullptr, std::move(kind), std::move(detail)};
    }

    // file — MIRROR ONLY (verbatim from the headless twin): the FILE fast-path decodes into the ImageView
    // in image_handler.cpp (load_file_source_sync), so the service keeps only the kind/detail mirror.
    void file_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* file_src = dynamic_cast<const i_file_image_source*>(&source);
        if (file_src == nullptr || file_src->is_empty())
        {
            on_result(image_source_result{}); // not a file source / empty → nothing loaded
            return;
        }
        // No native handle here: mirror kind="file" + the resolved path, marked loaded.
        on_result(image_source_result{nullptr, nullptr, "file", std::string(file_src->file())});
    }

    // uri — MIRROR ONLY (verbatim from the headless twin): no Glide AAR yet, so a remote decode awaits the
    // deferred android image-source pipeline; the result carries no native bitmap.
    void uri_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                        completion on_result)
    {
        auto* uri_src = dynamic_cast<i_uri_image_source*>(&source);
        if (uri_src == nullptr || uri_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        const std::string uri(uri_src->uri());
        on_result(decode_image_bytes(read_uri_bytes(uri), "uri", uri));
    }

    // stream — MIRROR ONLY (verbatim from the headless twin).
    void stream_image_source_service::load(i_image_source& source, const cancellation_token& token,
                                           completion on_result)
    {
        auto* stream_src = dynamic_cast<i_stream_image_source*>(&source);
        if (stream_src == nullptr || stream_src->is_empty())
        {
            on_result(image_source_result{});
            return;
        }
        const image_bytes bytes = stream_src->get_bytes(token);
        on_result(decode_image_bytes(bytes, "stream", "<bytes:" + std::to_string(bytes.size()) + ">"));
    }

    // font — the real rasterizer (FontModelResourceDecoder + FontImageSourceService.Android over JNI).
    void font_image_source_service::load(i_image_source& source, const cancellation_token& /*token*/,
                                         completion on_result)
    {
        const auto* font_src = dynamic_cast<const i_font_image_source*>(&source);
        if (font_src == nullptr || font_src->is_empty())
        {
            on_result(image_source_result{}); // not a font source / empty glyph → nothing rendered
            return;
        }
        const std::string glyph(font_src->glyph());
        // The headless font MIRROR — returned VM-less or on any JNI failure (see the file header): keeps the
        // cross-platform suite's loaded() + is_resolution_dependent() observations true, and degrades the
        // app host to a blank glyph rather than a crash. Font results are RESOLUTION-DEPENDENT (C# true).
        const auto mirror = [&glyph] {
            return image_source_result{nullptr, nullptr, "font", glyph, /*resolution_dependent*/ true};
        };
        const scoped_env env;
        if (!env || app_context() == nullptr)
        {
            on_result(mirror());
            return;
        }
        // Typeface: bold = weight >= Bold, italic = slant != Default (FontManager.ToTypefaceStyle).
        const font f = font_src->font();
        const bool italic = f.slant() != font_slant::normal;
        const bool bold = f.weight() >= font_weight::bold;
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
        jobject typeface = typeface_for(env.get(), f.family(), style); // borrowed (process-lifetime global)

        // Text size px: FontManager.GetFontSize (14sp default for Size <= 0 / NaN) → applyDimension(unit,…).
        auto size = static_cast<float>(f.size());
        if (!(size > 0) || std::isnan(size))
        {
            size = k_default_font_size;
        }
        const jint unit = f.auto_scaling_enabled() ? k_complex_unit_sp : k_complex_unit_dip;
        const float text_px = apply_dimension(env.get(), unit, size);
        if (!(text_px > 0))
        {
            on_result(mirror());
            return;
        }
        // (Color ?? White) → the port's concrete color directly (see the file-header color deviation).
        const auto color_argb = static_cast<jint>(font_src->color().to_int());

        const local_ref<jobject> bitmap = rasterize_glyph(env.get(), glyph, typeface, text_px, color_argb);
        if (!bitmap)
        {
            on_result(mirror());
            return;
        }
        // Hand the Bitmap back as a process-wide global ref: apply_loaded_result pushes it via
        // setImageBitmap (the ImageView's drawable then holds its own Java reference, so the Bitmap
        // survives when the disposer fires). The disposer DeleteGlobalRef's on the loader's NEXT load
        // (RAII — the loader drops the previous result), obtaining an env for whatever thread it runs on.
        jobject global_bitmap = env->NewGlobalRef(bitmap.get());
        on_result(image_source_result{static_cast<void*>(global_bitmap),
                                      [global_bitmap] {
                                          const scoped_env teardown;
                                          if (teardown)
                                          {
                                              teardown->DeleteGlobalRef(global_bitmap);
                                          }
                                      },
                                      "font", glyph, /*resolution_dependent*/ true});
    }
} // namespace maui::core
