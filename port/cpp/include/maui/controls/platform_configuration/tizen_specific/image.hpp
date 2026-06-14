#pragma once
// maui::controls::platform_configuration::tizen_specific::image
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.Image
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/Image.cs. STORED-INERT (no
// Tizen backend).

#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/image.hpp"
#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls::platform_configuration::tizen_specific::image
{
    using forms_element = maui::controls::image; // C# `using FormsElement = ...Image`

    inline constexpr std::string_view blend_color_key = "tizen.Image.BlendColor";
    inline constexpr std::string_view file_key = "tizen.Image.File";

    // ---- BlendColor (Color, default null → nullopt) ----
    [[nodiscard]] inline std::optional<maui::graphics::color> get_blend_color(const element& target)
    {
        return target.platform_spec<std::optional<maui::graphics::color>>(blend_color_key, std::nullopt);
    }
    inline void set_blend_color(element& target, std::optional<maui::graphics::color> color)
    {
        target.set_platform_spec(blend_color_key, color);
    }
    template <std::derived_from<forms_element> TElement>
    [[nodiscard]] std::optional<maui::graphics::color> get_blend_color(config<tizen, TElement> cfg)
    {
        return get_blend_color(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_blend_color(config<tizen, TElement> cfg, std::optional<maui::graphics::color> color)
    {
        set_blend_color(cfg.element(), color);
        return cfg;
    }

    // ---- File (string, default null → "") ----
    [[nodiscard]] inline std::string get_file(const element& target)
    {
        return target.platform_spec<std::string>(file_key, std::string{});
    }
    inline void set_file(element& target, std::string file)
    {
        target.set_platform_spec(file_key, std::move(file));
    }
    template <std::derived_from<forms_element> TElement> [[nodiscard]] std::string get_file(config<tizen, TElement> cfg)
    {
        return get_file(cfg.element());
    }
    template <std::derived_from<forms_element> TElement>
    config<tizen, TElement> set_file(config<tizen, TElement> cfg, std::string file)
    {
        set_file(cfg.element(), std::move(file));
        return cfg;
    }
} // namespace maui::controls::platform_configuration::tizen_specific::image
