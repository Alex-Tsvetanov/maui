// collection_view_handler — cross-platform part: the collapsed ItemsView→Reorderable mapper chain,
// the items-source plumbing, and the WHOLE fake-viewport virtualization simulator (shared by every
// backend; the per-backend partial only mints the platform struct + placeholder native view).
// Ported from ItemsViewHandler.cs/.iOS.cs + StructuredItemsViewHandler + SelectableItemsViewHandler
// (+Controller) + GroupableItemsViewHandler + ReorderableItemsViewHandler + ItemsViewDelegator
// (Scrolled/threshold) + ItemsViewLayout (the ItemsUpdatingScrollMode choreography), collapsed onto
// the simulator's flat layout model (see the header).
//
// Simulator simplifications (documented):
//   - every source update reloads the realization pass (cells recycle into the pool and re-bind);
//     the PRECISE translated op trail still lands in platform.source_updates, which is what the
//     wave-3 native controllers consume 1:1;
//   - the view-level Header/Footer sit outside the scroll extent; group headers/footers are rows in
//     the flow (realized only when their template is set, like the C# supplementary registration);
//   - default cells (no ItemTemplate) host no content object — they realize fresh each time and
//     mirror item.text() (the C# DefaultCell label).

#include "maui/controls/items/collection_view_handler.hpp"

#include <algorithm>
#include <any>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/groupable_items_view.hpp"
#include "maui/controls/items/i_items_view.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_source_factory.hpp"
#include "maui/controls/items/items_updating_scroll_mode.hpp"
#include "maui/controls/items/items_view_scrolled_event_args.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/reorderable_items_view.hpp"
#include "maui/controls/items/scroll_to_request_event_args.hpp"
#include "maui/controls/items/selectable_items_view.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/items/snap_points_alignment.hpp"
#include "maui/controls/items/snap_points_type.hpp"
#include "maui/controls/items/structured_items_view.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/command_mapper.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"
#include "maui/essentials/device_display.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    namespace
    {
        constexpr const char* default_cell_reuse_id = "default_cell";

        // Resolve a possibly-selector template against one item (DataTemplateSelector.SelectTemplate;
        // the container is the items view itself, like C# passes the ItemsView).
        std::shared_ptr<data_template> resolve_template(const std::shared_ptr<data_template>& candidate,
                                                        const boxed_item& item, maui::core::bindable_object* container)
        {
            if (auto selector = std::dynamic_pointer_cast<data_template_selector>(candidate))
            {
                return selector->select_template(item.context_box(), container);
            }
            return candidate;
        }

        [[nodiscard]] double clamp_offset(double offset, double max_offset)
        {
            return std::max(0.0, std::min(offset, max_offset));
        }
    } // namespace

    collection_view_platform::collection_view_platform() = default;

    // Mirrors the stacked C# mappers (ItemsViewMapper → Structured → Selectable → Groupable →
    // Reorderable → CollectionViewHandler.Mapper), chained onto the shared view_mapper. The C#
    // Header/HeaderTemplate (and Footer twin) keys share one map function exactly as the C# table
    // maps both to MapHeaderTemplate; the group templates re-map like the Android/Windows table
    // (the simulator realizes group supplementals, which iOS leaves to the controller).
    maui::core::property_mapper<i_items_view, collection_view_handler>& collection_view_handler::mapper()
    {
        static maui::core::property_mapper<i_items_view, collection_view_handler> table{
            maui::core::view_mapper(),
            {
                {"items_source", &collection_view_handler::map_items_source},
                {"horizontal_scroll_bar_visibility", &collection_view_handler::map_horizontal_scroll_bar_visibility},
                {"vertical_scroll_bar_visibility", &collection_view_handler::map_vertical_scroll_bar_visibility},
                {"item_template", &collection_view_handler::map_item_template},
                {"empty_view", &collection_view_handler::map_empty_view},
                {"empty_view_template", &collection_view_handler::map_empty_view},
                {"items_updating_scroll_mode", &collection_view_handler::map_items_updating_scroll_mode},
                {"header", &collection_view_handler::map_header_footer},
                {"header_template", &collection_view_handler::map_header_footer},
                {"footer", &collection_view_handler::map_header_footer},
                {"footer_template", &collection_view_handler::map_header_footer},
                {"items_layout", &collection_view_handler::map_items_layout},
                {"item_sizing_strategy", &collection_view_handler::map_item_sizing_strategy},
                {"selected_item", &collection_view_handler::map_selected_item},
                {"selected_items", &collection_view_handler::map_selected_items},
                {"selection_mode", &collection_view_handler::map_selection_mode},
                {"is_grouped", &collection_view_handler::map_is_grouped},
                {"group_header_template", &collection_view_handler::map_group_templates},
                {"group_footer_template", &collection_view_handler::map_group_templates},
                {"can_reorder_items", &collection_view_handler::map_can_reorder_items},
                // CarouselViewHandler2.Mapper: the carousel reuses this handler, so its three extra
                // knobs register here and reach carousel_view by dynamic_cast (no-ops for collection_view).
                {"is_swipe_enabled", &collection_view_handler::map_is_swipe_enabled},
                {"is_bounce_enabled", &collection_view_handler::map_is_bounce_enabled},
                {"peek_area_insets", &collection_view_handler::map_peek_area_insets},
            },
        };
        return table;
    }

    // The ScrollTo funnel (C# wires ItemsView.ScrollToRequested in ConnectHandler; the port's control
    // invokes this command — items_view.hpp).
    maui::core::command_mapper<i_items_view, collection_view_handler>& collection_view_handler::command_mapper()
    {
        static maui::core::command_mapper<i_items_view, collection_view_handler> table{
            {"scroll_to", &collection_view_handler::map_scroll_to},
        };
        return table;
    }

    collection_view_handler::collection_view_handler() : view_handler(&mapper(), &command_mapper())
    {
    }

    void collection_view_handler::on_disconnect_handler(collection_view_platform& /*platform*/)
    {
        // C# DisconnectHandler: Controller.Disconnect — drop the source plumbing while everything is
        // still alive (subscriptions first, §8).
        source_updated_.reset();
        layout_changed_.reset();
        source_.reset();
        tracked_layout_.reset();
#ifdef MAUI_PLATFORM_IOS
        // The native data source reads the handler's (now-null) source on each query; ReloadData here
        // flushes the UICollectionView to an empty state so it never touches freed data after the
        // platform struct's destructor releases the controller (§8: drop data while alive).
        native_reload();
#endif
#ifdef MAUI_PLATFORM_APPLE
        // The NSCollectionView datasource reads the now-null source on each query; ReloadData flushes
        // it to an empty state so it never touches freed data after the platform struct's destructor
        // releases the native tree (§8: drop data while alive).
        native_reload();
#endif
    }

    // ItemsView.OnMeasure: clamp to the scaled screen size (the C# 40x40 minimum SizeRequest has no
    // port analog — header note in items_view.hpp).
    maui::graphics::size collection_view_handler::get_desired_size(double width_constraint,
                                                                   double height_constraint) const
    {
        const maui::devices::display_info info = maui::devices::device_display::main_display_info();
        const double density = info.density > 0 ? info.density : 1.0;
        const double scaled_width = info.width / density;
        const double scaled_height = info.height / density;
        return {std::min(scaled_width, width_constraint), std::min(scaled_height, height_constraint)};
    }

    void collection_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (platform->orientation == items_layout_orientation::vertical)
        {
            platform->viewport_main_extent = frame.height;
            platform->viewport_cross_extent = frame.width;
        }
        else
        {
            platform->viewport_main_extent = frame.width;
            platform->viewport_cross_extent = frame.height;
        }
        refresh_realization();
    }

    // ---- mapper entries ----

    void collection_view_handler::map_items_source(collection_view_handler& handler, i_items_view& view)
    {
        // C# MapItemsSource: MapItemsUpdatingScrollMode first, then UpdateItemsSource.
        map_items_updating_scroll_mode(handler, view);
        handler.update_items_source();
    }

    void collection_view_handler::map_item_template(collection_view_handler& handler, i_items_view& /*view*/)
    {
        // C# MapItemTemplate → UpdateLayout (reload): every realized cell recycles and re-realizes
        // through the new template.
        handler.refresh_realization();
#ifdef MAUI_PLATFORM_IOS
        // ItemsViewHandler2.MapItemTemplate → UpdateLayout: the reuse-id keys off the template, so the
        // layout + cells rebuild.
        handler.native_rebuild_layout();
        handler.native_reload();
#endif
#ifdef MAUI_PLATFORM_APPLE
        handler.native_reload(); // C# MapItemTemplate → reload (the cell hosts the template's content)
#endif
    }

    void collection_view_handler::map_empty_view(collection_view_handler& handler, i_items_view& /*view*/)
    {
        handler.update_empty_view();
#ifdef MAUI_PLATFORM_IOS
        handler.native_update_empty_view();
#endif
#ifdef MAUI_PLATFORM_APPLE
        handler.native_update_empty_view();
#endif
    }

    void collection_view_handler::map_items_updating_scroll_mode(collection_view_handler& handler, i_items_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->scroll_mode = view.items_updating_scroll_mode();
        }
    }

    void collection_view_handler::map_horizontal_scroll_bar_visibility(collection_view_handler& handler,
                                                                       i_items_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->horizontal_bar_visibility = view.horizontal_scroll_bar_visibility();
        }
    }

    void collection_view_handler::map_vertical_scroll_bar_visibility(collection_view_handler& handler,
                                                                     i_items_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_bar_visibility = view.vertical_scroll_bar_visibility();
        }
    }

    void collection_view_handler::map_header_footer(collection_view_handler& handler, i_items_view& /*view*/)
    {
        handler.update_header_footer();
#ifdef MAUI_PLATFORM_IOS
        // C# MapHeaderTemplate/MapFooterTemplate → UpdateLayout (the section's boundary supplementary
        // items change) + UpdateHeaderView/UpdateFooterView (reload realizes them).
        handler.native_rebuild_layout();
        handler.native_reload();
#endif
#ifdef MAUI_PLATFORM_APPLE
        // C# MapHeaderTemplate/MapFooterTemplate → UpdateLayout (the flow layout's section
        // header/footer reference size changes) + reload realizes them.
        handler.native_rebuild_layout();
        handler.native_reload();
#endif
    }

    void collection_view_handler::map_items_layout(collection_view_handler& handler, i_items_view& view)
    {
        // Re-seat the layout subscription: disconnect FIRST (while the old layout is still pinned),
        // then pin the new layout, then connect (§8).
        handler.layout_changed_.reset();
        auto* structured = dynamic_cast<structured_items_view*>(&view);
        handler.tracked_layout_ = structured != nullptr ? structured->items_layout() : nullptr;
        if (handler.tracked_layout_)
        {
            // The native layouts observe the ItemsLayout's INPC (e.g. a GridItemsLayout.Span change
            // re-lays-out) — the simulator mirrors that here.
            collection_view_handler* self = &handler;
            handler.layout_changed_ =
                maui::core::connect_scoped(handler.tracked_layout_->property_changed, [self](std::string_view) {
                    self->refresh_layout_mirrors();
                    self->refresh_realization();
                });
        }
        handler.refresh_layout_mirrors();
        handler.refresh_realization();
#ifdef MAUI_PLATFORM_IOS
        // C# MapItemsLayout: UpdateItemsLayoutSubscription + UpdateLayout (rebuild the compositional
        // layout from the new ItemsLayout — orientation/span/spacing/snap all feed LayoutFactory2).
        handler.native_rebuild_layout();
        handler.native_reload();
#endif
#ifdef MAUI_PLATFORM_APPLE
        // C# MapItemsLayout → UpdateLayout: rebuild the NSCollectionViewFlowLayout from the new
        // ItemsLayout (orientation → scrollDirection, span → item width, spacing → inter-item/line).
        handler.native_rebuild_layout();
        handler.native_reload();
#endif
    }

    void collection_view_handler::map_item_sizing_strategy(collection_view_handler& handler, i_items_view& view)
    {
        auto* platform = handler.typed_platform_view();
        auto* structured = dynamic_cast<structured_items_view*>(&view);
        if (platform != nullptr && structured != nullptr)
        {
            platform->sizing_strategy = structured->item_sizing_strategy();
        }
#ifdef MAUI_PLATFORM_IOS
        handler.native_rebuild_layout(); // C# MapItemSizingStrategy → UpdateLayout
#endif
#ifdef MAUI_PLATFORM_APPLE
        handler.native_rebuild_layout(); // C# MapItemSizingStrategy → UpdateLayout
#endif
    }

    void collection_view_handler::map_selected_item(collection_view_handler& handler, i_items_view& /*view*/)
    {
        handler.update_platform_selection();
#ifdef MAUI_PLATFORM_IOS
        handler.native_update_platform_selection();
#endif
#ifdef MAUI_PLATFORM_APPLE
        handler.native_update_platform_selection();
#endif
    }

    void collection_view_handler::map_selected_items(collection_view_handler& handler, i_items_view& /*view*/)
    {
        handler.update_platform_selection();
#ifdef MAUI_PLATFORM_IOS
        handler.native_update_platform_selection();
#endif
#ifdef MAUI_PLATFORM_APPLE
        handler.native_update_platform_selection();
#endif
    }

    void collection_view_handler::map_selection_mode(collection_view_handler& handler, i_items_view& /*view*/)
    {
        handler.update_selection_mode();
        handler.update_platform_selection();
#ifdef MAUI_PLATFORM_IOS
        handler.native_update_selection_mode();
        handler.native_update_platform_selection();
#endif
#ifdef MAUI_PLATFORM_APPLE
        handler.native_update_selection_mode();
        handler.native_update_platform_selection();
#endif
    }

    void collection_view_handler::map_is_grouped(collection_view_handler& handler, i_items_view& /*view*/)
    {
        // The source must be re-minted in the other shape (flat ⇄ grouped).
        handler.update_items_source();
#ifdef MAUI_PLATFORM_IOS
        // C# MapIsGrouped: Controller.UpdateItemsSource (done above) + handler.UpdateLayout — the
        // grouped path puts headers/footers in per-section boundary supplementary items.
        handler.native_rebuild_layout();
        handler.native_reload();
#endif
#ifdef MAUI_PLATFORM_APPLE
        // C# MapIsGrouped: UpdateItemsSource (done above) + UpdateLayout — the grouped path puts
        // headers/footers in per-section flow-layout supplementary views.
        handler.native_rebuild_layout();
        handler.native_reload();
#endif
    }

    void collection_view_handler::map_group_templates(collection_view_handler& handler, i_items_view& /*view*/)
    {
        // Group header/footer rows enter/leave the flow.
        handler.refresh_realization();
#ifdef MAUI_PLATFORM_IOS
        // C# MapHeaderTemplate/MapFooterTemplate (the group twins): UpdateLayout adds/removes the
        // section boundary supplementary items, then reload realizes them.
        handler.native_rebuild_layout();
        handler.native_reload();
#endif
#ifdef MAUI_PLATFORM_APPLE
        // C# group-template twins: UpdateLayout toggles the section header/footer reference size, then
        // reload realizes them.
        handler.native_rebuild_layout();
        handler.native_reload();
#endif
    }

    void collection_view_handler::map_can_reorder_items(collection_view_handler& handler, i_items_view& view)
    {
        auto* platform = handler.typed_platform_view();
        auto* reorderable = dynamic_cast<reorderable_items_view*>(&view);
        if (platform != nullptr && reorderable != nullptr)
        {
            platform->can_reorder_items = reorderable->can_reorder_items();
        }
#ifdef MAUI_PLATFORM_IOS
        handler.native_update_can_reorder(); // C# MapCanReorderItems → UpdateCanReorderItems
#endif
#ifdef MAUI_PLATFORM_APPLE
        handler.native_update_can_reorder(); // C# MapCanReorderItems → UpdateCanReorderItems
#endif
    }

    // CarouselViewHandler2.MapIsSwipeEnabled: CollectionView.ScrollEnabled = IsSwipeEnabled. The carousel
    // reuses this handler, so it reaches the concrete carousel_view by dynamic_cast (no-op otherwise).
    void collection_view_handler::map_is_swipe_enabled(collection_view_handler& handler, i_items_view& view)
    {
        auto* platform = handler.typed_platform_view();
        auto* carousel = dynamic_cast<carousel_view*>(&view);
        if (platform == nullptr || carousel == nullptr)
        {
            return;
        }
        platform->swipe_enabled = carousel->is_swipe_enabled();
#ifdef MAUI_PLATFORM_IOS
        handler.native_update_swipe_enabled(); // CollectionView.ScrollEnabled
#endif
    }

    // CarouselViewHandler2.MapIsBounceEnabled: CollectionView.Bounces = IsBounceEnabled.
    void collection_view_handler::map_is_bounce_enabled(collection_view_handler& handler, i_items_view& view)
    {
        auto* platform = handler.typed_platform_view();
        auto* carousel = dynamic_cast<carousel_view*>(&view);
        if (platform == nullptr || carousel == nullptr)
        {
            return;
        }
        platform->bounce_enabled = carousel->is_bounce_enabled();
#ifdef MAUI_PLATFORM_IOS
        handler.native_update_bounce_enabled(); // CollectionView.Bounces
#endif
    }

    // CarouselViewHandler2.MapPeekAreaInsets: handler.UpdateLayout() — adjust the layout insets so the
    // adjacent items "peek" in. The port records the inset on the mirror and pushes the native
    // section/content insets on iOS (where a real UICollectionView applies them). On headless/appkit the
    // mirror is the asserted surface — no native carousel inset realization (documented simplification).
    void collection_view_handler::map_peek_area_insets(collection_view_handler& handler, i_items_view& view)
    {
        auto* platform = handler.typed_platform_view();
        auto* carousel = dynamic_cast<carousel_view*>(&view);
        if (platform == nullptr || carousel == nullptr)
        {
            return;
        }
        platform->peek_area_insets = carousel->peek_area_insets();
#ifdef MAUI_PLATFORM_IOS
        handler.native_update_peek_area_insets(); // UpdateLayout → section/content insets
#endif
    }

    // CarouselViewController2.SetPosition: write the settled scroll position BACK to the carousel
    // (SetValueFromRenderer), then chain into SetCurrentItem (the C# SetPosition → SetCurrentItem order).
    // The suppress gate (C# _isInternalCollectionUpdate) drops the writeback during a batch source
    // update so spurious UIKit scroll callbacks can't clobber the position the update is computing.
    void collection_view_handler::set_position_from_scroll(int position)
    {
        if (suppress_scroll_writeback_)
        {
            return;
        }
        auto* carousel = dynamic_cast<carousel_view*>(virtual_view());
        if (carousel == nullptr || !source_ || source_->item_count() == 0)
        {
            return; // C# SetPosition guards: ItemsView is CarouselView, ItemsSource non-empty
        }
        if (position < 0 || position >= source_->item_count())
        {
            return; // C# `position == -1` early-out; reject out-of-range (source_->item would throw)
        }
        carousel->set_position(position);
        set_current_item_from_scroll(position);
    }

    // CarouselViewController2.SetCurrentItem: resolve the item at `position` from the source and write it
    // back (SetValueFromRenderer). Shares the suppress gate with set_position_from_scroll.
    void collection_view_handler::set_current_item_from_scroll(int position)
    {
        if (suppress_scroll_writeback_)
        {
            return;
        }
        auto* carousel = dynamic_cast<carousel_view*>(virtual_view());
        if (carousel == nullptr || !source_ || source_->item_count() == 0 || position < 0 ||
            position >= source_->item_count())
        {
            return; // reject out-of-range (source_->item would throw out_of_range)
        }
        // The carousel is single-section (section 0); resolve the item at the centered ordinal.
        carousel->set_current_item(source_->item(index_path{.section = 0, .item = position}));
    }

    // C# MapScrollTo (ItemsViewHandler.ScrollToRequested): resolve the request to an index path,
    // validate it, and move the viewport per the requested position.
    void collection_view_handler::map_scroll_to(collection_view_handler& handler, i_items_view& /*view*/,
                                                const std::any& args)
    {
        const auto* request = std::any_cast<scroll_to_request_event_args>(&args);
        auto* platform = handler.typed_platform_view();
        if (request == nullptr || platform == nullptr || !handler.source_ || handler.source_->group_count() == 0)
        {
            return;
        }

        index_path path{};
        if (request->mode == scroll_to_mode::position)
        {
            path = {.section = request->group_index == -1 ? 0 : request->group_index, .item = request->index};
        }
        else
        {
            path = handler.source_->get_index_for_item(request->item);
        }
        // IsIndexPathValid.
        if (path.section < 0 || path.section >= handler.source_->group_count() || path.item < 0 ||
            path.item >= handler.source_->item_count_in_group(path.section))
        {
            return;
        }

        platform->scroll_requests.push_back(*request);

#ifdef MAUI_PLATFORM_IOS
        // C# ItemsViewHandler2.ScrollToRequested: move the native viewport (the simulator math below is
        // the cross-platform state mirror; the native scroll is what the on-simulator suite asserts).
        handler.native_scroll_to(path, request->scroll_to_position, request->is_animated);
#endif
#ifdef MAUI_PLATFORM_APPLE
        // C# ItemsViewHandler.ScrollToRequested: scroll the real NSCollectionView to the item.
        handler.native_scroll_to(path, request->scroll_to_position, request->is_animated);
#endif

        const std::vector<layout_entry> entries = handler.build_entries();
        const layout_entry* target = nullptr;
        for (const layout_entry& entry : entries)
        {
            if (entry.element == cell_element_kind::item && entry.path == path)
            {
                target = &entry;
                break;
            }
        }
        if (target == nullptr)
        {
            return;
        }

        const double viewport = platform->viewport_main_extent;
        double offset = platform->scroll_offset;
        switch (request->scroll_to_position)
        {
            case controls::scroll_to_position::start:
                offset = target->start;
                break;
            case controls::scroll_to_position::center:
                offset = target->start + (target->extent / 2) - (viewport / 2);
                break;
            case controls::scroll_to_position::end:
                offset = target->start + target->extent - viewport;
                break;
            case controls::scroll_to_position::make_visible:
                if (target->start < offset)
                {
                    offset = target->start;
                }
                else if (target->start + target->extent > offset + viewport)
                {
                    offset = target->start + target->extent - viewport;
                }
                break;
        }
        handler.apply_scroll(offset);
    }

    // ---- the items-source plumbing ----

    void collection_view_handler::update_items_source()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        // Disconnect FIRST, while the old source is still alive (§8), then re-mint.
        source_updated_.reset();

        bool grouped = false;
        if (auto* groupable = dynamic_cast<groupable_items_view*>(view))
        {
            grouped = groupable->is_grouped();
        }
        platform->grouped = grouped;
        source_ = grouped ? items_source_factory::create_grouped(view->items_source())
                          : items_source_factory::create(view->items_source());
        source_updated_ = maui::core::connect_scoped(
            source_->updated, [this](const source_update& update) { on_source_updated(update); });

        update_empty_view();
        refresh_realization();
        update_platform_selection();

#ifdef MAUI_PLATFORM_IOS
        // C# ItemsViewController2.UpdateItemsSource: re-create the source, ReloadData, invalidate the
        // layout (the grouped⇄flat shape change re-mints the compositional layout via map_is_grouped).
        native_reload();
        native_update_platform_selection();
#endif
#ifdef MAUI_PLATFORM_APPLE
        // C# ItemsViewController.UpdateItemsSource: re-create the source, ReloadData, re-sync selection.
        native_reload();
        native_update_platform_selection();
#endif
    }

    // One translated source op arrived (counts already adjusted): record it, apply the
    // ItemsUpdatingScrollMode choreography (the ItemsViewLayout port), and re-realize.
    void collection_view_handler::on_source_updated(const source_update& args)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->source_updates.push_back(args);

        const double previous_extent = platform->content_extent;

        // UpdateWillShiftVisibleItems: does this update land at-or-before the first visible row?
        std::optional<index_path> first_visible;
        for (const realized_cell& cell : platform->realized)
        {
            if (cell.element == cell_element_kind::item)
            {
                first_visible = cell.path;
                break;
            }
        }
        bool shifts = false;
        if (first_visible.has_value())
        {
            const index_path first = first_visible.value();
            switch (args.kind)
            {
                case source_update_kind::insert_items:
                case source_update_kind::delete_items:
                case source_update_kind::move_item:
                    shifts =
                        args.section < first.section || (args.section == first.section && args.index <= first.item);
                    break;
                case source_update_kind::insert_sections:
                case source_update_kind::delete_sections:
                case source_update_kind::move_section:
                    shifts = args.section <= first.section;
                    break;
                default:
                    break;
            }
        }

        refresh_realization();
        update_empty_view();

        switch (platform->scroll_mode)
        {
            case controls::items_updating_scroll_mode::keep_scroll_offset:
                // The iOS default — nothing to adjust (the refresh already clamped).
                break;

            case controls::items_updating_scroll_mode::keep_items_in_view:
                if (shifts)
                {
                    // TargetContentOffsetForProposedContentOffset: compensate by the content delta.
                    platform->scroll_offset = clamp_offset(
                        platform->scroll_offset + (platform->content_extent - previous_extent), max_scroll_offset());
                    refresh_realization();
                }
                break;

            case controls::items_updating_scroll_mode::keep_last_item_in_view:
                // FinalizeCollectionViewUpdates → ForceScrollToLastItem (item/section updates only;
                // a reload_data is not a "collection view update" in C# either).
                if (args.kind != source_update_kind::reload_data)
                {
                    platform->scroll_offset = max_scroll_offset();
                    refresh_realization();
                }
                break;
        }

#ifdef MAUI_PLATFORM_IOS
        // C# IObservableItemsViewSource translates each change into a UICollectionView batch update;
        // the port re-runs the full realization pass (ReloadData) on the native view too (the precise
        // op trail is already in platform->source_updates for any consumer that wants the diff).
        native_reload();
        native_update_empty_view();
#endif
#ifdef MAUI_PLATFORM_APPLE
        // The same on AppKit: re-run the realization pass (ReloadData) on the real NSCollectionView.
        native_reload();
        native_update_empty_view();
#endif
    }

    // ---- the simulator ----

    void collection_view_handler::refresh_layout_mirrors()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const items_layout* layout = tracked_layout_.get();
        if (layout == nullptr)
        {
            platform->orientation = items_layout_orientation::vertical;
            platform->span = 1;
            platform->item_spacing = 0;
            platform->snap_points_type = controls::snap_points_type::none;
            platform->snap_points_alignment = controls::snap_points_alignment::start;
            return;
        }
        platform->orientation = layout->orientation();
        platform->snap_points_type = layout->snap_points_type();
        platform->snap_points_alignment = layout->snap_points_alignment();
        if (const auto* linear = dynamic_cast<const linear_items_layout*>(layout))
        {
            platform->span = 1;
            platform->item_spacing = linear->item_spacing();
        }
        else if (const auto* grid = dynamic_cast<const grid_items_layout*>(layout))
        {
            platform->span = grid->span();
            // The main-axis row spacing: vertical lists stack rows vertically and vice versa.
            platform->item_spacing = layout->orientation() == items_layout_orientation::vertical
                                         ? grid->vertical_item_spacing()
                                         : grid->horizontal_item_spacing();
        }
        else
        {
            platform->span = 1;
            platform->item_spacing = 0;
        }
    }

    std::vector<collection_view_handler::layout_entry> collection_view_handler::build_entries() const
    {
        std::vector<layout_entry> entries;
        auto* platform = typed_platform_view();
        if (platform == nullptr || !source_)
        {
            return entries;
        }
        const double extent = platform->item_extent;
        const double spacing = platform->item_spacing;
        const int span = std::max(1, platform->span);
        double cursor = 0;

        auto add_row = [&](cell_element_kind element, int section, int first_item, int item_count) {
            if (element == cell_element_kind::item)
            {
                for (int item = first_item; item < first_item + item_count; ++item)
                {
                    entries.push_back({.element = element,
                                       .path = {.section = section, .item = item},
                                       .start = cursor,
                                       .extent = extent});
                }
            }
            else
            {
                entries.push_back(
                    {.element = element, .path = {.section = section, .item = -1}, .start = cursor, .extent = extent});
            }
            cursor += extent + spacing;
        };

        bool header_rows = false;
        bool footer_rows = false;
        if (platform->grouped)
        {
            if (const auto* groupable = dynamic_cast<const groupable_items_view*>(virtual_view()))
            {
                header_rows = groupable->group_header_template() != nullptr;
                footer_rows = groupable->group_footer_template() != nullptr;
            }
        }

        const int sections = source_->group_count();
        for (int section = 0; section < sections; ++section)
        {
            if (header_rows)
            {
                add_row(cell_element_kind::group_header, section, 0, 0);
            }
            const int count = source_->item_count_in_group(section);
            for (int first = 0; first < count; first += span)
            {
                add_row(cell_element_kind::item, section, first, std::min(span, count - first));
            }
            if (footer_rows)
            {
                add_row(cell_element_kind::group_footer, section, 0, 0);
            }
        }
        return entries;
    }

    double collection_view_handler::max_scroll_offset() const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return 0;
        }
        return std::max(0.0, platform->content_extent - platform->viewport_main_extent);
    }

    int collection_view_handler::flat_item_ordinal(const index_path& path) const
    {
        if (!source_)
        {
            return -1;
        }
        int ordinal = path.item;
        for (int section = 0; section < path.section; ++section)
        {
            ordinal += source_->item_count_in_group(section);
        }
        return ordinal;
    }

    std::shared_ptr<maui::core::bindable_object> collection_view_handler::take_from_pool(const std::string& reuse_id)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return nullptr;
        }
        for (auto it = platform->recycle_pool.begin(); it != platform->recycle_pool.end(); ++it)
        {
            if (it->first == reuse_id)
            {
                std::shared_ptr<maui::core::bindable_object> content = std::move(it->second);
                platform->recycle_pool.erase(it);
                return content;
            }
        }
        return nullptr;
    }

    // The realization pass: recycle whatever is realized, then realize + bind every entry that
    // intersects the viewport window (header simplification note at the top of this file).
    void collection_view_handler::refresh_realization()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }

        // Recycle the current pass (cells with template content return to the pool).
        for (realized_cell& cell : platform->realized)
        {
            if (cell.content)
            {
                platform->recycle_pool.emplace_back(cell.reuse_id, std::move(cell.content));
            }
            platform->events.push_back({.kind = cell_event_kind::recycled,
                                        .element = cell.element,
                                        .path = cell.path,
                                        .reuse_id = cell.reuse_id});
        }
        platform->realized.clear();

        const std::vector<layout_entry> entries = build_entries();
        platform->content_extent = 0;
        for (const layout_entry& entry : entries)
        {
            platform->content_extent = std::max(platform->content_extent, entry.start + entry.extent);
        }
        platform->scroll_offset = clamp_offset(platform->scroll_offset, max_scroll_offset());

        const double window_start = platform->scroll_offset;
        const double window_end = platform->scroll_offset + platform->viewport_main_extent;
        auto* container = dynamic_cast<maui::core::bindable_object*>(view);
        const auto* groupable = dynamic_cast<const groupable_items_view*>(view);

        for (const layout_entry& entry : entries)
        {
            if (entry.start >= window_end || entry.start + entry.extent <= window_start)
            {
                continue;
            }

            // The element's value + template.
            boxed_item value;
            std::shared_ptr<data_template> cell_template;
            switch (entry.element)
            {
                case cell_element_kind::item:
                    value = source_->item(entry.path);
                    cell_template = view->item_template();
                    break;
                case cell_element_kind::group_header:
                    value = source_->group(entry.path);
                    cell_template = groupable != nullptr ? groupable->group_header_template() : nullptr;
                    break;
                case cell_element_kind::group_footer:
                    value = source_->group(entry.path);
                    cell_template = groupable != nullptr ? groupable->group_footer_template() : nullptr;
                    break;
                default:
                    break;
            }

            realized_cell cell;
            cell.element = entry.element;
            cell.path = entry.path;
            cell.start = entry.start;
            cell.extent = entry.extent;
            if (cell_template)
            {
                const std::shared_ptr<data_template> resolved = resolve_template(cell_template, value, container);
                cell.reuse_id = resolved->id_string();
                cell.content = take_from_pool(cell.reuse_id);
                if (!cell.content)
                {
                    cell.content = resolved->create_content();
                    platform->events.push_back({.kind = cell_event_kind::realized,
                                                .element = entry.element,
                                                .path = entry.path,
                                                .reuse_id = cell.reuse_id});
                }
            }
            else
            {
                // The DefaultCell: no content object, fresh each time (file header note).
                cell.reuse_id = default_cell_reuse_id;
                platform->events.push_back({.kind = cell_event_kind::realized,
                                            .element = entry.element,
                                            .path = entry.path,
                                            .reuse_id = cell.reuse_id});
            }
            if (cell.content)
            {
                cell.content->set_binding_context_box(value.context_box());
            }
            cell.text = value.text();
            platform->events.push_back({.kind = cell_event_kind::bound,
                                        .element = entry.element,
                                        .path = entry.path,
                                        .reuse_id = cell.reuse_id});
            platform->realized.push_back(std::move(cell));
        }
    }

    // ItemsViewController.UpdateEmptyView: shown while the source has no items.
    void collection_view_handler::update_empty_view()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        const bool empty = !source_ || source_->item_count() == 0;
        if (!empty)
        {
            if (platform->empty_view.present)
            {
                platform->events.push_back({.kind = cell_event_kind::recycled,
                                            .element = cell_element_kind::empty_view,
                                            .path = {.section = -1, .item = -1},
                                            .reuse_id = platform->empty_view.reuse_id});
                platform->empty_view = {};
            }
            return;
        }
        realize_supplemental(platform->empty_view, cell_element_kind::empty_view, view->empty_view_template(),
                             view->empty_view());
    }

    void collection_view_handler::update_header_footer()
    {
        auto* platform = typed_platform_view();
        auto* structured = dynamic_cast<structured_items_view*>(virtual_view());
        if (platform == nullptr || structured == nullptr)
        {
            return;
        }
        realize_supplemental(platform->header, cell_element_kind::header, structured->header_template(),
                             structured->header());
        realize_supplemental(platform->footer, cell_element_kind::footer, structured->footer_template(),
                             structured->footer());
    }

    // The C# CreateEmptyView/UpdateHeaderFooterPosition split, reduced: template → create + bind;
    // a boxed VIEW hosts directly; any other value renders its text; null clears.
    void collection_view_handler::realize_supplemental(realized_supplemental& slot, cell_element_kind element,
                                                       const std::shared_ptr<data_template>& content_template,
                                                       const boxed_item& value)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        slot = {};
        if (content_template)
        {
            const std::shared_ptr<data_template> resolved =
                resolve_template(content_template, value, dynamic_cast<maui::core::bindable_object*>(virtual_view()));
            slot.present = true;
            slot.reuse_id = resolved->id_string();
            slot.content = resolved->create_content();
            slot.content->set_binding_context_box(value.context_box());
            platform->events.push_back({.kind = cell_event_kind::realized,
                                        .element = element,
                                        .path = {.section = -1, .item = -1},
                                        .reuse_id = slot.reuse_id});
        }
        else if (value.as_bindable())
        {
            slot.present = true;
            slot.reuse_id = "view";
            slot.content = value.as_bindable();
        }
        else if (value.has_value())
        {
            slot.present = true;
            slot.reuse_id = "text";
        }
        else
        {
            return; // nothing to show
        }
        slot.text = value.text();
        platform->events.push_back({.kind = cell_event_kind::bound,
                                    .element = element,
                                    .path = {.section = -1, .item = -1},
                                    .reuse_id = slot.reuse_id});
    }

    // ---- selection (SelectableItemsViewController + UpdateSelectionMode) ----

    void collection_view_handler::update_selection_mode()
    {
        auto* platform = typed_platform_view();
        auto* selectable = dynamic_cast<selectable_items_view*>(virtual_view());
        if (platform == nullptr || selectable == nullptr)
        {
            return;
        }
        switch (selectable->selection_mode())
        {
            case controls::selection_mode::none:
                platform->allows_selection = false;
                platform->allows_multiple_selection = false;
                break;
            case controls::selection_mode::single:
                platform->allows_selection = true;
                platform->allows_multiple_selection = false;
                break;
            case controls::selection_mode::multiple:
                platform->allows_selection = true;
                platform->allows_multiple_selection = true;
                break;
        }
    }

    void collection_view_handler::update_platform_selection()
    {
        auto* platform = typed_platform_view();
        auto* selectable = dynamic_cast<selectable_items_view*>(virtual_view());
        if (platform == nullptr || selectable == nullptr)
        {
            return;
        }
        // C# UpdatePlatformSelection: a null ItemsSource (or the EmptySource) never syncs.
        if (selectable->items_source() == nullptr || !source_ || source_->group_count() == 0)
        {
            return;
        }
        switch (selectable->selection_mode())
        {
            case controls::selection_mode::none:
                return; // C# returns without touching the native selection

            case controls::selection_mode::single: {
                platform->selected_paths.clear();
                const boxed_item& selected = selectable->selected_item();
                if (selected.has_value())
                {
                    const index_path path = source_->get_index_for_item(selected);
                    if (path.section > -1 && path.item > -1)
                    {
                        platform->selected_paths.push_back(path);
                    }
                }
                return;
            }

            case controls::selection_mode::multiple: {
                platform->selected_paths.clear();
                for (const boxed_item& item : selectable->selected_items().items())
                {
                    const index_path path = source_->get_index_for_item(item);
                    if (path.section > -1 && path.item > -1)
                    {
                        platform->selected_paths.push_back(path);
                    }
                }
                return;
            }
        }
    }

    // ---- the user-side inbound channels ----

    void collection_view_handler::simulate_viewport(double main_extent, double cross_extent)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->viewport_main_extent = main_extent;
        platform->viewport_cross_extent = cross_extent;
        refresh_realization();
    }

    void collection_view_handler::simulate_scroll(double offset)
    {
        apply_scroll(offset);
    }

    void collection_view_handler::apply_scroll(double offset)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->scroll_offset = clamp_offset(offset, max_scroll_offset());
        refresh_realization();
        report_scrolled();
    }

    // ItemsViewDelegator.Scrolled: report offsets/deltas + the visible-index trio, then trip the
    // remaining-items threshold.
    void collection_view_handler::report_scrolled()
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }

        int first_visible = -1;
        int center_visible = -1;
        int last_visible = -1;
        std::vector<int> ordinals;
        for (const realized_cell& cell : platform->realized)
        {
            if (cell.element == cell_element_kind::item)
            {
                ordinals.push_back(flat_item_ordinal(cell.path));
            }
        }
        const bool visible_items = !ordinals.empty();
        if (visible_items)
        {
            first_visible = *std::ranges::min_element(ordinals);
            last_visible = *std::ranges::max_element(ordinals);
            std::ranges::sort(ordinals);
            center_visible = ordinals[ordinals.size() / 2];
        }

        // C#: with no visible items the reported offset is 0.
        const double offset = visible_items ? platform->scroll_offset : 0;
        items_view_scrolled_event_args args;
        if (platform->orientation == items_layout_orientation::vertical)
        {
            args.vertical_offset = offset;
            args.vertical_delta = offset - platform->previous_scroll_offset;
        }
        else
        {
            args.horizontal_offset = offset;
            args.horizontal_delta = offset - platform->previous_scroll_offset;
        }
        args.first_visible_item_index = first_visible;
        args.center_item_index = center_visible;
        args.last_visible_item_index = last_visible;

        view->send_scrolled(args);
        platform->previous_scroll_offset = offset;

        if (!visible_items || !source_)
        {
            return;
        }
        switch (view->remaining_items_threshold())
        {
            case -1:
                return;
            case 0:
                if (last_visible == source_->item_count() - 1)
                {
                    view->send_remaining_items_threshold_reached();
                }
                break;
            default:
                if (source_->item_count() - 1 - last_visible <= view->remaining_items_threshold())
                {
                    view->send_remaining_items_threshold_reached();
                }
                break;
        }
    }

    // SelectableItemsViewController.ItemSelected (user-initiated only — programmatic selection goes
    // the other way, through the mapper).
    void collection_view_handler::simulate_select(const index_path& path)
    {
        auto* selectable = dynamic_cast<selectable_items_view*>(virtual_view());
        if (selectable == nullptr || selectable->items_source() == nullptr || !source_)
        {
            return; // C#: ItemsView?.ItemsSource is null → ignore
        }
        switch (selectable->selection_mode())
        {
            case controls::selection_mode::none:
                break;
            case controls::selection_mode::single:
                selectable->set_selected_item(source_->item(path));
                break;
            case controls::selection_mode::multiple:
                selectable->selected_items().add(source_->item(path));
                break;
        }
    }

    // SelectableItemsViewController.ItemDeselected.
    void collection_view_handler::simulate_deselect(const index_path& path)
    {
        auto* selectable = dynamic_cast<selectable_items_view*>(virtual_view());
        if (selectable == nullptr || selectable->items_source() == nullptr || !source_)
        {
            return;
        }
        switch (selectable->selection_mode())
        {
            case controls::selection_mode::none:
            case controls::selection_mode::single: // C# ignores single-mode deselects
                break;
            case controls::selection_mode::multiple:
                selectable->selected_items().remove(source_->item(path));
                break;
        }
    }

    // The reorder surface: the drag is native (wave 3); a completed user reorder reports through
    // SendReorderCompleted, gated on CanReorderItems.
    void collection_view_handler::simulate_reorder_completed()
    {
        auto* reorderable = dynamic_cast<reorderable_items_view*>(virtual_view());
        if (reorderable != nullptr && reorderable->can_reorder_items())
        {
            reorderable->send_reorder_completed();
        }
    }
} // namespace maui::controls
