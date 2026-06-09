// maui::controls::visual_state_manager — the GoToState transition core (visual_state_manager.hpp).
#include "maui/controls/visual_state_manager.hpp"

#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
    bool is_system_driven_state(std::string_view name)
    {
        return name == common_states::disabled || name == common_states::focused || name == common_states::unfocused ||
               name == common_states::selected || name == common_states::pointer_over || name == common_states::pressed;
    }

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

    maui::core::setter_specificity visual_state_manager::base_specificity() const
    {
        // A directly-driven VSM applies at the full VSM specificity (above a manual value). When the groups
        // arrived via an implicit style, the base is the downgraded system specificity (below manual) —
        // mirroring C#'s `vsgSpecificity.CopyStyle(1, 0, 0, 0)`, where vsgSpecificity is StyleImplicit.
        return from_implicit_style_ ? maui::core::setter_specificity::visual_state_setter_system
                                    : maui::core::setter_specificity::visual_state_setter;
    }

    bool visual_state_manager::go_to_state(maui::core::bindable_object& target, std::string_view name)
    {
        const maui::core::setter_specificity base = base_specificity();
        // System-driven states (Disabled/Focused/…) promote a downgraded implicit-style VSM back above a
        // manual value; Normal + custom states keep the base. with_full_vsm_priority is a no-op when `base`
        // is the full (non-implicit) VSM specificity, so a directly-driven manager is unaffected.
        const maui::core::setter_specificity apply_specificity =
            is_system_driven_state(name) ? base.with_full_vsm_priority() : base;

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
            // Un-apply the outgoing state before applying the incoming one, at ITS own promoted/base
            // specificity (the outgoing state may have been a system state — mirror the apply rule).
            if (const visual_state* current = group.find_state(group.current_state_name()); current != nullptr)
            {
                const maui::core::setter_specificity unapply_specificity =
                    is_system_driven_state(group.current_state_name()) ? base.with_full_vsm_priority() : base;
                current->unapply(target, unapply_specificity);
            }
            group.set_current_state_name(std::string{name});
            target_state->apply(target, apply_specificity);
            return true;
        }
        return false;
    }
} // namespace maui::controls
