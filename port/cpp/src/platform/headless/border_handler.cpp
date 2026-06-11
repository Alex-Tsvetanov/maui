// border_handler — headless platform recipe. The "native host" is the single-content mirror plus the
// border_stroke_spec mirror in border_platform: tests observe that the host tracks the control's
// content and that every stroke property push lands as the resolved IBorderStroke snapshot (the
// MauiCALayer values, sans drawing). The Apple twin (a real NSView + CAShapeLayer stroke) is
// src/platform/apple/border_handler.mm.

#include "maui/core/border_handler.hpp"

#include <memory>

#include "maui/core/i_border_view.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::core
{
    border_platform::~border_platform() = default;

    std::unique_ptr<border_platform> border_handler::create_platform_view()
    {
        return std::make_unique<border_platform>();
    }

    void border_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Mirror the control's current content (C#'s UpdateContent reads VirtualView.PresentedContent).
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
    }

    void border_handler::update_border()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // Mirror the full resolved stroke surface (C# UpdateMauiCALayer pushes the same set of values).
        platform->border = make_border_stroke_spec(*virtual_view());
    }

    void border_handler::arrange_native(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native host to position; the content is arranged by the control directly.
    }
} // namespace maui::core
