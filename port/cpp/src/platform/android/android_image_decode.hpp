#pragma once
// Shared Android image-byte fetch + decode helpers: resolve a from_file() name to bytes (APK assets/<name>
// first, then an on-disk file:// / absolute path) and decode those bytes into an android.graphics.Bitmap
// via BitmapFactory. Extracted so BOTH the image control (image_handler.cpp — which owns the original
// copies) and the button/image-button icon path can share ONE decode: the port has no Glide/AAR image
// service, so this bundled-bytes fast-path is how every bundled from_file() image (dotnet_bot.png,
// settings.png, cog.png, …) renders on the android backend. All JNI-guarded (empty on any failure).

#include <cstddef>
#include <string_view>
#include <vector>

#include <jni.h>

#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/i_stream_image_source.hpp" // maui::core::image_bytes (= std::vector<std::byte>)
#include "maui/core/uri_bytes.hpp"             // read_uri_bytes (disk fallback)

namespace maui::platform::android::image_decode
{
    inline constexpr const char* k_bitmap_factory_class = "android/graphics/BitmapFactory";

    // A pending-exception check+clear, self-contained here (each handler .cpp has its own file-local copy).
    [[nodiscard]] inline bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }

    // Drain a java.io.InputStream fully into image_bytes (empty on any JNI failure), in 64 KiB chunks.
    [[nodiscard]] inline maui::core::image_bytes drain_input_stream(JNIEnv* env, jobject stream)
    {
        auto& cache = maui::platform::android::default_jni_cache();
        jmethodID read = cache.method(env, "java/io/InputStream", "read", "([B)I");
        jmethodID close = cache.method(env, "java/io/InputStream", "close", "()V");
        if (read == nullptr)
        {
            return {};
        }
        constexpr jsize k_chunk = 64 * 1024;
        const maui::platform::android::local_ref<jbyteArray> buffer{env, env->NewByteArray(k_chunk)};
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

    // Read assets/<name> from the process Context's AssetManager (empty if no Context / asset absent).
    [[nodiscard]] inline maui::core::image_bytes read_asset_bytes(JNIEnv* env, std::string_view name)
    {
        jobject context = maui::platform::android::app_context();
        if (env == nullptr || context == nullptr)
        {
            return {};
        }
        auto& cache = maui::platform::android::default_jni_cache();
        jmethodID get_assets =
            cache.method(env, "android/content/Context", "getAssets", "()Landroid/content/res/AssetManager;");
        jmethodID open =
            cache.method(env, "android/content/res/AssetManager", "open", "(Ljava/lang/String;)Ljava/io/InputStream;");
        if (get_assets == nullptr || open == nullptr)
        {
            return {};
        }
        const maui::platform::android::local_ref<jobject> assets{env, env->CallObjectMethod(context, get_assets)};
        if (clear_pending(env) || !assets)
        {
            return {};
        }
        const maui::platform::android::local_ref<jstring> jname = maui::platform::android::to_jstring(env, name);
        if (!jname)
        {
            return {};
        }
        const maui::platform::android::local_ref<jobject> stream{
            env, env->CallObjectMethod(assets.get(), open, jname.get())};
        if (clear_pending(env) || !stream) // missing asset throws → cleared, empty
        {
            return {};
        }
        return drain_input_stream(env, stream.get());
    }

    // Resolve a from_file() name to bytes: assets/<name> first, then the on-disk reader (absolute / file://).
    [[nodiscard]] inline maui::core::image_bytes resolve_file_bytes(JNIEnv* env, std::string_view file)
    {
        maui::core::image_bytes bytes = read_asset_bytes(env, file);
        if (!bytes.empty())
        {
            return bytes;
        }
        return maui::core::read_uri_bytes(file);
    }

    // BitmapFactory.decodeByteArray(bytes, 0, len) → android.graphics.Bitmap (null local ref on failure).
    [[nodiscard]] inline maui::platform::android::local_ref<jobject> decode_bitmap(JNIEnv* env,
                                                                                   const maui::core::image_bytes& bytes)
    {
        if (env == nullptr || bytes.empty())
        {
            return {};
        }
        auto& cache = maui::platform::android::default_jni_cache();
        jmethodID decode =
            cache.static_method(env, k_bitmap_factory_class, "decodeByteArray", "([BII)Landroid/graphics/Bitmap;");
        jclass factory_class = cache.find_class(env, k_bitmap_factory_class);
        if (decode == nullptr || factory_class == nullptr)
        {
            return {};
        }
        const auto len = static_cast<jsize>(bytes.size());
        const maui::platform::android::local_ref<jbyteArray> array{env, env->NewByteArray(len)};
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
        maui::platform::android::local_ref<jobject> bitmap{
            env, env->CallStaticObjectMethod(factory_class, decode, array.get(), 0, len)};
        if (clear_pending(env))
        {
            return {};
        }
        return bitmap;
    }
} // namespace maui::platform::android::image_decode
