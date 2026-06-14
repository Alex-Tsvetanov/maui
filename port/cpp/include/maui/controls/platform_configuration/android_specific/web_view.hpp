#pragma once
// maui::controls::platform_configuration::android_specific::web_view
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.WebView
// Ported from src/Controls/src/Core/PlatformConfiguration/AndroidSpecific/WebView.cs.
// STORED-INERT until the Android JNI per-control fan-out (STATUS.md W2-24).

#include <concepts>
#include <cstdint>
#include <string_view>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/web_view.hpp"

namespace maui::controls::platform_configuration::android_specific
{
    // C# AndroidSpecific.MixedContentHandling (declared alongside WebView).
    enum class mixed_content_handling : std::uint8_t
    {
        always_allow = 0,
        never_allow = 1,
        compatibility_mode = 2,
    };

    namespace web_view
    {
        using forms_element = maui::controls::web_view; // C# `using FormsElement = ...WebView`

        inline constexpr std::string_view mixed_content_mode_key = "android.WebView.MixedContentMode";
        inline constexpr std::string_view enable_zoom_controls_key = "android.WebView.EnableZoomControls";
        inline constexpr std::string_view display_zoom_controls_key = "android.WebView.DisplayZoomControls";
        inline constexpr std::string_view javascript_enabled_key = "android.WebView.JavaScriptEnabled";

        // ---- MixedContentMode (MixedContentHandling, default NeverAllow) ----
        [[nodiscard]] inline mixed_content_handling get_mixed_content_mode(const element& target)
        {
            return target.platform_spec<mixed_content_handling>(mixed_content_mode_key,
                                                                mixed_content_handling::never_allow);
        }
        inline void set_mixed_content_mode(element& target, mixed_content_handling value)
        {
            target.set_platform_spec(mixed_content_mode_key, value);
        }
        template <std::derived_from<forms_element> TElement>
        [[nodiscard]] mixed_content_handling mixed_content_mode(config<android, TElement> cfg)
        {
            return get_mixed_content_mode(cfg.element());
        }
        template <std::derived_from<forms_element> TElement>
        config<android, TElement> set_mixed_content_mode(config<android, TElement> cfg, mixed_content_handling value)
        {
            set_mixed_content_mode(cfg.element(), value);
            return cfg;
        }

        // ---- EnableZoomControls (bool, default false) ----
        [[nodiscard]] inline bool get_enable_zoom_controls(const element& target)
        {
            return target.platform_spec<bool>(enable_zoom_controls_key, false);
        }
        inline void set_enable_zoom_controls(element& target, bool value)
        {
            target.set_platform_spec(enable_zoom_controls_key, value);
        }
        // C#'s void extension EnableZoomControls(config, value) + bool ZoomControlsEnabled(config) +
        // chaining SetEnableZoomControls.
        template <std::derived_from<forms_element> TElement>
        void enable_zoom_controls(config<android, TElement> cfg, bool value)
        {
            set_enable_zoom_controls(cfg.element(), value);
        }
        template <std::derived_from<forms_element> TElement>
        [[nodiscard]] bool zoom_controls_enabled(config<android, TElement> cfg)
        {
            return get_enable_zoom_controls(cfg.element());
        }
        template <std::derived_from<forms_element> TElement>
        config<android, TElement> set_enable_zoom_controls(config<android, TElement> cfg, bool value)
        {
            set_enable_zoom_controls(cfg.element(), value);
            return cfg;
        }

        // ---- DisplayZoomControls (bool, default true) ----
        [[nodiscard]] inline bool get_display_zoom_controls(const element& target)
        {
            return target.platform_spec<bool>(display_zoom_controls_key, true);
        }
        inline void set_display_zoom_controls(element& target, bool value)
        {
            target.set_platform_spec(display_zoom_controls_key, value);
        }
        template <std::derived_from<forms_element> TElement>
        void display_zoom_controls(config<android, TElement> cfg, bool value)
        {
            set_display_zoom_controls(cfg.element(), value);
        }
        template <std::derived_from<forms_element> TElement>
        [[nodiscard]] bool zoom_controls_displayed(config<android, TElement> cfg)
        {
            return get_display_zoom_controls(cfg.element());
        }
        template <std::derived_from<forms_element> TElement>
        config<android, TElement> set_display_zoom_controls(config<android, TElement> cfg, bool value)
        {
            set_display_zoom_controls(cfg.element(), value);
            return cfg;
        }

        // ---- JavaScriptEnabled (bool, default true) ----
        [[nodiscard]] inline bool get_javascript_enabled(const element& target)
        {
            return target.platform_spec<bool>(javascript_enabled_key, true);
        }
        inline void set_javascript_enabled(element& target, bool value)
        {
            target.set_platform_spec(javascript_enabled_key, value);
        }
        template <std::derived_from<forms_element> TElement>
        void javascript_enabled(config<android, TElement> cfg, bool value)
        {
            set_javascript_enabled(cfg.element(), value);
        }
        template <std::derived_from<forms_element> TElement>
        [[nodiscard]] bool is_javascript_enabled(config<android, TElement> cfg)
        {
            return get_javascript_enabled(cfg.element());
        }
        template <std::derived_from<forms_element> TElement>
        config<android, TElement> set_javascript_enabled(config<android, TElement> cfg, bool value)
        {
            set_javascript_enabled(cfg.element(), value);
            return cfg;
        }
    } // namespace web_view
} // namespace maui::controls::platform_configuration::android_specific
