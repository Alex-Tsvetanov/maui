// maui::xaml::hydration_context  <=  Microsoft.Maui.Controls.Xaml.HydrationContext
// (src/Controls/src/Xaml/HydrationContext.cs). See hydration_context.hpp for the port-specific
// ownership/registry additions.
#include "maui/xaml/hydration_context.hpp"

#include "maui/xaml/xaml_parse_exception.hpp"

namespace maui::xaml
{
    void hydration_context::handle(const xaml_parse_exception& error) const
    {
        if (handler_)
        {
            handler_(error);
            return;
        }
        // Re-raise as a prvalue (throwing the lvalue would COPY-construct the exception object, and
        // xaml_parse_exception's string members make that copy throwing). Reconstructing from the
        // parts yields the identical formatted what().
        throw xaml_parse_exception{error.unformatted_message(), error.line_number(), error.line_position()};
    }
} // namespace maui::xaml
