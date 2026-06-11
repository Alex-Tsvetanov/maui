#pragma once
// maui::controls::web_view  <=  Microsoft.Maui.Controls.WebView
//
// A view that presents HTML content. Ported from src/Controls/src/Core/WebView/WebView.cs. Same API
// shape as the other controls: method accessors backed by private property<T> engines whose changes flow
// through view::on_property_changed to the handler (virtual→native); the reverse direction is the send_*
// methods the handler's platform partial calls (send_navigating/send_navigated raise the public events;
// set_can_go_back/set_can_go_forward refresh the handler-pushed read-onlys).
//
// Source semantics (mirroring SourceProperty's propertyChanging/propertyChanged + OnSourceChanged):
//   - setting a source subscribes the control to its SourceChanged and pushes the inherited
//     BindingContext into it (SetInheritedBindingContext); replacing it unsubscribes from the old one;
//   - a source whose content changes (e.g. url_web_view_source::set_url) re-raises the control's
//     "source" property change, so the handler re-runs map_source and reloads (WebView.OnSourceChanged →
//     OnPropertyChanged(SourceProperty.PropertyName));
//   - set_source(string) ports C#'s `implicit operator WebViewSource(string url)` — it mints a
//     url_web_view_source for the url.
//
// can_go_back / can_go_forward are READ-ONLY bindable properties (C#'s CanGoBackPropertyKey /
// CanGoForwardPropertyKey): only the handler writes them, through the i_web_view setters.
//
// go_back/go_forward/reload/eval dispatch handler COMMANDS (Handler?.Invoke(nameof(IWebView.GoBack))
// etc.); eval_js ports EvaluateJavaScriptAsync — it escapes + wraps the script exactly as C# does
// ("try{JSON.stringify(eval('…'))}catch(e){'null'};"), routes it through the "evaluate_java_script"
// command as an evaluate_java_script_request, and post-processes the raw result ("null" → nullopt,
// otherwise the JSON.stringify quotes are trimmed) before invoking the caller's callback — the
// callback-based form of C#'s Task<string>, per the port's async idiom. The legacy renderer events
// (EvalRequested/GoBackRequested/…, the IWebViewController seam) are Compatibility-era and out of scope.
//
// OUT OF SCOPE (documented, not stubbed): Cookies, UserAgent, ProcessTerminated, and the
// platform-configuration (On<Android>() zoom/mixed-content) surface.

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/controls/web_navigation_event_args.hpp"
#include "maui/controls/web_view_source.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_web_view.hpp"
#include "maui/core/i_web_view_source.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"
#include "maui/core/web_navigation_event.hpp"
#include "maui/core/web_navigation_result.hpp"

namespace maui::controls
{
    class web_view : public view<maui::core::i_web_view>
    {
    public:
        // The JavaScript-evaluation completion: nullopt when the script errored or yielded null/undefined
        // (C#'s null Task<string> result), otherwise the unquoted result string.
        using eval_js_callback = maui::core::move_only_function<void(std::optional<std::string> result)>;

        // Declare the style TargetType so an implicit / class style targeting `web_view` matches.
        web_view()
        {
            this->set_style_target_type<web_view>();
        }

        // Shared bindable-property descriptors (WebView.SourceProperty + the CanGoBack/CanGoForward
        // read-only property keys).
        static const maui::core::bindable_property<std::shared_ptr<web_view_source>>& source_property();
        static const maui::core::bindable_property<bool>& can_go_back_property();
        static const maui::core::bindable_property<bool>& can_go_forward_property();

        // ---- i_web_view ----
        // A raw borrow of the owned source (null when unset), like i_image::source().
        [[nodiscard]] maui::core::i_web_view_source* source() const override
        {
            return source_.get().get();
        }
        [[nodiscard]] bool can_go_back() const override
        {
            return can_go_back_.get();
        }
        [[nodiscard]] bool can_go_forward() const override
        {
            return can_go_forward_.get();
        }
        // HANDLER-ONLY writers (C#'s CanGoBackPropertyKey path): the platform partial refreshes the two
        // read-onlys after every navigation (WebViewExtensions.UpdateCanGoBackForward).
        void set_can_go_back(bool value) override
        {
            can_go_back_.set(value);
        }
        void set_can_go_forward(bool value) override
        {
            can_go_forward_.set(value);
        }

        // C# IWebView.Navigating: mint the args (with a UrlWebViewSource for the url), raise the public
        // event, and report whether a subscriber cancelled.
        bool send_navigating(maui::core::web_navigation_event navigation_event, std::string_view url) override;
        // C# IWebView.Navigated.
        void send_navigated(maui::core::web_navigation_event navigation_event, std::string_view url,
                            maui::core::web_navigation_result result) override;

        // ---- the developer-facing source surface ----
        // The owned source (the typed form of the i_web_view borrow).
        [[nodiscard]] const std::shared_ptr<web_view_source>& web_source() const
        {
            return source_.get();
        }
        // The control takes ownership of the source (WebView.Source = value).
        void set_source(std::shared_ptr<web_view_source> value)
        {
            source_.set(std::move(value));
        }
        // C#'s implicit string → UrlWebViewSource conversion.
        void set_source(std::string url);

        // ---- navigation + scripting (handler commands) ----
        void go_back();    // C# WebView.GoBack
        void go_forward(); // C# WebView.GoForward
        void reload();     // C# WebView.Reload
        // C# WebView.Eval: fire-and-forget script evaluation.
        void eval(std::string_view script);
        // C# WebView.EvaluateJavaScriptAsync, callback form (see the header comment).
        void eval_js(std::string_view script, eval_js_callback on_result);

        // ---- developer-facing events ----
        // Raised before a navigation; a handler may set args.cancel (C# WebView.Navigating). The args are
        // shared MUTABLE state, hence the by-reference event parameter.
        maui::core::event<web_navigating_event_args&> navigating;
        // Raised after a navigation completes or fails (C# WebView.Navigated).
        maui::core::event<web_navigated_event_args> navigated;

    protected:
        // SourceProperty's propertyChanging/propertyChanged hooks: rewire the SourceChanged subscription
        // + inherited BindingContext whenever the source slot changes (any set path, including bindings).
        void on_property_changed(std::string_view name) override;
        // C# WebView.OnBindingContextChanged: keep the source's inherited context in sync.
        void on_binding_context_changed() override;

    private:
        // (Re)subscribe to the current source's SourceChanged + push the inherited BindingContext.
        void wire_source();

        maui::core::property<std::shared_ptr<web_view_source>> source_{*this, source_property()};
        maui::core::property<bool> can_go_back_{*this, can_go_back_property()};
        maui::core::property<bool> can_go_forward_{*this, can_go_forward_property()};

        // The source the subscription below is attached to — kept alive until rewired so the
        // scoped_connection's disconnect never runs against a destroyed event.
        std::shared_ptr<web_view_source> wired_source_;
        maui::core::scoped_connection source_connection_;
    };
} // namespace maui::controls
