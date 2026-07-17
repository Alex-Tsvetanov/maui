// iOS (UIKit, on-simulator) backend tests for the collection_view native virtualization (W3-29) — the
// Items2 COMPOSITIONAL path over a REAL UICollectionView + UICollectionViewController, driven through
// the shared collection_view_handler's ios #ifdef bridge. These assert the genuine native recycler, not
// the cross-platform simulator (collection_view_tests.cpp covers that on every backend):
//   - data → realized cells after a layout pass;
//   - CELL REUSE under programmatic scroll: the recycler vends a BOUNDED set of cell INSTANCES as the
//     visible window sweeps the whole source (not one fresh cell per item);
//   - selection single / multiple through the delegate fan-out + the native selected set;
//   - grouping supplementaries (group header/footer) present in the compositional layout;
//   - reorder moves the bound model and re-renders the cells, gated on CanReorderItems;
//   - scroll-to moves the native content offset;
//   - empty-view shown while the source is empty.
//
// The run loop is pumped via tests/support/run_loop_pump.hpp so the compositional layout + cell
// realization happen deterministically (UIKit does both lazily inside the loop). Compiled as
// Objective-C++ with ARC for the `ios` backend; run ON the booted simulator via tools/ios-sim-run.sh.
// §8: the items collection (publisher) is declared before the view/handler (subscriber).
//
// DOCUMENTED DEVIATION: the classic-Items iOS flow-layout path is not ported (Items2 compositional
// only) — these tests exercise the compositional stack exclusively.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/service_registry.hpp"
#include "maui/graphics/rect.hpp"
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
    using maui::controls::make_item_collection;
    using maui::controls::selection_mode;
    using maui::core::observable_collection;
    using maui::tests::pump_run_loop;
    using maui::tests::pump_until;

    using string_collection = observable_collection<std::string>;

    // The native UICollectionView the handler composes into the tree (the controller's collectionView).
    UICollectionView* native_collection_view(const std::shared_ptr<collection_view_handler>& handler)
    {
        return (__bridge UICollectionView*)handler->native_view();
    }

    // A key+visible host window for the collection view (so UIKit runs the compositional layout pass).
    // [[UIWindow alloc] init] adopts a placeholder scene in the spawned test process (the same
    // initializer window_handler.mm uses); the deprecation pragma matches that established precedent.
    UIWindow* make_host_window()
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        UIWindow* const window = [[UIWindow alloc] init]; // SDK-deprecated; see window_handler.mm precedent
#pragma clang diagnostic pop
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

        // Build N distinct string items ("item-0", "item-1", …).
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

        // Mount the native collection view in a host window and pump a layout pass so cells realize.
        // A window-hosted collection view is what makes UIKit actually run the compositional layout.
        UIWindow* mount(double width = 200, double height = 400) const
        {
            UIWindow* const window = make_host_window();
            UICollectionView* const collection_view = native_collection_view(handler);
            [window addSubview:collection_view];
            [window makeKeyAndVisible];
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

        UIWindow* mount(double width = 200, double height = 400) const
        {
            UIWindow* const window = make_host_window();
            UICollectionView* const collection_view = native_collection_view(handler);
            [window addSubview:collection_view];
            [window makeKeyAndVisible];
            handler->native_force_layout(width, height);
            pump_until([&] { return handler->native_visible_cell_count() > 0; });
            return window;
        }
    };

    // ---- realization ----

    TEST(collection_view_ios, native_view_is_a_uicollectionview)
    {
        const rig r;
        EXPECT_TRUE([native_collection_view(r.handler) isKindOfClass:[UICollectionView class]]);
    }

    // C# ItemsViewController2.ViewDidLoad (:163-183) — on iOS 11+ AND Mac Catalyst 11+ (the `else` of the
    // negated version check) MAUI sets:
    //     CollectionView.ContentInsetAdjustmentBehavior = UIScrollViewContentInsetAdjustmentBehavior.Never;
    // "We set this property to keep iOS from trying to be helpful about insetting all the CollectionView
    // content … The SetUseSafeArea Platform Specific is already taking care of this for us."
    //
    // The port never set it, so UIKit's DEFAULT (.automatic) applied and insets the content below the safe
    // area. Measured on the first iOS pixel board: every full-page CollectionView page ran a ~status-bar
    // height low against MAUI — MAUI's group header sits at y=0 under the status bar with the view-level
    // Header scrolled out of sight, the port's sat below the bar. Identical content, 8 pages red
    // (grid_grouping 47%, grouping_plus_selection 47%, basic_grouping 45%, header_footer_template 35%, …).
    // iOS ONLY: C# sets .Never for Catalyst too, but MAUI's Catalyst render shows the content BELOW the
    // titlebar, which the port's full-bounds CV frame reproduces only with .automatic (forcing .Never there
    // measured red 44% vs green 0.12%). See the handler's comment + PARITY_REVIEW.md item 3.
    TEST(collection_view_ios, content_inset_adjustment_behavior_is_never)
    {
        const rig r;
#if TARGET_OS_MACCATALYST
        EXPECT_EQ(native_collection_view(r.handler).contentInsetAdjustmentBehavior,
                  UIScrollViewContentInsetAdjustmentAutomatic);
#else
        EXPECT_EQ(native_collection_view(r.handler).contentInsetAdjustmentBehavior,
                  UIScrollViewContentInsetAdjustmentNever);
#endif
    }

    TEST(collection_view_ios, data_realizes_cells_after_layout)
    {
        const rig r;
        UIWindow* const window = r.mount();
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        // The native view sees the whole source as its item count.
        EXPECT_EQ([native_collection_view(r.handler) numberOfItemsInSection:0], 30);
        (void)window;
    }

    // The recycler reuses cell INSTANCES: as the window sweeps the full source under programmatic
    // scroll, the count of DISTINCT cell pointers ever vended stays small (bounded by the visible
    // window + a recycling slack), NOT one-per-item.
    TEST(collection_view_ios, programmatic_scroll_reuses_cell_instances)
    {
        rig r;
        UIWindow* const window = r.mount();
        const int visible = r.handler->native_visible_cell_count();
        ASSERT_GT(visible, 0);

        // Scroll to the very last item and back to the top, pumping each step so cells recycle.
        for (const int target : {29, 0, 29, 0})
        {
            r.view.scroll_to(target, -1, maui::controls::scroll_to_position::center, /*animate=*/false);
            pump_run_loop(0.1);
            [native_collection_view(r.handler) layoutIfNeeded];
        }
        pump_run_loop(0.1);

        // The recycler should have vended far fewer distinct cell instances than the 30 items — a small
        // multiple of the visible window proves instances are reused, not grown unbounded.
        const int distinct = r.handler->native_distinct_cell_instances();
        EXPECT_GT(distinct, 0);
        EXPECT_LT(distinct, 30) << "expected the recycler to reuse cell instances, not one per item";
        (void)window;
    }

    TEST(collection_view_ios, default_cells_mirror_the_item_text)
    {
        const rig r;
        UIWindow* const window = r.mount();
        // The first realized cell mirrors item-0's text (the DefaultCell2 label).
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 0}), "item-0");
        (void)window;
    }

    TEST(collection_view_ios, item_template_binds_and_renders)
    {
        rig r;
        auto tmpl = data_template::of<label>();
        tmpl->set_binding<std::string, std::string>(label::text_property(),
                                                    [](const std::string& value) { return value; });
        r.view.set_item_template(tmpl);
        UIWindow* const window = r.mount();
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 0}), "item-0");
        (void)window;
    }

    // A collection_view over a custom-struct source with a label template bound to a struct FIELD must
    // realize cells whose native content renders the bound field — the regression where struct items
    // rendered blank because the native cell only mirrored item.text() (empty for a non-string struct)
    // and never realized the template's content. The text is read off the realized label's UILabel.
    TEST(collection_view_ios, struct_item_template_binds_and_renders)
    {
        struct_rig r;
        UIWindow* const window = r.mount();
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 0}), "photo-0");
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 1}), "photo-1");
        (void)window;
    }

    // ---- selection ----

    TEST(collection_view_ios, single_selection_through_the_native_tap)
    {
        rig r;
        r.view.set_selection_mode(selection_mode::single);
        UIWindow* const window = r.mount();

        r.handler->native_select({.section = 0, .item = 2});
        pump_run_loop(0.05);
        EXPECT_EQ(r.view.selected_item().text(), "item-2");
        EXPECT_EQ(r.handler->native_selected_count(), 1);
        (void)window;
    }

    TEST(collection_view_ios, multiple_selection_accumulates_natively)
    {
        rig r;
        r.view.set_selection_mode(selection_mode::multiple);
        UIWindow* const window = r.mount();

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

    TEST(collection_view_ios, programmatic_selection_syncs_the_native_view)
    {
        rig r;
        r.view.set_selection_mode(selection_mode::single);
        UIWindow* const window = r.mount();

        r.view.set_selected_item(boxed_item::of(std::string{"item-4"}));
        pump_run_loop(0.05);
        EXPECT_EQ(r.handler->native_selected_count(), 1);

        r.view.set_selected_item({}); // null clears the native selection
        pump_run_loop(0.05);
        EXPECT_EQ(r.handler->native_selected_count(), 0);
        (void)window;
    }

    // ---- flat (non-grouped) CV header/footer ----

    // The plain Items page: a non-grouped CollectionView with a view-level Header + Footer renders them
    // as the global boundary supplementaries (the C# LayoutHeaderFooterInfo on the layout configuration),
    // bound to their boxed strings. This guards the fix's move of the CV header/footer onto the global
    // config (away from the section) — the flat path must keep working.
    TEST(collection_view_ios, flat_cv_header_footer_render_as_global_supplementaries)
    {
        // Few items + a tall window so BOTH the global header and footer sit on-screen at once.
        rig r{rig::make_alphabet(3)};
        r.view.set_header(boxed_item::of(std::string{"Today"}));
        r.view.set_footer(boxed_item::of(std::string{"Pick a task"}));
        UIWindow* const window = r.mount(200, 600);
        [native_collection_view(r.handler) layoutIfNeeded];

        EXPECT_EQ([native_collection_view(r.handler) numberOfSections], 1);
        EXPECT_GT(r.handler->native_visible_supplementary_count(/*header=*/true), 0);
        EXPECT_GT(r.handler->native_visible_supplementary_count(/*header=*/false), 0);
        // The global (CV-level) header/footer mirror their boxed strings (section < 0 reads the global).
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/-1, /*header=*/true), "Today");
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/-1, /*header=*/false), "Pick a task");
        (void)window;
    }

    // ---- grouping supplementaries ----

    TEST(collection_view_ios, grouped_sections_show_supplementary_headers_and_footers)
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

        UIWindow* const window = make_host_window();
        [window addSubview:native_collection_view(handler)];
        [window makeKeyAndVisible];
        handler->native_force_layout(200, 600);
        pump_until([&] { return handler->native_visible_cell_count() > 0; });

        // Two sections, each with a boundary supplementary header + footer in the compositional layout.
        EXPECT_EQ([native_collection_view(handler) numberOfSections], 2);
        EXPECT_GT(handler->native_visible_supplementary_count(/*header=*/true), 0);
        EXPECT_GT(handler->native_visible_supplementary_count(/*header=*/false), 0);
        (void)window;
    }

    // The reflection-free Team KEY of the BasicGrouping gallery page: Name (group header) + Count (group
    // footer). The group header/footer templates bind against this struct, not a string — so the group
    // supplementaries must REALIZE their template, not just mirror the key's (empty) text.
    struct team_key
    {
        std::string name;
        int count = 0;
        bool operator==(const team_key&) const = default;
    };

    // Build the gallery's grouped CollectionView: a CV-level Header + Footer, a green group-header
    // template bound to team_key.name, an orange group-footer template bound to team_key.count, over a
    // struct-keyed grouped source. `ctx` outlives the handler (§8 — caller owns both).
    template <class Rig> void wire_basic_grouping(Rig& r)
    {
        auto avengers =
            std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Thor", "Iron Man"});
        auto ff = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"The Thing"});
        r.groups = std::make_shared<observable_collection<grouping_ptr>>();
        r.groups->add(
            std::make_shared<grouping>(boxed_item::of(team_key{"Avengers", 2}), make_item_collection(avengers)));
        r.groups->add(
            std::make_shared<grouping>(boxed_item::of(team_key{"Fantastic Four", 1}), make_item_collection(ff)));

        auto group_header = data_template::of<label>();
        group_header->set_binding<std::string, team_key>(label::text_property(),
                                                         [](const team_key& key) { return key.name; });
        auto group_footer = data_template::of<label>();
        group_footer->set_binding<std::string, team_key>(
            label::text_property(), [](const team_key& key) { return "Total members: " + std::to_string(key.count); });

        r.handler->set_maui_context(&r.ctx);
        r.view.set_group_header_template(group_header);
        r.view.set_group_footer_template(group_footer);
        r.view.set_is_grouped(true);
        r.view.set_header(boxed_item::of(std::string{"This is a header"}));
        r.view.set_footer(boxed_item::of(std::string{"Hey, a footer."}));
        r.view.set_items_source(make_item_collection(r.groups));
        r.view.set_handler(r.handler);
    }

    // A grouped CollectionView with per-group header/footer TEMPLATES (bound to a struct group key) AND a
    // CV-level Header/Footer renders ALL of its supplementaries: a CV header + footer (one each, global),
    // and a green group-header + orange group-footer per section, bound to their group key. This is the
    // BasicGrouping gallery: earlier the grouped path dropped the CV header/footer entirely and bound the
    // group supplementaries only to the key's (empty, for a struct) text, so nothing displayed.
    TEST(collection_view_ios, grouped_supplementaries_bind_group_templates_and_cv_header_footer)
    {
        struct local_rig
        {
            std::shared_ptr<observable_collection<grouping_ptr>> groups; // publisher FIRST (§8)
            test_context ctx;                                            // before the handler (back-pointer)
            collection_view view;
            std::shared_ptr<collection_view_handler> handler = std::make_shared<collection_view_handler>();
        } r;
        wire_basic_grouping(r);

        UIWindow* const window = make_host_window();
        [window addSubview:native_collection_view(r.handler)];
        [window makeKeyAndVisible];
        r.handler->native_force_layout(200, 600);
        pump_until([&] { return r.handler->native_visible_cell_count() > 0; });

        // Two groups → two sections, plus the CV-level header/footer.
        EXPECT_EQ([native_collection_view(r.handler) numberOfSections], 2);

        // A supplementary header AND footer are visible (regression guard: the bug rendered zero).
        EXPECT_GT(r.handler->native_visible_supplementary_count(/*header=*/true), 0);
        EXPECT_GT(r.handler->native_visible_supplementary_count(/*header=*/false), 0);

        // The per-group header binds the green Label to the group key's name; the footer binds the orange
        // Label to "Total members: N". (Section 0 = Avengers/2, section 1 = Fantastic Four/1.)
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/0, /*header=*/true), "Avengers");
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/0, /*header=*/false), "Total members: 2");
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/1, /*header=*/true), "Fantastic Four");
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/1, /*header=*/false), "Total members: 1");

        // The CV-level (global) Header + Footer render their boxed strings (section < 0 reads the global).
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/-1, /*header=*/true), "This is a header");
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/-1, /*header=*/false), "Hey, a footer.");
        (void)window;
    }

    // …and the GRID twin of the test above renders NO view-level Header/Footer — MAUI's LayoutFactory2
    // treats the two layouts asymmetrically:
    //   * CreateListLayout (:89-94) sets `layoutConfiguration.BoundarySupplementaryItems =
    //     CreateSupplementaryItems(null, layoutHeaderFooterInfo, …)` — "//create global header and footer"
    //     — and passes NULL headerFooterInfo at section level, so a grouped LIST shows both.
    //   * CreateGridLayout (:153-198) NEVER touches layoutConfiguration.BoundarySupplementaryItems. Its
    //     only call is section-level and passes the groupingInfo, and CreateSupplementaryItems (:30-55)
    //     EARLY-RETURNS on `groupingInfo.IsGrouped` having added just the group header/footer — so the
    //     LayoutHeaderFooterInfo is never consulted and the view-level Header/Footer is dropped.
    // Confirmed against real MAUI on BOTH iOS and Mac Catalyst: grid_grouping's authored
    // Header="This is a header" appears on neither (the port rendered it, putting the whole page 16px
    // low). A non-grouped grid is unaffected — there IsGrouped is false, so the same call falls through
    // to the LayoutHeaderFooterInfo branch and the header renders as a section item.
    TEST(collection_view_ios, grouped_grid_drops_the_cv_header_and_footer)
    {
        struct local_rig
        {
            std::shared_ptr<observable_collection<grouping_ptr>> groups; // publisher FIRST (§8)
            test_context ctx;                                            // before the handler (back-pointer)
            collection_view view;
            std::shared_ptr<collection_view_handler> handler = std::make_shared<collection_view_handler>();
        } r;
        wire_basic_grouping(r); // grouped, CV Header + Footer, group templates — as the list test above
        // …but a GRID items layout (the grid_grouping page's `ItemsLayout="VerticalGrid, 2"`).
        r.view.set_items_layout(std::make_shared<grid_items_layout>(2, items_layout_orientation::vertical));

        UIWindow* const window = make_host_window();
        [window addSubview:native_collection_view(r.handler)];
        [window makeKeyAndVisible];
        r.handler->native_force_layout(200, 600);
        pump_until([&] { return r.handler->native_visible_cell_count() > 0; });

        EXPECT_EQ([native_collection_view(r.handler) numberOfSections], 2);

        // The per-group supplementaries still render — only the VIEW-level pair is dropped.
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/0, /*header=*/true), "Avengers");
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/0, /*header=*/false), "Total members: 2");

        // No global (CV-level) header/footer exists at all (section < 0 reads the global; absent -> "").
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/-1, /*header=*/true), "");
        EXPECT_EQ(r.handler->native_supplementary_text(/*section=*/-1, /*header=*/false), "");
        (void)window;
    }

    // ---- reorder ----

    // A completed reorder moves the bound model (exactly what a native drag's IList mutation does) and
    // re-renders the native cells, gated on CanReorderItems.
    TEST(collection_view_ios, reorder_moves_the_model_and_rerenders)
    {
        rig r{std::vector<std::string>{"A", "B", "C", "D", "E"}};
        int completed = 0;
        const maui::core::connection_token token = r.view.reorder_completed.connect([&completed] { ++completed; });
        UIWindow* const window = r.mount();
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
        [native_collection_view(r.handler) layoutIfNeeded];
        pump_run_loop(0.05);

        // The model reordered and the native cells re-rendered in the new order.
        EXPECT_EQ(r.items->at(0), "C");
        EXPECT_EQ(r.handler->native_cell_text({.section = 0, .item = 0}), "C");
        r.view.reorder_completed.disconnect(token);
        (void)window;
    }

    // ---- scroll-to ----

    TEST(collection_view_ios, scroll_to_moves_the_native_content_offset)
    {
        rig r;
        UIWindow* const window = r.mount();
        UICollectionView* const collection_view = native_collection_view(r.handler);
        const CGFloat before = collection_view.contentOffset.y;

        r.view.scroll_to(29, -1, maui::controls::scroll_to_position::end, /*animate=*/false);
        pump_run_loop(0.1);
        [collection_view layoutIfNeeded];

        EXPECT_GT(collection_view.contentOffset.y, before);
        (void)window;
    }

    // ---- empty view ----

    TEST(collection_view_ios, empty_view_shows_while_the_source_is_empty)
    {
        rig r{std::vector<std::string>{}};
        r.view.set_empty_view(boxed_item::of(std::string{"No data"}));
        UICollectionView* const collection_view = native_collection_view(r.handler);

        UIWindow* const window = make_host_window();
        [window addSubview:collection_view];
        [window makeKeyAndVisible];
        r.handler->native_force_layout(200, 400);
        pump_run_loop(0.1);

        // The empty-view host (tag 333) is a subview of the collection view while empty.
        EXPECT_NE([collection_view viewWithTag:333], nil);
        EXPECT_EQ([collection_view numberOfItemsInSection:0], 0);

        // Items arrive → the empty view retires.
        r.items->add("Now there is data");
        pump_run_loop(0.1);
        EXPECT_EQ([collection_view viewWithTag:333], nil);
        (void)window;
    }

    // ---- grid layout ----

    TEST(collection_view_ios, grid_layout_lays_out_multiple_columns)
    {
        rig r;
        r.view.set_items_layout(std::make_shared<grid_items_layout>(2, items_layout_orientation::vertical));
        UIWindow* const window = r.mount();
        // A 2-column grid still realizes a window of cells; the section reports the full item count.
        EXPECT_GT(r.handler->native_visible_cell_count(), 0);
        EXPECT_EQ([native_collection_view(r.handler) numberOfItemsInSection:0], 30);
        (void)window;
    }

    // ---- platform_arrange frames the native view (the embedded-stack overlap fix) ----

    // The real layout seam is view::arrange → handler::platform_arrange. Every other handler frames its
    // native view there; collection_view_handler must too, or an embedded CollectionView keeps its
    // creation-time native frame (a UICollectionViewController vends a FULL-SCREEN collectionView) and
    // paints over its stack siblings. After arranging to a bounded sub-rect, the native frame must equal
    // that rect — NOT the screen.
    TEST(collection_view_ios, arrange_frames_the_native_view_to_the_bounded_rect)
    {
        rig r;
        UIWindow* const window = r.mount();
        UICollectionView* const collection_view = native_collection_view(r.handler);

        // Arrange the view into a bounded slot well inside the screen — the slot a vertical stack would
        // hand an embedded CollectionView below its labels.
        auto& as_view = static_cast<maui::core::i_view&>(r.view);
        const maui::graphics::rect slot{0, 120, 200, 220};
        as_view.arrange(slot);
        pump_run_loop(0.1);

        const CGRect native = collection_view.frame;
        EXPECT_DOUBLE_EQ(native.origin.x, slot.x);
        EXPECT_DOUBLE_EQ(native.origin.y, slot.y);
        EXPECT_DOUBLE_EQ(native.size.width, slot.width);
        EXPECT_DOUBLE_EQ(native.size.height, slot.height);
        (void)window;
    }

    // The full-screen autoresizing mask the controller vends would re-stretch the collection view to the
    // panel on the next UIKit layout pass (re-introducing the overlap). arrange_native clears it so MAUI's
    // arrange owns the frame.
    TEST(collection_view_ios, arrange_clears_the_autoresizing_mask_so_uikit_does_not_re_stretch)
    {
        rig r;
        UIWindow* const window = r.mount();
        UICollectionView* const collection_view = native_collection_view(r.handler);

        auto& as_view = static_cast<maui::core::i_view&>(r.view);
        as_view.arrange(maui::graphics::rect{0, 120, 200, 220});
        pump_run_loop(0.1);

        EXPECT_EQ(collection_view.autoresizingMask, UIViewAutoresizingNone);
        (void)window;
    }
} // namespace
