// maui::controls::html_web_view_source — out-of-line definitions: the shared Html/BaseUrl
// bindable-property descriptors (HtmlWebViewSource.HtmlProperty / BaseUrlProperty; defaults the empty
// string, C#'s default(string)).

#include "maui/controls/html_web_view_source.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& html_web_view_source::html_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"html", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& html_web_view_source::base_url_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"base_url", std::string{}};
        return descriptor;
    }
} // namespace maui::controls
