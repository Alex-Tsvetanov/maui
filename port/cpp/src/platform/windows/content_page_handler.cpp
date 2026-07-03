// content_page_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Canvas
// that HOSTS the single content child. The windows twin of src/platform/apple/content_page_handler.mm
// (a plain NSView host) and the real-native sibling of the headless single-content mirror. This handler
// backs BOTH content_view and content_page controls (registered for i_content_view).
//
// Ported from ContentViewHandler.cs + ContentViewHandler.Windows.cs (MAUI's ContentPanel → the port's
// Canvas manual-frame host — the apple flipped_container twin; the C++ side owns measure/arrange, so
// the panel never lays out its child itself, exactly like ContentPanel whose CrossPlatformLayout the
// port's control performs):
//   - set_content: C# UpdateContent — CachedChildren.Clear() + Add(content.ToPlatform()); here
//     Canvas.Children().Clear() + Append(the content's native FrameworkElement). The content is then
//     framed by its own platform_arrange (absolute Canvas coordinates).
//   - get_desired_size returns {0,0}: a content view sizes itself through the control (which ports
//     MeasureContent), not the handler — the headless/apple/android twins' shape.
//   - platform_arrange: the shared Canvas recipe (Canvas.SetLeft/SetTop + Width/Height).
//
// DOCUMENTED DEVIATIONS (infrastructure gaps, not behavior guesses):
//   - The host is a stock Canvas, not C#'s ContentPanel (whose border/clip path geometry lives in the
//     deferred border fan-out); IsHitTestVisible is the Canvas default (true), matching C#'s ctor.
//   - map_prefers_status_bar_hidden / map_home_indicator_auto_hidden are iOS-specific Page knobs
//     (IiOSPageSpecifics); like the headless twin they only count the request on this backend.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors (hosted_content + the
// view_platform_base mirrors) are ALWAYS maintained so that suite observes exactly the headless
// partial's behavior.

#include "maui/core/content_page_handler.hpp"

#include <memory>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/base.h>

#include "maui/core/i_content_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace wnative = maui::platform::win;

    // The content's native FrameworkElement, via its view-handler's native_view() (C#'s ToPlatform() =
    // ContainerView ?? PlatformView). Mirrors the apple twin's native_child / the android helper.
    [[nodiscard]] void* content_native_view(maui::core::i_view& content)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(content.handler().get());
        return handler != nullptr ? handler->native_view() : nullptr;
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the Canvas host (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSView here).
    content_page_platform::~content_page_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real Canvas when one exists.

    void content_page_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void content_page_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void content_page_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void content_page_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto panel = wnative::borrow<muxc::Canvas>(native);
        if (panel == nullptr)
        {
            return;
        }
        // ViewExtensions.UpdatePlatformViewBackground's Panel branch: panel.Background =
        // paint.ToPlatform(); null clears the value.
        if (value == nullptr)
        {
            panel.ClearValue(muxc::Panel::BackgroundProperty());
            return;
        }
        // Paint.ToPlatform: solid + linear/radial gradient (to_paint_brush); image/pattern still fall back to solid.
        panel.Background(wnative::to_paint_brush(value));
        return;
        // deferred: gradient / image-source paints (Paint.ToPlatform's LinearGradientBrush et al.) —
        // the base mirror above keeps the borrow observable.
    }

    std::unique_ptr<content_page_platform> content_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<content_page_platform>();
        try
        {
            // ContentViewHandler.CreatePlatformView: new ContentPanel { IsHitTestVisible = true } — the
            // port's manual-frame Canvas host (header deviation).
            const muxc::Canvas host;
            platform->native = wnative::store(host); // released in ~content_page_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void content_page_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C#'s UpdateContent reads
        // VirtualView.PresentedContent) — the XAML-less cross-platform suite observes it.
        i_view* content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        platform->hosted_content = content;

        // The real single-child re-host (when the XAML runtime + the Canvas exist): C# UpdateContent —
        // CachedChildren.Clear() + Add(content.ToPlatform(mauiContext)).
        if (platform->native == nullptr)
        {
            return;
        }
        auto host = wnative::borrow<muxc::Canvas>(platform->native);
        if (host == nullptr)
        {
            return;
        }
        host.Children().Clear();
        if (content == nullptr)
        {
            return;
        }
        if (auto element = wnative::borrow_as<mux::UIElement>(content_native_view(*content)))
        {
            host.Children().Append(element);
        }
    }

    maui::graphics::size content_page_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        // A content view computes its own size through the control (which ports MeasureContent measuring
        // the content within the padding), not the handler — the headless/apple/android twins' shape.
        return {0, 0};
    }

    void content_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native host to position
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the host to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }

    // --- platform configuration (W2-24): the iOSSpecific Page knob nudges — count the appearance-update
    // request like the headless twin (C#'s SetNeedsStatusBarAppearanceUpdate /
    // SetNeedsUpdateOfHomeIndicatorAutoHidden have no WinUI object; only the iOS twin pokes the real
    // UIViewController).
    void content_page_handler::map_prefers_status_bar_hidden(content_page_handler& handler, i_content_view& /*view*/)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            ++platform->status_bar_appearance_requests;
        }
    }

    void content_page_handler::map_home_indicator_auto_hidden(content_page_handler& handler, i_content_view& /*view*/)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            ++platform->home_indicator_requests;
        }
    }

    // W2-24: nothing to wire — only the iOS twin needs the host→handler backref (safe-area push).
    void content_page_handler::on_connect_handler(content_page_platform& /*platform*/)
    {
    }

    void content_page_handler::on_disconnect_handler(content_page_platform& /*platform*/)
    {
    }
} // namespace maui::core
