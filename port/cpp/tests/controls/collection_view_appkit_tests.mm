// macOS (AppKit) backend tests for the collection_view native virtualization (W3-30) — the AppKit
// FlowLayout path over a REAL NSCollectionView + NSCollectionViewFlowLayout +
// NSCollectionViewDataSource/Delegate, driven through the shared collection_view_handler's apple
// #ifdef bridge. These assert the genuine native recycler, not the cross-platform simulator
// (collection_view_tests.cpp covers that on every backend):
//   - data → realized items after a layout pass;
//   - ITEM REUSE under programmatic scroll: the recycler (makeItemWithIdentifier) vends a BOUNDED set
//     of item INSTANCES as the visible window sweeps the whole source (not one fresh item per item);
//   - selection single / multiple through the delegate fan-out + the native selection set;
//   - grouping supplementaries (section header/footer) present in the flow layout;
//   - reorder moves the bound model and re-renders the items, gated on CanReorderItems;
//   - empty-view shown while the source is empty;
//   - grid layout (multiple columns) + horizontal orientation.
//
// The run loop is pumped via tests/support/run_loop_pump.hpp so the flow layout + item realization
// happen deterministically (AppKit does both lazily inside the loop). Compiled as Objective-C++ with
// ARC for the `apple` backend.
// §8: the items collection (publisher) is declared before the view/handler (subscriber).
//
// DOCUMENTED DEVIATION: the iOS compositional-layout path + snap points are iOS-only — these tests
// exercise the NSCollectionViewFlowLayout stack exclusively (see collection_view_handler.mm).

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/service_registry.hpp"
#include "tests/support/run_loop_pump.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::boxed_item;
    using maui::controls::collection_view;
    using maui::controls::collection_view_handler;
    using maui::controls::data_template;
    using maui::controls::grid_items_layout;
    using maui::controls::grouping;
    using maui::controls::grouping_ptr;
    using maui::controls::items_layout_orientation;
    using maui::controls::label;
    using maui::controls::linear_items_layout;
    using maui::controls::make_item_collection;
    using maui::controls::selection_mode;
    using maui::core::observable_collection;
    using maui::tests::pump_run_loop;
    using maui::tests::pump_until;

    using string_collection = observable_collection<std::string>;

    // The native NSScrollView the handler composes into the tree (its documentView is the collection).
    NSScrollView* native_scroll(const std::shared_ptr<collection_view_handler>& handler)
    {
        return (__bridge NSScrollView*)handler->native_view();
    }

    NSCollectionView* native_collection_view(const std::shared_ptr<collection_view_handler>& handler)
    {
        return (NSCollectionView*)native_scroll(handler).documentView;
    }

    // A borderless host window that hosts the scroll view (so AppKit runs the flow-layout pass).
    NSWindow* make_host_window()
    {
        NSWindow* const window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 200, 400)
                                                             styleMask:NSWindowStyleMaskBorderless
                                                               backing:NSBackingStoreBuffered
                                                                 defer:NO];
        return window;
    }

    // A minimal i_maui_context with the label handler registered, so a templated cell can realize the
    // label template's native view (the C# view.ToPlatform(mauiContext) step). Implemented inline (no
    // hosting-library dependency) — only the two accessors the handler uses are real. Owned by the rig
    // so it outlives the handler that references it (§8). label is the only control the templates use.
    struct test_context final : maui::core::i_maui_context
    {
        maui::core::service_registry services_;
        maui::core::handler_registry handlers_;
        test_context()
        {
            handlers_.register_handler<maui::controls::label, maui::core::label_handler>();
        }
        [[nodiscard]] maui::core::service_registry& services() override
        {
            return services_;
        }
        [[nodiscard]] maui::core::handler_registry& handlers() override
        {
            return handlers_;
        }
    };

    // A rig: 30 string items so the visible window is a small fraction of the source (so the recycler
    // has something to recycle). The collection (publisher) is declared FIRST (§8); the context precedes
    // the handler (the handler holds a back-pointer into it).
    struct rig
    {
        std::shared_ptr<string_collection> items; // publisher FIRST (§8)
        test_context ctx;                         // before the handler (back-pointer target)
        collection_view view;
        std::shared_ptr<collection_view_handler> handler = std::make_shared<collection_view_handler>();

        explicit rig(std::vector<std::string> initial = make_alphabet(30))
            : items(std::make_shared<string_collection>(std::move(initial)))
        {
            handler->set_maui_context(&ctx);
            view.set_items_source(items);
            view.set_handler(handler);
        }

        static std::vector<std::string> make_alphabet(int count)
        {
            std::vector<std::string> values;
            values.reserve(static_cast<std::size_t>(count));
            for (int i = 0; i < count; ++i)
            {
                values.push_back("item-" + std::to_string(i));
            }
            return values;
        }

        // Mount the native scroll view in a host window and pump a layout pass so items realize.
        NSWindow* mount(double width = 200, double height = 400) const
        {
            NSWindow* const window = make_host_window();
            NSScrollView* const scroll = native_scroll(handler);
            window.contentView = scroll;
            [window makeKeyAndOrderFront:nil];
            handler->native_force_layout(width, height);
            pump_until([&] { return handler->native_visible_cell_count() > 0; });
            return window;
        }
    };

    // A NON-string custom item type — the selection_mode gallery's photo_item shape. A templated cell
    // over a collection of these binds a label to a struct FIELD (not item.text(), which is empty for a
    // struct), so the native cell must realize the template's content to render anything.
    struct photo_item
    {
        std::string title;
        bool operator==(const photo_item&) const = default;
    };
    using photo_collection = observable_collection<photo_item>;

    // The struct twin of `rig`: a collection_view over observable_collection<photo_item> with a label
    // template bound to the struct's `title`. Shares the test_context (label handler registered).
    struct struct_rig
    {
        std::shared_ptr<photo_collection> items; // publisher FIRST (§8)
        test_context ctx;
        collection_view view;
        std::shared_ptr<collection_view_handler> handler = std::make_shared<collection_view_handler>();

        struct_rig() : items(std::make_shared<photo_collection>(make_photos(30)))
        {
            auto tmpl = data_template::of<label>();
            tmpl->set_binding<std::string, photo_item>(label::text_property(),
                                                       [](const photo_item& item) { return item.title; });
            handler->set_maui_context(&ctx);
            view.set_item_template(tmpl);
            view.set_items_source(items);
            view.set_handler(handler);
        }

        static std::vector<photo_item> make_photos(int count)
        {
            std::vector<photo_item> values;
            values.reserve(static_cast<std::size_t>(count));
            for (int i = 0; i < count; ++i)
            {
                values.push_back({.title = "photo-" + std::to_string(i)});
            }
            return values;
        }

        NSWindow* mount(double width = 200, double height = 400) const
        {
            NSWindow* const window = make_host_window();
            NSScrollView* const scroll = native_scroll(handler);
            window.contentView = scroll;
            [window makeKeyAndOrderFront:nil];
            handler->native_force_layout(width, height);
            pump_until([&] { return handler->native_visible_cell_count() > 0; });
            return window;
        }
    };

    // ---- realization ----

    TEST(collection_view_appkit, native_view_is_an_nscollectionview)
    {
        const rig r;
        EXPECT_TRUE([native_scroll(r.handler) isKindOfClass:[NSScrollView class]]);
        EXPECT_TRUE([native_collection_view(r.handler) isKindOfClass:[NSCollectionView class]]);
    }

    TEST(collection_view_appkit, data_realizes_items_after_layout)
    {
        const rig r;
        NSWindow* const window = r.mount();
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        // The native view sees the whole source as its item count.
        EXPECT_EQ([native_collection_view(r.handler) numberOfItemsInSection:0], 30);
        (void)window;
    }

    // The recycler reuses item INSTANCES: as the window sweeps the full source under programmatic
    // scroll, the count of DISTINCT item pointers ever vended stays small (bounded by the visible
    // window + recycling slack), NOT one-per-item.
    TEST(collection_view_appkit, programmatic_scroll_reuses_item_instances)
    {
        rig r;
        NSWindow* const window = r.mount();
        const int visible = r.handler->native_visible_cell_count();
        ASSERT_GT(visible, 0);

        // Scroll to the very last item and back to the top, pumping each step so items recycle.
        for (const int target : {29, 0, 29, 0})
        {
            r.view.scroll_to(target, -1, maui::controls::scroll_to_position::center, /*animate=*/false);
            pump_run_loop(0.1);
            [native_collection_view(r.handler) layoutSubtreeIfNeeded];
        }
        pump_run_loop(0.1);

        const int distinct = r.handler->native_distinct_cell_instances();
        EXPECT_GT(distinct, 0);
        EXPECT_LT(distinct, 30) << "expected the recycler to reuse item instances, not one per item";
        (void)window;
    }

    TEST(collection_view_appkit, default_items_mirror_the_item_text)
    {
        const rig r;
        NSWindow* const window = r.mount();
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 0}), "item-0");
        (void)window;
    }

    TEST(collection_view_appkit, item_template_binds_and_renders)
    {
        rig r;
        auto tmpl = data_template::of<label>();
        tmpl->set_binding<std::string, std::string>(label::text_property(),
                                                    [](const std::string& value) { return value; });
        r.view.set_item_template(tmpl);
        NSWindow* const window = r.mount();
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 0}), "item-0");
        (void)window;
    }

    // A collection_view over a custom-struct source with a label template bound to a struct FIELD must
    // realize cells whose native content renders the bound field — the regression where struct items
    // rendered blank because the native cell only mirrored item.text() (empty for a non-string struct)
    // and never realized the template's content. The text is read off the realized label's NSTextField.
    TEST(collection_view_appkit, struct_item_template_binds_and_renders)
    {
        struct_rig r;
        NSWindow* const window = r.mount();
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 0}), "photo-0");
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 1}), "photo-1");
        (void)window;
    }

    // ---- selection ----

    TEST(collection_view_appkit, single_selection_through_the_native_click)
    {
        rig r;
        r.view.set_selection_mode(selection_mode::single);
        NSWindow* const window = r.mount();

        r.handler->native_select({.section = 0, .item = 2});
        pump_run_loop(0.05);
        EXPECT_EQ(r.view.selected_item().text(), "item-2");
        EXPECT_EQ(r.handler->native_selected_count(), 1);
        (void)window;
    }

    TEST(collection_view_appkit, multiple_selection_accumulates_natively)
    {
        rig r;
        r.view.set_selection_mode(selection_mode::multiple);
        NSWindow* const window = r.mount();

        r.handler->native_select({.section = 0, .item = 1});
        r.handler->native_select({.section = 0, .item = 3});
        pump_run_loop(0.05);
        EXPECT_EQ(r.view.selected_items().count(), 2U);
        EXPECT_EQ(r.handler->native_selected_count(), 2);

        r.handler->native_deselect({.section = 0, .item = 1});
        pump_run_loop(0.05);
        EXPECT_EQ(r.view.selected_items().count(), 1U);
        EXPECT_EQ(r.handler->native_selected_count(), 1);
        (void)window;
    }

    TEST(collection_view_appkit, programmatic_selection_syncs_the_native_view)
    {
        rig r;
        r.view.set_selection_mode(selection_mode::single);
        NSWindow* const window = r.mount();

        r.view.set_selected_item(boxed_item::of(std::string{"item-4"}));
        pump_run_loop(0.05);
        EXPECT_EQ(r.handler->native_selected_count(), 1);

        r.view.set_selected_item({}); // null clears the native selection
        pump_run_loop(0.05);
        EXPECT_EQ(r.handler->native_selected_count(), 0);
        (void)window;
    }

    // ---- grouping supplementaries ----

    TEST(collection_view_appkit, grouped_sections_show_supplementary_headers_and_footers)
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

        NSWindow* const window = make_host_window();
        window.contentView = native_scroll(handler);
        [window makeKeyAndOrderFront:nil];
        handler->native_force_layout(200, 600);
        pump_until([&] { return handler->native_visible_cell_count() > 0; });

        // Two sections, each with a flow-layout section header + footer supplementary.
        EXPECT_EQ([native_collection_view(handler) numberOfSections], 2);
        EXPECT_GT(handler->native_visible_supplementary_count(/*header=*/true), 0);
        EXPECT_GT(handler->native_visible_supplementary_count(/*header=*/false), 0);
        (void)window;
    }

    // ---- reorder ----

    // A completed reorder moves the bound model (exactly what a native drag's IList mutation does) and
    // re-renders the native items, gated on CanReorderItems.
    TEST(collection_view_appkit, reorder_moves_the_model_and_rerenders)
    {
        rig r{std::vector<std::string>{"A", "B", "C", "D", "E"}};
        int completed = 0;
        const maui::core::connection_token token = r.view.reorder_completed.connect([&completed] { ++completed; });
        NSWindow* const window = r.mount();
        ASSERT_EQ(r.handler->native_cell_text({.section = 0, .item = 0}), "A");

        // Reorder is gated on CanReorderItems: while false, a completion is ignored.
        r.items->move(0, 2); // {A,B,C,D,E} -> {B,C,A,D,E}; the model moves but…
        r.handler->simulate_reorder_completed();
        pump_run_loop(0.05);
        EXPECT_EQ(completed, 0); // …no reorder_completed (CanReorderItems false)

        r.view.set_can_reorder_items(true);
        r.items->move(0, 2); // {B,C,A,D,E} -> {C,A,B,D,E}: "C" is now at index 0
        r.handler->simulate_reorder_completed();
        ASSERT_TRUE(pump_until([&] { return completed == 1; }));
        [native_collection_view(r.handler) layoutSubtreeIfNeeded];
        pump_run_loop(0.05);

        // The model reordered and the native items re-rendered in the new order.
        EXPECT_EQ(r.items->at(0), "C");
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 0}), "C");
        r.view.reorder_completed.disconnect(token);
        (void)window;
    }

    // ---- empty view ----

    TEST(collection_view_appkit, empty_view_shows_while_the_source_is_empty)
    {
        rig r{std::vector<std::string>{}};
        r.view.set_empty_view(boxed_item::of(std::string{"No data"}));
        NSCollectionView* const collection_view = native_collection_view(r.handler);

        NSWindow* const window = make_host_window();
        window.contentView = native_scroll(r.handler);
        [window makeKeyAndOrderFront:nil];
        r.handler->native_force_layout(200, 400);
        pump_run_loop(0.1);

        // The empty-view host (a marker subview) is shown while the source is empty.
        EXPECT_TRUE(r.handler->native_empty_view_shown());
        EXPECT_EQ([collection_view numberOfItemsInSection:0], 0);

        // Items arrive → the empty view retires.
        r.items->add("Now there is data");
        pump_run_loop(0.1);
        EXPECT_FALSE(r.handler->native_empty_view_shown());
        (void)window;
    }

    // ---- grid + orientation ----

    TEST(collection_view_appkit, grid_layout_lays_out_multiple_columns)
    {
        rig r;
        r.view.set_items_layout(std::make_shared<grid_items_layout>(2, items_layout_orientation::vertical));
        NSWindow* const window = r.mount();
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        EXPECT_EQ([native_collection_view(r.handler) numberOfItemsInSection:0], 30);
        // A 2-column grid: the flow layout's scrollDirection stays vertical.
        auto* layout = (NSCollectionViewFlowLayout*)native_collection_view(r.handler).collectionViewLayout;
        EXPECT_EQ(layout.scrollDirection, NSCollectionViewScrollDirectionVertical);
        (void)window;
    }

    TEST(collection_view_appkit, horizontal_orientation_sets_scroll_direction)
    {
        rig r;
        r.view.set_items_layout(std::make_shared<linear_items_layout>(items_layout_orientation::horizontal));
        NSWindow* const window = r.mount();
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        auto* layout = (NSCollectionViewFlowLayout*)native_collection_view(r.handler).collectionViewLayout;
        EXPECT_EQ(layout.scrollDirection, NSCollectionViewScrollDirectionHorizontal);
        (void)window;
    }
} // namespace
