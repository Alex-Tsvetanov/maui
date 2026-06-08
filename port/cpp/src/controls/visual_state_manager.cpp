// maui::controls::visual_state_manager — the GoToState transition core (visual_state_manager.hpp).
#include "maui/controls/visual_state_manager.hpp"

#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
    void visual_state::apply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
    {
        for (const auto& value : setters_)
        {
            value.apply(target, specificity);
        }
    }

    void visual_state::unapply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const
    {
        for (const auto& value : setters_)
        {
            value.unapply(target, specificity);
        }
    }

    const visual_state* visual_state_group::find_state(std::string_view name) const
    {
        for (const auto& state : states_)
        {
            if (state.name() == name)
            {
                return &state;
            }
        }
        return nullptr;
    }

    bool visual_state_manager::go_to_state(maui::core::bindable_object& target, std::string_view name)
    {
        // For a directly-driven VSM, every state's setters apply at the VSM specificity (above a manual
        // value). The system-vs-custom split (implicit-style VSM downgraded below manual, then promoted
        // back for system states via WithFullVsmPriority) only manifests when VisualStateGroups is assigned
        // through an implicit style — deferred with implicit styles (STATUS.md), so all states share one
        // specificity here.
        const maui::core::setter_specificity specificity = maui::core::setter_specificity::visual_state_setter;

        for (auto& group : groups_)
        {
            if (group.current_state_name() == name)
            {
                return true; // already in the target state for this group
            }
            const visual_state* target_state = group.find_state(name);
            if (target_state == nullptr)
            {
                continue; // this group does not own `name` — try the next
            }
            // Un-apply the outgoing state before applying the incoming one.
            if (const visual_state* current = group.find_state(group.current_state_name()); current != nullptr)
            {
                current->unapply(target, specificity);
            }
            group.set_current_state_name(std::string{name});
            target_state->apply(target, specificity);
            return true;
        }
        return false;
    }
} // namespace maui::controls
