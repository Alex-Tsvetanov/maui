#pragma once
// maui::controls::shapes::path  <=  Microsoft.Maui.Controls.Shapes.Path
//
// A shape drawing an arbitrary geometry (Data) with an optional RenderTransform. Ported from
// Path.cs: GetPath appends the geometry; the "data"/"render_transform" keys ride
// shape_view_handler's absorbed sub-handler table (PathHandler.MapData/MapRenderTransform — the
// transform reaches the drawable through the i_shape_view render_transform_matrix() port
// extension). Path is one of the C# margin-adding measure types.
//
// data / render_transform are OWNED via shared_ptr properties; the geometry/transform objects are
// plain (the geometry.hpp collapse), so after mutating one IN PLACE call invalidate_data() /
// invalidate_render_transform() to retrigger the mapper (standing in for C#'s
// PropertyChanged/InvalidatePathGeometryRequested resubscription).

#include <memory>
#include <optional>
#include <utility>

#include "maui/controls/shapes/geometry.hpp"
#include "maui/controls/shapes/matrix.hpp"
#include "maui/controls/shapes/shape.hpp"
#include "maui/controls/shapes/transform.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/matrix3x2.hpp"
#include "maui/graphics/path_f.hpp"

namespace maui::controls::shapes
{
    class path final : public shape
    {
    public:
        path()
        {
            this->set_style_target_type<path>();
        }
        // C# Path(Geometry data).
        explicit path(std::shared_ptr<geometry> data) : path()
        {
            set_data(std::move(data));
        }

        static const maui::core::bindable_property<std::shared_ptr<geometry>>& data_property();
        static const maui::core::bindable_property<std::shared_ptr<transform>>& render_transform_property();

        // ---- Data (the geometry to draw; owned) ----
        [[nodiscard]] const std::shared_ptr<geometry>& data() const
        {
            return data_.get();
        }
        void set_data(std::shared_ptr<geometry> value)
        {
            data_.set(std::move(value));
        }
        // Re-run the "data" mapper after an in-place geometry mutation (header note).
        void invalidate_data()
        {
            if (const auto& element_handler = handler())
            {
                element_handler->update_value("data");
            }
        }

        // ---- RenderTransform (owned) ----
        [[nodiscard]] const std::shared_ptr<transform>& render_transform() const
        {
            return render_transform_.get();
        }
        void set_render_transform(std::shared_ptr<transform> value)
        {
            render_transform_.set(std::move(value));
        }
        // Re-run the "render_transform" mapper after an in-place transform mutation (header note).
        void invalidate_render_transform()
        {
            if (const auto& element_handler = handler())
            {
                element_handler->update_value("render_transform");
            }
        }

        // The C# PathHandler.MapRenderTransform push, surfaced through the contract.
        [[nodiscard]] std::optional<maui::graphics::matrix3x2> render_transform_matrix() const override
        {
            const std::shared_ptr<transform>& value = render_transform_.get();
            if (value == nullptr)
            {
                return std::nullopt;
            }
            return to_matrix3x2(value->value());
        }

        [[nodiscard]] maui::graphics::path_f get_path() const override
        {
            maui::graphics::path_f result;
            if (const std::shared_ptr<geometry>& value = data_.get())
            {
                value->append_path(result);
            }
            return result;
        }

    protected:
        [[nodiscard]] bool adds_margin_to_measure() const override
        {
            return true;
        }

    private:
        maui::core::property<std::shared_ptr<geometry>> data_{*this, data_property()};
        maui::core::property<std::shared_ptr<transform>> render_transform_{*this, render_transform_property()};
    };
} // namespace maui::controls::shapes
