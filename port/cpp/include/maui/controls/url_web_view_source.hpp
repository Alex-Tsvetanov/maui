#pragma once
// maui::controls::url_web_view_source  <=  Microsoft.Maui.Controls.UrlWebViewSource
//
// A web-view source that loads content from a URL. Ported from
// src/Controls/src/Core/UrlWebViewSource.cs: a bindable Url (default empty — C#'s null string) whose
// change raises SourceChanged (the UrlProperty propertyChanged → OnSourceChanged hook, ported as the
// on_property_changed override), and Load(renderer) → renderer.LoadUrl(Url).

#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/web_view_source.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_web_view_delegate.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class url_web_view_source : public web_view_source
    {
    public:
        url_web_view_source() = default;
        explicit url_web_view_source(std::string url)
        {
            set_url(std::move(url));
        }

        // C# UrlWebViewSource.UrlProperty (shared descriptor).
        static const maui::core::bindable_property<std::string>& url_property();

        [[nodiscard]] std::string_view url() const
        {
            return url_.get();
        }
        void set_url(std::string value)
        {
            url_.set(std::move(value));
        }

        // C# UrlWebViewSource.Load: renderer.LoadUrl(Url).
        void load(maui::core::i_web_view_delegate& web_view_delegate) override
        {
            web_view_delegate.load_url(url_.get());
        }

    protected:
        // The UrlProperty propertyChanged callback: any url change raises SourceChanged.
        void on_property_changed(std::string_view name) override
        {
            maui::core::bindable_object::on_property_changed(name);
            if (name == "url")
            {
                on_source_changed();
            }
        }

    private:
        maui::core::property<std::string> url_{*this, url_property()};
    };
} // namespace maui::controls
