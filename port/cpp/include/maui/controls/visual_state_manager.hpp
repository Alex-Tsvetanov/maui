#pragma once
// maui::controls::visual_state_manager  <=  Microsoft.Maui.Controls.VisualStateManager
//
// Visual states (Normal / Disabled / Focused / …) bundle setters that are applied while a control is in
// that state and un-applied on the way out. States are organized into mutually-exclusive groups; a
// control is in exactly one state per group. go_to_state() performs the transition: it finds the group
// owning the named state, un-applies the outgoing state's setters and applies the incoming state's, at
// the VSM specificity. Ported from VisualStateManager.cs (the GoToState core + IsSystemDrivenState).
//
// Specificity: state setters apply at setter_specificity::visual_state_setter, which sits ABOVE a manual
// value (manual < trigger < visual_state < handler). The #18103/#34363 nuance — implicit-style VSM
// downgraded below manual, then promoted back for system-driven states (Disabled/Focused/…) via
// WithFullVsmPriority — only arises when VisualStateGroups is assigned through an implicit style; for a
// directly-driven manager every state shares the one VSM specificity. common_states carries the
// framework state names so that nuance can slot in later without renaming.
//
// Scope (M5b): the groups/states/setters data model + go_to_state. The attached-property storage on the
// element (VisualStateGroupsProperty), the implicit-style downgrade + system-state promotion,
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
        // Mirrors VisualStateManager.GoToState.
        bool go_to_state(maui::core::bindable_object& target, std::string_view name);

    private:
        std::vector<visual_state_group> groups_;
    };
} // namespace maui::controls
