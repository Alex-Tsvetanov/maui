// device_info - Android platform partial. Ported from DeviceInfo.android.cs
// (src/Essentials/src/DeviceInfo/DeviceInfo.android.cs):
//   Model         => Build.MODEL
//   Manufacturer  => Build.MANUFACTURER
//   Name          => Settings.Global "device_name" (API 25+) else Model
//   VersionString => Build.VERSION.RELEASE
//   Platform      => DevicePlatform.Android (fixed)
//   Idiom         => UiModeManager.CurrentModeType then Configuration.SmallestScreenWidthDp
//                    (>= 600dp => Tablet else Phone)
//   DeviceType    => emulator heuristics over Build.{BRAND,DEVICE,FINGERPRINT,HARDWARE,MODEL,
//                    MANUFACTURER,PRODUCT} => Virtual, else Physical
//
// The C# partial reads Java `android.os.Build` static string fields + a couple of Context services;
// the C++ port does the same through the JNI seam (jni_cache/scoped_env/app_context). Unlike the
// unconfigured headless fake — whose model()/version_string() THROW feature_not_supported (the
// netstandard-partial mirror) and so terminated the DevicePage on Android — every read here degrades
// to a sane default on any JNI failure or when no VM/Context is registered (the VM-less unit host).
// A rendering page beats a crash: the page must never throw during construction.
//
// JNI discipline (LESSON 1 in docs/MACOS_ANDROID_RESUME.md): default_jni_cache() to pin classes,
// scoped_env for the per-thread env, app_context() for the Context, and clear_pending(env) after
// every JNI call so no Java pending-exception state leaks out.

#include "maui/essentials/device_info.hpp"

#include <jni.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>

#include "src/platform/android/jni/app_context.hpp"
#include "src/platform/android/jni/jni_cache.hpp"
#include "src/platform/android/jni/jni_env.hpp"
#include "src/platform/android/jni/jni_ref.hpp"
#include "src/platform/android/jni/jni_string.hpp"

namespace maui::devices
{
    namespace
    {
        using maui::platform::android::app_context;
        using maui::platform::android::default_jni_cache;
        using maui::platform::android::local_ref;
        using maui::platform::android::scoped_env;
        using maui::platform::android::to_utf8;

        // Clear (and swallow) any pending Java exception; true when one was pending. Every JNI call a
        // read makes is followed by this so the port keeps explicit control flow (a failed Build read
        // becomes an empty string / a default, never a leaked exception that would crash the mount).
        bool clear_pending(JNIEnv* env)
        {
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
                return true;
            }
            return false;
        }

        // Read one static java.lang.String field of android/os/Build (or Build$VERSION) as UTF-8;
        // empty on any failure. Build.MODEL / .MANUFACTURER / .FINGERPRINT / … and VERSION.RELEASE are
        // all `public static final String` — GetStaticFieldID + GetStaticObjectField, the shape the
        // image_handler uses for ScaleType. The jni_cache only memoizes instance fields, so the static
        // field id is resolved directly through the pinned class.
        std::string build_string_field(JNIEnv* env, const char* class_name, const char* field_name)
        {
            jclass build_class = default_jni_cache().find_class(env, class_name);
            if (build_class == nullptr)
            {
                return {};
            }
            const jfieldID field = env->GetStaticFieldID(build_class, field_name, "Ljava/lang/String;");
            if (clear_pending(env) || field == nullptr)
            {
                return {};
            }
            local_ref<jstring> value{env, static_cast<jstring>(env->GetStaticObjectField(build_class, field))};
            if (clear_pending(env) || !value)
            {
                return {};
            }
            return to_utf8(env, value.get());
        }

        std::string build_field(JNIEnv* env, const char* field_name)
        {
            return build_string_field(env, "android/os/Build", field_name);
        }

        // Configuration.smallestScreenWidthDp (an int field of android/content/res/Configuration), read
        // off the Context's Resources; <= 0 (or any failure) leaves the idiom detection to fall through.
        int smallest_screen_width_dp(JNIEnv* env, jobject context)
        {
            if (context == nullptr)
            {
                return 0;
            }
            auto& cache = default_jni_cache();
            const jmethodID get_resources =
                cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
            if (get_resources == nullptr)
            {
                return 0;
            }
            local_ref<jobject> resources{env, env->CallObjectMethod(context, get_resources)};
            if (clear_pending(env) || !resources)
            {
                return 0;
            }
            const jmethodID get_config = cache.method(env, "android/content/res/Resources", "getConfiguration",
                                                      "()Landroid/content/res/Configuration;");
            if (get_config == nullptr)
            {
                return 0;
            }
            local_ref<jobject> config{env, env->CallObjectMethod(resources.get(), get_config)};
            if (clear_pending(env) || !config)
            {
                return 0;
            }
            const jfieldID width_field =
                cache.field(env, "android/content/res/Configuration", "smallestScreenWidthDp", "I");
            if (width_field == nullptr)
            {
                return 0;
            }
            const jint width = env->GetIntField(config.get(), width_field);
            if (clear_pending(env))
            {
                return 0;
            }
            return static_cast<int>(width);
        }

        // C#'s emulator heuristic: any of the Build.* prefix/substring checks below marks a virtual
        // device. Substring vs prefix matches the oracle field-by-field.
        bool starts_with(std::string_view text, std::string_view prefix)
        {
            return text.size() >= prefix.size() && text.compare(0, prefix.size(), prefix) == 0;
        }
        bool contains(std::string_view text, std::string_view needle)
        {
            return text.find(needle) != std::string_view::npos;
        }

        class android_device_info final : public i_device_info
        {
        public:
            [[nodiscard]] std::string model() const override
            {
                const scoped_env env;
                return env ? build_field(env.get(), "MODEL") : std::string{};
            }

            [[nodiscard]] std::string manufacturer() const override
            {
                const scoped_env env;
                return env ? build_field(env.get(), "MANUFACTURER") : std::string{};
            }

            // C#: Settings.Global "device_name" (API 25+), else Model. The settings read needs a
            // ContentResolver; degrade to Model (which is what C# also falls back to when the setting is
            // blank), so we skip the extra JNI hop and mirror the common-case result.
            [[nodiscard]] std::string name() const override
            {
                return model();
            }

            // Build.VERSION.RELEASE — the human OS version ("14", "34" pre-release, …). VERSION is the
            // nested class android/os/Build$VERSION.
            [[nodiscard]] std::string version_string() const override
            {
                const scoped_env env;
                if (!env)
                {
                    return {};
                }
                return build_string_field(env.get(), "android/os/Build$VERSION", "RELEASE");
            }

            [[nodiscard]] device_platform platform() const override
            {
                return device_platform::android();
            }

            // C#: UiModeManager first (TV/Desk/Watch), then SmallestScreenWidthDp >= 600 => Tablet else
            // Phone. The port implements the width path (the dominant one for phones/tablets) off the
            // Context Resources; TV/Desk/Watch UiModeManager typing is deferred (documented). Unknown
            // when there is no Context (the VM-less host), matching C#'s "hope we got it" fallthrough.
            [[nodiscard]] device_idiom idiom() const override
            {
                const scoped_env env;
                if (!env)
                {
                    return device_idiom::unknown();
                }
                constexpr int tablet_crossover = 600;
                const int min_width = smallest_screen_width_dp(env.get(), app_context());
                if (min_width <= 0)
                {
                    return device_idiom::unknown();
                }
                return min_width >= tablet_crossover ? device_idiom::tablet() : device_idiom::phone();
            }

            // C#: the Build.* emulator heuristic; every gallery capture runs on the maui-test AVD, so
            // this reports Virtual there (matching a real .NET MAUI DeviceInfo on the same emulator).
            [[nodiscard]] enum device_type device_type() const override
            {
                const scoped_env env;
                if (!env)
                {
                    return device_type::unknown;
                }
                JNIEnv* raw = env.get();
                const std::string brand = build_field(raw, "BRAND");
                const std::string device = build_field(raw, "DEVICE");
                const std::string fingerprint = build_field(raw, "FINGERPRINT");
                const std::string hardware = build_field(raw, "HARDWARE");
                const std::string model_id = build_field(raw, "MODEL");
                const std::string manufacturer_id = build_field(raw, "MANUFACTURER");
                const std::string product = build_field(raw, "PRODUCT");

                const bool is_emulator =
                    (starts_with(brand, "generic") && starts_with(device, "generic")) ||
                    starts_with(fingerprint, "generic") || starts_with(fingerprint, "unknown") ||
                    contains(hardware, "goldfish") || contains(hardware, "ranchu") ||
                    contains(model_id, "google_sdk") || contains(model_id, "Emulator") ||
                    contains(model_id, "Android SDK built for x86") || contains(manufacturer_id, "Genymotion") ||
                    contains(manufacturer_id, "VS Emulator") || contains(product, "emulator") ||
                    contains(product, "google_sdk") || contains(product, "sdk") || contains(product, "sdk_google") ||
                    contains(product, "sdk_x86") || contains(product, "simulator") || contains(product, "vbox86p");

                return is_emulator ? device_type::virtual_ : device_type::physical;
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_device_info> make_device_info()
        {
            return std::make_shared<android_device_info>();
        }
    } // namespace detail
} // namespace maui::devices
