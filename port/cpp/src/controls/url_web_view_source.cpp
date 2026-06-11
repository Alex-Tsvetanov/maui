// maui::controls::url_web_view_source — out-of-line definitions: the shared Url bindable-property
// descriptor (UrlWebViewSource.UrlProperty; default the empty string, C#'s default(string)).

#include "maui/controls/url_web_view_source.hpp"

#include <string>

#include "maui/core/bindable_property.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& url_web_view_source::url_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"url", std::string{}};
        return descriptor;
    }
} // namespace maui::controls
