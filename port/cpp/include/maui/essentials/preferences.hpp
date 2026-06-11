#pragma once
// maui::storage::preferences    <=  Microsoft.Maui.Storage.Preferences (static facade)
// maui::storage::i_preferences  <=  Microsoft.Maui.Storage.IPreferences
//
// The application key/value preference store. C#'s generic Get<T>/Set<T> (whose SupportedTypes
// whitelist is string/int/bool/long/double/float/DateTime/DateTimeOffset) becomes one typed
// get/set pair per supported type - the C# type-whitelist throw (CheckIsSupportedType's
// NotSupportedException) cannot arise because unsupported types are unrepresentable here.
// Port notes vs the C# surface:
//   * string is nullable in C#: Set(key, null) removes the key and Get(key, null) reports a
//     missing key as null. The port models that as std::optional<std::string> on both sides.
//   * DateTime maps to std::chrono::system_clock::time_point. C++ has no DateTimeKind, so the
//     Kind round-trip (DateTimePreservesKind) collapses to the instant itself; the storage format
//     keeps C#'s "encode as an int64 string" shape (ToBinary -> time_since_epoch ticks).
//   * DateTimeOffset is NOT ported - the port has no offset-carrying date type; DateTime covers
//     the instant semantics.
//   * C#'s null sharedName means the default (standard) container; the port uses the empty string.
//
// Backends (suffix oracle): apple/macOS + ios REAL (Preferences.ios.tvos.watchos.macos.cs -
// NSUserDefaults; sharedName -> NSUserDefaults suite). Headless mirrors netstandard (throws)
// until faked (an in-memory map of containers).

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace maui::storage
{
    // The portable instant the DateTime overloads store (100 ns ticks survive the round trip).
    using date_time = std::chrono::system_clock::time_point;

    class i_preferences
    {
    public:
        virtual ~i_preferences() = default;

        // ContainsKey / Remove / Clear (sharedName "" = the default container).
        [[nodiscard]] virtual bool contains_key(std::string_view key, std::string_view shared_name) const = 0;
        virtual void remove(std::string_view key, std::string_view shared_name) = 0;
        virtual void clear(std::string_view shared_name) = 0;

        // Get<T>/Set<T>, one pair per C# supported type (the C# overload set's type keywords:
        // string / bool / int / long / float / double / DateTime).
        [[nodiscard]] virtual std::optional<std::string> get_string(std::string_view key,
                                                                    std::optional<std::string> default_value,
                                                                    std::string_view shared_name) const = 0;
        virtual void set_string(std::string_view key, const std::optional<std::string>& value,
                                std::string_view shared_name) = 0;

        [[nodiscard]] virtual bool get_bool(std::string_view key, bool default_value,
                                            std::string_view shared_name) const = 0;
        virtual void set_bool(std::string_view key, bool value, std::string_view shared_name) = 0;

        [[nodiscard]] virtual std::int32_t get_int(std::string_view key, std::int32_t default_value,
                                                   std::string_view shared_name) const = 0;
        virtual void set_int(std::string_view key, std::int32_t value, std::string_view shared_name) = 0;

        [[nodiscard]] virtual std::int64_t get_long(std::string_view key, std::int64_t default_value,
                                                    std::string_view shared_name) const = 0;
        virtual void set_long(std::string_view key, std::int64_t value, std::string_view shared_name) = 0;

        [[nodiscard]] virtual float get_float(std::string_view key, float default_value,
                                              std::string_view shared_name) const = 0;
        virtual void set_float(std::string_view key, float value, std::string_view shared_name) = 0;

        [[nodiscard]] virtual double get_double(std::string_view key, double default_value,
                                                std::string_view shared_name) const = 0;
        virtual void set_double(std::string_view key, double value, std::string_view shared_name) = 0;

        [[nodiscard]] virtual date_time get_date_time(std::string_view key, date_time default_value,
                                                      std::string_view shared_name) const = 0;
        virtual void set_date_time(std::string_view key, date_time value, std::string_view shared_name) = 0;

    protected:
        i_preferences() = default;
        i_preferences(const i_preferences&) = default;
        i_preferences(i_preferences&&) = default;
        i_preferences& operator=(const i_preferences&) = default;
        i_preferences& operator=(i_preferences&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (PreferencesImplementation), one per backend under
        // src/platform/<backend>/essentials_preferences.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_preferences> make_preferences();

        // Preferences.GetPrivatePreferencesSharedName(feature):
        // "{AppInfo.PackageName}.microsoft.maui.essentials.{feature}".
        [[nodiscard]] std::string private_preferences_shared_name(std::string_view feature);
    } // namespace detail

    // The static facade over preferences::default_() (C# Preferences.Default). Each member has the
    // C# convenience overload (no sharedName -> the default container).
    class preferences final
    {
    public:
        preferences() = delete;

        [[nodiscard]] static bool contains_key(std::string_view key, std::string_view shared_name = {})
        {
            return default_().contains_key(key, shared_name);
        }
        static void remove(std::string_view key, std::string_view shared_name = {})
        {
            default_().remove(key, shared_name);
        }
        static void clear(std::string_view shared_name = {})
        {
            default_().clear(shared_name);
        }

        [[nodiscard]] static std::optional<std::string> get_string(std::string_view key,
                                                                   std::optional<std::string> default_value,
                                                                   std::string_view shared_name = {})
        {
            return default_().get_string(key, std::move(default_value), shared_name);
        }
        static void set_string(std::string_view key, const std::optional<std::string>& value,
                               std::string_view shared_name = {})
        {
            default_().set_string(key, value, shared_name);
        }

        [[nodiscard]] static bool get_bool(std::string_view key, bool default_value, std::string_view shared_name = {})
        {
            return default_().get_bool(key, default_value, shared_name);
        }
        static void set_bool(std::string_view key, bool value, std::string_view shared_name = {})
        {
            default_().set_bool(key, value, shared_name);
        }

        [[nodiscard]] static std::int32_t get_int(std::string_view key, std::int32_t default_value,
                                                  std::string_view shared_name = {})
        {
            return default_().get_int(key, default_value, shared_name);
        }
        static void set_int(std::string_view key, std::int32_t value, std::string_view shared_name = {})
        {
            default_().set_int(key, value, shared_name);
        }

        [[nodiscard]] static std::int64_t get_long(std::string_view key, std::int64_t default_value,
                                                   std::string_view shared_name = {})
        {
            return default_().get_long(key, default_value, shared_name);
        }
        static void set_long(std::string_view key, std::int64_t value, std::string_view shared_name = {})
        {
            default_().set_long(key, value, shared_name);
        }

        [[nodiscard]] static float get_float(std::string_view key, float default_value,
                                             std::string_view shared_name = {})
        {
            return default_().get_float(key, default_value, shared_name);
        }
        static void set_float(std::string_view key, float value, std::string_view shared_name = {})
        {
            default_().set_float(key, value, shared_name);
        }

        [[nodiscard]] static double get_double(std::string_view key, double default_value,
                                               std::string_view shared_name = {})
        {
            return default_().get_double(key, default_value, shared_name);
        }
        static void set_double(std::string_view key, double value, std::string_view shared_name = {})
        {
            default_().set_double(key, value, shared_name);
        }

        [[nodiscard]] static date_time get_date_time(std::string_view key, date_time default_value,
                                                     std::string_view shared_name = {})
        {
            return default_().get_date_time(key, default_value, shared_name);
        }
        static void set_date_time(std::string_view key, date_time value, std::string_view shared_name = {})
        {
            default_().set_date_time(key, value, shared_name);
        }

        // Preferences.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_preferences& default_();
        static void set_default(std::shared_ptr<i_preferences> implementation);

        // The owning handle behind default_() - the port's analog of passing Preferences.Default
        // into VersionTrackingImplementation's constructor (shared_ptr ownership doctrine).
        [[nodiscard]] static std::shared_ptr<i_preferences> default_shared();
    };
} // namespace maui::storage
