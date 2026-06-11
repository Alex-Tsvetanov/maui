#pragma once
// The NSUserDefaults preferences implementation, ONE Obj-C++ definition for BOTH Apple backends -
// the analog of the single Preferences.ios.tvos.watchos.macos.cs partial (NSUserDefaults is
// identical on AppKit and UIKit). Ported 1:1 from PreferencesImplementation:
//   * GetUserDefaults(sharedName): a suite ([initWithSuiteName:]) for a non-blank shared name,
//     else standardUserDefaults.
//   * Set: string -> setObject, int -> setInteger, bool -> setBool, double -> setDouble,
//     float -> setFloat; LONG and DATETIME are stored as STRINGS (Convert.ToString invariant /
//     the ToBinary int64 - here the 100ns-tick count since the system_clock epoch); a null
//     string removes the key.
//   * Get: a missing key ([objectForKey:] == nil) reports the caller's default; otherwise the
//     typed read (integerForKey / boolForKey / doubleForKey / floatForKey / stringForKey, with
//     long/DateTime parsed back from their string encodings).
//   * Clear removes every key visible in the dictionary representation (removeObjectForKey: only
//     touches the app/suite domain, exactly like the C# loop).
// Included by src/platform/{apple,ios}/essentials_preferences.mm (the .mm provides make_preferences).

#import <Foundation/Foundation.h>

#include <charconv>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "maui/essentials/preferences.hpp"

namespace maui::storage::apple_shared
{
    namespace user_defaults_detail
    {
        inline NSString* to_ns_string(std::string_view value)
        {
            return [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
        }

        inline std::string to_std_string(NSString* value)
        {
            const char* const utf8 = [value UTF8String]; // messaging nil yields nullptr
            return utf8 != nullptr ? std::string(utf8) : std::string();
        }

        // GetUserDefaults(sharedName).
        inline NSUserDefaults* user_defaults(std::string_view shared_name)
        {
            if (!shared_name.empty())
            {
                return [[NSUserDefaults alloc] initWithSuiteName:to_ns_string(shared_name)];
            }
            return [NSUserDefaults standardUserDefaults];
        }

        // The DateTime/long string encodings (Convert.ToString invariant culture).
        using tick_duration = std::chrono::duration<std::int64_t, std::ratio<1, 10000000>>; // 100 ns ticks

        inline std::optional<std::int64_t> parse_int64(std::string_view text)
        {
            std::int64_t value = 0;
            const auto [ptr, error] = std::from_chars(text.data(), text.data() + text.size(), value);
            if (error != std::errc{} || ptr != text.data() + text.size())
            {
                return std::nullopt;
            }
            return value;
        }
    } // namespace user_defaults_detail

    class user_defaults_preferences final : public i_preferences
    {
    public:
        [[nodiscard]] bool contains_key(std::string_view key, std::string_view shared_name) const override
        {
            return [defaults(shared_name) objectForKey:ns(key)] != nil;
        }

        void remove(std::string_view key, std::string_view shared_name) override
        {
            NSUserDefaults* const store = defaults(shared_name);
            if ([store objectForKey:ns(key)] != nil)
            {
                [store removeObjectForKey:ns(key)];
            }
        }

        void clear(std::string_view shared_name) override
        {
            NSUserDefaults* const store = defaults(shared_name);
            NSDictionary<NSString*, id>* const items = [store dictionaryRepresentation];
            for (NSString* item in items.allKeys)
            {
                [store removeObjectForKey:item];
            }
        }

        [[nodiscard]] std::optional<std::string> get_string(std::string_view key,
                                                            std::optional<std::string> default_value,
                                                            std::string_view shared_name) const override
        {
            NSUserDefaults* const store = defaults(shared_name);
            if ([store objectForKey:ns(key)] == nil)
            {
                return default_value;
            }
            return user_defaults_detail::to_std_string([store stringForKey:ns(key)]);
        }

        void set_string(std::string_view key, const std::optional<std::string>& value,
                        std::string_view shared_name) override
        {
            NSUserDefaults* const store = defaults(shared_name);
            if (!value.has_value())
            {
                // The C# null-value Set removes the key.
                if ([store objectForKey:ns(key)] != nil)
                {
                    [store removeObjectForKey:ns(key)];
                }
                return;
            }
            [store setObject:ns(*value) forKey:ns(key)];
        }

        [[nodiscard]] bool get_bool(std::string_view key, bool default_value,
                                    std::string_view shared_name) const override
        {
            NSUserDefaults* const store = defaults(shared_name);
            return [store objectForKey:ns(key)] == nil ? default_value : [store boolForKey:ns(key)] == YES;
        }
        void set_bool(std::string_view key, bool value, std::string_view shared_name) override
        {
            [defaults(shared_name) setBool:(value ? YES : NO) forKey:ns(key)];
        }

        [[nodiscard]] std::int32_t get_int(std::string_view key, std::int32_t default_value,
                                           std::string_view shared_name) const override
        {
            NSUserDefaults* const store = defaults(shared_name);
            if ([store objectForKey:ns(key)] == nil)
            {
                return default_value;
            }
            // C#: (int)(nint)userDefaults.IntForKey(key).
            return static_cast<std::int32_t>([store integerForKey:ns(key)]);
        }
        void set_int(std::string_view key, std::int32_t value, std::string_view shared_name) override
        {
            [defaults(shared_name) setInteger:value forKey:ns(key)];
        }

        [[nodiscard]] std::int64_t get_long(std::string_view key, std::int64_t default_value,
                                            std::string_view shared_name) const override
        {
            NSUserDefaults* const store = defaults(shared_name);
            if ([store objectForKey:ns(key)] == nil)
            {
                return default_value;
            }
            const std::string saved = user_defaults_detail::to_std_string([store stringForKey:ns(key)]);
            return user_defaults_detail::parse_int64(saved).value_or(default_value);
        }
        void set_long(std::string_view key, std::int64_t value, std::string_view shared_name) override
        {
            [defaults(shared_name) setObject:ns(std::to_string(value)) forKey:ns(key)];
        }

        [[nodiscard]] float get_float(std::string_view key, float default_value,
                                      std::string_view shared_name) const override
        {
            NSUserDefaults* const store = defaults(shared_name);
            return [store objectForKey:ns(key)] == nil ? default_value : [store floatForKey:ns(key)];
        }
        void set_float(std::string_view key, float value, std::string_view shared_name) override
        {
            [defaults(shared_name) setFloat:value forKey:ns(key)];
        }

        [[nodiscard]] double get_double(std::string_view key, double default_value,
                                        std::string_view shared_name) const override
        {
            NSUserDefaults* const store = defaults(shared_name);
            return [store objectForKey:ns(key)] == nil ? default_value : [store doubleForKey:ns(key)];
        }
        void set_double(std::string_view key, double value, std::string_view shared_name) override
        {
            [defaults(shared_name) setDouble:value forKey:ns(key)];
        }

        [[nodiscard]] date_time get_date_time(std::string_view key, date_time default_value,
                                              std::string_view shared_name) const override
        {
            NSUserDefaults* const store = defaults(shared_name);
            if ([store objectForKey:ns(key)] == nil)
            {
                return default_value;
            }
            const std::string saved = user_defaults_detail::to_std_string([store stringForKey:ns(key)]);
            const std::optional<std::int64_t> ticks = user_defaults_detail::parse_int64(saved);
            if (!ticks.has_value())
            {
                return default_value;
            }
            return date_time(
                std::chrono::duration_cast<date_time::duration>(user_defaults_detail::tick_duration(*ticks)));
        }
        void set_date_time(std::string_view key, date_time value, std::string_view shared_name) override
        {
            const auto ticks =
                std::chrono::duration_cast<user_defaults_detail::tick_duration>(value.time_since_epoch()).count();
            [defaults(shared_name) setObject:ns(std::to_string(ticks)) forKey:ns(key)];
        }

    private:
        [[nodiscard]] static NSUserDefaults* defaults(std::string_view shared_name)
        {
            return user_defaults_detail::user_defaults(shared_name);
        }
        [[nodiscard]] static NSString* ns(std::string_view value)
        {
            return user_defaults_detail::to_ns_string(value);
        }
    };
} // namespace maui::storage::apple_shared
