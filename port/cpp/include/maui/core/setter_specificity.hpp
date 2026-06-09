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
// M1 scope: the named specificities + comparison + is_default/is_handler. M5b added as_base_style (the
// BasedOn precedence lowering); M5d adds with_full_vsm_priority (the implicit-style VSM promotion for
// system-driven states) + the visual_state_setter_system constant. CopyStyle / StyleInfo / trigger_index
// stay deferred; the full bit layout is preserved so they slot in later.

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

        // Lower this specificity by one style level for a BasedOn (inherited) style — so a derived style's
        // own setters outrank the setters it inherits (Style.ApplyCore passes specificity.AsBaseStyle() to
        // the based-on style). Already-base specificities (<= StyleBasedOn) are returned unchanged, and the
        // result is clamped to StyleBasedOn so base styles stay above implicit styles but below local ones.
        // Direct port of SetterSpecificity.AsBaseStyle.
        [[nodiscard]] constexpr setter_specificity as_base_style() const
        {
            auto const current_style = static_cast<std::uint16_t>((value_ >> 44) & 0xFFFULL);
            if (current_style <= k_style_based_on)
            {
                return *this;
            }
            std::uint64_t new_value = value_ - (1ULL << 44);
            auto const style_value = static_cast<std::uint16_t>((new_value >> 44) & 0xFFFULL);
            if (style_value < k_style_based_on)
            {
                new_value = (new_value & 0xFFF000FFFFFFFFFFULL) | (static_cast<std::uint64_t>(k_style_based_on) << 44);
            }
            return setter_specificity{new_value};
        }

        // Promote an implicit-style VSM value back to full VSM priority (SetterSpecificity
        // .WithFullVsmPriority): a no-op unless this is an implicit-VSM value, in which case the implicit-VSM
        // bit is cleared and the full-VSM bit is set — lifting the value back ABOVE a manual one. The
        // visual_state_manager calls this for system-driven states (Disabled/Focused/…) when the VSGroups
        // came from an implicit style; custom states keep the downgrade (dotnet/maui#18103 / #34363).
        [[nodiscard]] constexpr setter_specificity with_full_vsm_priority() const
        {
            if ((value_ & k_implicit_vsm_mask) == 0)
            {
                return *this; // not an implicit-VSM value — nothing to promote
            }
            return setter_specificity{(value_ & ~k_implicit_vsm_mask) | k_vsm_mask};
        }

        constexpr bool operator==(const setter_specificity&) const = default;
        constexpr std::strong_ordering operator<=>(const setter_specificity&) const = default;

        // Named specificities (the C# SetterSpecificity static readonly fields). Defined below.
        static const setter_specificity default_value;
        static const setter_specificity style_implicit; // an implicit (TargetType-keyed) style setter
        static const setter_specificity style_based_on; // a BasedOn (inherited) style setter
        static const setter_specificity style_local;    // an explicit (inline) style setter
        // A class-style setter (a Style selected via StyleClass): StyleLocal with the CSS class byte set to 1
        // (MergedStyle applies class styles at `new SetterSpecificity(StyleLocal, 0, 1, 0)`), so it outranks
        // the local style (class 0) for the same property — CSS-like.
        static const setter_specificity style_class;
        static const setter_specificity visual_state_setter; // a VSM state setter (above a manual value)
        // A VSM state setter whose VSGroups arrived via an implicit style: DOWNGRADED below a manual value
        // (the #18103 fix — built with style == StyleImplicit so the ctor sets the implicit-VSM bit and
        // clears the high VSM bit). visual_state_manager promotes it back with with_full_vsm_priority() for
        // system-driven states; custom states keep this downgraded specificity.
        static const setter_specificity visual_state_setter_system;
        static const setter_specificity from_binding;
        static const setter_specificity manual_value_setter;
        static const setter_specificity trigger;
        static const setter_specificity dynamic_resource_setter;
        static const setter_specificity from_handler;

    private:
        // Build directly from a packed value (the C# private SetterSpecificity(ulong) — used by
        // as_base_style to assemble a lowered specificity). Distinct arity from the public ctors.
        explicit constexpr setter_specificity(std::uint64_t raw) noexcept : value_(raw)
        {
        }

        static constexpr std::uint8_t k_extras_vsm = 0x01;
        static constexpr std::uint8_t k_extras_handler = 0xFF;
        static constexpr std::uint64_t k_handler_value = 0xFFFFFFFFFFFFFFFFULL;
        // The full-VSM and implicit-VSM single-bit masks (SetterSpecificity.VsmMask / ImplicitVsmMask):
        // full VSM sits at bit 56 (above manual); implicit VSM at bit 26 (below manual). with_full_vsm_-
        // priority moves a value from the latter to the former.
        static constexpr std::uint64_t k_vsm_mask = 0x0100000000000000ULL;
        static constexpr std::uint64_t k_implicit_vsm_mask = 0x0000000004000000ULL;
        static constexpr std::uint16_t k_style_none = 0xFFF;
        static constexpr std::uint16_t k_style_implicit = 0x080; // SetterSpecificity.StyleImplicit
        static constexpr std::uint16_t k_style_based_on = 0x0C0; // SetterSpecificity.StyleBasedOn
        static constexpr std::uint16_t k_style_local = 0x100;    // SetterSpecificity.StyleLocal
        static constexpr std::uint16_t k_manual_trigger_baseline = 2;

        std::uint64_t value_ = 0;
    };

    // constexpr ctor + constant args => these are constant-initialized (compile-time, no SIOF).
    inline const setter_specificity setter_specificity::default_value{};
    inline const setter_specificity setter_specificity::style_implicit{k_style_implicit, 0, 0, 0};
    inline const setter_specificity setter_specificity::style_based_on{k_style_based_on, 0, 0, 0};
    inline const setter_specificity setter_specificity::style_local{k_style_local, 0, 0, 0};
    inline const setter_specificity setter_specificity::style_class{k_style_local, 0, 1, 0};
    // A VSM state setter: with no style supplied (style 0), the ctor keeps the high vsm bit, so VSM states
    // sit ABOVE a manual value (the precedence specificity_tests pins: manual < trigger < visual_state <
    // handler). This is the specificity a DIRECTLY-driven visual_state_manager uses.
    inline const setter_specificity setter_specificity::visual_state_setter{k_extras_vsm, 0, 0, 0, 0, 0, 0, 0};
    // The implicit-style-sourced VSM specificity (#18103/#34363): VSM extras with style == StyleImplicit, so
    // the ctor sets the implicit-VSM bit (bit 26) and clears the high VSM bit — landing BELOW a manual
    // value. This is the C# `vsgSpecificity.CopyStyle(1, 0, 0, 0)` for a VSGroupList set at StyleImplicit.
    inline const setter_specificity setter_specificity::visual_state_setter_system{k_extras_vsm,     0, 0, 0,
                                                                                   k_style_implicit, 0, 0, 0};
    inline const setter_specificity setter_specificity::from_binding{0, 0, 0, 1, 0, 0, 0, 0};
    inline const setter_specificity setter_specificity::manual_value_setter{0, 1, 0, 0, 0, 0, 0, 0};
    inline const setter_specificity setter_specificity::trigger{0, k_manual_trigger_baseline, 0, 0, 0, 0, 0, 0};
    inline const setter_specificity setter_specificity::dynamic_resource_setter{0, 0, 1, 0, 0, 0, 0, 0};
    inline const setter_specificity setter_specificity::from_handler{k_extras_handler, 0, 0, 0, 0, 0, 0, 0};
} // namespace maui::core
