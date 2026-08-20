#pragma once
// maui::controls::selectable_items_view  <=  Microsoft.Maui.Controls.SelectableItemsView
//
// Adds selection over structured_items_view: SelectionMode (None default), SelectedItem (TwoWay, the
// single-selection slot), SelectedItems (the selection_list face — never null), the SelectionChanged
// event + the move_only_function command convention, and UpdateSelectedItems (the platform's batch
// write-back, one notification). Ported from SelectableItemsView.cs + SelectionList.cs.
//
// Notification choreography (the C# order, verbatim):
//   - SelectedItem change   → SelectionChanged(args(old-single, new-single)) via the shared
//     SelectionPropertyChanged (command → event → OnSelectionChanged virtual);
//   - any SelectedItems mutation → selected_items_property_changed(shadow, live) → the same shared
//     path, THEN on_property_changed("selected_items") (the mapper key);
//   - a SelectionMode change diffs the old/new effective selections and only signals when they
//     differ (the C# same-single-item / both-empty suppression);
//   - update_selected_items suppresses the per-mutation notifications and raises ONE change.
//
// SelectedItems is NOT a property<T> here: the selection_list member IS the property value (C#'s
// bindable SelectedItems always coerces into a SelectionList wrapping this view); replacing it via
// set_selected_items mirrors the C# setter (wrap, then notify old vs new). Documented collapse.

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_list.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/items/structured_items_view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class selectable_items_view : public structured_items_view
    {
    public:
        selectable_items_view();

        static const maui::core::bindable_property<controls::selection_mode>& selection_mode_property();
        static const maui::core::bindable_property<boxed_item>& selected_item_property();
        // SelectedItemsProperty (SelectableItemsView.cs:31) — the BINDABLE INPUT SLOT for the multiple
        // selection, so `SelectedItems="{Binding SelectedItems}"` works as it does in MAUI.
        //
        // WHY A SEPARATE SLOT FROM selected_items() ABOVE. C#'s setter is
        // `SetValue(SelectedItemsProperty, new SelectionList(this, value))`: the property holds the raw
        // assigned list and a SelectionList wrapping THIS view is what everything downstream reads. This
        // port already collapsed that second half into the selection_list MEMBER (see the header note),
        // so the member cannot also be the bindable slot — a property<T> needs a value of its own. The
        // slot therefore holds the ASSIGNED collection and its property_changed coerces it into the
        // member, which is exactly the C# wrap-then-notify order with the wrap already done.
        //
        // The descriptor key is "selected_items_source", NOT "selected_items": the latter is the mapper
        // key the handlers listen on and set_selected_items already raises it. Sharing the name would
        // fire it twice per assignment.
        static const maui::core::bindable_property<std::shared_ptr<i_item_collection>>&
        selected_items_source_property();

        [[nodiscard]] controls::selection_mode selection_mode() const
        {
            return selection_mode_.get();
        }
        void set_selection_mode(controls::selection_mode value)
        {
            selection_mode_.set(value);
        }

        [[nodiscard]] const boxed_item& selected_item() const
        {
            return selected_item_.get();
        }
        void set_selected_item(boxed_item value)
        {
            selected_item_.set(std::move(value));
        }

        // SelectedItems — never null (DefaultValueCreator mints the empty list in the ctor).
        [[nodiscard]] selection_list& selected_items()
        {
            return *selected_items_;
        }
        [[nodiscard]] const selection_list& selected_items() const
        {
            return *selected_items_;
        }
        // The C# SelectedItems SETTER: wrap the incoming list in a fresh selection_list (subscribing
        // when it is observable) and notify old vs new.
        void set_selected_items(std::shared_ptr<maui::core::observable_collection<boxed_item>> value);
        void set_selected_items(std::vector<boxed_item> value);

        [[nodiscard]] const std::shared_ptr<i_item_collection>& selected_items_source() const
        {
            return selected_items_source_.get();
        }
        void set_selected_items_source(std::shared_ptr<i_item_collection> value)
        {
            selected_items_source_.set(std::move(value));
        }

        // UpdateSelectedItems: replace the selection contents with ONE change notification.
        void update_selected_items(const std::vector<boxed_item>& new_selection);

        // ---- events + the command convention ----
        maui::core::event<const selection_changed_event_args&> selection_changed;
        maui::core::move_only_function<void()> selection_changed_command;

        // INTERNAL (C# internal SelectedItemsPropertyChanged) — the selection_list write-back channel.
        void selected_items_property_changed(const std::vector<boxed_item>& old_selection,
                                             const std::vector<boxed_item>& new_selection);

    protected:
        // C# OnSelectionChanged hook.
        virtual void on_selection_changed(const selection_changed_event_args& args)
        {
            (void)args;
        }

    private:
        friend struct selectable_items_view_descriptor_access;

        // The shared command → event → virtual path (C# SelectionPropertyChanged).
        void selection_property_changed(const selection_changed_event_args& args);
        // The SelectionMode diff (C# SelectionModePropertyChanged).
        void on_selection_mode_changed(controls::selection_mode old_mode, controls::selection_mode new_mode);
        void replace_selected_items(std::unique_ptr<selection_list> incoming);

        bool suppress_selection_change_notification_ = false;
        std::unique_ptr<selection_list> selected_items_;
        maui::core::property<boxed_item> selected_item_{*this, selected_item_property()};
        maui::core::property<controls::selection_mode> selection_mode_{*this, selection_mode_property()};
        maui::core::property<std::shared_ptr<i_item_collection>> selected_items_source_{
            *this, selected_items_source_property()};
    };
} // namespace maui::controls
