// maui::controls::swipe_item_view — out-of-line definitions: the Padding descriptor, the
// MeasureContent/ArrangeContent layout (the content_page recipe), and the default handler
// self-registration. A swipe_item_view hosts custom content like a content view, so it resolves to the
// same content_page_handler (C# SwipeItemView : ContentView → ContentViewHandler).

#include "maui/controls/swipe_item_view.hpp"

#include <algorithm>

#include "maui/core/bindable_property.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<maui::core::thickness>& swipe_item_view::padding_property()
    {
        // ContentView's Padding default is default(Thickness) = 0.
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    maui::graphics::size swipe_item_view::measure(double width_constraint, double height_constraint)
    {
        const maui::core::thickness inset = padding();
        // Margin, per C# LayoutExtensions.ComputeDesiredSize (LayoutExtensions.cs:11-32): the constraint
        // loses it, the reported size regains it, so the PARENT reserves the gap. A measure() OVERRIDE does
        // not inherit view<>::measure's fold and has to repeat it — omitting it is not an under-reserve but
        // an IMBALANCE, because compute_frame subtracts the margin from desired_size regardless (view.hpp
        // :1069/1076). See layout.hpp:175-180 and border.cpp for the same defect, measured. No-op at zero.
        const maui::core::thickness view_margin = margin();
        const double margin_h = view_margin.horizontal_thickness();
        const double margin_v = view_margin.vertical_thickness();
        maui::graphics::size content_size{0, 0};
        if (content_ != nullptr)
        {
            content_size = content_->measure(width_constraint - margin_h - inset.horizontal_thickness(),
                                             height_constraint - margin_v - inset.vertical_thickness());
        }
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};
        // AFTER the size-request clamp, not before: in C# that clamp lives inside the handler's
        // GetDesiredSize and ComputeDesiredSize adds the margin to whatever it returned, so an explicit
        // WidthRequest cannot swallow the margin.
        desired_size_ = {resolve_size_request(measured.width, width(), minimum_width(), maximum_width()) + margin_h,
                         resolve_size_request(measured.height, height(), minimum_height(), maximum_height()) +
                             margin_v};
        return desired_size_;
    }

    maui::graphics::size swipe_item_view::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (content_ != nullptr)
        {
            const maui::core::thickness inset = padding();
            content_->arrange({bounds.x + inset.left, bounds.y + inset.top,
                               std::max(0.0, bounds.width - inset.horizontal_thickness()),
                               std::max(0.0, bounds.height - inset.vertical_thickness())});
        }
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds);
        }
        return {bounds.width, bounds.height};
    }
} // namespace maui::controls

// Self-register: a swipe_item_view hosts content, resolving to the shared content_page_handler.
MAUI_REGISTER_HANDLER(maui::controls::swipe_item_view, maui::core::content_page_handler)
