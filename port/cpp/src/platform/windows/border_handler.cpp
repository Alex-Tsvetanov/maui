// border_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.Canvas host
// (the port's manual-frame content host, the content_page twin) carrying a child
// Microsoft.UI.Xaml.Controls.Border as the stroke/fill CHROME layer, plus the single content child
// appended after it. The windows twin of src/platform/apple/border_handler.mm (NSView host +
// CAShapeLayer stroke) / the android MauiLayout + GradientDrawable partial, and the real-native
// sibling of the headless single-content + stroke-spec mirror (src/platform/headless/border_handler.cpp).
//
// Ported from BorderHandler.cs + BorderHandler.Windows.cs (whose ContentPanel hosts the content AND
// draws the border path) + the StrokeExtensions funnel (every stroke map → one refresh →
// update_border here).
//
// DOCUMENTED DEVIATIONS (infrastructure gaps of this first cut, not behavior guesses):
//   - C#'s ContentPanel draws the border along ANY StrokeShape path through a Path/geometry pipeline.
//     The port expresses the border through a stock winrt Border element pinned behind the content:
//     stroke color/thickness ride BorderBrush/BorderThickness, the StrokeShape's rounded-rect corners
//     ride CornerRadius (per-corner, read off the port's round_rectangle shape; a plain rectangle is
//     radius 0). Non-rectangular StrokeShapes (Ellipse / Polygon / Path), dash patterns, caps, joins
//     and miter limits are // deferred — the border_stroke_spec mirror still carries them (the
//     headless contract), the chrome just cannot trace them (the windows analog of the android
//     GradientDrawable lossiness; a faithful cut needs the Path-geometry chrome).
//   - The chrome Border does NOT host the content (Border.Child would inset it by
//     BorderThickness+Padding, double-applying what the cross-platform Border control already
//     arranges): the control computes the stroke+padding inset itself and arranges the content
//     HOST-RELATIVE (src/controls/border.cpp), so the content's own platform_arrange lands it at the
//     right Canvas coordinates inside the host. For the same reason IView.Padding is NOT pushed to
//     any native Padding property.
//   - The generic IView background is pushed onto the CHROME Border's Background (not the Canvas), so
//     the fill respects the corner radius — mirroring C# painting the background along the border path.
//   - Content clipping to the border shape (ContentPanel's clip geometry) is // deferred.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native/chrome null, while the headless mirrors (hosted_content +
// the border_stroke_spec + the view_platform_base mirrors) are ALWAYS maintained so that suite
// observes exactly the headless partial's behavior.

#include "maui/core/border_handler.hpp"

#include <memory>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/base.h>

#include "maui/core/i_border_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/shapes/rectangle.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace wnative = maui::platform::win;

    // The content's native FrameworkElement, via its view-handler's native_view() (C#'s ToPlatform() =
    // ContainerView ?? PlatformView). Mirrors the content_page windows partial's helper.
    [[nodiscard]] void* content_native_view(maui::core::i_view& content)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(content.handler().get());
        return handler != nullptr ? handler->native_view() : nullptr;
    }

    // The StrokeShape's per-corner radii for the chrome Border's CornerRadius. The port owns the
    // round_rectangle shape a Border almost always carries, so the exact four values are read straight
    // off it; a plain rectangle (or an absent shape) is square. Any OTHER shape kind (Ellipse /
    // Polygon / Path) returns square too — the chrome cannot trace it (header deviations; // deferred:
    // the Path-geometry chrome).
    [[nodiscard]] maui::graphics::corner_radius corner_radii_of(const maui::graphics::i_shape* shape)
    {
        if (const auto* rounded = dynamic_cast<const maui::graphics::shapes::round_rectangle*>(shape))
        {
            return rounded->corner_radius();
        }
        return {}; // rectangle / null / non-rect shapes → sharp corners
    }
} // namespace

namespace maui::core
{
    // Releases the strong refs pinning the Canvas host + the chrome Border (the wnative shape of the
    // pimpl-owned-native doctrine; the apple twin CFReleases its NSView host here).
    border_platform::~border_platform()
    {
        wnative::release(chrome);
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real elements when they exist.

    void border_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void border_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void border_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void border_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto chrome_border = wnative::borrow<muxc::Border>(chrome);
        if (chrome_border == nullptr)
        {
            return;
        }
        // The border's background paints the CHROME Border (not the Canvas host) so the fill rounds
        // with the corner radius — the windows expression of C# painting the fill along the border
        // path (header deviations). Null clears the value.
        if (value == nullptr)
        {
            chrome_border.ClearValue(muxc::Border::BackgroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            chrome_border.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform) — the base mirror above keeps
        // the borrow observable.
    }

    std::unique_ptr<border_platform> border_handler::create_platform_view()
    {
        auto platform = std::make_unique<border_platform>();
        try
        {
            // BorderHandler.Windows.CreatePlatformView: new ContentPanel { CrossPlatformLayout = ... }
            // — the port's Canvas host + the chrome Border stand-in (header deviations). The chrome is
            // child 0 (renders BEHIND the content, which never overlaps the stroke — the control
            // insets the content inside it) and never hit-tests (it is pure chrome).
            const muxc::Canvas host;
            const muxc::Border chrome_border;
            chrome_border.IsHitTestVisible(false);
            host.Children().Append(chrome_border);
            platform->native = wnative::store(host);          // released in ~border_platform
            platform->chrome = wnative::store(chrome_border); // released in ~border_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
            platform->chrome = nullptr;
        }
        return platform;
    }

    void border_handler::set_content()
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
        // CachedChildren.Clear() + Content = content.ToPlatform(). The chrome layer is re-appended
        // first so it stays child 0 (behind the content).
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
        if (auto chrome_border = wnative::borrow<muxc::Border>(platform->chrome))
        {
            host.Children().Append(chrome_border);
        }
        if (content == nullptr)
        {
            return;
        }
        if (auto element = wnative::borrow_as<mux::UIElement>(content_native_view(*content)))
        {
            host.Children().Append(element);
        }
    }

    void border_handler::update_border()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        // The headless mirror is ALWAYS maintained (C# UpdateMauiCALayer reads the full IBorderStroke
        // surface each refresh) — the XAML-less cross-platform suite asserts on it.
        platform->border = make_border_stroke_spec(*virtual_view());

        auto chrome_border = wnative::borrow<muxc::Border>(platform->chrome);
        if (chrome_border == nullptr)
        {
            return; // XAML-less: the spec mirror above is the whole push
        }
        const border_stroke_spec& spec = platform->border;
        // Stroke brush: a null IBorderStroke.Stroke draws no outline (C#'s null-paint branch) —
        // ClearValue restores the Border default (no brush → no stroke).
        if (spec.has_stroke)
        {
            chrome_border.BorderBrush(wnative::to_brush(spec.stroke_color));
        }
        else
        {
            chrome_border.ClearValue(muxc::Border::BorderBrushProperty());
        }
        // StrokeThickness → a uniform BorderThickness (negative/unset → 0, the C# GetStrokeProperties
        // clamp).
        const double thickness_value = spec.thickness > 0 ? spec.thickness : 0.0;
        chrome_border.BorderThickness(wnative::to_thickness(thickness{thickness_value}));
        // The StrokeShape's rounded-rect corners → CornerRadius (per-corner; the port's corner_radius
        // order is TL,TR,BL,BR — winrt's struct is TL,TR,BR,BL). Non-rect shapes stay sharp (header
        // deviations; // deferred: the Path-geometry chrome for Ellipse/Polygon/Path StrokeShapes).
        const maui::graphics::corner_radius radii = corner_radii_of(spec.shape);
        chrome_border.CornerRadius(mux::CornerRadius{.TopLeft = radii.top_left,
                                                     .TopRight = radii.top_right,
                                                     .BottomRight = radii.bottom_right,
                                                     .BottomLeft = radii.bottom_left});
        // deferred: dash pattern / dash offset / line cap / line join / miter limit — the spec mirror
        // carries them; a stock winrt Border cannot trace them (header deviations).
    }

    void border_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native host to position
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the host to the
        // frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
        // The chrome layer fills the host (the border path spans the full bounds — C# draws it over
        // the panel's own size). The size-change-driven stroke re-push happens in the cross-platform
        // platform_arrange (border_handler.cpp's _lastSize gate → update_border).
        if (platform->chrome != nullptr)
        {
            wnative::arrange_native(platform->chrome, maui::graphics::rect{0, 0, frame.width, frame.height});
        }
    }
} // namespace maui::core
