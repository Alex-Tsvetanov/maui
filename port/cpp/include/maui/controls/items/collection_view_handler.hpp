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
#include "maui/core/thickness.hpp"
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

        // The native view the handler composes into a real view tree:
        //   - headless: null (no native tree);
        //   - apple: a plain placeholder NSView (the macOS NSCollectionView host is a later wave —
        //     the cross-platform simulator below is the macOS state mirror, documented);
        //   - ios: the REAL UICollectionView the items_view_controller drives (W3-29).
        void* native = nullptr;

#ifdef MAUI_PLATFORM_IOS
        // W3-29 — the iOS native virtualization stack (the Items2 compositional path). All slots are
        // retained Obj-C objects, released in the backend-defined destructor (the .mm). The classic
        // flow-layout Items path is NOT ported (Items2 compositional only — documented deviation).
        // The items_view_controller (a UICollectionViewController subclass) IS the data source +
        // delegate + cell/source adapter (the Items2 controller/cell/source collapsed into one class).
        void* controller = nullptr;        // the items_view_controller (owns the UICollectionView)
        void* layout = nullptr;            // the current UICollectionViewCompositionalLayout (from layout_factory)
        void* empty_view_native = nullptr; // the realized EmptyView's native UIView while shown
#endif

        // --- appkit (W3-30) ---
        // The AppKit native virtualization stack — a REAL NSCollectionView driven by an
        // NSCollectionViewFlowLayout + NSCollectionViewDataSource/Delegate, mirroring the iOS
        // controller/cell/source architecture (W3-29) adapted to AppKit. The classic FlowLayout
        // realizes items lazily inside the run loop exactly like UICollectionView; the cross-platform
        // simulator above still runs as the in-memory state mirror, and these natives are what the
        // apple seam suite asserts against. All slots are retained Obj-C objects, released in the
        // backend-defined destructor (the .mm). DOCUMENTED DEVIATION: the compositional-layout +
        // snap-points path is iOS-only — AppKit's NSCollectionView has no compositional layout, so the
        // port uses NSCollectionViewFlowLayout (linear list = 1 column; grid = `span` columns; both
        // orientations via scrollDirection). Header/footer use the flow layout's section
        // header/footer supplementary views.
#ifdef MAUI_PLATFORM_APPLE
        void* scroll = nullptr;            // the NSScrollView hosting the collection view (the composed native)
        void* data_source = nullptr;       // the MauiCollectionDataSource (datasource + delegate adapter)
        void* flow_layout = nullptr;       // the current NSCollectionViewFlowLayout (rebuilt from items_layout)
        void* empty_view_native = nullptr; // the realized EmptyView's native NSView while shown
#endif

        // --- android (W8-CV) ---
        // The Android native render stack — a real android.widget.ScrollView whose single document child is
        // a dev.mauicpp.MauiLayout host ViewGroup. C++ drives measure/arrange and pushes ABSOLUTE child
        // frames (the android container convention: MauiLayout.onLayout is a no-op so the frames survive —
        // see src/platform/android/java/MauiLayout.java), so realized item/header/footer views attach to the
        // host and arrange_native positions them. The retained-natives vector keeps each realized template-
        // content C++ subtree (which OWNS its attached handler + native view) alive for as long as it is
        // hosted — the android twin of the apple cell's _realizedContent retain. All four slots are
        // released in the backend-defined destructor (collection_view_handler.cpp's android partial).
        // DOCUMENTED DEVIATION: no RecyclerView view-recycling — the gallery pages have small fixed item
        // counts, so the partial favors render correctness, realizing every in-content element directly into
        // the host (the resume-doc "favor correctness of render over recycling" guidance). The cross-platform
        // simulator above still runs as the in-memory state mirror; these natives are the on-device surface.
#ifdef MAUI_PLATFORM_ANDROID
        void* scroll = nullptr;            // the android.widget.ScrollView (the composed native, global ref)
        void* host = nullptr;              // the inner dev.mauicpp.MauiLayout hosting the realized children
        void* empty_view_native = nullptr; // the realized EmptyView's native android.view.View while shown
        // The realized template-content subtrees currently hosted (owns handler + native view); cleared and
        // rebuilt each arrange_native pass, freed in the destructor (the apple _realizedContent analog).
        std::vector<std::shared_ptr<maui::core::bindable_object>> retained_natives;
#endif

        // --- windows (W-CV) ---
        // The Windows (WinUI 3) native render stack — a real Microsoft.UI.Xaml.Controls.ScrollViewer
        // whose Content is a Canvas host panel (the manual-frame container every windows handler uses:
        // C++ drives measure/arrange and children keep the absolute Canvas frames their own
        // platform_arrange set). arrange_native realizes one native element per collection element into
        // the host and positions each absolutely — the windows twin of the android ScrollView →
        // MauiCollectionContent recipe (static realization, no ItemsRepeater/ListView virtualization —
        // documented deviation in the .cpp header). The cross-platform simulator above still runs as the
        // in-memory state mirror; on the XAML-less test host all three slots stay null and that mirror
        // is the observable surface. NO winrt types here — void* slots only (windows_native.hpp
        // store/borrow/release localize the ABI dance to the windows .cpp TU); `native` aliases `scroll`
        // (not separately retained). retained_natives keeps each realized template-content C++ subtree
        // (which owns its attached handler + native view) alive while hosted — the android twin's field.
#ifdef MAUI_PLATFORM_WINDOWS
        void* scroll = nullptr; // the ScrollViewer (the composed native; one strong ref via wnative::store)
        void* host = nullptr;   // the inner Canvas hosting the realized children (one strong ref)
        std::vector<std::shared_ptr<maui::core::bindable_object>> retained_natives;
#endif

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

        // ---- carousel mirrors (CarouselViewHandler2.MapIsSwipeEnabled / MapIsBounceEnabled /
        //      MapPeekAreaInsets). Defaults match CarouselView (swipe/bounce true, no peek). On iOS the
        //      mappers push straight to the native UICollectionView; these mirrors are the headless/appkit
        //      state surface (no native carousel scroll-lock there — documented simplification). ----
        bool swipe_enabled = true;
        bool bounce_enabled = true;
        maui::core::thickness peek_area_insets{};

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

        // Frame the native view (the backend half of platform_arrange — the per-backend `.mm`/`.cpp`
        // half), mirroring every other handler's platform_arrange (e.g. table_view_handler / border_
        // handler::arrange_native). The cross-platform platform_arrange calls this FIRST, then updates the
        // viewport-extent mirror and re-realizes. Without it an embedded CollectionView keeps its
        // creation-time native frame — on iOS a UICollectionViewController vends a FULL-SCREEN collectionView
        // with flexible autoresizing, so it paints full-bleed over its stack siblings. Headless: no-op.
        void arrange_native(const maui::graphics::rect& frame);

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

        // ---- carousel scroll writeback (CarouselViewController2.SetPosition / SetCurrentItem →
        //      SetValueFromRenderer). The platform's scroll-end delegate (the .mm's
        //      scrollViewDidEndDecelerating/Dragging) resolves the centered item index and calls these to
        //      write the settled scroll position BACK to the carousel's Position + CurrentItem. No-ops
        //      unless the virtual view is a carousel_view (the carousel reuses this collection handler).
        //      Guarded by suppress_scroll_writeback (the C# _isInternalCollectionUpdate gate) so the
        //      spurious UIKit scroll callbacks fired during a batch source update can't clobber the
        //      position the update is computing. set_position_from_scroll writes Position then chains into
        //      set_current_item_from_scroll (the C# SetPosition → SetCurrentItem order); the latter
        //      resolves the item at `position` from the source and writes it. ----
        void set_position_from_scroll(int position);
        void set_current_item_from_scroll(int position);
        // C# CarouselViewController2._isInternalCollectionUpdate: while true, set_position_from_scroll /
        // set_current_item_from_scroll are dropped (the batch-update suppression gate).
        void set_suppress_scroll_writeback(bool suppress)
        {
            suppress_scroll_writeback_ = suppress;
        }
        [[nodiscard]] bool suppress_scroll_writeback() const
        {
            return suppress_scroll_writeback_;
        }

        // C# CarouselViewController2.InitialPositionSet (set true in UpdateInitialPosition once the view is
        // loaded + laid out, reset in TearDown). The iOS partial flips it on the first carousel layout pass
        // (native_force_layout); SetPosition then allows writeback. Exposed so backends (and the cross-
        // platform writeback tests, which stand in for the layout pass) can establish the initial position.
        void mark_initial_position_set()
        {
            initial_position_set_ = true;
        }
        [[nodiscard]] bool has_initial_position_set() const
        {
            return initial_position_set_;
        }

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

        // ---- carousel-specific mapper entries (CarouselViewHandler2.Map* — the carousel reuses this
        //      collapsed collection handler, so its three extra knobs register here and reach the concrete
        //      carousel_view by dynamic_cast, exactly like map_selected_item reaches selectable_items_view.
        //      No-ops when the virtual view is a plain collection_view). map_is_swipe_enabled →
        //      CollectionView.ScrollEnabled; map_is_bounce_enabled → CollectionView.Bounces;
        //      map_peek_area_insets → UpdateLayout (adjust the native section/content insets). On the
        //      headless/appkit backends these update the platform mirrors only (no native carousel
        //      scroll-lock surface — documented simplification). ----
        static void map_is_swipe_enabled(collection_view_handler& handler, i_items_view& view);
        static void map_is_bounce_enabled(collection_view_handler& handler, i_items_view& view);
        static void map_peek_area_insets(collection_view_handler& handler, i_items_view& view);

#ifdef MAUI_PLATFORM_IOS
        // ---- the iOS native bridge (W3-29) ----
        // The cross-platform simulator below still runs as the in-memory state mirror; in PARALLEL these
        // entry points drive the REAL UICollectionView controller (defined in the .mm) so the on-
        // simulator suite asserts genuine cell reuse / supplementaries / native selection. The .cpp
        // calls each at the same moments C# does (ItemsViewController2.ReloadData / UpdateItemsSource /
        // UpdateSelectionMode / UpdatePlatformSelection / UpdateLayout / UpdateEmptyView / ScrollTo).
        // No-ops until the controller exists (create_platform_view mints it).
        void native_reload();                    // CollectionView.ReloadData (re-realize visible cells)
        void native_rebuild_layout();            // SelectLayout → UpdateLayout (rebuild the compositional layout)
        void native_update_selection_mode();     // SelectableItemsViewController2.UpdateSelectionMode
        void native_update_platform_selection(); // SelectableItemsViewController2.UpdatePlatformSelection
        void native_update_empty_view();         // ItemsViewController2.UpdateEmptyView
        void native_update_can_reorder();        // ReorderableItemsViewController2.UpdateCanReorderItems
        // ScrollTo (ItemsViewHandler2.ScrollToRequested): move the native viewport to `path`/`position`.
        void native_scroll_to(const index_path& path, controls::scroll_to_position position, bool animate);
        // Mount the native UICollectionView in a host window + force a layout pass (test seam — the
        // run_loop_pump helper turns the loop so cells realize). Returns the visible cell count.
        int native_force_layout(double width, double height);
        // Inspection seams for the on-simulator tests (read straight off the live UICollectionView).
        [[nodiscard]] int native_visible_cell_count() const;                     // CollectionView.VisibleCells.Length
        [[nodiscard]] int native_distinct_cell_instances() const;                // unique cell pointers ever seen
        [[nodiscard]] int native_visible_supplementary_count(bool header) const; // group/section supplementals
        // The text a realized supplementary view at `section` currently displays (the group/CV header or
        // footer label / its bound template's first label), so a test can assert a group header bound its
        // key. `section < 0` reads a CV-level (global) supplementary; otherwise the per-group one for that
        // section. Empty when no such supplementary is realized.
        [[nodiscard]] std::string native_supplementary_text(int section, bool header) const;
        [[nodiscard]] int native_selected_count() const; // GetIndexPathsForSelectedItems.Length
        // Simulate a user tap selecting/deselecting the cell at `path` on the native collection view
        // (the delegate's ItemSelected/ItemDeselected path — fans back to the control).
        void native_select(const index_path& path);
        void native_deselect(const index_path& path);
        // The text the realized cell at `path` currently displays (the DefaultCell2 label / the item's
        // text mirror) — the test seam that proves a model reorder re-rendered the native cells. Empty
        // when the path is not realized.
        [[nodiscard]] std::string native_cell_text(const index_path& path) const;
        // ---- the carousel knobs on the native UICollectionView (CarouselViewHandler2.Map*) ----
        void native_update_swipe_enabled();    // CollectionView.ScrollEnabled = swipe_enabled
        void native_update_bounce_enabled();   // CollectionView.Bounces = bounce_enabled
        void native_update_peek_area_insets(); // UpdateLayout: apply the peek as content insets
#endif

        // --- appkit (W3-30) ---
        // The AppKit native bridge — the macOS twin of the iOS bridge above. The cross-platform .cpp
        // calls each at the same moments C# does (Controller.ReloadData / UpdateItemsSource /
        // UpdateLayout / UpdateSelectionMode / UpdatePlatformSelection / UpdateEmptyView / ScrollTo),
        // guarded by #ifdef MAUI_PLATFORM_APPLE. No-ops until the data source is wired
        // (on_connect_handler installs it).
#ifdef MAUI_PLATFORM_APPLE
        void on_connect_handler(collection_view_platform& platform); // wire datasource/delegate + initial reload
        void native_reload();                                        // Controller.ReloadData (re-realize cells)
        void native_rebuild_layout();            // SelectLayout → UpdateLayout (rebuild the flow layout)
        void native_update_selection_mode();     // SelectableItemsViewController.UpdateSelectionMode
        void native_update_platform_selection(); // SelectableItemsViewController.UpdatePlatformSelection
        void native_update_empty_view();         // ItemsViewController.UpdateEmptyView
        void native_update_can_reorder();        // ReorderableItemsViewController.UpdateCanReorderItems
        // ScrollTo (ItemsViewHandler.ScrollToRequested): move the native viewport to `path`/`position`.
        void native_scroll_to(const index_path& path, controls::scroll_to_position position, bool animate);
        // Mount/force a layout pass (test seam — pump the loop so cells realize). Returns visible count.
        int native_force_layout(double width, double height);
        // Inspection seams for the apple seam tests (read straight off the live NSCollectionView).
        [[nodiscard]] int native_visible_cell_count() const;                     // visibleItems.count
        [[nodiscard]] int native_distinct_cell_instances() const;                // unique item pointers ever vended
        [[nodiscard]] int native_visible_supplementary_count(bool header) const; // section header/footer supplementals
        [[nodiscard]] int native_selected_count() const;                         // selectionIndexPaths.count
        // Whether the EmptyView host is currently mounted in the collection view (NSView.tag is
        // read-only, so the empty host is a marker subclass — this is the test seam, the C# EmptyTag
        // viewWithTag analog).
        [[nodiscard]] bool native_empty_view_shown() const;
        // Simulate a user click selecting/deselecting the item at `path` (the delegate's
        // didSelect/didDeselect path — fans back to the control).
        void native_select(const index_path& path);
        void native_deselect(const index_path& path);
        // The text the realized item at `path` currently displays (the default-item label / the item's
        // text mirror). Empty when the path is not realized.
        [[nodiscard]] std::string native_cell_text(const index_path& path) const;
#endif

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

        // C# CarouselViewController2._isInternalCollectionUpdate: gates set_position_from_scroll /
        // set_current_item_from_scroll so a batch source update's spurious UIKit scroll callbacks don't
        // clobber the position the update is computing.
        bool suppress_scroll_writeback_ = false;

        // C# CarouselViewController2.InitialPositionSet (declared line 199, reset in TearDown line 204):
        // false until the carousel's initial layout/position is established, then guards SetPosition /
        // SetCurrentItem (lines 507/536/562) so scroll callbacks firing BEFORE the first layout pass can't
        // clobber a programmatically-set Position. The port flips it true at the end of the first carousel
        // native_force_layout (its UpdateInitialPosition / view-loaded analog).
        bool initial_position_set_ = false;

        std::shared_ptr<i_items_view_source> source_;
        maui::core::scoped_connection source_updated_; // after source_ (§8)
        // The observed items_layout, PINNED so the subscription can never outlive it (§8: replacing
        // the control's layout would otherwise free the event under our connection).
        std::shared_ptr<items_layout> tracked_layout_;
        maui::core::scoped_connection layout_changed_; // after tracked_layout_ (§8)
    };
} // namespace maui::controls
