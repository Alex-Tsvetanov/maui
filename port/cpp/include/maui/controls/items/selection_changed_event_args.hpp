#pragma once
// maui::controls::selection_changed_event_args  <=  Microsoft.Maui.Controls.SelectionChangedEventArgs
//
// The SelectionChanged payload: the previous and current selections as value snapshots (the C# ctors
// copy the lists too). The single-item ctor maps a null item to an empty list, exactly like C#.

#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"

namespace maui::controls
{
    struct selection_changed_event_args
    {
        std::vector<boxed_item> previous_selection;
        std::vector<boxed_item> current_selection;

        // SelectionChangedEventArgs(object, object): a null side becomes an empty list.
        [[nodiscard]] static selection_changed_event_args from_single(const boxed_item& previous,
                                                                      const boxed_item& current)
        {
            selection_changed_event_args args;
            if (previous.has_value())
            {
                args.previous_selection.push_back(previous);
            }
            if (current.has_value())
            {
                args.current_selection.push_back(current);
            }
            return args;
        }

        // SelectionChangedEventArgs(IList<object>, IList<object>) — list snapshots.
        [[nodiscard]] static selection_changed_event_args from_lists(std::vector<boxed_item> previous,
                                                                     std::vector<boxed_item> current)
        {
            selection_changed_event_args args;
            args.previous_selection = std::move(previous);
            args.current_selection = std::move(current);
            return args;
        }
    };
} // namespace maui::controls
