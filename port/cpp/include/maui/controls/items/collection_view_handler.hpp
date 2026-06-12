#pragma once
// maui::controls::collection_view_handler  <=  Microsoft.Maui.Controls.Handlers.Items.
// CollectionViewHandler (the ItemsViewHandler → Structured → Selectable → Groupable → Reorderable
// mapper chain COLLAPSED into one handler — the intermediate C# handlers exist for the carousel's
// reuse, which is wave-3 scope; documented collapse). Lives in the controls layer exactly like the
// C# original (Controls.Handlers.Items), typed on i_items_view with the deeper-level keys reaching
// the concrete controls by dynamic_cast (the `where TItemsView : ...` analog).
//
// The platform half is the HEADLESS FAKE-VIEWPORT VIRTUALIZATION SIMULATOR (this wave's stand-in
// for the wave-3 UICollectionView/NSCollectionView controllers, compiled on EVERY backend so the
// oracle semantics are tested everywhere; apple/ios only swap in a placeholder native view):
//   - a viewport with a settable main/cross extent and scroll offset;
//   - a flat layout model (fixed `item_extent` per cell row; linear or grid-span packing per the
//     control's items_layout; grouped sections interleave group header/footer rows when their
//     templates are set);
//   - cells realized through the data_template machinery (selector-aware, recycled by the
//     template's id_string — the C# reuse identifier) with every REALIZE / BIND / RECYCLE recorded
//     in `events`, plus the translated source_update trail in `source_updates`;
//   - EmptyView / Header / Footer realized as supplementals (template → content + BindingContext;
//     a boxed VIEW hosts directly; otherwise the text mirror renders — the C# View-vs-ToString
//     split). The view-level header/footer sit OUTSIDE the scroll math (simulator simplification,
//     documented);
//   - the user-side inbound channels tests drive: simulate_scroll (Scrolled + the
//     remaining-items-threshold trip, the ItemsViewDelegator.Scrolled port), simulate_select /
//     simulate_deselect (SelectableItemsViewController.ItemSelected/Deselected),
//     simulate_reorder_completed (the reorder surface — the drag itself is native, wave 3);
//   - ItemsUpdatingScrollMode semantics on source updates (the ItemsViewLayout port): keep_scroll_
//     offset leaves the offset, keep_items_in_view compensates when rows enter/leave at-or-before
//     the first visible row, keep_last_item_in_view forces the tail into view;
//   - the "scroll_to" command (MapScrollTo): position- and element-mode requests resolved against
//     the items source, recorded in scroll_requests, applied per scroll_to_position.

#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/i_items_view.hpp"
#include "maui/controls/items/item_sizing_strategy.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_updating_scroll_mode.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/controls/items/scroll_to_request_event_args.hpp"
#include "maui/controls/items/snap_points_alignment.hpp"
#include "maui/controls/items/snap_points_type.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    class items_layout;

    // What kind of element one simulator cell hosts.
    enum class cell_element_kind : std::uint8_t
    {
        item = 0,
        group_header,
        group_footer,
        header,
        footer,
        empty_view,
    };

    // The recorded realize/recycle/bind trail (the task's oracle record).
    enum class cell_event_kind : std::uint8_t
    {
        realized = 0, // fresh content created (a pool hit re-binds WITHOUT a new realize)
        bound,        // BindingContext pushed onto the cell content
        recycled,     // content returned to the reuse pool
    };

    struct cell_event
    {
        cell_event_kind kind = cell_event_kind::realized;
        cell_element_kind element = cell_element_kind::item;
        index_path path{};
        std::string reuse_id;
    };

    // One realized (visible) cell.
    struct realized_cell
    {
        cell_element_kind element = cell_element_kind::item;
        index_path path{};
        std::string reuse_id;
        std::shared_ptr<maui::core::bindable_object> content; // null for the default text cell
        std::string text;                                     // the default cell's display (item.text())
        double start = 0;                                     // main-axis position
        double extent = 0;
    };

    // One realized supplemental (EmptyView / Header / Footer).
    struct realized_supplemental
    {
        bool present = false;
        std::string reuse_id;
        std::shared_ptr<maui::core::bindable_object> content; // template-created or the boxed view
        std::string text;                                     // the text mirror otherwise
    };

    struct collection_view_platform : maui::core::view_platform_base
    {
        collection_view_platform();
        ~collection_view_platform() override; // backend-defined: releases the retained placeholder
        collection_view_platform(const collection_view_platform&) = delete;
        collection_view_platform(collection_view_platform&&) = delete;
        collection_view_platform& operator=(const collection_view_platform&) = delete;
        collection_view_platform& operator=(collection_view_platform&&) = delete;

        // The native placeholder view (headless: null; apple/ios: a plain NSView/UIView until the
        // wave-3 native collection views land — per-backend members stay in the #ifdef blocks then).
        void* native = nullptr;

        // ---- the fake viewport ----
        double viewport_main_extent = 400;  // along the scroll axis
        double viewport_cross_extent = 400; // across it
        double scroll_offset = 0;
        double previous_scroll_offset = 0; // the delegator's PreviousH/VOffset
        double item_extent = 100;          // simulated fixed row extent (settable per test)
        double content_extent = 0;         // derived from the last realization pass

        // ---- mirrors of the mapped surface ----
        items_layout_orientation orientation = items_layout_orientation::vertical;
        int span = 1;
        double item_spacing = 0; // main-axis row spacing (linear ItemSpacing / grid main-axis spacing)
        controls::snap_points_type snap_points_type = controls::snap_points_type::none;
        controls::snap_points_alignment snap_points_alignment = controls::snap_points_alignment::start;
        controls::items_updating_scroll_mode scroll_mode = controls::items_updating_scroll_mode::keep_items_in_view;
        controls::item_sizing_strategy sizing_strategy = controls::item_sizing_strategy::measure_all_items;
        maui::core::scroll_bar_visibility horizontal_bar_visibility = maui::core::scroll_bar_visibility::default_;
        maui::core::scroll_bar_visibility vertical_bar_visibility = maui::core::scroll_bar_visibility::default_;
        bool grouped = false;
        bool can_reorder_items = false;

        // ---- selection mirrors (UpdateSelectionMode / UpdatePlatformSelection) ----
        bool allows_selection = false;
        bool allows_multiple_selection = false;
        std::vector<index_path> selected_paths;

        // ---- realization state + records ----
        std::vector<realized_cell> realized; // visible cells in layout order
        // The reuse pool, keyed by reuse_id (the template id_string — the C# reuse identifier).
        std::vector<std::pair<std::string, std::shared_ptr<maui::core::bindable_object>>> recycle_pool;
        std::vector<cell_event> events;            // the realize/bind/recycle trail
        std::vector<source_update> source_updates; // every translated source op observed
        std::vector<scroll_to_request_event_args> scroll_requests;

        realized_supplemental header;
        realized_supplemental footer;
        realized_supplemental empty_view;
    };

    class collection_view_handler
        : public maui::core::view_handler<collection_view_handler, i_items_view, collection_view_platform>
    {
    public:
        collection_view_handler();

        static maui::core::property_mapper<i_items_view, collection_view_handler>& mapper();
        static maui::core::command_mapper<i_items_view, collection_view_handler>& command_mapper();

        // Per-backend (headless .cpp / apple+ios .mm): mint the platform struct (+ placeholder native).
        static std::unique_ptr<collection_view_platform> create_platform_view();
        // Tear down the items source BEFORE the platform/virtual view detach (C# Controller.Disconnect).
        void on_disconnect_handler(collection_view_platform& platform);

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // The handler-side items source (what the wave-3 controllers will consume too).
        [[nodiscard]] const std::shared_ptr<i_items_view_source>& items_view_source() const
        {
            return source_;
        }

        // ---- the simulator's user-side inbound channels (header note) ----
        void simulate_viewport(double main_extent, double cross_extent);
        void simulate_scroll(double offset);
        void simulate_select(const index_path& path);
        void simulate_deselect(const index_path& path);
        void simulate_reorder_completed();

        // ---- mapper entries ----
        static void map_items_source(collection_view_handler& handler, i_items_view& view);
        static void map_item_template(collection_view_handler& handler, i_items_view& view);
        static void map_empty_view(collection_view_handler& handler, i_items_view& view);
        static void map_items_updating_scroll_mode(collection_view_handler& handler, i_items_view& view);
        static void map_horizontal_scroll_bar_visibility(collection_view_handler& handler, i_items_view& view);
        static void map_vertical_scroll_bar_visibility(collection_view_handler& handler, i_items_view& view);
        static void map_header_footer(collection_view_handler& handler, i_items_view& view);
        static void map_items_layout(collection_view_handler& handler, i_items_view& view);
        static void map_item_sizing_strategy(collection_view_handler& handler, i_items_view& view);
        static void map_selected_item(collection_view_handler& handler, i_items_view& view);
        static void map_selected_items(collection_view_handler& handler, i_items_view& view);
        static void map_selection_mode(collection_view_handler& handler, i_items_view& view);
        static void map_is_grouped(collection_view_handler& handler, i_items_view& view);
        static void map_group_templates(collection_view_handler& handler, i_items_view& view);
        static void map_can_reorder_items(collection_view_handler& handler, i_items_view& view);
        static void map_scroll_to(collection_view_handler& handler, i_items_view& view, const std::any& args);

    private:
        // One slot of the flat layout model (the simulator's UICollectionViewLayout stand-in).
        struct layout_entry
        {
            cell_element_kind element = cell_element_kind::item;
            index_path path{};
            double start = 0;
            double extent = 0;
        };

        void update_items_source();                        // C# UpdateItemsSource (reload)
        void on_source_updated(const source_update& args); // scroll-mode adjust + re-realize
        void update_empty_view();
        void update_header_footer();
        void update_platform_selection(); // C# UpdatePlatformSelection
        void update_selection_mode();     // C# UpdateSelectionMode
        void refresh_layout_mirrors();
        void refresh_realization();
        void apply_scroll(double offset); // shared by simulate_scroll + scroll_to: move, realize, report
        void report_scrolled();           // the ItemsViewDelegator.Scrolled port (event + threshold)

        [[nodiscard]] std::vector<layout_entry> build_entries() const;
        [[nodiscard]] double max_scroll_offset() const;
        [[nodiscard]] int flat_item_ordinal(const index_path& path) const;
        void realize_supplemental(realized_supplemental& slot, cell_element_kind element,
                                  const std::shared_ptr<data_template>& content_template, const boxed_item& value);
        [[nodiscard]] std::shared_ptr<maui::core::bindable_object> take_from_pool(const std::string& reuse_id);

        std::shared_ptr<i_items_view_source> source_;
        maui::core::scoped_connection source_updated_; // after source_ (§8)
        // The observed items_layout, PINNED so the subscription can never outlive it (§8: replacing
        // the control's layout would otherwise free the event under our connection).
        std::shared_ptr<items_layout> tracked_layout_;
        maui::core::scoped_connection layout_changed_; // after tracked_layout_ (§8)
    };
} // namespace maui::controls
