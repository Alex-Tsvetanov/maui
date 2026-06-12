#pragma once
// maui::controls::items_view_scrolled_event_args  <=  Microsoft.Maui.Controls.ItemsViewScrolledEventArgs
// The Scrolled event payload: offsets, deltas, and the visible-index trio the platform reports.

namespace maui::controls
{
    struct items_view_scrolled_event_args
    {
        double horizontal_delta = 0;
        double vertical_delta = 0;
        double horizontal_offset = 0;
        double vertical_offset = 0;
        int first_visible_item_index = 0;
        int center_item_index = 0;
        int last_visible_item_index = 0;
    };
} // namespace maui::controls
