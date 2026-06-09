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
// system-state promotion. The attached-property storage on the element (VisualStateGroupsProperty),
// StateTriggers, the is_enabled→Disabled auto-drive, and Clone/duplicate-name validation are deferred
// (STATUS.md). The manager is created and driven explicitly.

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/setter.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::controls
{
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

    private:
        std::string name_;
        std::vector<setter> setters_;
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

    private:
        std::string name_;
        std::vector<visual_state> states_;
        std::string current_state_name_;
    };

    class visual_state_manager
    {
    public:
        visual_state_manager& add_group(visual_state_group group)
        {
            groups_.push_back(std::move(group));
            return *this;
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

    private:
        // The base specificity for a state's setters: the downgraded system specificity when the groups came
        // from an implicit style, else the full VSM specificity. is_system_driven_state then decides whether
        // to promote it (only meaningful in the downgraded case — with_full_vsm_priority no-ops otherwise).
        [[nodiscard]] maui::core::setter_specificity base_specificity() const;

        std::vector<visual_state_group> groups_;
        bool from_implicit_style_ = false;
    };
} // namespace maui::controls
