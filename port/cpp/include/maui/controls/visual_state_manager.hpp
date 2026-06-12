#pragma once
// maui::controls::visual_state_manager  <=  Microsoft.Maui.Controls.VisualStateManager
//
// Visual states (Normal / Disabled / Focused / …) bundle setters that are applied while a control is in
// that state and un-applied on the way out. States are organized into mutually-exclusive groups; a
// control is in exactly one state per group. go_to_state() performs the transition: it finds the group
// owning the named state, un-applies the outgoing state's setters and applies the incoming state's, at
// the VSM specificity. Ported from VisualStateManager.cs (the GoToState core + IsSystemDrivenState).
//
// Specificity: by default state setters apply at setter_specificity::visual_state_setter, which sits
// ABOVE a manual value (manual < trigger < visual_state < handler) — the specificity a DIRECTLY-driven
// manager uses. When the VSGroups arrived through an IMPLICIT style (from_implicit_style), the base
// specificity is downgraded to visual_state_setter_system (BELOW a manual value, #18103); system-driven
// states (Disabled/Focused/Unfocused/Selected/PointerOver/Pressed) are then promoted back ABOVE manual
// via with_full_vsm_priority (#34363), while Normal + custom states keep the downgrade. The manager is
// told which case it is via mark_from_implicit_style() (merged_style sets it when it assigns the groups).
//
// Scope (M5d): the groups/states/setters data model + go_to_state + the implicit-style downgrade and
// system-state promotion. The manager can also be created and driven explicitly (the M5b face).
//
// W1-15 (styles tail) adds the ATTACHED-PROPERTY storage face + StateTriggers:
//   - view<>::set_visual_state_groups(manager) is the port of VisualStateManager.SetVisualStateGroups —
//     it calls replace_from(), which un-applies the OLD groups' current states (the
//     VisualStateGroupsPropertyChanged old-value branch), takes over the new groups, wires every
//     visual_state's state triggers (the trigger → group → element back-references, collapsed into the
//     owner hooks on state_trigger_base), runs ChangeVisualState, and re-evaluates the triggers.
//   - Trigger attach/detach (StateTriggerBase.SendAttached/SendDetached) follows the host element's
//     loaded/unloaded events — the port of VisualElement.OnWindowChanged → InvalidateStateTriggers.
//   - update_group_state_triggers ports VisualStateGroup.UpdateStateTriggers + GetActiveTrigger +
//     ResolveStateTriggersConflict (custom triggers outrank adaptive; ties broken by declaration order /
//     the largest MinWindowWidth then MinWindowHeight).
// A wired manager must not be moved (the trigger hooks capture its address); only the view's own member
// is ever wired, so the by-value manager builders in tests stay movable. Clone + duplicate-name
// validation remain deferred (STATUS.md).

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/setter.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
    class element;            // forward — the host of an attached group list (the VisualElement back-ref)
    class state_trigger_base; // forward — visual_state stores the triggers, state_trigger.hpp defines them

    // The framework-driven state names (VisualStateManager.CommonStates + ButtonElement.PressedVisualState).
    struct common_states
    {
        static constexpr std::string_view normal = "Normal";
        static constexpr std::string_view disabled = "Disabled";
        static constexpr std::string_view focused = "Focused";
        static constexpr std::string_view unfocused = "Unfocused";
        static constexpr std::string_view selected = "Selected";
        static constexpr std::string_view pointer_over = "PointerOver";
        static constexpr std::string_view pressed = "Pressed";
    };

    // Whether `name` is a state the MAUI framework drives automatically (VisualStateManager
    // .IsSystemDrivenState): Disabled / Focused / Unfocused / Selected / PointerOver / Pressed. Only these
    // promote an implicit-style VSM setter back to full priority, so a custom developer-defined state can't
    // unexpectedly override a manually-set value (#34363).
    [[nodiscard]] bool is_system_driven_state(std::string_view name);

    // One named state: the setters applied while the owning group is in this state.
    class visual_state
    {
    public:
        explicit visual_state(std::string name) : name_(std::move(name))
        {
        }

        visual_state& add(setter value)
        {
            setters_.push_back(std::move(value));
            return *this;
        }

        [[nodiscard]] const std::string& name() const
        {
            return name_;
        }

        // Apply / un-apply every setter at `specificity` (the manager picks system vs downgraded VSM).
        void apply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const;
        void unapply(maui::core::bindable_object& target, maui::core::setter_specificity specificity) const;

        // VisualState.StateTriggers: a trigger that activates this state automatically. Shared ownership —
        // the state owns the trigger; the manager wires its owner hooks when the groups are stored on a
        // view (set_visual_state_groups). Fluent, like add().
        visual_state& add_state_trigger(std::shared_ptr<state_trigger_base> trigger)
        {
            if (trigger != nullptr)
            {
                state_triggers_.push_back(std::move(trigger));
            }
            return *this;
        }
        [[nodiscard]] const std::vector<std::shared_ptr<state_trigger_base>>& state_triggers() const
        {
            return state_triggers_;
        }

    private:
        std::string name_;
        std::vector<setter> setters_;
        std::vector<std::shared_ptr<state_trigger_base>> state_triggers_; // VisualState.StateTriggers
    };

    // A set of mutually-exclusive states; the control is in at most one at a time (current_state).
    class visual_state_group
    {
    public:
        explicit visual_state_group(std::string name) : name_(std::move(name))
        {
        }

        visual_state_group& add(visual_state state)
        {
            states_.push_back(std::move(state));
            return *this;
        }

        [[nodiscard]] const std::string& name() const
        {
            return name_;
        }
        // The state named `name`, or nullptr if this group does not contain it.
        [[nodiscard]] const visual_state* find_state(std::string_view name) const;
        // The current state's name, or empty if the group has never transitioned.
        [[nodiscard]] std::string_view current_state_name() const
        {
            return current_state_name_;
        }
        // Manager-driven: record the active state (used by visual_state_manager::go_to_state).
        void set_current_state_name(std::string name)
        {
            current_state_name_ = std::move(name);
        }

        [[nodiscard]] const std::vector<visual_state>& states() const
        {
            return states_;
        }
        // VisualStateGroup.HasStateTriggers — whether any state in this group carries a state trigger.
        [[nodiscard]] bool has_state_triggers() const;
        // VisualStateGroup.GetActiveTrigger (+ ResolveStateTriggersConflict): the state selected by the
        // currently-active triggers, or nullptr when none is active. Conflict scoring: a custom
        // (non-adaptive) trigger wins (first declared on a tie); else the adaptive trigger with the
        // largest MinWindowWidth != -1; else the largest MinWindowHeight != -1.
        [[nodiscard]] const visual_state* active_trigger_state() const;

    private:
        friend class visual_state_manager; // wires the per-state triggers (mutable state access)

        std::string name_;
        std::vector<visual_state> states_;
        std::string current_state_name_;
    };

    class visual_state_manager
    {
    public:
        visual_state_manager() = default;
        // Moves stay available for the unwired by-value builders (a WIRED manager — the view's member —
        // is never moved; the trigger hooks capture its address). Copies are unavailable (the wiring
        // subscriptions are move-only).
        visual_state_manager(visual_state_manager&&) = default;
        visual_state_manager& operator=(visual_state_manager&&) = default;
        visual_state_manager(const visual_state_manager&) = delete;
        visual_state_manager& operator=(const visual_state_manager&) = delete;
        // Unwire on destruction so an EXTERNALLY-held shared state trigger never keeps a hook into a
        // dead manager (C# relies on weak refs there; the port detaches deterministically — §8).
        ~visual_state_manager()
        {
            unwire_state_triggers();
        }

        visual_state_manager& add_group(visual_state_group group)
        {
            groups_.push_back(std::move(group));
            if (visual_element_ != nullptr)
            {
                // A group added to an ATTACHED list wires (and possibly attaches) its triggers at once —
                // C# VisualStateGroupList.Validate sets group.VisualElement + UpdateStateTriggers on add.
                wire_group(groups_.size() - 1);
                update_group_state_triggers(groups_.size() - 1);
            }
            return *this;
        }

        [[nodiscard]] const std::vector<visual_state_group>& groups() const
        {
            return groups_;
        }
        // VisualStateManager.HasVisualStateGroups (collapsed: the port has no default-list sentinel).
        [[nodiscard]] bool has_groups() const
        {
            return !groups_.empty();
        }

        // Transition `target` to the state named `name`: find the group containing it, un-apply the
        // outgoing state's setters and apply the incoming state's, at the VSM specificity. Returns true if
        // the transition happened (or the target is already in that state), false if no group owns `name`.
        // Mirrors VisualStateManager.GoToState. System-driven states get full VSM priority; custom states
        // (and Normal) keep the base specificity (downgraded if from_implicit_style).
        bool go_to_state(maui::core::bindable_object& target, std::string_view name);

        // Mark these groups as having arrived through an implicit style (VSGroupList set at StyleImplicit) —
        // so their base specificity is the downgraded visual_state_setter_system. merged_style calls this
        // when it assigns the groups; a directly-driven manager leaves it false (full visual_state_setter).
        void mark_from_implicit_style()
        {
            from_implicit_style_ = true;
        }
        [[nodiscard]] bool from_implicit_style() const
        {
            return from_implicit_style_;
        }

        // ---- the attached-property storage face (W1-15) ----
        // VisualStateManager.SetVisualStateGroups + VisualStateGroupsPropertyChanged, called by
        // view<>::set_visual_state_groups on ITS value member (so `this` stays address-stable for the
        // trigger hooks): un-apply the OLD groups' current states from `target` (system-driven states at
        // the promoted specificity), detach + unwire the old triggers, take over `source`'s groups +
        // implicit-style flag, wire the new triggers (attach/detach driven by the element's
        // loaded/unloaded — InvalidateStateTriggers), run `change_visual_state` (the C#
        // visualElement.ChangeVisualState() call), then re-evaluate every group's triggers.
        void replace_from(visual_state_manager source, element& target, std::function<void()> change_visual_state);

        // VisualStateManager.UpdateStateTriggers — re-evaluate every group's active trigger.
        void update_state_triggers();

    private:
        // The base specificity for a state's setters: the downgraded system specificity when the groups came
        // from an implicit style, else the full VSM specificity. is_system_driven_state then decides whether
        // to promote it (only meaningful in the downgraded case — with_full_vsm_priority no-ops otherwise).
        [[nodiscard]] maui::core::setter_specificity base_specificity() const;

        // The VisualStateGroupsPropertyChanged old-value branch: un-apply each group's current state.
        void unapply_current_states(maui::core::bindable_object& target);
        // Wire / unwire one group's (or every) state triggers: the owner hooks + the attach lifecycle.
        void wire_group(std::size_t group_index);
        void unwire_state_triggers();
        // VisualElement.InvalidateStateTriggers: send_attached (true) / send_detached (false) to all.
        void invalidate_state_triggers(bool attach);
        // VisualStateGroup.UpdateStateTriggers: transition to the group's active-trigger state (if any).
        void update_group_state_triggers(std::size_t group_index);

        std::vector<visual_state_group> groups_;
        bool from_implicit_style_ = false;
        // The attached host (VisualStateGroupList.VisualElement) + its ChangeVisualState callback. Both
        // null/empty until the groups are stored on a view via set_visual_state_groups. NON-owning — the
        // manager is a member of the host view.
        element* visual_element_ = nullptr;
        std::function<void()> change_visual_state_;
        maui::core::scoped_connection loaded_token_;   // host loaded → InvalidateStateTriggers(true)
        maui::core::scoped_connection unloaded_token_; // host unloaded → InvalidateStateTriggers(false)
    };
} // namespace maui::controls
