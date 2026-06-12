#pragma once
// maui::controls::radio_button_group  <=  Microsoft.Maui.Controls.RadioButtonGroup
//                                         (+ the internal RadioButtonGroupController)
//
// The attached grouping surface for radio buttons: a GroupName + SelectedValue pair attached to ANY
// container element, with a per-container controller that pushes the group name down to descendant
// radio buttons, tracks the selected value, and (with the radio_button control) enforces the
// one-checked-per-group invariant. Ported from RadioButtonGroup.cs + RadioButtonGroupController.cs.
//
// Attached-property storage (the C# RadioButtonGroupControllerProperty, translated): C# stores the
// controller in the layout's per-instance BindableProperty store; the port stores a
// shared_ptr<radio_button_group_controller> in the element's own resource dictionary under
// k_controller_resource_key — the element's general per-instance store. The controller therefore dies
// WITH its container (deterministic teardown, PROFILE §8); each radio button holds only a weak_ptr
// association (the ConditionalWeakTable analog), so a dangling group is impossible.
//
// DescendantAdded translation: the port's element tree has no DescendantAdded event — instead
// radio_button overrides element::on_resource_chain_changed (called on the element and its whole
// subtree whenever an ancestor attaches/detaches) and walks UP the parent chain to the NEAREST
// element carrying a named controller, running the C# AddRadioButton semantics (adopt the group name
// when the button has none, associate, push/auto-check the selection). The downward walks
// (UpdateGroupNames / SetSelectedValue / the mutual exclusion) enumerate descendants through the
// public i_container / i_content_view faces — the port's LogicalChildren equivalents.
//
// Scope notes (documented deviations): SelectedValue is the port's boxed std::any compared via
// maui::core::boxed_equals (the object.Equals analog — see boxed_value.hpp for the lattice);
// `selected_value_changed` stands in for the TwoWay attached-property observability; the visual root
// (RadioButtonGroup.GetVisualRoot) stops at the nearest content_page ancestor (the port's concrete
// page type). The controller is public (C#'s is internal) because radio_button's inline members
// reference it — it shares this header as a tightly-coupled cluster (PROFILE §3).

#include <any>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/event.hpp"

namespace maui::controls
{
    class element;      // the attach target (any container element)
    class radio_button; // forward — the controller drives the buttons; radio_button.hpp includes us back

    // The reserved resource key the controller is stored under (the port's analog of the private
    // RadioButtonGroupControllerProperty attached storage).
    inline constexpr std::string_view k_controller_resource_key = "Maui.RadioButtonGroup.Controller";

    // RadioButtonGroupController: the per-container grouping state. Created lazily by
    // radio_button_group's setters (the C# defaultValueCreator) and owned by the container element.
    // enable_shared_from_this lets the downward walks hand each adopted button its weak association
    // (the controller is always created into a shared_ptr by radio_button_group::controller_of).
    class radio_button_group_controller : public std::enable_shared_from_this<radio_button_group_controller>
    {
    public:
        explicit radio_button_group_controller(element& container) : layout_(&container)
        {
        }

        // The container this controller manages (RadioButtonGroupController.Layout). Non-owning
        // back-reference: the container owns the controller, so it always outlives it.
        [[nodiscard]] element& layout() const
        {
            return *layout_;
        }

        [[nodiscard]] const std::string& group_name() const
        {
            return group_name_;
        }
        // SetGroupName: store, then push the (new) name down to every descendant radio button whose
        // name is empty or still carries the old name (UpdateGroupNames → UpdateGroupName).
        void set_group_name(std::string value);

        [[nodiscard]] const std::any& selected_value() const
        {
            return selected_value_;
        }
        // SetSelectedValue: no-op when boxed-equal to the current value; otherwise store and sweep the
        // group — a non-null value checks the matching button, null unchecks the checked one.
        void set_selected_value(std::any value);

        // Raised after the selected value changes (the port's observability for the TwoWay attached
        // SelectedValueProperty), with the new boxed value.
        maui::core::event<const std::any&> selected_value_changed;

        // ---- the radio_button-facing seams (internal in C#) ----
        // HandleRadioButtonValueChanged: a checked button's Value changed — refresh the selection.
        void handle_radio_button_value_changed(radio_button& button);
        // HandleRadioButtonGroupNameChanged: a button left this group — clear the selection.
        void handle_radio_button_group_name_changed(std::string_view old_group_name);
        // HandleRadioButtonGroupSelectionChanged: a button of this group was checked — record its value.
        void handle_radio_button_group_selection_changed(radio_button& button);
        // AddRadioButton (the DescendantAdded path): adopt the group name when the button has none,
        // associate the button with this controller (first association wins), push a checked button's
        // value into the selection, and auto-check a button matching a pre-set non-null selection.
        void add_radio_button(radio_button& button);

    private:
        // UpdateGroupName: adopt `name` when the button's group name is empty or equals `old_name`,
        // then associate the button with this controller if it has none.
        void update_group_name(radio_button& button, const std::string& name, const std::string& old_name);

        element* layout_;        // non-owning: the container owns this controller (see the header note)
        std::string group_name_; // RadioButtonGroupController._groupName
        std::any selected_value_;
    };

    // The attached-property API (the C# static RadioButtonGroup class). The getters return the
    // attached defaults (empty name / null value) when no controller exists; the setters create the
    // controller on demand (the C# defaultValueCreator).
    class radio_button_group
    {
    public:
        radio_button_group() = delete; // static class (C# `public static class RadioButtonGroup`)

        // RadioButtonGroup.SetGroupName / GetGroupName (attached to a container element).
        static void set_group_name(element& container, std::string value);
        [[nodiscard]] static std::string group_name(element& container);

        // RadioButtonGroup.SetSelectedValue / GetSelectedValue.
        static void set_selected_value(element& container, std::any value);
        [[nodiscard]] static std::any selected_value(element& container);

        // The container's controller, created on demand (GetRadioButtonGroupController + the
        // defaultValueCreator). Shared ownership lives in the container's resource dictionary; the
        // returned shared_ptr is a borrow of it.
        [[nodiscard]] static std::shared_ptr<radio_button_group_controller> controller_of(element& container);
        // The container's controller if one was ever created, else null (no side effects). Non-const
        // because the lookup goes through element::resources() (lazily created; here it short-circuits
        // on is_resources_created(), so no dictionary is materialized).
        [[nodiscard]] static std::shared_ptr<radio_button_group_controller> existing_controller_of(element& container);

        // ---- the radio_button-facing seams (internal in C#) ----
        // UpdateRadioButtonGroup: uncheck every other button in the checked button's scope, then let
        // its controller record the new selection.
        static void update_radio_button_group(radio_button& button);
    };
} // namespace maui::controls
