// maui::controls::visual_state_manager — the GoToState transition core + the attached-property storage
// face and StateTriggers wiring (visual_state_manager.hpp). Ported from VisualStateManager.cs (GoToState,
// VisualStateGroupsPropertyChanged, UpdateStateTriggers, VisualStateGroup.GetActiveTrigger /
// ResolveStateTriggersConflict) and VisualElement.InvalidateStateTriggers.
#include "maui/controls/visual_state_manager.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/state_trigger.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
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

    bool visual_state_group::has_state_triggers() const
    {
        // VisualStateGroup.HasStateTriggers
        return std::ranges::any_of(states_,
                                   [](const visual_state& state) { return !state.state_triggers().empty(); });
    }

    namespace
    {
        // One active trigger + the state that owns it (the port's stand-in for trigger.VisualState).
        struct active_trigger
        {
            const state_trigger_base* trigger;
            const visual_state* state;
        };

        // VisualStateGroup.ResolveStateTriggersConflict: 1. custom triggers (non-adaptive) beat adaptive
        // ones (first declared wins a custom tie); 2. else the adaptive trigger with the LARGEST
        // MinWindowWidth != -1; 3. else the largest MinWindowHeight != -1; 4. else none.
        const visual_state* resolve_state_triggers_conflict(const std::vector<active_trigger>& conflicts)
        {
            const active_trigger* first_custom = nullptr;
            std::size_t custom_count = 0;
            for (const active_trigger& candidate : conflicts)
            {
                if (dynamic_cast<const adaptive_trigger*>(candidate.trigger) == nullptr)
                {
                    if (first_custom == nullptr)
                    {
                        first_custom = &candidate;
                    }
                    ++custom_count;
                }
            }
            if (custom_count > 1)
            {
                return first_custom->state;
            }
            const active_trigger* best_width = nullptr;
            const active_trigger* best_height = nullptr;
            for (const active_trigger& candidate : conflicts)
            {
                const auto* adaptive = dynamic_cast<const adaptive_trigger*>(candidate.trigger);
                if (adaptive == nullptr)
                {
                    continue;
                }
                if (adaptive->min_window_width() != -1.0 &&
                    (best_width == nullptr ||
                     adaptive->min_window_width() >
                         dynamic_cast<const adaptive_trigger*>(best_width->trigger)->min_window_width()))
                {
                    best_width = &candidate;
                }
                if (adaptive->min_window_height() != -1.0 &&
                    (best_height == nullptr ||
                     adaptive->min_window_height() >
                         dynamic_cast<const adaptive_trigger*>(best_height->trigger)->min_window_height()))
                {
                    best_height = &candidate;
                }
            }
            if (best_width != nullptr)
            {
                return best_width->state;
            }
            if (best_height != nullptr)
            {
                return best_height->state;
            }
            return nullptr;
        }
    } // namespace

    const visual_state* visual_state_group::active_trigger_state() const
    {
        // VisualStateGroup.GetActiveTrigger: the first state with an active trigger wins outright unless
        // several triggers are active at once — then the conflict scoring above decides.
        const visual_state* result = nullptr;
        std::vector<active_trigger> conflicts;
        for (const visual_state& state : states_)
        {
            for (const std::shared_ptr<state_trigger_base>& trigger : state.state_triggers())
            {
                if (trigger->is_active())
                {
                    if (result == nullptr)
                    {
                        result = &state;
                    }
                    conflicts.push_back({.trigger = trigger.get(), .state = &state});
                }
            }
        }
        if (conflicts.size() > 1)
        {
            result = resolve_state_triggers_conflict(conflicts);
        }
        return result;
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

    // ---- the attached-property storage face + StateTriggers wiring (W1-15) ----

    void visual_state_manager::replace_from(visual_state_manager source, element& target,
                                            std::function<void()> change_visual_state)
    {
        // The VisualStateGroupsPropertyChanged old-value branch: un-apply the outgoing groups' current
        // states (still at the OLD list's base specificity) and detach/unwire their triggers. C# leaves
        // the old triggers' platform subscriptions to the GC; the port detaches deterministically (§8).
        unapply_current_states(target);
        unwire_state_triggers();

        groups_ = std::move(source.groups_);
        from_implicit_style_ = source.from_implicit_style_;
        visual_element_ = &target;
        change_visual_state_ = std::move(change_visual_state);

        // SendAttached/SendDetached follow the host's window membership (VisualElement.OnWindowChanged →
        // InvalidateStateTriggers). The host element outlives the manager (its value member), so the
        // subscriptions are torn down first on destruction.
        loaded_token_ = maui::core::connect_scoped(target.loaded, [this] { invalidate_state_triggers(true); });
        unloaded_token_ = maui::core::connect_scoped(target.unloaded, [this] { invalidate_state_triggers(false); });

        for (std::size_t i = 0; i < groups_.size(); ++i)
        {
            wire_group(i);
        }
        if (change_visual_state_)
        {
            change_visual_state_(); // visualElement.ChangeVisualState()
        }
        update_state_triggers(); // VisualStateManager.UpdateStateTriggers(visualElement)
    }

    void visual_state_manager::update_state_triggers()
    {
        for (std::size_t i = 0; i < groups_.size(); ++i)
        {
            update_group_state_triggers(i);
        }
    }

    void visual_state_manager::unapply_current_states(maui::core::bindable_object& target)
    {
        const maui::core::setter_specificity base = base_specificity();
        for (visual_state_group& group : groups_)
        {
            if (group.current_state_name().empty())
            {
                continue;
            }
            const visual_state* current = group.find_state(group.current_state_name());
            if (current != nullptr)
            {
                // Mirror the go_to_state rule: a system-driven state was applied promoted, so it
                // un-applies promoted too (VisualStateGroupsPropertyChanged's unapplySpecificity).
                const maui::core::setter_specificity specificity =
                    is_system_driven_state(group.current_state_name()) ? base.with_full_vsm_priority() : base;
                current->unapply(target, specificity);
            }
            group.set_current_state_name(std::string{});
        }
    }

    void visual_state_manager::wire_group(std::size_t group_index)
    {
        // The collapsed back-reference chain: trigger → (VisualState → VisualStateGroup → VisualElement)
        // becomes the host element + a group-update hook. The hook captures `this` + the group INDEX
        // (never a pointer into the groups vector), so add_group's reallocation cannot dangle it; a
        // wired manager itself is address-stable (the view's value member).
        for (const visual_state& state : groups_[group_index].states_)
        {
            for (const std::shared_ptr<state_trigger_base>& trigger : state.state_triggers())
            {
                trigger->set_owner(visual_element_, [this, group_index] { update_group_state_triggers(group_index); });
            }
        }
    }

    void visual_state_manager::unwire_state_triggers()
    {
        for (const visual_state_group& group : groups_)
        {
            for (const visual_state& state : group.states_)
            {
                for (const std::shared_ptr<state_trigger_base>& trigger : state.state_triggers())
                {
                    trigger->send_detached();
                    trigger->set_owner(nullptr, {});
                }
            }
        }
    }

    void visual_state_manager::invalidate_state_triggers(bool attach)
    {
        // VisualElement.InvalidateStateTriggers: walk every group/state/trigger and flip the attachment.
        for (const visual_state_group& group : groups_)
        {
            for (const visual_state& state : group.states_)
            {
                for (const std::shared_ptr<state_trigger_base>& trigger : state.state_triggers())
                {
                    if (attach)
                    {
                        trigger->send_attached();
                    }
                    else
                    {
                        trigger->send_detached();
                    }
                }
            }
        }
    }

    void visual_state_manager::update_group_state_triggers(std::size_t group_index)
    {
        // VisualStateGroup.UpdateStateTriggers: nothing to do without a host or without triggers; a
        // no-active-trigger evaluation leaves the current state alone; otherwise transition to the
        // selected state (go_to_state no-ops when it is already current).
        if (visual_element_ == nullptr || group_index >= groups_.size())
        {
            return;
        }
        const visual_state_group& group = groups_[group_index];
        if (!group.has_state_triggers())
        {
            return;
        }
        const visual_state* target_state = group.active_trigger_state();
        if (target_state == nullptr || group.current_state_name() == target_state->name())
        {
            return;
        }
        go_to_state(*visual_element_, target_state->name());
    }
} // namespace maui::controls
