// maui::hosting::add_maui_controls_handlers — the default control → handler table
// (maui_controls_handlers.hpp). Ported from Microsoft.Maui.Controls.Hosting.AppHostBuilderExtensions
// .AddControlsHandlers: the SAME pairs the controls' MAUI_REGISTER_HANDLER registrars publish
// (src/controls/*.cpp + src/core/*.cpp), listed once per control here so a builder-booted maui_app can
// resolve EVERY built-in control through attach_handler. (The static MAUI_REGISTER_HANDLER registrars
// feed the global registrar table; this explicit list is what the maui_app handler_registry is seeded
// from — they must stay in parity, so this enumerates the full registrar set.)

#include "maui/hosting/maui_controls_handlers.hpp"

#include "maui/controls/absolute_layout.hpp"
#include "maui/controls/activity_indicator.hpp"
#include "maui/controls/border.hpp"
#include "maui/controls/box_view.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/content_view.hpp"
#include "maui/controls/date_picker.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/flex_layout.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/controls/frame.hpp"
#include "maui/controls/graphics_view.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/hybrid_web_view.hpp"
#include "maui/controls/hybrid_web_view_handler.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/image_button.hpp"
#include "maui/controls/indicator_view.hpp"
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/picker.hpp"
#include "maui/controls/progress_bar.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/refresh_view.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/shapes/ellipse.hpp"
#include "maui/controls/shapes/line.hpp"
#include "maui/controls/shapes/path.hpp"
#include "maui/controls/shapes/polygon.hpp"
#include "maui/controls/shapes/polyline.hpp"
#include "maui/controls/shapes/rectangle.hpp"
#include "maui/controls/shapes/round_rectangle.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell_handler.hpp"
#include "maui/controls/slider.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/stepper.hpp"
#include "maui/controls/swipe_item_view.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/controls/table_view.hpp"
#include "maui/controls/table_view_handler.hpp"
#include "maui/controls/templates/content_presenter.hpp"
#include "maui/controls/time_picker.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/web_view.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/activity_indicator_handler.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/check_box_handler.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/date_picker_handler.hpp"
#include "maui/core/editor_handler.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/flyout_page_handler.hpp"
#include "maui/core/graphics_view_handler.hpp"
#include "maui/core/image_button_handler.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/indicator_view_handler.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/picker_handler.hpp"
#include "maui/core/progress_bar_handler.hpp"
#include "maui/core/radio_button_handler.hpp"
#include "maui/core/refresh_view_handler.hpp"
#include "maui/core/scroll_view_handler.hpp"
#include "maui/core/search_bar_handler.hpp"
#include "maui/core/shape_view_handler.hpp"
#include "maui/core/slider_handler.hpp"
#include "maui/core/stepper_handler.hpp"
#include "maui/core/swipe_view_handler.hpp"
#include "maui/core/switch_handler.hpp"
#include "maui/core/tabbed_page_handler.hpp"
#include "maui/core/time_picker_handler.hpp"
#include "maui/core/web_view_handler.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/i_maui_handlers_collection.hpp"

namespace maui::hosting
{
    i_maui_handlers_collection& add_maui_controls_handlers(i_maui_handlers_collection& handlers)
    {
        // --- text + buttons ---
        handlers.add_handler<maui::controls::button, maui::core::button_handler>();
        handlers.add_handler<maui::controls::label, maui::core::label_handler>();
        handlers.add_handler<maui::controls::entry, maui::core::entry_handler>();
        handlers.add_handler<maui::controls::editor, maui::core::editor_handler>();
        handlers.add_handler<maui::controls::search_bar, maui::core::search_bar_handler>();
        handlers.add_handler<maui::controls::image, maui::core::image_handler>();
        handlers.add_handler<maui::controls::image_button, maui::core::image_button_handler>();

        // --- value / indicator controls ---
        handlers.add_handler<maui::controls::toggle_switch, maui::core::switch_handler>();
        handlers.add_handler<maui::controls::check_box, maui::core::check_box_handler>();
        handlers.add_handler<maui::controls::radio_button, maui::core::radio_button_handler>();
        handlers.add_handler<maui::controls::slider, maui::core::slider_handler>();
        handlers.add_handler<maui::controls::stepper, maui::core::stepper_handler>();
        handlers.add_handler<maui::controls::progress_bar, maui::core::progress_bar_handler>();
        handlers.add_handler<maui::controls::activity_indicator, maui::core::activity_indicator_handler>();

        // --- pickers ---
        handlers.add_handler<maui::controls::picker, maui::core::picker_handler>();
        handlers.add_handler<maui::controls::date_picker, maui::core::date_picker_handler>();
        handlers.add_handler<maui::controls::time_picker, maui::core::time_picker_handler>();

        // --- layouts (the layout controls share the one layout_handler; managers differ, not the panel) ---
        handlers.add_handler<maui::controls::stack_layout, maui::core::layout_handler>();
        handlers.add_handler<maui::controls::vertical_stack_layout, maui::core::layout_handler>();
        handlers.add_handler<maui::controls::horizontal_stack_layout, maui::core::layout_handler>();
        handlers.add_handler<maui::controls::grid, maui::core::layout_handler>();
        handlers.add_handler<maui::controls::absolute_layout, maui::core::layout_handler>();
        handlers.add_handler<maui::controls::flex_layout, maui::core::layout_handler>();

        // --- containers (frame is the border facade → border_handler; content_view + swipe_item_view
        // host content through the content_page_handler, mirroring their MAUI_REGISTER_HANDLER pairs) ---
        handlers.add_handler<maui::controls::scroll_view, maui::core::scroll_view_handler>();
        handlers.add_handler<maui::controls::border, maui::core::border_handler>();
        handlers.add_handler<maui::controls::frame, maui::core::border_handler>();
        handlers.add_handler<maui::controls::content_view, maui::core::content_page_handler>();
        // content_presenter (a ControlTemplate's content placeholder) is an IContentView host too, so it
        // resolves the SAME content_page_handler — this is what mounts a templated control's packed
        // developer content as a native subview (mirrors its MAUI_REGISTER_HANDLER pair).
        handlers.add_handler<maui::controls::content_presenter, maui::core::content_page_handler>();

        // --- pages / nav ---
        handlers.add_handler<maui::controls::content_page, maui::core::content_page_handler>();
        handlers.add_handler<maui::controls::navigation_page, maui::core::navigation_page_handler>();
        handlers.add_handler<maui::controls::tabbed_page, maui::core::tabbed_page_handler>();
        handlers.add_handler<maui::controls::flyout_page, maui::core::flyout_page_handler>();
        handlers.add_handler<maui::controls::shell, maui::core::shell_handler>();
        handlers.add_handler<maui::controls::window, maui::core::window_handler>();

        // --- collection / items + swipe / refresh ---
        handlers.add_handler<maui::controls::collection_view, maui::controls::collection_view_handler>();
        handlers.add_handler<maui::controls::carousel_view, maui::controls::collection_view_handler>();
        handlers.add_handler<maui::controls::indicator_view, maui::core::indicator_view_handler>();
        handlers.add_handler<maui::controls::table_view, maui::controls::table_view_handler>();
        handlers.add_handler<maui::controls::refresh_view, maui::core::refresh_view_handler>();
        handlers.add_handler<maui::controls::swipe_view, maui::core::swipe_view_handler>();
        handlers.add_handler<maui::controls::swipe_item_view, maui::core::content_page_handler>();

        // --- graphics: the canvas-drawn view + the shape family (one shared shape handler) ---
        handlers.add_handler<maui::controls::graphics_view, maui::core::graphics_view_handler>();
        handlers.add_handler<maui::controls::box_view, maui::core::shape_view_handler>();
        handlers.add_handler<maui::controls::shapes::rectangle, maui::core::shape_view_handler>();
        handlers.add_handler<maui::controls::shapes::round_rectangle, maui::core::shape_view_handler>();
        handlers.add_handler<maui::controls::shapes::ellipse, maui::core::shape_view_handler>();
        handlers.add_handler<maui::controls::shapes::line, maui::core::shape_view_handler>();
        handlers.add_handler<maui::controls::shapes::polyline, maui::core::shape_view_handler>();
        handlers.add_handler<maui::controls::shapes::polygon, maui::core::shape_view_handler>();
        handlers.add_handler<maui::controls::shapes::path, maui::core::shape_view_handler>();

        // --- web ---
        handlers.add_handler<maui::controls::web_view, maui::core::web_view_handler>();
        handlers.add_handler<maui::controls::hybrid_web_view, maui::controls::hybrid_web_view_handler>();

        return handlers;
    }
} // namespace maui::hosting
