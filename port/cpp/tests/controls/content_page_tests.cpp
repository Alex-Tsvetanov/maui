// Tests for the content_page control + its headless handler seam — a page hosting a single content
// child within a padding (+ a title). Two things are verified: (1) the control's measure/arrange port
// LayoutExtensions.MeasureContent/ArrangeContent (the content is sized + placed within the padding), and
// (2) the headless content_page_platform's single-content mirror tracks the control's content as it is
// set/replaced/cleared so the native host stays in sync.
#include "maui/controls/content_page.hpp"

#include <memory>

#include "maui/controls/platform_configuration/ios_specific/page.hpp" // U20: use_safe_area / safe_area knobs
#include "maui/core/content_page_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_content_view.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_padding.hpp"
#include "maui/core/i_safe_area_view.hpp"  // U20: the GetSafeAreaRegionsForEdge contract
#include "maui/core/safe_area_edges.hpp"   // U20: the per-edge SafeAreaEdges value
#include "maui/core/safe_area_regions.hpp" // U20
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "tests/layouts/layout_test_helpers.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::core::content_page_handler;
    using maui::core::i_content_view;
    using maui::core::i_element_handler;
    using maui::core::i_padding;
    using maui::core::i_safe_area_view2;
    using maui::core::safe_area_edges;
    using maui::core::safe_area_regions;
    using maui::core::thickness;
    using maui::graphics::rect;
    using maui::graphics::size;
    using maui::layouts::testing::mock_view;
    namespace ios_page = maui::controls::platform_configuration::ios_specific::page;

    // ---- the control in isolation (no handler) ----

    TEST(content_page, defaults_empty_with_no_content_zero_padding_empty_title)
    {
        content_page page;
        EXPECT_EQ(page.content(), nullptr);
        EXPECT_EQ(page.padding(), thickness());
        EXPECT_EQ(page.title(), "");
    }

    TEST(content_page, content_is_settable_and_clearable)
    {
        content_page page;
        mock_view child;
        page.set_content(child);
        EXPECT_EQ(page.content(), &child);

        page.set_content(nullptr);
        EXPECT_EQ(page.content(), nullptr);
    }

    TEST(content_page, title_and_padding_are_settable)
    {
        content_page page;
        page.set_title("Home");
        EXPECT_EQ(page.title(), "Home");

        page.set_padding(thickness(5));
        EXPECT_EQ(page.padding(), thickness(5));
    }

    TEST(content_page, usable_through_interface_references)
    {
        content_page page;
        mock_view child;
        page.set_content(child);
        page.set_padding(thickness(8));

        i_content_view& as_content = page;
        i_padding& as_padding = page;
        EXPECT_EQ(as_content.content(), &child);
        EXPECT_EQ(as_content.padding(), thickness(8));
        EXPECT_EQ(as_padding.padding(), thickness(8));
    }

    // ---- measure/arrange: MeasureContent / ArrangeContent within the padding ----

    TEST(content_page, measure_sizes_content_plus_padding)
    {
        content_page page;
        page.set_padding(thickness(10));
        mock_view child;
        child.configure({100, 40});
        page.set_content(child);

        // content 100x40 + padding {10} on all sides -> 120x60.
        const size measured = page.measure(1000, 1000);
        EXPECT_EQ(measured.width, 120.0);
        EXPECT_EQ(measured.height, 60.0);

        // The content was measured with the padding subtracted from the constraints.
        EXPECT_EQ(child.last_measure_width, 980.0);  // 1000 - (10 + 10)
        EXPECT_EQ(child.last_measure_height, 980.0); // 1000 - (10 + 10)
    }

    TEST(content_page, measure_with_no_content_is_padding_only)
    {
        content_page page;
        page.set_padding(thickness(10));

        const size measured = page.measure(1000, 1000);
        EXPECT_EQ(measured.width, 20.0);  // padding horizontal only
        EXPECT_EQ(measured.height, 20.0); // padding vertical only
    }

    TEST(content_page, arrange_places_content_within_padding)
    {
        content_page page;
        page.set_padding(thickness(10));
        mock_view child;
        child.configure({100, 40});
        page.set_content(child);

        page.measure(1000, 1000);
        page.arrange(rect(0, 0, 120, 60));

        // Content arranged at (left+padding, top+padding) with bounds shrunk by the padding.
        EXPECT_EQ(child.last_arrange, rect(10, 10, 100, 40)); // 120-20 wide, 60-20 tall
    }

    TEST(content_page, arrange_with_no_content_does_not_crash)
    {
        content_page page;
        page.set_padding(thickness(10));
        const size arranged = page.arrange(rect(0, 0, 50, 50));
        EXPECT_EQ(arranged.width, 50.0);
        EXPECT_EQ(arranged.height, 50.0);
    }

    // ---- the handler seam (control <-> handler <-> headless host): the host mirrors the content ----

    TEST(content_page_seam, attaching_handler_creates_host_and_mirrors_initial_content)
    {
        content_page page;
        mock_view child;
        page.set_content(child); // set before the handler is attached

        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &page);
        // The full property/command run on connect re-hosts the already-set content.
        EXPECT_EQ(handler->typed_platform_view()->hosted_content, &child);
    }

    TEST(content_page_seam, setting_content_after_attach_rehosts)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_EQ(platform->hosted_content, nullptr);

        mock_view first;
        page.set_content(first); // -> handler->invoke("set_content") -> map_set_content -> set_content()
        EXPECT_EQ(platform->hosted_content, &first);

        mock_view second;
        page.set_content(second); // replacing the content re-hosts the new child
        EXPECT_EQ(platform->hosted_content, &second);

        page.set_content(nullptr); // clearing the content empties the host
        EXPECT_EQ(platform->hosted_content, nullptr);
    }

    TEST(content_page_seam, handler_resolved_from_default_registry)
    {
        // content_page -> content_page_handler is self-registered (MAUI_REGISTER_HANDLER).
        const std::shared_ptr<i_element_handler> handler =
            maui::core::default_handler_registry().create_handler<content_page>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<content_page_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        content_page page;
        mock_view child;
        page.set_content(child);
        page.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->hosted_content, &child);
    }

    // ---- U20: per-control SafeAreaEdges bindable + GetSafeAreaRegionsForEdge + per-edge inset ----
    // Derived from ContentPage.cs (SafeAreaEdges / ISafeAreaView2.GetSafeAreaRegionsForEdge /
    // SafeAreaEdgesDefaultValueCreator) + MauiView.GetSafeAreaForEdge (the per-edge selective inset).

    // The per-element default-value creator gives every page SafeAreaEdges.None (edge-to-edge).
    TEST(content_page_safe_area, safe_area_edges_defaults_to_none)
    {
        content_page page;
        EXPECT_EQ(page.safe_area_edges(), safe_area_edges::none());
    }

    // Setting SafeAreaEdges flows through the bindable system (the descriptor + property<T>).
    TEST(content_page_safe_area, safe_area_edges_is_settable_through_the_bindable)
    {
        content_page page;
        const safe_area_edges all{safe_area_regions::all};
        page.set_safe_area_edges(all);
        EXPECT_EQ(page.safe_area_edges(), all);
    }

    // When SafeAreaEdges is explicitly set, GetSafeAreaRegionsForEdge returns that edge's region —
    // PER-EDGE: "None,All,None,All" → left+right None, top+bottom All (0=Left,1=Top,2=Right,3=Bottom).
    TEST(content_page_safe_area, get_regions_for_edge_uses_the_explicit_property_per_edge)
    {
        content_page page;
        page.set_safe_area_edges(safe_area_edges{safe_area_regions::none, safe_area_regions::all,
                                                 safe_area_regions::none, safe_area_regions::all});

        i_safe_area_view2& face = page;
        EXPECT_EQ(face.get_safe_area_regions_for_edge(0), safe_area_regions::none); // left
        EXPECT_EQ(face.get_safe_area_regions_for_edge(1), safe_area_regions::all);  // top
        EXPECT_EQ(face.get_safe_area_regions_for_edge(2), safe_area_regions::none); // right
        EXPECT_EQ(face.get_safe_area_regions_for_edge(3), safe_area_regions::all);  // bottom
    }

    // Unset SafeAreaEdges falls back to the legacy IgnoreSafeArea/UseSafeArea boolean:
    // !UseSafeArea → None (edge-to-edge); UseSafeArea → Container.
    TEST(content_page_safe_area, get_regions_for_edge_falls_back_to_legacy_use_safe_area)
    {
        content_page page;
        i_safe_area_view2& face = page;

        for (int edge = 0; edge < 4; ++edge)
        {
            EXPECT_EQ(face.get_safe_area_regions_for_edge(edge), safe_area_regions::none); // legacy: ignore
        }

        ios_page::set_use_safe_area(page, true);
        for (int edge = 0; edge < 4; ++edge)
        {
            EXPECT_EQ(face.get_safe_area_regions_for_edge(edge), safe_area_regions::container); // legacy: obey
        }
    }

    // The per-edge layout inset (MauiView.GetSafeAreaForEdge): an edge whose region != None gets that
    // edge's native safe-area inset; an edge whose region == None stays edge-to-edge (0 inset). The
    // host reports the realized insets through i_safe_area_view2::set_safe_area_insets.
    TEST(content_page_safe_area, arrange_applies_safe_area_per_edge_when_set)
    {
        content_page page;
        mock_view body;
        body.configure({0, 0});
        page.set_content(body);
        page.set_padding(thickness{10});

        i_safe_area_view2& insets_face = page;
        insets_face.set_safe_area_insets(thickness{7, 59, 11, 34}); // L,T,R,B realized insets

        // top + bottom obey the safe area, left + right go edge-to-edge.
        page.set_safe_area_edges(safe_area_edges{safe_area_regions::none, safe_area_regions::all,
                                                 safe_area_regions::none, safe_area_regions::all});
        page.arrange(rect{0, 0, 200, 400});
        // left/right insets = padding only (10); top/bottom = padding + safe area (10+59, 10+34).
        // x=10, y=10+59=69, width=200-(10+10)=180, height=400-(69)-(10+34)=287.
        EXPECT_EQ(body.last_arrange, (rect{10, 69, 180, 287}));
    }

    // With SafeAreaEdges.All on every edge, all four native insets join the padding.
    TEST(content_page_safe_area, arrange_applies_all_edges_when_all)
    {
        content_page page;
        mock_view body;
        body.configure({0, 0});
        page.set_content(body);
        page.set_padding(thickness{10});

        i_safe_area_view2& insets_face = page;
        insets_face.set_safe_area_insets(thickness{7, 59, 11, 34});
        page.set_safe_area_edges(safe_area_edges{safe_area_regions::all});
        page.arrange(rect{0, 0, 200, 400});
        // each edge = padding + its inset: left 17, top 69, right 200-(17+21)=162, bottom 400-(69+44)=287.
        EXPECT_EQ(body.last_arrange, (rect{17, 69, 162, 287}));
    }

    // With SafeAreaEdges.None on every edge, the page is edge-to-edge even though insets were reported.
    TEST(content_page_safe_area, arrange_is_edge_to_edge_when_all_none)
    {
        content_page page;
        mock_view body;
        body.configure({0, 0});
        page.set_content(body);
        page.set_padding(thickness{10});

        i_safe_area_view2& insets_face = page;
        insets_face.set_safe_area_insets(thickness{7, 59, 11, 34});
        page.set_safe_area_edges(safe_area_edges::none());
        page.arrange(rect{0, 0, 200, 400});
        EXPECT_EQ(body.last_arrange, (rect{10, 10, 180, 380})); // padding only
    }
} // namespace
