#pragma once
// maui::controls effect registry  <=  Microsoft.Maui.Controls.Internals.Registrar.Effects
//                                      + Microsoft.Maui.Controls.Effect.Resolve (the lookup half)
//
// MAUI resolves an effect from a string id by reflection over the [assembly: ExportEffect] attributes
// that populate Internals.Registrar.Effects, then DependencyResolver.ResolveOrCreate instantiates the
// CLR type. C++23 has no reflection (PROFILE §6), so this is EXPLICIT registration, exactly like the
// handler_registry: register_effect("Group.Name", factory) records a factory keyed by the id, and
// resolve_effect("Group.Name") instantiates one (or returns null when the id is unknown — the caller,
// effect::resolve, then substitutes a null_effect, mirroring Effect.Resolve's NullEffect fallback).
//
// The id is the C# "ResolutionGroupName.ExportEffect" string a developer passes to RoutingEffect's ctor.

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace maui::controls
{
    class effect; // forward — the abstract base produced by the factories (effect.hpp)

    // The factory shape: returns a freshly-constructed effect (the platform_effect a routing_effect wraps,
    // or any concrete effect). A null return is treated as "unknown" by resolve_effect (it falls through to
    // the not-registered path, like a missing Registrar.Effects entry).
    using effect_factory = std::function<std::shared_ptr<effect>()>;

    // Register (or replace) the factory that produces the effect for `id` — the explicit analog of the
    // [ExportEffect] attribute MAUI scans. The id is the "Group.Name" resolution id. Idempotent: a second
    // registration under the same id replaces the first.
    void register_effect(std::string id, effect_factory factory);

    // Instantiate the effect registered for `id`, or nullptr when no factory is registered (the unknown-id
    // case). Mirrors Registrar.Effects.TryGetValue + DependencyResolver.ResolveOrCreate. The returned effect
    // has NOT had its resolve_id set — effect::resolve assigns it (matching Effect.Resolve, which sets
    // ResolveId on the result regardless of whether it was found).
    [[nodiscard]] std::shared_ptr<effect> resolve_effect(std::string_view id);

    // Whether an `id` has a registered factory (test/diagnostic aid; no C# surface — the registry is
    // internal there).
    [[nodiscard]] bool is_effect_registered(std::string_view id);
} // namespace maui::controls
