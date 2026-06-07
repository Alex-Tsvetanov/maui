#pragma once
// maui::controls::view<ViewInterface>  <=  Microsoft.Maui.Controls.View / VisualElement (minimal M2)
//
// The reusable base for concrete controls: a bindable_object that implements the i_view boilerplate
// (handler ownership + the virtual-view ⇄ handler wiring, geometry, the measure/arrange seam) so each
// control only adds its own interface members. This is the FIRST cut — only the members the handler
// seam needs are real; the full VisualElement property set (transforms, the bindable IsEnabled/Opacity/
// Visibility/WidthRequest/… and real layout) arrives in M3/M4. Documented gaps, not silent.
//
// Why a template parameter instead of `view : i_view`? A concrete control is e.g. `button : i_button,
// i_text`, and i_button already derives i_view. If `view` also derived i_view, the control would
// inherit i_view twice (a diamond). Parameterizing on the control's view-interface (`view<i_button>`)
// gives a single i_view subobject with zero virtual-inheritance overhead: view<ViewInterface> derives
// ViewInterface (which derives i_view) and supplies the i_view method bodies; the control supplies the
// interface-specific members.

#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    template <class ViewInterface> class view : public maui::core::bindable_object, public ViewInterface
    {
        static_assert(std::is_base_of_v<maui::core::i_view, ViewInterface>,
                      "ViewInterface must derive maui::core::i_view");

    public:
        // ---- i_element ----
        [[nodiscard]] const std::shared_ptr<maui::core::i_element_handler>& handler() const override
        {
            return handler_;
        }
        // Setting the handler wires the seam: the incoming handler binds to this view (creating the
        // platform view + running the mapper), then any previous handler is disconnected. Mirrors
        // Element.Handler's setter (SetVirtualView(this) on the new one, DisconnectHandler() on the old).
        void set_handler(std::shared_ptr<maui::core::i_element_handler> value) override
        {
            if (handler_ == value)
            {
                return;
            }
            std::shared_ptr<maui::core::i_element_handler> const previous = handler_;
            handler_ = std::move(value);
            if (handler_)
            {
                handler_->set_virtual_view(*this);
            }
            if (previous && previous != handler_)
            {
                previous->disconnect_handler();
            }
        }
        [[nodiscard]] std::shared_ptr<maui::core::i_element> parent() const override
        {
            return parent_.lock();
        }

        // ---- i_transform (identity defaults for M2; real transforms in M4) ----
        [[nodiscard]] double translation_x() const override
        {
            return 0;
        }
        [[nodiscard]] double translation_y() const override
        {
            return 0;
        }
        [[nodiscard]] double scale() const override
        {
            return 1;
        }
        [[nodiscard]] double scale_x() const override
        {
            return 1;
        }
        [[nodiscard]] double scale_y() const override
        {
            return 1;
        }
        [[nodiscard]] double rotation() const override
        {
            return 0;
        }
        [[nodiscard]] double rotation_x() const override
        {
            return 0;
        }
        [[nodiscard]] double rotation_y() const override
        {
            return 0;
        }
        [[nodiscard]] double anchor_x() const override
        {
            return 0.5;
        }
        [[nodiscard]] double anchor_y() const override
        {
            return 0.5;
        }

        // ---- i_view ----
        [[nodiscard]] std::string_view automation_id() const override
        {
            return automation_id_;
        }
        [[nodiscard]] maui::core::flow_direction flow_direction() const override
        {
            return maui::core::flow_direction::match_parent;
        }
        [[nodiscard]] maui::core::layout_alignment horizontal_layout_alignment() const override
        {
            return maui::core::layout_alignment::fill;
        }
        [[nodiscard]] maui::core::layout_alignment vertical_layout_alignment() const override
        {
            return maui::core::layout_alignment::fill;
        }
        [[nodiscard]] maui::core::semantics* semantics() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::graphics::i_shape* clip() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::core::i_shadow* shadow() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::graphics::paint* background() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::core::visibility visibility() const override
        {
            return visibility_;
        }
        [[nodiscard]] double opacity() const override
        {
            return opacity_;
        }
        [[nodiscard]] bool is_enabled() const override
        {
            return is_enabled_;
        }
        void set_is_enabled(bool value)
        {
            is_enabled_ = value;
        }
        [[nodiscard]] bool is_focused() const override
        {
            return is_focused_;
        }
        void set_is_focused(bool value) override
        {
            is_focused_ = value;
        }
        [[nodiscard]] bool input_transparent() const override
        {
            return false;
        }
        [[nodiscard]] maui::graphics::rect frame() const override
        {
            return frame_;
        }
        void set_frame(maui::graphics::rect value) override
        {
            frame_ = value;
        }
        [[nodiscard]] double width() const override
        {
            return frame_.width;
        }
        [[nodiscard]] double minimum_width() const override
        {
            return 0;
        }
        [[nodiscard]] double maximum_width() const override
        {
            return std::numeric_limits<double>::infinity();
        }
        [[nodiscard]] double height() const override
        {
            return frame_.height;
        }
        [[nodiscard]] double minimum_height() const override
        {
            return 0;
        }
        [[nodiscard]] double maximum_height() const override
        {
            return std::numeric_limits<double>::infinity();
        }
        [[nodiscard]] maui::core::thickness margin() const override
        {
            return {};
        }
        [[nodiscard]] maui::graphics::size desired_size() const override
        {
            return desired_size_;
        }
        [[nodiscard]] int z_index() const override
        {
            return 0;
        }
        // The measure/arrange seam delegates to the view handler (C# IViewHandler.GetDesiredSize /
        // PlatformArrange), the cross-platform layout calling into the platform view.
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override
        {
            frame_ = bounds;
            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler_.get()))
            {
                view_handler->platform_arrange(bounds);
            }
            return {bounds.width, bounds.height};
        }
        maui::graphics::size measure(double width_constraint, double height_constraint) override
        {
            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler_.get()))
            {
                desired_size_ = view_handler->get_desired_size(width_constraint, height_constraint);
            }
            else
            {
                desired_size_ = {};
            }
            return desired_size_;
        }
        void invalidate_measure() override
        {
            // Layout invalidation is wired in M3 (the layout pass); no-op for the M2 seam.
        }
        void invalidate_arrange() override
        {
        }
        bool focus() override
        {
            is_focused_ = true;
            return true;
        }
        void unfocus() override
        {
            is_focused_ = false;
        }

    protected:
        view() = default;

        // The virtual→native seam: any bindable property change notifies the handler, which re-runs
        // that property's mapper. Mirrors MAUI, where a BindableProperty change calls Handler.UpdateValue.
        void on_property_changed(std::string_view name) override
        {
            maui::core::bindable_object::on_property_changed(name);
            if (handler_)
            {
                handler_->update_value(name);
            }
        }

        std::shared_ptr<maui::core::i_element_handler> handler_;
        std::weak_ptr<maui::core::i_element> parent_;
        maui::graphics::rect frame_;
        maui::graphics::size desired_size_;
        std::string automation_id_;
        double opacity_ = 1.0;
        maui::core::visibility visibility_ = maui::core::visibility::visible;
        bool is_enabled_ = true;
        bool is_focused_ = false;
    };
} // namespace maui::controls
