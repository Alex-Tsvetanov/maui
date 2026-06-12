// maui::controls::radio_button_group — the attached grouping machinery: the controller's downward
// walks (UpdateGroupNames / SetSelectedValue / AddRadioButton), the attached API over the element's
// resource-dictionary storage, and the mutual-exclusion scope walk (UncheckOtherRadioButtonsInScope +
// GetVisualRoot). See radio_button_group.hpp for the C# mapping notes.

#include "maui/controls/radio_button_group.hpp"

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/core/boxed_value.hpp"
#include "maui/core/i_container.hpp"
#include "maui/core/i_content_view.hpp"

namespace maui::controls
{
    namespace
    {
        // Visit every descendant element of `root`, in tree order — the port's Element.Descendants()
        // over the public container faces: i_container exposes a layout's children, i_content_view a
        // page's / content view's single child. (Templated content presents through these same faces.)
        void for_each_descendant(element& root, const std::function<void(element&)>& visit)
        {
            if (auto* container = dynamic_cast<maui::core::i_container*>(&root))
            {
                for (int index = 0; index < container->count(); ++index)
                {
                    if (auto* child = dynamic_cast<element*>(&container->at(index)))
                    {
                        visit(*child);
                        for_each_descendant(*child, visit);
                    }
                }
            }
            if (auto* content_host = dynamic_cast<maui::core::i_content_view*>(&root))
            {
                if (auto* child = dynamic_cast<element*>(content_host->content()))
                {
                    visit(*child);
                    for_each_descendant(*child, visit);
                }
            }
        }

        // RadioButtonGroup.GetVisualRoot: the nearest page ancestor (content_page — the port's concrete
        // page type), or null when the element isn't hosted in a page yet.
        element* visual_root(element& start)
        {
            element* parent = start.logical_parent();
            while (parent != nullptr && dynamic_cast<content_page*>(parent) == nullptr)
            {
                parent = parent->logical_parent();
            }
            return parent;
        }

        // RadioButtonGroup.UncheckRadioButtonIfChecked — the from-handler uncheck.
        void uncheck_if_checked(radio_button& candidate, const radio_button& checked_button)
        {
            if (&candidate != &checked_button && candidate.is_checked())
            {
                candidate.send_is_checked(false);
            }
        }

        // RadioButtonGroup.UncheckOtherRadioButtonsInScope: a NAMED group unchecks matching descendants
        // of the visual root (falling back to the controller's container, then the parent); the implied
        // (nameless) group unchecks only nameless SIBLINGS under the same parent.
        void uncheck_other_radio_buttons_in_scope(radio_button& button)
        {
            if (!button.group_name().empty())
            {
                element* root = visual_root(button);
                if (root == nullptr)
                {
                    if (const auto controller = button.group_controller())
                    {
                        root = &controller->layout();
                    }
                }
                if (root == nullptr)
                {
                    root = button.logical_parent();
                }
                if (root == nullptr)
                {
                    return;
                }
                for_each_descendant(*root, [&button](element& descendant) {
                    auto* other = dynamic_cast<radio_button*>(&descendant);
                    if (other != nullptr && other->group_name() == button.group_name())
                    {
                        uncheck_if_checked(*other, button);
                    }
                });
                return;
            }

            auto* parent_container = dynamic_cast<maui::core::i_container*>(button.logical_parent());
            if (parent_container == nullptr)
            {
                return;
            }
            for (int index = 0; index < parent_container->count(); ++index)
            {
                auto* sibling = dynamic_cast<radio_button*>(&parent_container->at(index));
                if (sibling != nullptr && sibling->group_name().empty())
                {
                    uncheck_if_checked(*sibling, button);
                }
            }
        }
    } // namespace

    // ---- radio_button_group_controller ----

    void radio_button_group_controller::set_group_name(std::string value)
    {
        // SetGroupName: store FIRST (a button renamed by the walk below consults the controller's
        // CURRENT name in OnGroupNamePropertyChanged — C# order), then push down to the descendants.
        const std::string old_name = std::exchange(group_name_, std::move(value));
        for_each_descendant(*layout_, [this, &old_name](element& descendant) {
            if (auto* button = dynamic_cast<radio_button*>(&descendant))
            {
                update_group_name(*button, group_name_, old_name);
            }
        });
    }

    void radio_button_group_controller::set_selected_value(std::any value)
    {
        // SetSelectedValue: the boxed-equality gate also absorbs the re-entrant call a checked button
        // makes back through HandleRadioButtonGroupSelectionChanged.
        if (maui::core::boxed_equals(selected_value_, value))
        {
            return;
        }
        selected_value_ = std::move(value);
        for_each_descendant(*layout_, [this](element& descendant) {
            auto* button = dynamic_cast<radio_button*>(&descendant);
            if (button == nullptr || button->group_name() != group_name_)
            {
                return;
            }
            if (selected_value_.has_value())
            {
                if (button->value().has_value() && maui::core::boxed_equals(button->value(), selected_value_))
                {
                    button->send_is_checked(true);
                }
            }
            else if (button->is_checked())
            {
                // Setting null unchecks the group's selected button.
                button->send_is_checked(false);
            }
        });
        selected_value_changed.raise(selected_value_);
    }

    void radio_button_group_controller::handle_radio_button_value_changed(radio_button& button)
    {
        if (button.group_name() != group_name_)
        {
            return;
        }
        set_selected_value(button.value());
    }

    void radio_button_group_controller::handle_radio_button_group_name_changed(std::string_view old_group_name)
    {
        if (old_group_name != group_name_)
        {
            return;
        }
        // C# ClearValue(SelectedValueProperty) → SetSelectedValue(null).
        set_selected_value(std::any{});
    }

    void radio_button_group_controller::handle_radio_button_group_selection_changed(radio_button& button)
    {
        if (button.group_name() != group_name_)
        {
            return;
        }
        set_selected_value(button.value());
    }

    void radio_button_group_controller::add_radio_button(radio_button& button)
    {
        update_group_name(button, group_name_, std::string{});
        if (button.is_checked())
        {
            set_selected_value(button.value());
        }
        // Only auto-check against an EXPLICIT (non-null) selection — adding a button whose Value is
        // also null must not auto-check it (the C# #34759 guard).
        if (selected_value_.has_value() && maui::core::boxed_equals(button.value(), selected_value_))
        {
            button.send_is_checked(true);
        }
    }

    void radio_button_group_controller::update_group_name(radio_button& button, const std::string& name,
                                                          const std::string& old_name)
    {
        const std::string current = button.group_name();
        if (current.empty() || current == old_name)
        {
            button.set_group_name(name);
        }
        // First association wins (the C# groupControllers TryGetValue-before-Add guard).
        if (!button.group_controller())
        {
            button.set_group_controller(shared_from_this());
        }
    }

    // ---- radio_button_group (the attached API) ----

    void radio_button_group::set_group_name(element& container, std::string value)
    {
        controller_of(container)->set_group_name(std::move(value));
    }

    std::string radio_button_group::group_name(element& container)
    {
        const auto controller = existing_controller_of(container);
        return controller ? controller->group_name() : std::string{};
    }

    void radio_button_group::set_selected_value(element& container, std::any value)
    {
        controller_of(container)->set_selected_value(std::move(value));
    }

    std::any radio_button_group::selected_value(element& container)
    {
        const auto controller = existing_controller_of(container);
        return controller ? controller->selected_value() : std::any{};
    }

    std::shared_ptr<radio_button_group_controller> radio_button_group::controller_of(element& container)
    {
        if (auto existing = existing_controller_of(container))
        {
            return existing;
        }
        auto controller = std::make_shared<radio_button_group_controller>(container);
        container.resources().set(std::string{k_controller_resource_key}, std::any{controller});
        return controller;
    }

    std::shared_ptr<radio_button_group_controller> radio_button_group::existing_controller_of(element& container)
    {
        if (!container.is_resources_created())
        {
            return nullptr;
        }
        const auto* held =
            container.resources().get<std::shared_ptr<radio_button_group_controller>>(k_controller_resource_key);
        return held != nullptr ? *held : nullptr;
    }

    void radio_button_group::update_radio_button_group(radio_button& button)
    {
        uncheck_other_radio_buttons_in_scope(button);
        // RadioButton.OnGroupSelectionChanged: the button's own controller records the selection.
        if (const auto controller = button.group_controller())
        {
            controller->handle_radio_button_group_selection_changed(button);
        }
    }
} // namespace maui::controls
