#pragma once
// maui::controls::shapes::transform_group  <=  Microsoft.Maui.Controls.Shapes.TransformGroup
//   (+ TransformCollection.cs — the children vector)
//
// A composite transform applying its children in sequence. Ported from TransformGroup.cs; value()
// folds Matrix.Multiply over the children exactly like UpdateTransformMatrix (computed on read — the
// transform.hpp port collapse replaces the CollectionChanged/PropertyChanged resubscription).
//
// Ownership: the group owns its children (shared_ptr vector — TransformCollection).

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/transform.hpp"

namespace maui::controls::shapes
{
    // Microsoft.Maui.Controls.Shapes.TransformCollection (ObservableCollection<Transform> collapsed
    // to a plain vector; see the transform.hpp invalidation note).
    using transform_collection = std::vector<std::shared_ptr<transform>>;

    class transform_group : public transform
    {
    public:
        transform_group() = default;

        [[nodiscard]] const transform_collection& children() const
        {
            return children_;
        }
        [[nodiscard]] transform_collection& children()
        {
            return children_;
        }
        void set_children(transform_collection value)
        {
            children_ = std::move(value);
        }

        // C# TransformGroup.UpdateTransformMatrix: matrix = Multiply(matrix, child.Value) over all
        // children, starting from the identity.
        [[nodiscard]] matrix value() const override
        {
            matrix result;
            for (const std::shared_ptr<transform>& child : children_)
            {
                if (child != nullptr)
                {
                    result = matrix::multiply(result, child->value());
                }
            }
            return result;
        }

    private:
        transform_collection children_;
    };
} // namespace maui::controls::shapes
