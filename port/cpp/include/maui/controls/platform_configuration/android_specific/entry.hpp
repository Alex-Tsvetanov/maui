#pragma once
// maui::controls::platform_configuration::android_specific::entry
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.Entry
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/Entry.cs.
// STORED-INERT until the Android JNI per-control fan-out (STATUS.md W2-24).

#include <concepts>
#include <cstdint>
#include <string_view>

#include "maui/controls/entry.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"

namespace maui::controls::platform_configuration::android_specific
{
    // C# AndroidSpecific.ImeFlags (declared alongside Entry) — the Android imeOptions flag values.
    enum class ime_flags : std::int32_t
    {
        default_flags = 0, // C# Default (reserved-word stand-in)
        none = 1,
        go = 2,
        search = 3,
        send = 4,
        next = 5,
        done = 6,
        previous = 7,
        ime_mask_action = 255,
        no_personalized_learning = 16777216,
        no_fullscreen = 33554432,
        no_extract_ui = 268435456,
        no_accessory_action = 536870912,
    };

    namespace entry
    {
        using forms_element = maui::controls::entry; // C# `using FormsElement = ...Entry` (implicit)

        inline constexpr std::string_view ime_options_key = "android.Entry.ImeOptions";

        // ---- ImeOptions (ImeFlags, default Default) ----
        [[nodiscard]] inline ime_flags get_ime_options(const element& target)
        {
            return target.platform_spec<ime_flags>(ime_options_key, ime_flags::default_flags);
        }
        inline void set_ime_options(element& target, ime_flags value)
        {
            target.set_platform_spec(ime_options_key, value);
        }
        template <std::derived_from<forms_element> TElement>
        [[nodiscard]] ime_flags ime_options(config<android, TElement> cfg)
        {
            return get_ime_options(cfg.element());
        }
        template <std::derived_from<forms_element> TElement>
        config<android, TElement> set_ime_options(config<android, TElement> cfg, ime_flags value)
        {
            set_ime_options(cfg.element(), value);
            return cfg;
        }
    } // namespace entry
} // namespace maui::controls::platform_configuration::android_specific
