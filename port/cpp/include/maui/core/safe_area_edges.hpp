#pragma once
// maui::core::safe_area_edges  <=  Microsoft.Maui.SafeAreaEdges
//
// Per-edge safe-area settings (left / top / right / bottom), each a safe_area_regions. Ported from
// src/Core/src/Primitives/SafeAreaEdges.cs — a readonly value struct with three ctors (uniform,
// horizontal/vertical, and the four explicit edges), the Default/None/All/Container singletons, the
// SafeArea.Ignore probe helpers (IsSoftInput / IsOnlySoftInput / IsContainer / GetEdge), value
// equality, and ToString ("Left, Top, Right, Bottom"). The SafeAreaEdgesTypeConverter (XAML) builds
// these from "All" / "None,All" / "left,top,right,bottom" markup (see xaml_converters.hpp).

#include <format>
#include <string>

#include "maui/core/safe_area_regions.hpp"

namespace maui::core
{
    class safe_area_edges
    {
    public:
        // SafeAreaEdges() — all edges None (the struct's default-init; C#'s default(SafeAreaEdges)).
        constexpr safe_area_edges() = default;

        // SafeAreaEdges(uniformValue) — the same value on all edges.
        constexpr explicit safe_area_edges(safe_area_regions uniform_value)
            : left_(uniform_value), top_(uniform_value), right_(uniform_value), bottom_(uniform_value)
        {
        }

        // SafeAreaEdges(horizontal, vertical) — left/right = horizontal, top/bottom = vertical.
        constexpr safe_area_edges(safe_area_regions horizontal, safe_area_regions vertical)
            : left_(horizontal), top_(vertical), right_(horizontal), bottom_(vertical)
        {
        }

        // SafeAreaEdges(left, top, right, bottom) — each edge explicit.
        constexpr safe_area_edges(safe_area_regions left, safe_area_regions top, safe_area_regions right,
                                  safe_area_regions bottom)
            : left_(left), top_(top), right_(right), bottom_(bottom)
        {
        }

        [[nodiscard]] constexpr safe_area_regions left() const
        {
            return left_;
        }
        [[nodiscard]] constexpr safe_area_regions top() const
        {
            return top_;
        }
        [[nodiscard]] constexpr safe_area_regions right() const
        {
            return right_;
        }
        [[nodiscard]] constexpr safe_area_regions bottom() const
        {
            return bottom_;
        }

        // SafeAreaEdges.GetEdge(int): 0=Left, 1=Top, 2=Right, 3=Bottom, anything else None.
        [[nodiscard]] constexpr safe_area_regions edge(int index) const
        {
            switch (index)
            {
                case 0:
                    return left_;
                case 1:
                    return top_;
                case 2:
                    return right_;
                case 3:
                    return bottom_;
                default:
                    return safe_area_regions::none;
            }
        }

        // SafeAreaEdges.IsSoftInput: Default => false, All => true, else the SoftInput bit.
        [[nodiscard]] static constexpr bool is_soft_input(safe_area_regions region)
        {
            if (region == safe_area_regions::default_value)
            {
                return false;
            }
            if (region == safe_area_regions::all)
            {
                return true;
            }
            return has_flag(region, safe_area_regions::soft_input);
        }

        // SafeAreaEdges.IsOnlySoftInput: exactly SoftInput (no other flags, not All).
        [[nodiscard]] static constexpr bool is_only_soft_input(safe_area_regions region)
        {
            return region == safe_area_regions::soft_input;
        }

        // SafeAreaEdges.IsContainer: Default => false, All => true, else the Container bit.
        [[nodiscard]] static constexpr bool is_container(safe_area_regions region)
        {
            if (region == safe_area_regions::default_value)
            {
                return false;
            }
            if (region == safe_area_regions::all)
            {
                return true;
            }
            return has_flag(region, safe_area_regions::container);
        }

        // The static singletons (C# SafeAreaEdges.Default / None / All / Container).
        [[nodiscard]] static constexpr safe_area_edges default_edges()
        {
            return safe_area_edges{safe_area_regions::default_value};
        }
        [[nodiscard]] static constexpr safe_area_edges none()
        {
            return safe_area_edges{safe_area_regions::none};
        }
        [[nodiscard]] static constexpr safe_area_edges all()
        {
            return safe_area_edges{safe_area_regions::all};
        }
        [[nodiscard]] static constexpr safe_area_edges container()
        {
            return safe_area_edges{safe_area_regions::container};
        }

        // SafeAreaEdges.Equals: edge-by-edge value equality.
        [[nodiscard]] constexpr bool operator==(const safe_area_edges& other) const = default;

        // SafeAreaEdges.ToString: "Left, Top, Right, Bottom" — each edge as its [Flags] enum name (C#
        // interpolates SafeAreaRegions, so a single-flag value prints its member name). DEVIATION: a
        // combined value (e.g. SoftInput|Container) prints its numeric form rather than the .NET
        // comma-joined member names ("SoftInput, Container") — reproducing the full [Flags] formatter is
        // out of scope and the loader never round-trips a SafeAreaEdges back through ToString.
        [[nodiscard]] std::string to_string() const
        {
            return std::format("{}, {}, {}, {}", region_name(left_), region_name(top_), region_name(right_),
                               region_name(bottom_));
        }

    private:
        // The single-flag member names (C# SafeAreaRegions.ToString); a combined value falls back to its
        // numeric spelling (the documented to_string deviation above).
        [[nodiscard]] static std::string region_name(safe_area_regions region)
        {
            switch (region)
            {
                case safe_area_regions::none:
                    return "None";
                case safe_area_regions::soft_input:
                    return "SoftInput";
                case safe_area_regions::container:
                    return "Container";
                case safe_area_regions::default_value:
                    return "Default";
                case safe_area_regions::all:
                    return "All";
            }
            return std::to_string(static_cast<std::int32_t>(region));
        }

        safe_area_regions left_ = safe_area_regions::none;
        safe_area_regions top_ = safe_area_regions::none;
        safe_area_regions right_ = safe_area_regions::none;
        safe_area_regions bottom_ = safe_area_regions::none;
    };
} // namespace maui::core
