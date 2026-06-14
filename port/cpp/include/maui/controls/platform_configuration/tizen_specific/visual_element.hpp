#pragma once
// maui::controls::platform_configuration::tizen_specific::visual_element
//   <=  Microsoft.Maui.Controls.PlatformConfiguration.TizenSpecific.VisualElement
// Ported from src/Controls/src/Core/PlatformConfiguration/TizenSpecific/VisualElement.cs.
// STORED-INERT (no Tizen backend). Notes:
//   - Style's C# property-name string is "ThemeStyle" (the API name is Style) — the key follows the
//     descriptor string here because the style_values constants document the same Tizen theme names.
//   - NextFocusDirection is a ONE-SHOT trigger in C#: the propertyChanged handler immediately resets
//     the value to FocusDirection.None, so a read always returns "None" — ported faithfully (the set
//     raises two property changes, value then reset, exactly like the C# SetValue pair).
//   - IsFocusAllowed is bool? (getter IsFocusAllowed, setter SetFocusAllowed(bool) — C# naming kept).

#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/platform_configuration/configuration.hpp"
#include "maui/controls/platform_configuration/element_concepts.hpp"
#include "maui/controls/platform_configuration/tizen_specific/focus_direction.hpp"

#include <optional>

namespace maui::controls::platform_configuration::tizen_specific::visual_element
{
    inline constexpr std::string_view style_key = "tizen.VisualElement.ThemeStyle";
    inline constexpr std::string_view is_focus_allowed_key = "tizen.VisualElement.IsFocusAllowed";
    inline constexpr std::string_view next_focus_direction_key = "tizen.VisualElement.NextFocusDirection";
    inline constexpr std::string_view next_focus_up_view_key = "tizen.VisualElement.NextFocusUpView";
    inline constexpr std::string_view next_focus_down_view_key = "tizen.VisualElement.NextFocusDownView";
    inline constexpr std::string_view next_focus_left_view_key = "tizen.VisualElement.NextFocusLeftView";
    inline constexpr std::string_view next_focus_right_view_key = "tizen.VisualElement.NextFocusRightView";
    inline constexpr std::string_view next_focus_back_view_key = "tizen.VisualElement.NextFocusBackView";
    inline constexpr std::string_view next_focus_forward_view_key = "tizen.VisualElement.NextFocusForwardView";
    inline constexpr std::string_view tool_tip_key = "tizen.VisualElement.ToolTip";

    // ---- Style / "ThemeStyle" (string, default null → ""; see style_values.hpp for the Tizen names) ----
    [[nodiscard]] inline std::string get_style(const element& target)
    {
        return target.platform_spec<std::string>(style_key, std::string{});
    }
    inline void set_style(element& target, std::string value)
    {
        target.set_platform_spec(style_key, std::move(value));
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] std::string get_style(config<tizen, TElement> cfg)
    {
        return get_style(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_style(config<tizen, TElement> cfg, std::string value)
    {
        set_style(cfg.element(), std::move(value));
        return cfg;
    }

    // ---- IsFocusAllowed (bool?, default null → nullopt; C# getter IsFocusAllowed / setter
    // SetFocusAllowed(bool)) ----
    [[nodiscard]] inline std::optional<bool> is_focus_allowed(const element& target)
    {
        return target.platform_spec<std::optional<bool>>(is_focus_allowed_key, std::nullopt);
    }
    inline void set_focus_allowed(element& target, bool value)
    {
        target.set_platform_spec(is_focus_allowed_key, std::optional<bool>{value});
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] std::optional<bool> is_focus_allowed(config<tizen, TElement> cfg)
    {
        return is_focus_allowed(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_focus_allowed(config<tizen, TElement> cfg, bool value)
    {
        set_focus_allowed(cfg.element(), value);
        return cfg;
    }

    // ---- NextFocusDirection (string, default FocusDirection.None; ONE-SHOT — see the header note) ----
    [[nodiscard]] inline std::string get_next_focus_direction(const element& target)
    {
        return target.platform_spec<std::string>(next_focus_direction_key, std::string{focus_direction::none});
    }
    inline void set_next_focus_direction(element& target, std::string value)
    {
        target.set_platform_spec(next_focus_direction_key, std::move(value));
        // C# OnNextFocusDirectionPropertyChanged: immediately bounce the stored value back to None (the
        // change notification for the requested direction has already fired — the Tizen renderer treats
        // it as a focus-move COMMAND, not state).
        target.set_platform_spec(next_focus_direction_key, std::string{focus_direction::none});
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] std::string get_next_focus_direction(config<tizen, TElement> cfg)
    {
        return get_next_focus_direction(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_next_focus_direction(config<tizen, TElement> cfg, std::string value)
    {
        set_next_focus_direction(cfg.element(), std::move(value));
        return cfg;
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> move_focus_up(config<tizen, TElement> cfg)
    {
        set_next_focus_direction(cfg.element(), std::string{focus_direction::up});
        return cfg;
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> move_focus_down(config<tizen, TElement> cfg)
    {
        set_next_focus_direction(cfg.element(), std::string{focus_direction::down});
        return cfg;
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> move_focus_left(config<tizen, TElement> cfg)
    {
        set_next_focus_direction(cfg.element(), std::string{focus_direction::left});
        return cfg;
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> move_focus_right(config<tizen, TElement> cfg)
    {
        set_next_focus_direction(cfg.element(), std::string{focus_direction::right});
        return cfg;
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> move_focus_back(config<tizen, TElement> cfg)
    {
        set_next_focus_direction(cfg.element(), std::string{focus_direction::back});
        return cfg;
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> move_focus_forward(config<tizen, TElement> cfg)
    {
        set_next_focus_direction(cfg.element(), std::string{focus_direction::forward});
        return cfg;
    }

    // ---- NextFocusUpView / Down / Left / Right / Back / Forward (View, default null — NON-owning) ----
    [[nodiscard]] inline element* get_next_focus_up_view(const element& target)
    {
        return target.platform_spec<element*>(next_focus_up_view_key, nullptr);
    }
    inline void set_next_focus_up_view(element& target, element* value)
    {
        target.set_platform_spec(next_focus_up_view_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] element* get_next_focus_up_view(config<tizen, TElement> cfg)
    {
        return get_next_focus_up_view(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_next_focus_up_view(config<tizen, TElement> cfg, element* value)
    {
        set_next_focus_up_view(cfg.element(), value);
        return cfg;
    }

    [[nodiscard]] inline element* get_next_focus_down_view(const element& target)
    {
        return target.platform_spec<element*>(next_focus_down_view_key, nullptr);
    }
    inline void set_next_focus_down_view(element& target, element* value)
    {
        target.set_platform_spec(next_focus_down_view_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] element* get_next_focus_down_view(config<tizen, TElement> cfg)
    {
        return get_next_focus_down_view(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_next_focus_down_view(config<tizen, TElement> cfg, element* value)
    {
        set_next_focus_down_view(cfg.element(), value);
        return cfg;
    }

    [[nodiscard]] inline element* get_next_focus_left_view(const element& target)
    {
        return target.platform_spec<element*>(next_focus_left_view_key, nullptr);
    }
    inline void set_next_focus_left_view(element& target, element* value)
    {
        target.set_platform_spec(next_focus_left_view_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] element* get_next_focus_left_view(config<tizen, TElement> cfg)
    {
        return get_next_focus_left_view(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_next_focus_left_view(config<tizen, TElement> cfg, element* value)
    {
        set_next_focus_left_view(cfg.element(), value);
        return cfg;
    }

    [[nodiscard]] inline element* get_next_focus_right_view(const element& target)
    {
        return target.platform_spec<element*>(next_focus_right_view_key, nullptr);
    }
    inline void set_next_focus_right_view(element& target, element* value)
    {
        target.set_platform_spec(next_focus_right_view_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] element* get_next_focus_right_view(config<tizen, TElement> cfg)
    {
        return get_next_focus_right_view(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_next_focus_right_view(config<tizen, TElement> cfg, element* value)
    {
        set_next_focus_right_view(cfg.element(), value);
        return cfg;
    }

    [[nodiscard]] inline element* get_next_focus_back_view(const element& target)
    {
        return target.platform_spec<element*>(next_focus_back_view_key, nullptr);
    }
    inline void set_next_focus_back_view(element& target, element* value)
    {
        target.set_platform_spec(next_focus_back_view_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] element* get_next_focus_back_view(config<tizen, TElement> cfg)
    {
        return get_next_focus_back_view(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_next_focus_back_view(config<tizen, TElement> cfg, element* value)
    {
        set_next_focus_back_view(cfg.element(), value);
        return cfg;
    }

    [[nodiscard]] inline element* get_next_focus_forward_view(const element& target)
    {
        return target.platform_spec<element*>(next_focus_forward_view_key, nullptr);
    }
    inline void set_next_focus_forward_view(element& target, element* value)
    {
        target.set_platform_spec(next_focus_forward_view_key, value);
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] element* get_next_focus_forward_view(config<tizen, TElement> cfg)
    {
        return get_next_focus_forward_view(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_next_focus_forward_view(config<tizen, TElement> cfg, element* value)
    {
        set_next_focus_forward_view(cfg.element(), value);
        return cfg;
    }

    // ---- ToolTip (string, default null → ""; distinct from the cross-platform ToolTipProperties) ----
    [[nodiscard]] inline std::string get_tool_tip(const element& target)
    {
        return target.platform_spec<std::string>(tool_tip_key, std::string{});
    }
    inline void set_tool_tip(element& target, std::string value)
    {
        target.set_platform_spec(tool_tip_key, std::move(value));
    }
    template <platform_configuration::visual_element TElement>
    [[nodiscard]] std::string get_tool_tip(config<tizen, TElement> cfg)
    {
        return get_tool_tip(cfg.element());
    }
    template <platform_configuration::visual_element TElement>
    config<tizen, TElement> set_tool_tip(config<tizen, TElement> cfg, std::string value)
    {
        set_tool_tip(cfg.element(), std::move(value));
        return cfg;
    }
} // namespace maui::controls::platform_configuration::tizen_specific::visual_element
