// Tests for the safe-area value types (X4): maui::core::safe_area_regions ([Flags]) and
// safe_area_edges. Behavior derived from src/Core/src/Primitives/SafeAreaRegions.cs and
// SafeAreaEdges.cs — the bit values, the three ctors, the static singletons, the IsSoftInput /
// IsOnlySoftInput / IsContainer / GetEdge probes, equality, and ToString.
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"

#include <cstdint>

#include <gtest/gtest.h>

namespace
{
    using maui::core::safe_area_edges;
    using maui::core::safe_area_regions;

    TEST(safe_area_regions, bit_values_match_the_oracle)
    {
        EXPECT_EQ(static_cast<std::int32_t>(safe_area_regions::none), 0);
        EXPECT_EQ(static_cast<std::int32_t>(safe_area_regions::soft_input), 1);
        EXPECT_EQ(static_cast<std::int32_t>(safe_area_regions::container), 2);
        EXPECT_EQ(static_cast<std::int32_t>(safe_area_regions::default_value), -1);
        EXPECT_EQ(static_cast<std::int32_t>(safe_area_regions::all), 32768);
    }

    TEST(safe_area_regions, flag_operators_and_has_flag)
    {
        const safe_area_regions combined = safe_area_regions::soft_input | safe_area_regions::container;
        EXPECT_TRUE(has_flag(combined, safe_area_regions::soft_input));
        EXPECT_TRUE(has_flag(combined, safe_area_regions::container));
        EXPECT_FALSE(has_flag(safe_area_regions::soft_input, safe_area_regions::container));
    }

    TEST(safe_area_edges, uniform_ctor_sets_all_edges)
    {
        const safe_area_edges edges{safe_area_regions::all};
        EXPECT_EQ(edges.left(), safe_area_regions::all);
        EXPECT_EQ(edges.top(), safe_area_regions::all);
        EXPECT_EQ(edges.right(), safe_area_regions::all);
        EXPECT_EQ(edges.bottom(), safe_area_regions::all);
    }

    TEST(safe_area_edges, horizontal_vertical_ctor)
    {
        const safe_area_edges edges{safe_area_regions::none, safe_area_regions::all};
        EXPECT_EQ(edges.left(), safe_area_regions::none);  // horizontal
        EXPECT_EQ(edges.right(), safe_area_regions::none); // horizontal
        EXPECT_EQ(edges.top(), safe_area_regions::all);    // vertical
        EXPECT_EQ(edges.bottom(), safe_area_regions::all); // vertical
    }

    TEST(safe_area_edges, get_edge_indexes_left_top_right_bottom)
    {
        const safe_area_edges edges{safe_area_regions::none, safe_area_regions::soft_input,
                                    safe_area_regions::container, safe_area_regions::all};
        EXPECT_EQ(edges.edge(0), safe_area_regions::none);
        EXPECT_EQ(edges.edge(1), safe_area_regions::soft_input);
        EXPECT_EQ(edges.edge(2), safe_area_regions::container);
        EXPECT_EQ(edges.edge(3), safe_area_regions::all);
        EXPECT_EQ(edges.edge(4), safe_area_regions::none); // out of range -> None
    }

    TEST(safe_area_edges, soft_input_probe)
    {
        EXPECT_FALSE(safe_area_edges::is_soft_input(safe_area_regions::default_value));
        EXPECT_TRUE(safe_area_edges::is_soft_input(safe_area_regions::all));
        EXPECT_TRUE(safe_area_edges::is_soft_input(safe_area_regions::soft_input));
        EXPECT_FALSE(safe_area_edges::is_soft_input(safe_area_regions::container));
        EXPECT_TRUE(safe_area_edges::is_only_soft_input(safe_area_regions::soft_input));
        EXPECT_FALSE(safe_area_edges::is_only_soft_input(safe_area_regions::all));
    }

    TEST(safe_area_edges, container_probe)
    {
        EXPECT_FALSE(safe_area_edges::is_container(safe_area_regions::default_value));
        EXPECT_TRUE(safe_area_edges::is_container(safe_area_regions::all));
        EXPECT_TRUE(safe_area_edges::is_container(safe_area_regions::container));
        EXPECT_FALSE(safe_area_edges::is_container(safe_area_regions::soft_input));
    }

    TEST(safe_area_edges, singletons_and_equality)
    {
        EXPECT_EQ(safe_area_edges::none(), safe_area_edges{safe_area_regions::none});
        EXPECT_EQ(safe_area_edges::all(), safe_area_edges{safe_area_regions::all});
        EXPECT_EQ(safe_area_edges::default_edges(), safe_area_edges{safe_area_regions::default_value});
        EXPECT_EQ(safe_area_edges::container(), safe_area_edges{safe_area_regions::container});
        EXPECT_NE(safe_area_edges::all(), safe_area_edges::none());
        // The default-constructed value is all-None.
        EXPECT_EQ(safe_area_edges{}, safe_area_edges::none());
    }

    TEST(safe_area_edges, to_string_uses_member_names)
    {
        const safe_area_edges edges{safe_area_regions::none, safe_area_regions::soft_input,
                                    safe_area_regions::container, safe_area_regions::all};
        EXPECT_EQ(edges.to_string(), "None, SoftInput, Container, All");
        EXPECT_EQ(safe_area_edges{safe_area_regions::default_value}.to_string(), "Default, Default, Default, Default");
    }
} // namespace
