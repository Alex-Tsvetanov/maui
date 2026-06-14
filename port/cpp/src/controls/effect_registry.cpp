// maui::controls effect registry (effect_registry.hpp) — the explicit, reflection-free effect factory
// table standing in for Internals.Registrar.Effects + DependencyResolver.ResolveOrCreate.
#include "maui/controls/effect_registry.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "maui/controls/effect.hpp"

namespace maui::controls
{
    namespace
    {
        // Process-wide id -> factory table (the analog of Registrar.Effects). A Meyers singleton avoids
        // static-init-order issues for any registrar that runs during dynamic initialization, mirroring
        // default_handler_registry().
        std::unordered_map<std::string, effect_factory>& effect_factories()
        {
            static std::unordered_map<std::string, effect_factory> table;
            return table;
        }
    } // namespace

    void register_effect(std::string id, effect_factory factory)
    {
        effect_factories().insert_or_assign(std::move(id), std::move(factory));
    }

    std::shared_ptr<effect> resolve_effect(std::string_view id)
    {
        const auto& table = effect_factories();
        if (const auto it = table.find(std::string{id}); it != table.end() && it->second)
        {
            return it->second();
        }
        return nullptr;
    }

    bool is_effect_registered(std::string_view id)
    {
        return effect_factories().contains(std::string{id});
    }
} // namespace maui::controls
