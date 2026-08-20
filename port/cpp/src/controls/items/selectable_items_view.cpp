// selectable_items_view — descriptors + the selection choreography (SelectableItemsView.cs; the
// notification order is documented in the header).

#include "maui/controls/items/selectable_items_view.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_list.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::controls
{
    // The descriptor callbacks reach the private machinery (the picker_descriptor_access precedent).
    struct selectable_items_view_descriptor_access
    {
        static void on_selected_item_changed(selectable_items_view& self, const boxed_item& old_value,
                                             const boxed_item& new_value)
        {
            // C# SelectedItemPropertyChanged: a single-item args through the shared path.
            self.selection_property_changed(selection_changed_event_args::from_single(old_value, new_value));
        }

        static void on_selection_mode_changed(selectable_items_view& self, controls::selection_mode old_mode,
                                              controls::selection_mode new_mode)
        {
            self.on_selection_mode_changed(old_mode, new_mode);
        }
    };

    selectable_items_view::selectable_items_view() : selected_items_(std::make_unique<selection_list>(*this))
    {
    }

    // SelectableItemsView.SelectionModeProperty: default None; a change diffs the selections.
    const maui::core::bindable_property<selection_mode>& selectable_items_view::selection_mode_property()
    {
        static const maui::core::bindable_property<controls::selection_mode> descriptor{
            "selection_mode",
            controls::selection_mode::none,
            {.property_changed = [](maui::core::bindable_object& bindable, const controls::selection_mode& old_value,
                                    const controls::selection_mode& new_value) {
                selectable_items_view_descriptor_access::on_selection_mode_changed(
                    dynamic_cast<selectable_items_view&>(bindable), old_value, new_value);
            }}};
        return descriptor;
    }

    // SelectableItemsView.SelectedItemProperty: default null, TwoWay.
    // SelectedItemsProperty (SelectableItemsView.cs:31) — the bindable INPUT slot. See the header for why
    // this is a slot distinct from the selection_list member and why the key is not "selected_items".
    //
    // C#: `set => SetValue(SelectedItemsProperty, new SelectionList(this, value))`. The wrap is what
    // set_selected_items already does, so property_changed simply forwards the assigned collection into
    // it — and set_selected_items then runs the ordinary notification choreography (one change, the
    // mapper key raised once), which is why nothing here notifies on its own.
    //
    // A NULL assignment CLEARS rather than being ignored: C#'s SelectionList wraps a null list as empty,
    // and `SelectedItems="{Binding Missing}"` resolving to nothing must deselect, not silently keep the
    // previous selection.
    const maui::core::bindable_property<std::shared_ptr<i_item_collection>>& selectable_items_view::
        selected_items_source_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<i_item_collection>> descriptor{
            "selected_items_source",
            nullptr,
            {.property_changed = [](maui::core::bindable_object& bindable, const std::shared_ptr<i_item_collection>&,
                                    const std::shared_ptr<i_item_collection>& new_value) {
                std::vector<boxed_item> items;
                if (new_value)
                {
                    const std::size_t count = new_value->count();
                    items.reserve(count);
                    for (std::size_t i = 0; i < count; i++)
                    {
                        items.push_back(new_value->at(i));
                    }
                }
                dynamic_cast<selectable_items_view&>(bindable).set_selected_items(std::move(items));
            }}};
        return descriptor;
    }

    const maui::core::bindable_property<boxed_item>& selectable_items_view::selected_item_property()
    {
        static const maui::core::bindable_property<boxed_item> descriptor{
            "selected_item",
            boxed_item{},
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const boxed_item& old_value, const boxed_item& new_value) {
                     selectable_items_view_descriptor_access::on_selected_item_changed(
                         dynamic_cast<selectable_items_view&>(bindable), old_value, new_value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    void selectable_items_view::set_selected_items(std::shared_ptr<maui::core::observable_collection<boxed_item>> value)
    {
        // C#: SetValue(SelectedItemsProperty, new SelectionList(this, value)) — null coerces to the
        // fresh empty list.
        replace_selected_items(value ? std::make_unique<selection_list>(*this, std::move(value))
                                     : std::make_unique<selection_list>(*this));
    }

    void selectable_items_view::set_selected_items(std::vector<boxed_item> value)
    {
        replace_selected_items(std::make_unique<selection_list>(*this, std::move(value)));
    }

    void selectable_items_view::replace_selected_items(std::unique_ptr<selection_list> incoming)
    {
        const std::vector<boxed_item> old_selection = selected_items_->items();
        const std::vector<boxed_item> new_selection = incoming->items();
        selected_items_ = std::move(incoming);
        // The bindable propertyChanged → the internal SelectedItemsPropertyChanged(old, new).
        selected_items_property_changed(old_selection, new_selection);
    }

    void selectable_items_view::update_selected_items(const std::vector<boxed_item>& new_selection)
    {
        const std::vector<boxed_item> old_selection = selected_items_->items();

        suppress_selection_change_notification_ = true;
        selected_items_->clear();
        for (const boxed_item& item : new_selection)
        {
            selected_items_->add(item);
        }
        suppress_selection_change_notification_ = false;

        selected_items_property_changed(old_selection, new_selection);
    }

    void selectable_items_view::selected_items_property_changed(const std::vector<boxed_item>& old_selection,
                                                                const std::vector<boxed_item>& new_selection)
    {
        if (suppress_selection_change_notification_)
        {
            return;
        }
        selection_property_changed(selection_changed_event_args::from_lists(old_selection, new_selection));
        on_property_changed("selected_items"); // OnPropertyChanged(SelectedItemsProperty.PropertyName)
    }

    // C# SelectionPropertyChanged: command (no CanExecute analog) → event → the virtual hook.
    void selectable_items_view::selection_property_changed(const selection_changed_event_args& args)
    {
        if (selection_changed_command)
        {
            selection_changed_command();
        }
        selection_changed.raise(args);
        on_selection_changed(args);
    }

    // C# SelectionModePropertyChanged: diff the effective selections of the two modes; both-empty or
    // the same single item suppresses the signal.
    void selectable_items_view::on_selection_mode_changed(controls::selection_mode old_mode,
                                                          controls::selection_mode new_mode)
    {
        std::vector<boxed_item> previous_selection;
        std::vector<boxed_item> new_selection;

        switch (old_mode)
        {
            case controls::selection_mode::none:
                break;
            case controls::selection_mode::single:
                if (selected_item().has_value())
                {
                    previous_selection.push_back(selected_item());
                }
                break;
            case controls::selection_mode::multiple:
                previous_selection = selected_items_->items();
                break;
        }

        switch (new_mode)
        {
            case controls::selection_mode::none:
                break;
            case controls::selection_mode::single:
                if (selected_item().has_value())
                {
                    new_selection.push_back(selected_item());
                }
                break;
            case controls::selection_mode::multiple:
                new_selection = selected_items_->items();
                break;
        }

        if (previous_selection.size() == new_selection.size())
        {
            if (previous_selection.empty() || previous_selection.front() == new_selection.front())
            {
                // Both selections are empty or share the same single item; no reason to signal.
                return;
            }
        }

        selection_property_changed(
            selection_changed_event_args::from_lists(std::move(previous_selection), std::move(new_selection)));
    }
} // namespace maui::controls
