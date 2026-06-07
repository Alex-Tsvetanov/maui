// content_page_handler — headless platform recipe. The "native host" is a single-content mirror
// (hosted_content) in content_page_platform so tests can observe that the host tracks the control's
// content as it is set/cleared. The Apple twin (a real NSView subview) is
// src/platform/apple/content_page_handler.mm.

#include "maui/core/content_page_handler.hpp"

#include <memory>

#include "maui/core/i_content_view.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    content_page_platform::~content_page_platform() = default;

    std::unique_ptr<content_page_platform> content_page_handler::create_platform_view()
    {
        return std::make_unique<content_page_platform>();
    }

    void content_page_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Mirror the control's current content (C#'s UpdateContent reads VirtualView.PresentedContent).
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
    }

    maui::graphics::size content_page_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        // A content view computes its own size through the control (which ports MeasureContent measuring
        // the content within the padding), not the handler, so the handler reports nothing here.
        return {0, 0};
    }

    void content_page_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native host to position; the content is arranged by the control directly.
    }
} // namespace maui::core
