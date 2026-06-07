// maui::controls::content_page — out-of-line definitions: the shared bindable-property descriptors, the
// measure/arrange content layout (LayoutExtensions.MeasureContent/ArrangeContent), and the default-
// handler self-registration. See content_page.hpp.

#include "maui/controls/content_page.hpp"

#include <string>

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
    const maui::core::bindable_property<maui::core::thickness>& content_page::padding_property()
    {
        // C# Page.PaddingDefaultValueCreator returns Thickness(0); the typed default T{} is all-zero.
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& content_page::title_property()
    {
        // C# Page.TitleProperty default is null → the empty string here.
        static const maui::core::bindable_property<std::string> descriptor{"title", std::string{}};
        return descriptor;
    }

    // C# LayoutExtensions.MeasureContent (this M4c cut omits the explicit Width/Height short-circuit, as
    // the control has no bindable WidthRequest/HeightRequest yet — the deferred VisualElement surface):
    // measure the content within the padding, then add the padding back. With no content, the measured
    // size is the padding only.
    maui::graphics::size content_page::measure(double width_constraint, double height_constraint)
    {
        const maui::core::thickness inset = padding();
        maui::graphics::size content_size{0, 0};
        if (content_ != nullptr)
        {
            content_size = content_->measure(width_constraint - inset.horizontal_thickness(),
                                             height_constraint - inset.vertical_thickness());
        }
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};
        desired_size_ = measured;
        return measured;
    }

    // C# ContentPage.ArrangeOverride/CrossPlatformArrange + LayoutExtensions.ArrangeContent: record the
    // frame, size the native host panel, then arrange the single content within the padding inset. With
    // no content there is nothing to arrange (ArrangeContent returns early).
    maui::graphics::size content_page::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds); // size/position the host panel
        }
        if (content_ != nullptr)
        {
            const maui::core::thickness inset = padding();
            const maui::graphics::rect target{bounds.x + inset.left, bounds.y + inset.top,
                                              bounds.width - inset.horizontal_thickness(),
                                              bounds.height - inset.vertical_thickness()};
            content_->arrange(target);
        }
        return {bounds.width, bounds.height};
    }
} // namespace maui::controls

// Self-register the default handler for content_page (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::content_page, maui::core::content_page_handler)
