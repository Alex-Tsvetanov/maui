#pragma once
// maui::core::binding_mode  <=  Microsoft.Maui.Controls.BindingMode
//
// The direction(s) a binding flows. Ported from src/Controls/src/Core/BindingMode.cs.
//   - one_way            source -> target (the target updates when the source changes)
//   - two_way            source <-> target (both update when either changes)
//   - one_time           source -> target once, at bind time (no later updates)
//   - one_way_to_source  target -> source (the source updates when the target changes)
//   - default_mode       resolve to the target bindable_property's default_binding_mode (else one_way)
//
// C# resolves `Default` against `BindableProperty.DefaultBindingMode`, and downgrades two_way to
// one_way_to_source on a read-only target — see binding.hpp's resolve_binding_mode().

#include <cstdint>

namespace maui::core
{
    enum class binding_mode : std::uint8_t
    {
        default_mode,
        one_way,
        two_way,
        one_time,
        one_way_to_source,
    };
} // namespace maui::core
