// Tests for collection_view + collection_view_handler — the headless fake-viewport VIRTUALIZATION
// SIMULATOR (W2-19 unit point 4): realize/recycle/bind through the data_template machinery, the
// EmptyView/Header/Footer supplementals, selection through the simulated taps
// (SelectableItemsViewController), grouping rows, ItemsUpdatingScrollMode semantics
// (ItemsViewLayout), the remaining-items threshold (ItemsViewDelegator.Scrolled), and the
// "scroll_to" command (MapScrollTo). Backend-agnostic: the simulator is the shared platform recipe
// on every backend. §8: collections (publishers) are declared before the view/handler (subscribers).

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_updating_scroll_mode.hpp"
#include "maui/controls/items/items_view_scrolled_event_args.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/event.hpp"
#include "maui/core/observable_collection.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::boxed_item;
    using maui::controls::cell_element_kind;
    using maui::controls::cell_event;
    using maui::controls::cell_event_kind;
    using maui::controls::collection_view;
    using maui::controls::collection_view_handler;
    using maui::controls::collection_view_platform;
    using maui::controls::data_template;
    using maui::controls::grid_items_layout;
    using maui::controls::grouping;
    using maui::controls::grouping_ptr;
    using maui::controls::index_path;
    using maui::controls::items_layout_orientation;
    using maui::controls::items_updating_scroll_mode;
    using maui::controls::label;
    using maui::controls::make_item_collection;
    using maui::controls::realized_cell;
    using maui::controls::selection_mode;
    using maui::controls::source_update_kind;
    using maui::core::observable_collection;

    using string_collection = observable_collection<std::string>;

    // A NON-string custom item type (the selection_mode gallery's photo_item shape): the templated cell
    // binds a label to its `title` field, exercising the struct → BindingContext → typed-selector path
    // (boxed_item::of<photo_item>'s type_tag-checked context vs the std::string built-in text mirror).
    struct photo_item
    {
        std::string title;
        bool operator==(const photo_item&) const = default;
    };
    using photo_collection = observable_collection<photo_item>;

    // A recyclable (type-activated) label template whose Text binds to the string item itself.
    std::shared_ptr<data_template> make_label_template()
    {
        auto tmpl = data_template::of<label>();
        tmpl->set_binding<std::string, std::string>(label::text_property(),
                                                    [](const std::string& value) { return value; });
        return tmpl;
    }

    // A label template whose Text binds to a custom struct's `title` field (the non-std::string path).
    std::shared_ptr<data_template> make_photo_title_template()
    {
        auto tmpl = data_template::of<label>();
        tmpl->set_binding<std::string, photo_item>(label::text_property(),
                                                   [](const photo_item& item) { return item.title; });
        return tmpl;
    }

    // The standard rig: 10 string items, the default viewport (400 main / item extent 100).
    struct sim
    {
        std::shared_ptr<string_collection> items; // publisher FIRST (§8)
        collection_view view;
        std::shared_ptr<collection_view_handler> handler = std::make_shared<collection_view_handler>();
        // ORDER: the platform view exists only after set_handler connects the pair, so the initializer
        // routes through connect() (items is declared first, so it is constructed before this runs). A
        // plain member assignment trips prefer-member-initializer, whose hoist would deref null → segfault.
        collection_view_platform* platform = connect(view, handler, items);

        explicit sim(std::vector<std::string> initial = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"})
            : items(std::make_shared<string_collection>(std::move(initial)))
        {
        }

        [[nodiscard]] static collection_view_platform* connect(
            collection_view& view_ref, const std::shared_ptr<collection_view_handler>& handler_ref,
            const std::shared_ptr<string_collection>& items_ref)
        {
            view_ref.set_items_source(items_ref);
            view_ref.set_handler(handler_ref);
            return handler_ref->typed_platform_view();
        }

        [[nodiscard]] std::vector<index_path> realized_item_paths() const
        {
            std::vector<index_path> paths;
            for (const realized_cell& cell : platform->realized)
            {
                if (cell.element == cell_element_kind::item)
                {
                    paths.push_back(cell.path);
                }
            }
            return paths;
        }

        [[nodiscard]] long realized_event_count() const
        {
            return std::count_if(platform->events.begin(), platform->events.end(), [](const cell_event& entry) {
                return entry.kind == cell_event_kind::realized && entry.element == cell_element_kind::item;
            });
        }
    };

    // ---- realization + virtualization ----

    TEST(collection_view_sim, attach_realizes_only_the_visible_window)
    {
        sim const rig;
        // viewport 400 / extent 100 → items 0..3 fill the window; content extent = 10 * 100.
        const std::vector<index_path> expected{
            {.section = 0, .item = 0}, {.section = 0, .item = 1}, {.section = 0, .item = 2}, {.section = 0, .item = 3}};
        EXPECT_EQ(rig.realized_item_paths(), expected);
        EXPECT_DOUBLE_EQ(rig.platform->content_extent, 1000);
        // Default cells mirror item.ToString().
        EXPECT_EQ(rig.platform->realized.front().text, "A");
        EXPECT_EQ(rig.platform->realized.front().reuse_id, "default_cell");
        EXPECT_EQ(rig.platform->realized.front().content, nullptr);
    }

    TEST(collection_view_sim, scrolling_recycles_and_realizes_the_new_window)
    {
        sim const rig;
        rig.handler->simulate_scroll(250);
        // window [250, 650): rows starting 200..600 intersect → items 2..6.
        const std::vector<index_path> expected{{.section = 0, .item = 2},
                                               {.section = 0, .item = 3},
                                               {.section = 0, .item = 4},
                                               {.section = 0, .item = 5},
                                               {.section = 0, .item = 6}};
        EXPECT_EQ(rig.realized_item_paths(), expected);
        const bool recycled_first = std::ranges::any_of(rig.platform->events, [](const cell_event& entry) {
            return entry.kind == cell_event_kind::recycled && entry.path == index_path{.section = 0, .item = 0};
        });
        EXPECT_TRUE(recycled_first);
    }

    TEST(collection_view_sim, template_cells_bind_the_item_as_context)
    {
        sim rig;
        rig.view.set_item_template(make_label_template());
        ASSERT_FALSE(rig.platform->realized.empty());
        for (const realized_cell& cell : rig.platform->realized)
        {
            const auto content = std::dynamic_pointer_cast<label>(cell.content);
            ASSERT_NE(content, nullptr);
            EXPECT_EQ(content->text(), rig.items->at(static_cast<std::size_t>(cell.path.item)));
        }
    }

    // The struct twin of template_cells_bind_the_item_as_context: a collection_view over a
    // observable_collection<photo_item> (a NON-std::string item) with a label template bound to the
    // struct's `title` field must realize N cells whose template-bound content text equals the field.
    // Guards the regression where a custom-struct item rendered blank because the realized cell never
    // resolved its BindingContext against the struct's type (only string/arithmetic items have a
    // built-in text mirror; a struct must bind through the template).
    TEST(collection_view_sim, struct_template_cells_bind_the_struct_field)
    {
        std::shared_ptr<photo_collection> const items = std::make_shared<photo_collection>(
            std::vector<photo_item>{{.title = "Aurora"}, {.title = "Basalt"}, {.title = "Cirrus"}});
        collection_view view;
        auto const handler = std::make_shared<collection_view_handler>();
        view.set_item_template(make_photo_title_template());
        view.set_items_source(items);
        view.set_handler(handler);

        auto* const platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        ASSERT_FALSE(platform->realized.empty());
        for (const realized_cell& cell : platform->realized)
        {
            const auto content = std::dynamic_pointer_cast<label>(cell.content);
            ASSERT_NE(content, nullptr);
            EXPECT_EQ(content->text(), items->at(static_cast<std::size_t>(cell.path.item)).title);
        }
        // The first realized cell binds "Aurora" (proves the struct field, not an empty text mirror).
        EXPECT_EQ(std::dynamic_pointer_cast<label>(platform->realized.front().content)->text(), "Aurora");
    }

    TEST(collection_view_sim, recycling_reuses_pooled_template_content)
    {
        sim rig;
        rig.view.set_item_template(make_label_template());
        const long created_after_attach = rig.realized_event_count();

        rig.handler->simulate_scroll(100); // 4 cells visible again — all servable from the pool
        EXPECT_EQ(rig.realized_event_count(), created_after_attach); // zero fresh creates
        // …and the reused content re-bound to the new items.
        EXPECT_EQ(std::dynamic_pointer_cast<label>(rig.platform->realized.front().content)->text(), "B");
    }

    // ---- EmptyView ----

    TEST(collection_view_sim, empty_view_shows_while_the_source_is_empty)
    {
        sim rig{std::vector<std::string>{}};
        rig.view.set_empty_view(boxed_item::of(std::string{"No data"}));
        EXPECT_TRUE(rig.platform->empty_view.present);
        EXPECT_EQ(rig.platform->empty_view.text, "No data");

        rig.items->add("A"); // items arrived — the empty view retires
        EXPECT_FALSE(rig.platform->empty_view.present);
    }

    TEST(collection_view_sim, empty_view_template_renders_with_the_empty_view_as_context)
    {
        sim rig{std::vector<std::string>{}};
        auto tmpl = data_template::of<label>();
        tmpl->set_binding<std::string, std::string>(label::text_property(),
                                                    [](const std::string& value) { return value; });
        rig.view.set_empty_view(boxed_item::of(std::string{"Nothing here"}));
        rig.view.set_empty_view_template(tmpl);

        ASSERT_TRUE(rig.platform->empty_view.present);
        const auto content = std::dynamic_pointer_cast<label>(rig.platform->empty_view.content);
        ASSERT_NE(content, nullptr);
        EXPECT_EQ(content->text(), "Nothing here");
    }

    TEST(collection_view_sim, empty_view_that_is_a_view_hosts_directly)
    {
        sim rig{std::vector<std::string>{}};
        auto banner = std::make_shared<label>();
        banner->set_text("All clear");
        rig.view.set_empty_view(boxed_item::of(banner));
        ASSERT_TRUE(rig.platform->empty_view.present);
        EXPECT_EQ(rig.platform->empty_view.content, banner);
    }

    TEST(collection_view_sim, null_source_shows_the_empty_view)
    {
        sim rig;
        rig.view.set_empty_view(boxed_item::of(std::string{"Empty"}));
        EXPECT_FALSE(rig.platform->empty_view.present); // items present
        rig.view.clear_items_source();
        EXPECT_TRUE(rig.platform->empty_view.present);
        EXPECT_TRUE(rig.realized_item_paths().empty());
    }

    // ---- Header / Footer ----

    TEST(collection_view_sim, header_and_footer_realize_as_supplementals)
    {
        sim rig;
        rig.view.set_header(boxed_item::of(std::string{"Top"}));
        rig.view.set_footer(boxed_item::of(std::string{"Bottom"}));
        EXPECT_TRUE(rig.platform->header.present);
        EXPECT_EQ(rig.platform->header.text, "Top");
        EXPECT_TRUE(rig.platform->footer.present);
        EXPECT_EQ(rig.platform->footer.text, "Bottom");

        auto tmpl = data_template::of<label>();
        tmpl->set_binding<std::string, std::string>(label::text_property(),
                                                    [](const std::string& value) { return value; });
        rig.view.set_header_template(tmpl);
        const auto content = std::dynamic_pointer_cast<label>(rig.platform->header.content);
        ASSERT_NE(content, nullptr);
        EXPECT_EQ(content->text(), "Top"); // the template's context is the Header object
    }

    // ---- selection ----

    TEST(collection_view_sim, tap_in_single_mode_selects_through_the_view)
    {
        sim rig;
        rig.view.set_selection_mode(selection_mode::single);
        int changes = 0;
        const maui::core::connection_token token = rig.view.selection_changed.connect(
            [&changes](const maui::controls::selection_changed_event_args&) { ++changes; });

        rig.handler->simulate_select({.section = 0, .item = 2});

        EXPECT_EQ(rig.view.selected_item().text(), "C");
        EXPECT_EQ(changes, 1);
        ASSERT_EQ(rig.platform->selected_paths.size(), 1U); // the mapper synced the native mirror
        EXPECT_EQ(rig.platform->selected_paths[0], (index_path{0, 2}));
        rig.view.selection_changed.disconnect(token);
    }

    TEST(collection_view_sim, tap_in_none_mode_is_ignored)
    {
        sim const rig;
        rig.handler->simulate_select({.section = 0, .item = 2});
        EXPECT_FALSE(rig.view.selected_item().has_value());
        EXPECT_TRUE(rig.platform->selected_paths.empty());
    }

    TEST(collection_view_sim, taps_in_multiple_mode_accumulate_and_deselect)
    {
        sim rig;
        rig.view.set_selection_mode(selection_mode::multiple);
        rig.handler->simulate_select({.section = 0, .item = 1});
        rig.handler->simulate_select({.section = 0, .item = 3});
        EXPECT_EQ(rig.view.selected_items().count(), 2U);
        ASSERT_EQ(rig.platform->selected_paths.size(), 2U);

        rig.handler->simulate_deselect({.section = 0, .item = 1});
        EXPECT_EQ(rig.view.selected_items().count(), 1U);
        ASSERT_EQ(rig.platform->selected_paths.size(), 1U);
        EXPECT_EQ(rig.platform->selected_paths[0], (index_path{0, 3}));
    }

    TEST(collection_view_sim, programmatic_selection_syncs_the_native_mirror)
    {
        sim rig;
        rig.view.set_selection_mode(selection_mode::single);
        rig.view.set_selected_item(boxed_item::of(std::string{"E"}));
        ASSERT_EQ(rig.platform->selected_paths.size(), 1U);
        EXPECT_EQ(rig.platform->selected_paths[0], (index_path{0, 4}));

        rig.view.set_selected_item({}); // null clears the native selection
        EXPECT_TRUE(rig.platform->selected_paths.empty());
    }

    TEST(collection_view_sim, selection_mode_drives_the_allows_flags)
    {
        sim rig;
        EXPECT_FALSE(rig.platform->allows_selection);
        rig.view.set_selection_mode(selection_mode::single);
        EXPECT_TRUE(rig.platform->allows_selection);
        EXPECT_FALSE(rig.platform->allows_multiple_selection);
        rig.view.set_selection_mode(selection_mode::multiple);
        EXPECT_TRUE(rig.platform->allows_multiple_selection);
        rig.view.set_selection_mode(selection_mode::none);
        EXPECT_FALSE(rig.platform->allows_selection);
    }

    // ---- Scrolled + the remaining-items threshold (ItemsViewDelegator.Scrolled) ----

    TEST(collection_view_sim, scrolled_reports_offsets_and_visible_indexes)
    {
        sim rig;
        std::vector<maui::controls::items_view_scrolled_event_args> reports;
        const maui::core::connection_token token = rig.view.scrolled.connect(
            [&reports](const maui::controls::items_view_scrolled_event_args& args) { reports.push_back(args); });

        rig.handler->simulate_scroll(600);

        ASSERT_EQ(reports.size(), 1U);
        EXPECT_DOUBLE_EQ(reports[0].vertical_offset, 600);
        EXPECT_DOUBLE_EQ(reports[0].vertical_delta, 600);
        EXPECT_EQ(reports[0].first_visible_item_index, 6);
        EXPECT_EQ(reports[0].last_visible_item_index, 9);
        rig.view.scrolled.disconnect(token);
    }

    TEST(collection_view_sim, threshold_minus_one_never_trips)
    {
        sim rig;
        int reached = 0;
        const maui::core::connection_token token =
            rig.view.remaining_items_threshold_reached.connect([&reached] { ++reached; });
        rig.handler->simulate_scroll(600); // the very end
        EXPECT_EQ(reached, 0);
        rig.view.remaining_items_threshold_reached.disconnect(token);
    }

    TEST(collection_view_sim, threshold_zero_trips_only_at_the_last_item)
    {
        sim rig;
        rig.view.set_remaining_items_threshold(0);
        int reached = 0;
        const maui::core::connection_token token =
            rig.view.remaining_items_threshold_reached.connect([&reached] { ++reached; });

        rig.handler->simulate_scroll(100); // last visible = 4 ≠ 9
        EXPECT_EQ(reached, 0);
        rig.handler->simulate_scroll(600); // last visible = 9
        EXPECT_EQ(reached, 1);
        rig.view.remaining_items_threshold_reached.disconnect(token);
    }

    TEST(collection_view_sim, positive_threshold_trips_within_range)
    {
        sim rig;
        rig.view.set_remaining_items_threshold(3);
        int reached = 0;
        const maui::core::connection_token token =
            rig.view.remaining_items_threshold_reached.connect([&reached] { ++reached; });

        rig.handler->simulate_scroll(200); // last visible = 5 → remaining 4 > 3
        EXPECT_EQ(reached, 0);
        rig.handler->simulate_scroll(300); // last visible = 6 → remaining 3 ≤ 3
        EXPECT_EQ(reached, 1);
        rig.view.remaining_items_threshold_reached.disconnect(token);
    }

    // ---- ItemsUpdatingScrollMode (the ItemsViewLayout choreography) ----

    TEST(collection_view_sim, keep_scroll_offset_leaves_the_offset)
    {
        sim rig;
        rig.view.set_items_updating_scroll_mode(items_updating_scroll_mode::keep_scroll_offset);
        rig.handler->simulate_scroll(200);
        rig.items->add("K");
        EXPECT_DOUBLE_EQ(rig.platform->scroll_offset, 200);
    }

    TEST(collection_view_sim, keep_last_item_in_view_chases_the_tail)
    {
        sim rig;
        rig.view.set_items_updating_scroll_mode(items_updating_scroll_mode::keep_last_item_in_view);
        rig.items->add("K"); // content 1100, viewport 400
        EXPECT_DOUBLE_EQ(rig.platform->scroll_offset, 700);
        EXPECT_EQ(rig.realized_item_paths().back(), (index_path{0, 10}));
    }

    TEST(collection_view_sim, keep_items_in_view_compensates_inserts_before_the_window)
    {
        sim const rig;                     // keep_items_in_view is the default mode
        rig.handler->simulate_scroll(200); // first visible row = item 2
        rig.items->insert(0, "Z");         // lands before the window → everything shifts by one row
        EXPECT_DOUBLE_EQ(rig.platform->scroll_offset, 300);
        EXPECT_EQ(rig.realized_item_paths().front(), (index_path{0, 3})); // the old item 2 stays put
        EXPECT_EQ(rig.platform->realized.front().text, "C");
    }

    TEST(collection_view_sim, keep_items_in_view_ignores_appends_after_the_window)
    {
        sim const rig;
        rig.handler->simulate_scroll(200);
        rig.items->add("K"); // lands after the window → no shift
        EXPECT_DOUBLE_EQ(rig.platform->scroll_offset, 200);
    }

    // ---- the "scroll_to" command (MapScrollTo) ----

    TEST(collection_view_sim, scroll_to_position_start_moves_and_records)
    {
        sim rig;
        rig.view.scroll_to(8, -1, maui::controls::scroll_to_position::start, /*animate=*/false);
        // row 8 starts at 800; clamped to the max offset (content 1000 − viewport 400).
        EXPECT_DOUBLE_EQ(rig.platform->scroll_offset, 600);
        ASSERT_EQ(rig.platform->scroll_requests.size(), 1U);
        EXPECT_EQ(rig.platform->scroll_requests[0].index, 8);
    }

    TEST(collection_view_sim, scroll_to_center_centers_the_row)
    {
        sim rig;
        rig.view.scroll_to(5, -1, maui::controls::scroll_to_position::center);
        EXPECT_DOUBLE_EQ(rig.platform->scroll_offset, 350); // 500 + 50 − 200
    }

    TEST(collection_view_sim, scroll_to_element_resolves_through_the_source)
    {
        sim rig;
        rig.view.scroll_to(boxed_item::of(std::string{"H"})); // index 7, make_visible
        EXPECT_DOUBLE_EQ(rig.platform->scroll_offset, 400);   // 700 + 100 − 400
        EXPECT_EQ(rig.realized_item_paths().back(), (index_path{0, 7}));
    }

    TEST(collection_view_sim, scroll_to_invalid_index_is_ignored)
    {
        sim rig;
        rig.view.scroll_to(42);
        EXPECT_DOUBLE_EQ(rig.platform->scroll_offset, 0);
        EXPECT_TRUE(rig.platform->scroll_requests.empty());
    }

    // ---- items layouts ----

    TEST(collection_view_sim, grid_span_packs_rows_and_reacts_to_span_changes)
    {
        sim rig;
        auto grid = std::make_shared<grid_items_layout>(2, items_layout_orientation::vertical);
        rig.view.set_items_layout(grid);
        // 10 items / span 2 → 5 rows of 200pt total height 500; window 400 → rows 0..3 → 8 cells.
        EXPECT_DOUBLE_EQ(rig.platform->content_extent, 500);
        EXPECT_EQ(rig.realized_item_paths().size(), 8U);
        EXPECT_DOUBLE_EQ(rig.platform->realized[0].start, rig.platform->realized[1].start); // row mates

        grid->set_span(5); // the handler observes the layout's INPC
        EXPECT_EQ(rig.platform->span, 5);
        EXPECT_DOUBLE_EQ(rig.platform->content_extent, 200);
        EXPECT_EQ(rig.realized_item_paths().size(), 10U);
    }

    TEST(collection_view_sim, horizontal_layout_reports_horizontal_offsets)
    {
        sim rig;
        rig.view.set_items_layout(maui::controls::linear_items_layout::create_horizontal_default());
        std::vector<maui::controls::items_view_scrolled_event_args> reports;
        const maui::core::connection_token token = rig.view.scrolled.connect(
            [&reports](const maui::controls::items_view_scrolled_event_args& args) { reports.push_back(args); });
        rig.handler->simulate_scroll(150);
        ASSERT_EQ(reports.size(), 1U);
        EXPECT_DOUBLE_EQ(reports[0].horizontal_offset, 150);
        EXPECT_DOUBLE_EQ(reports[0].vertical_offset, 0);
        rig.view.scrolled.disconnect(token);
    }

    // ---- grouping ----

    TEST(collection_view_sim, grouped_sections_realize_group_headers_and_footers)
    {
        auto fruit = std::make_shared<string_collection>(std::vector<std::string>{"Apple", "Pear"});
        auto veg = std::make_shared<string_collection>(std::vector<std::string>{"Kale"});
        auto groups = std::make_shared<observable_collection<grouping_ptr>>();
        groups->add(std::make_shared<grouping>(boxed_item::of(std::string{"Fruit"}), make_item_collection(fruit)));
        groups->add(std::make_shared<grouping>(boxed_item::of(std::string{"Veg"}), make_item_collection(veg)));

        collection_view view;
        view.set_is_grouped(true);
        view.set_items_source(make_item_collection(groups));
        view.set_group_header_template(data_template::of<label>());
        view.set_group_footer_template(data_template::of<label>());
        auto handler = std::make_shared<collection_view_handler>();
        view.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        handler->simulate_viewport(800, 400); // room for all 7 rows

        ASSERT_EQ(platform->realized.size(), 7U);
        EXPECT_EQ(platform->realized[0].element, cell_element_kind::group_header);
        EXPECT_EQ(platform->realized[0].text, "Fruit"); // bound to the group KEY
        EXPECT_EQ(platform->realized[1].path, (index_path{0, 0}));
        EXPECT_EQ(platform->realized[3].element, cell_element_kind::group_footer);
        EXPECT_EQ(platform->realized[4].text, "Veg");
        EXPECT_EQ(platform->realized[5].path, (index_path{1, 0}));
        EXPECT_EQ(platform->realized[5].text, "Kale");

        // Selection across sections resolves the right path.
        view.set_selection_mode(selection_mode::single);
        handler->simulate_select({.section = 1, .item = 0});
        EXPECT_EQ(view.selected_item().text(), "Kale");

        // A nested change fans out and re-realizes.
        veg->add("Carrot");
        EXPECT_EQ(platform->source_updates.back().kind, source_update_kind::insert_items);
        EXPECT_EQ(platform->source_updates.back().section, 1);
        ASSERT_EQ(platform->realized.size(), 8U);
    }

    // ---- the reorder surface ----

    TEST(collection_view_sim, reorder_completed_is_gated_on_can_reorder_items)
    {
        sim rig;
        int completed = 0;
        const maui::core::connection_token token = rig.view.reorder_completed.connect([&completed] { ++completed; });

        rig.handler->simulate_reorder_completed(); // CanReorderItems is false
        EXPECT_EQ(completed, 0);
        EXPECT_FALSE(rig.platform->can_reorder_items);

        rig.view.set_can_reorder_items(true);
        EXPECT_TRUE(rig.platform->can_reorder_items); // the mapper mirrored it
        rig.handler->simulate_reorder_completed();
        EXPECT_EQ(completed, 1);
        rig.view.reorder_completed.disconnect(token);
    }

    // ---- source replacement ----

    TEST(collection_view_sim, replacing_the_items_source_drops_the_old_subscription)
    {
        sim rig;
        auto replacement = std::make_shared<string_collection>(std::vector<std::string>{"X", "Y"});
        rig.view.set_items_source(replacement);
        EXPECT_EQ(rig.platform->realized.front().text, "X");
        const std::size_t updates_before = rig.platform->source_updates.size();

        rig.items->add("ignored"); // the OLD collection — must not reach the handler
        EXPECT_EQ(rig.platform->source_updates.size(), updates_before);

        replacement->add("Z");
        EXPECT_EQ(rig.platform->source_updates.size(), updates_before + 1);
    }

    TEST(collection_view_sim, source_update_trail_records_the_translated_ops)
    {
        sim const rig;
        rig.items->add("K");
        rig.items->remove_at(0);
        rig.items->clear();
        ASSERT_EQ(rig.platform->source_updates.size(), 3U);
        EXPECT_EQ(rig.platform->source_updates[0].kind, source_update_kind::insert_items);
        EXPECT_EQ(rig.platform->source_updates[1].kind, source_update_kind::delete_items);
        EXPECT_EQ(rig.platform->source_updates[2].kind, source_update_kind::reload_data);
    }
} // namespace
