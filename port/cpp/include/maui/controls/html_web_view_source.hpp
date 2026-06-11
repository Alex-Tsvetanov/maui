#pragma once
// maui::controls::html_web_view_source  <=  Microsoft.Maui.Controls.HtmlWebViewSource
//
// A web-view source bound to an HTML-formatted string + an optional base URL for relative resources.
// Ported from src/Controls/src/Core/HtmlWebViewSource.cs: bindable Html and BaseUrl (default empty —
// C#'s null strings) whose changes raise SourceChanged (the Html/BaseUrl propertyChanged →
// OnSourceChanged hooks, ported as the on_property_changed override), and Load(renderer) →
// renderer.LoadHtml(Html, BaseUrl).

#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/web_view_source.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_web_view_delegate.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class html_web_view_source : public web_view_source
    {
    public:
        html_web_view_source() = default;
        explicit html_web_view_source(std::string html, std::string base_url = {})
        {
            set_html(std::move(html));
            if (!base_url.empty())
            {
                set_base_url(std::move(base_url));
            }
        }

        // C# HtmlWebViewSource.HtmlProperty / BaseUrlProperty (shared descriptors).
        static const maui::core::bindable_property<std::string>& html_property();
        static const maui::core::bindable_property<std::string>& base_url_property();

        [[nodiscard]] std::string_view html() const
        {
            return html_.get();
        }
        void set_html(std::string value)
        {
            html_.set(std::move(value));
        }

        [[nodiscard]] std::string_view base_url() const
        {
            return base_url_.get();
        }
        void set_base_url(std::string value)
        {
            base_url_.set(std::move(value));
        }

        // C# HtmlWebViewSource.Load: renderer.LoadHtml(Html, BaseUrl).
        void load(maui::core::i_web_view_delegate& web_view_delegate) override
        {
            web_view_delegate.load_html(html_.get(), base_url_.get());
        }

    protected:
        // The Html/BaseUrl propertyChanged callbacks: either change raises SourceChanged.
        void on_property_changed(std::string_view name) override
        {
            maui::core::bindable_object::on_property_changed(name);
            if (name == "html" || name == "base_url")
            {
                on_source_changed();
            }
        }

    private:
        maui::core::property<std::string> html_{*this, html_property()};
        maui::core::property<std::string> base_url_{*this, base_url_property()};
    };
} // namespace maui::controls
