#pragma once
// maui::controls::shapes::geometry_group  <=  Microsoft.Maui.Controls.Shapes.GeometryGroup
//   (+ GeometryCollection.cs — the children vector)
//
// A composite geometry combining multiple children into one shape. Ported from GeometryGroup.cs;
// append_path appends every child in order (its AppendPath). FillRule defaults to EvenOdd.
//
// PORT COLLAPSE (geometry.hpp note): the CollectionChanged/PropertyChanged resubscription +
// InvalidateGeometryRequested event disappear — re-set the owning path's data to retrigger.
// Ownership: the group owns its children (shared_ptr vector — GeometryCollection).

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/shapes/fill_rule.hpp"
#include "maui/controls/shapes/geometry.hpp"
#include "maui/graphics/path_f.hpp"

namespace maui::controls::shapes
{
    // Microsoft.Maui.Controls.Shapes.GeometryCollection (ObservableCollection<Geometry> collapsed to
    // a plain vector; see the geometry.hpp invalidation note).
    using geometry_collection = std::vector<std::shared_ptr<geometry>>;

    class geometry_group : public geometry
    {
    public:
        geometry_group() = default;

        [[nodiscard]] const geometry_collection& children() const
        {
            return children_;
        }
        [[nodiscard]] geometry_collection& children()
        {
            return children_;
        }
        void set_children(geometry_collection value)
        {
            children_ = std::move(value);
        }

        // C# GeometryGroup.FillRule (default EvenOdd).
        [[nodiscard]] shapes::fill_rule fill_rule() const
        {
            return fill_rule_;
        }
        void set_fill_rule(shapes::fill_rule value)
        {
            fill_rule_ = value;
        }

        void append_path(maui::graphics::path_f& path) const override
        {
            for (const std::shared_ptr<geometry>& child : children_)
            {
                if (child != nullptr)
                {
                    child->append_path(path);
                }
            }
        }

    private:
        geometry_collection children_;
        shapes::fill_rule fill_rule_ = shapes::fill_rule::even_odd;
    };
} // namespace maui::controls::shapes
