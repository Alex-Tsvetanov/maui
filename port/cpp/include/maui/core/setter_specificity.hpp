#pragma once
// maui::core::setter_specificity  <=  Microsoft.Maui.Controls.SetterSpecificity (internal)
//
// The value-precedence key for bindable_property values: a single packed uint64 whose integer order
// *is* the precedence. Ported from src/Controls/src/Core/SetterSpecificity.cs. Ascending order:
//   default < (style) < binding < dynamic-resource < manual < trigger < visual-state < handler
// (handler is stored at the max value but is special-cased away by bindable_object when any other
// value is set — see SetValueActual). Non-style values pack the style field to its max so they win
// over style-set values.
//
// This is a constexpr packed value type, so it lives entirely in the header (PROFILE.md §3 allows
// constexpr inline). That keeps the comparison a single integer compare (the C# "fastest comparison
// possible" intent) and makes the named constants constant-initialized — no static-init-order fiasco.
//
// M1 scope: the named specificities + comparison + is_default/is_handler. The style/VSM/CSS
// manipulation helpers (CopyStyle, AsBaseStyle, WithFullVsmPriority, StyleInfo, trigger_index) are
// deferred to the styles/VSM milestone (M5); the full bit layout is preserved so they slot in later.

#include <compare>
#include <cstdint>

namespace maui::core
{
    class setter_specificity
    {
    public:
        // == C# default(SetterSpecificity): value 0, the lowest precedence ("no value").
        constexpr setter_specificity() noexcept = default;

        // The eight C# specificity dimensions (see SetterSpecificity.cs for field meanings).
        constexpr setter_specificity(std::uint8_t extras, std::uint16_t manual, std::uint8_t is_dynamic_resource,
                                     std::uint8_t is_binding, std::uint16_t style, std::uint8_t id,
                                     std::uint8_t css_class, std::uint8_t type) noexcept
        {
            // Handlers win on everything else (then bindable_object removes them when anything is set).
            if (extras == k_extras_handler)
            {
                value_ = k_handler_value;
                return;
            }
            std::uint16_t style_v = style;
            std::uint8_t id_v = id;
            std::uint8_t class_v = css_class;
            std::uint8_t type_v = type;
            // Not from a style => set the style field to its max so non-style values supersede styles.
            if (style_v == 0)
            {
                style_v = k_style_none;
                id_v = 0xFF;
                class_v = 0xFF;
                type_v = 0xFF;
            }
            std::uint64_t vsm = (extras == k_extras_vsm) ? 0x01ULL : 0x00ULL;
            std::uint64_t implicit_vsm = 0;
            std::uint64_t const binding = (is_binding > 0) ? 0x01ULL : 0x00ULL;
            std::uint64_t const dynamic_resource = (is_dynamic_resource > 0) ? 0x02ULL : 0x00ULL;
            // Implicit-style VSM has less priority than manually set values (dotnet/maui#18103).
            if (vsm != 0 && style_v < k_style_based_on)
            {
                implicit_vsm = 0x04ULL;
                vsm = 0;
            }
            value_ = static_cast<std::uint64_t>(type_v) | (static_cast<std::uint64_t>(class_v) << 8) |
                     (static_cast<std::uint64_t>(id_v) << 16) | ((implicit_vsm | dynamic_resource | binding) << 24) |
                     (static_cast<std::uint64_t>(manual) << 28) | (static_cast<std::uint64_t>(style_v) << 44) |
                     (vsm << 56);
        }

        constexpr setter_specificity(std::uint16_t style, std::uint8_t id, std::uint8_t css_class,
                                     std::uint8_t type) noexcept
            : setter_specificity(0, 0, 0, 0, style, id, css_class, type)
        {
        }

        [[nodiscard]] constexpr bool is_default() const
        {
            return value_ == 0ULL;
        }
        [[nodiscard]] constexpr bool is_handler() const
        {
            return value_ == k_handler_value;
        }

        constexpr bool operator==(const setter_specificity &) const = default;
        constexpr std::strong_ordering operator<=>(const setter_specificity &) const = default;

        // Named specificities (the C# SetterSpecificity static readonly fields). Defined below.
        static const setter_specificity default_value;
        static const setter_specificity visual_state_setter;
        static const setter_specificity from_binding;
        static const setter_specificity manual_value_setter;
        static const setter_specificity trigger;
        static const setter_specificity dynamic_resource_setter;
        static const setter_specificity from_handler;

    private:
        static constexpr std::uint8_t k_extras_vsm = 0x01;
        static constexpr std::uint8_t k_extras_handler = 0xFF;
        static constexpr std::uint64_t k_handler_value = 0xFFFFFFFFFFFFFFFFULL;
        static constexpr std::uint16_t k_style_none = 0xFFF;
        static constexpr std::uint16_t k_style_based_on = 0x0C0; // SetterSpecificity.StyleBasedOn
        static constexpr std::uint16_t k_manual_trigger_baseline = 2;

        std::uint64_t value_ = 0;
    };

    // constexpr ctor + constant args => these are constant-initialized (compile-time, no SIOF).
    inline const setter_specificity setter_specificity::default_value{};
    inline const setter_specificity setter_specificity::visual_state_setter{k_extras_vsm, 0, 0, 0, 0, 0, 0, 0};
    inline const setter_specificity setter_specificity::from_binding{0, 0, 0, 1, 0, 0, 0, 0};
    inline const setter_specificity setter_specificity::manual_value_setter{0, 1, 0, 0, 0, 0, 0, 0};
    inline const setter_specificity setter_specificity::trigger{0, k_manual_trigger_baseline, 0, 0, 0, 0, 0, 0};
    inline const setter_specificity setter_specificity::dynamic_resource_setter{0, 0, 1, 0, 0, 0, 0, 0};
    inline const setter_specificity setter_specificity::from_handler{k_extras_handler, 0, 0, 0, 0, 0, 0, 0};
} // namespace maui::core
