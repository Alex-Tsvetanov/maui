// maui::controls::web_view — out-of-line definitions: the shared bindable-property descriptors, the
// source wiring (SourceChanged subscription + inherited BindingContext), the navigation/scripting command
// dispatch, and the default-handler self-registration. See web_view.hpp.

#include "maui/controls/web_view.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "maui/controls/url_web_view_source.hpp"
#include "maui/controls/view.hpp"
#include "maui/controls/web_navigation_event_args.hpp"
#include "maui/controls/web_view_source.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/evaluate_java_script_request.hpp"
#include "maui/core/event.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"
#include "maui/core/web_view_handler.hpp"
#include "maui/core/web_view_helper.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::shared_ptr<web_view_source>>& web_view::source_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<web_view_source>> descriptor{"source"};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& web_view::can_go_back_property()
    {
        // C# CanGoBackPropertyKey: read-only, default false.
        static const maui::core::bindable_property<bool> descriptor{"can_go_back", false};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& web_view::can_go_forward_property()
    {
        // C# CanGoForwardPropertyKey: read-only, default false.
        static const maui::core::bindable_property<bool> descriptor{"can_go_forward", false};
        return descriptor;
    }

    bool web_view::send_navigating(maui::core::web_navigation_event navigation_event, std::string_view url)
    {
        // C#: new WebNavigatingEventArgs(evnt, new UrlWebViewSource { Url = url }, url) → SendNavigating.
        web_navigating_event_args args;
        args.navigation_event = navigation_event;
        args.source = std::make_shared<url_web_view_source>(std::string(url));
        args.url = std::string(url);
        navigating.raise(args);
        return args.cancel;
    }

    void web_view::send_navigated(maui::core::web_navigation_event navigation_event, std::string_view url,
                                  maui::core::web_navigation_result result)
    {
        // C#: new WebNavigatedEventArgs(evnt, new UrlWebViewSource { Url = url }, url, result) →
        // SendNavigated.
        web_navigated_event_args args;
        args.navigation_event = navigation_event;
        args.source = std::make_shared<url_web_view_source>(std::string(url));
        args.url = std::string(url);
        args.result = result;
        navigated.raise(args);
    }

    void web_view::set_source(std::string url)
    {
        // C# implicit operator WebViewSource(string url) => new UrlWebViewSource { Url = url }.
        set_source(std::make_shared<url_web_view_source>(std::move(url)));
    }

    void web_view::go_back()
    {
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("go_back");
        }
    }

    void web_view::go_forward()
    {
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("go_forward");
        }
    }

    void web_view::reload()
    {
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("reload");
        }
    }

    void web_view::eval(std::string_view script)
    {
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("eval", std::string(script));
        }
    }

    void web_view::eval_js(std::string_view script, eval_js_callback on_result)
    {
        if (!on_result)
        {
            return;
        }
        // C# EvaluateJavaScriptAsync: escape the script, then wrap it so the platform returns a
        // JSON.stringify'd value (or 'null' when it errors) — "make all the platforms mimic Android".
        std::string wrapped = "try{JSON.stringify(eval('";
        wrapped += maui::core::escape_js_string(script);
        wrapped += "'))}catch(e){'null'};";

        auto request = std::make_shared<maui::core::evaluate_java_script_request>(
            std::move(wrapped), [callback = std::move(on_result)](std::string result) mutable {
                // C#: a "null" result (errored/undefined script) is a null return; any other result has
                // the JSON.stringify quotes trimmed (Trim('"') — every leading/trailing quote).
                if (result == "null")
                {
                    callback(std::nullopt);
                    return;
                }
                std::size_t begin = 0;
                std::size_t end = result.size();
                while (begin < end && result[begin] == '"')
                {
                    ++begin;
                }
                while (end > begin && result[end - 1] == '"')
                {
                    --end;
                }
                callback(result.substr(begin, end - begin));
            });

        if (const auto& element_handler = handler())
        {
            element_handler->invoke("evaluate_java_script", request);
            return;
        }
        // DEVIATION: without a handler C# would fault the awaited task (Handler is null); the port's
        // callback channel completes with the null result instead.
        request->complete("null");
    }

    void web_view::on_property_changed(std::string_view name)
    {
        // C# runs SourceProperty's propertyChanging/propertyChanged (the subscription rewire +
        // SetInheritedBindingContext) before the PropertyChanged notification reaches the handler.
        if (name == "source")
        {
            wire_source();
        }
        view::on_property_changed(name);
    }

    void web_view::on_binding_context_changed()
    {
        // C# WebView.OnBindingContextChanged: base first, then SetInheritedBindingContext(Source, …).
        view::on_binding_context_changed();
        if (const auto& source = source_.get())
        {
            source->set_inherited_binding_context(raw_binding_context());
        }
    }

    void web_view::wire_source()
    {
        // Drop the old subscription first (C#'s propertyChanging unsubscribe); wired_source_ keeps the
        // previous source alive until after the disconnect ran against its event.
        source_connection_.reset();
        wired_source_ = source_.get();
        if (!wired_source_)
        {
            return;
        }
        source_connection_ = maui::core::connect_scoped(wired_source_->source_changed, [this] {
            // C# WebView.OnSourceChanged: OnPropertyChanged(SourceProperty.PropertyName) — re-announce
            // the source slot so the handler re-runs map_source (and the property_changed event fires).
            on_property_changed("source");
        });
        // C#'s propertyChanged hook: SetInheritedBindingContext(source, BindingContext).
        wired_source_->set_inherited_binding_context(raw_binding_context());
    }
} // namespace maui::controls

// Opt-in self-registration: web_view resolves to web_view_handler in the default registry.
MAUI_REGISTER_HANDLER(maui::controls::web_view, maui::core::web_view_handler)
