// scroll_view_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.ScrollViewer whose
// Content hosts a Canvas panel, which in turn hosts the content child's native view. The real-native
// twin of the headless partial, ported from ScrollViewHandler.Windows.cs + ScrollViewerExtensions.cs.
//
// LAYOUT SEAM DECISION: the port runs its OWN cross-platform measure/arrange
// (maui::controls::scroll_view::arrange), which positions the content child HOST-RELATIVE inside the
// scroll view's own coordinate space — the same convention layout_handler's Canvas panel and
// content_page_handler's Canvas host already rely on (Canvas.Left/Top + Width/Height stamped by the
// CHILD's own platform_arrange, not by this panel). A bare ScrollViewer cannot host that child directly
// AND provide a scrollable extent: ScrollViewer.Content is a ContentControl slot (one child), and a
// Canvas positions its children absolutely WITHOUT sizing itself to them (Canvas::MeasureOverride
// returns (0,0) regardless of children) — the "fixed-size Canvas child gives the ScrollViewer nothing to
// scroll" trap. So the seam is:
//   ScrollViewer.Content = a Canvas panel (the same single-child-host shape as content_page_handler),
//   and platform_arrange stamps the PANEL's own Width/Height (not just the child's) to
//   max(content_size(), viewport frame) on every arrange pass — the WinUI expression of the
//   MauiScrollView LayoutSubviews -> ContentSize push every other backend does structurally (Apple's
//   NSScrollView reads its documentView's own frame directly; here the ScrollViewer reads the panel's
//   Width/Height instead). ScrollableWidth/Height then falls out of native ScrollViewer arithmetic
//   (ExtentWidth - ViewportWidth), matching the headless/Apple `max(content, frame) - frame` clamp
//   arithmetic in scroll_to exactly.
//
// NOT ported: HorizontalScrollMode/VerticalScrollMode. Neither oracle file (ScrollViewHandler.Windows.cs
// nor ScrollViewerExtensions.cs) touches ScrollMode — only the ScrollBarVisibility pair — so this partial
// leaves it at the WinUI default rather than inventing a mapping. If a future capture shows Horizontal/
// Both-oriented scrolling not actually dragging on Windows, that is the first place to look.

#include "maui/core/scroll_view_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string_view>

#include "maui/core/i_scroll_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/scroll_bar_visibility.hpp"
#include "maui/core/scroll_orientation.hpp"
#include "maui/core/scroll_to_request.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using scroll_viewer = winui::Controls::ScrollViewer;
    using canvas = winui::Controls::Canvas;

    scroll_viewer as_scroll_viewer(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<scroll_viewer>();
    }

    // The Canvas panel inside the ScrollViewer's Content slot (created in create_platform_view).
    // ScrollViewer derives from ContentControl, so Content() reads back as `object`/IInspectable — the
    // same reason button_handler's content_text() needs `.as<>()` rather than a direct UIElement read
    // (unlike Border.Child, which IS typed UIElement — see label_handler's as_text_block).
    canvas as_panel(void* native)
    {
        return as_scroll_viewer(native).Content().as<canvas>();
    }

    // The child's native UIElement, via its view-handler's native_view() — the same helper every
    // single-content host partial defines (content_page_handler.cpp's native_of, layout_handler.cpp's
    // native_child). Null when the child is unattached or its handler has no native view yet.
    winui::UIElement native_of(maui::core::i_view* view)
    {
        if (view == nullptr)
        {
            return nullptr;
        }
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(view->handler().get());
        if (handler == nullptr || handler->native_view() == nullptr)
        {
            return nullptr;
        }
        return maui::platform::windows::ref<winui::UIElement>(handler->native_view());
    }

    winui::Controls::ScrollBarVisibility to_windows_scroll_bar_visibility(maui::core::scroll_bar_visibility value)
    {
        switch (value)
        {
            case maui::core::scroll_bar_visibility::always:
                return winui::Controls::ScrollBarVisibility::Visible;
            case maui::core::scroll_bar_visibility::never:
                return winui::Controls::ScrollBarVisibility::Hidden;
            case maui::core::scroll_bar_visibility::default_:
            default:
                return winui::Controls::ScrollBarVisibility::Auto;
        }
    }

    // ScrollViewerExtensions.UpdateScrollBarVisibility, ported 1:1 — INCLUDING the fact that
    // MapOrientation / MapHorizontalScrollBarVisibility / MapVerticalScrollBarVisibility each pass a
    // DIFFERENT single `visibility` argument that this function then applies to BOTH bars wherever the
    // orientation allows (see update_orientation's ternary for the one MapOrientation computes). That
    // asymmetry — one property's setting can echo onto the OTHER bar — is the oracle's own behavior, not
    // a port simplification. Neither == both bars Disabled, full stop.
    void apply_scroll_bar_visibility(const scroll_viewer& viewer, maui::core::scroll_orientation orientation,
                                     maui::core::scroll_bar_visibility visibility)
    {
        using maui::core::scroll_bar_visibility;
        using maui::core::scroll_orientation;
        if (orientation == scroll_orientation::neither)
        {
            viewer.HorizontalScrollBarVisibility(winui::Controls::ScrollBarVisibility::Disabled);
            viewer.VerticalScrollBarVisibility(winui::Controls::ScrollBarVisibility::Disabled);
            return;
        }
        const bool scrolls_horizontally =
            orientation == scroll_orientation::horizontal || orientation == scroll_orientation::both;
        const bool scrolls_vertically =
            orientation == scroll_orientation::vertical || orientation == scroll_orientation::both;
        if (visibility == scroll_bar_visibility::default_)
        {
            viewer.HorizontalScrollBarVisibility(scrolls_horizontally ? winui::Controls::ScrollBarVisibility::Auto
                                                                      : winui::Controls::ScrollBarVisibility::Disabled);
            viewer.VerticalScrollBarVisibility(scrolls_vertically ? winui::Controls::ScrollBarVisibility::Auto
                                                                  : winui::Controls::ScrollBarVisibility::Disabled);
        }
        else
        {
            const winui::Controls::ScrollBarVisibility native_value = to_windows_scroll_bar_visibility(visibility);
            viewer.HorizontalScrollBarVisibility(scrolls_horizontally ? native_value
                                                                      : winui::Controls::ScrollBarVisibility::Disabled);
            viewer.VerticalScrollBarVisibility(scrolls_vertically ? native_value
                                                                  : winui::Controls::ScrollBarVisibility::Disabled);
        }
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // Unhook the ViewChanged subscription. Called from on_disconnect_handler AND from
        // ~scroll_view_platform (button_platform's exact discipline — see the header field comment): the
        // native lambda captures ONLY this platform struct, so it is safe to detach from either path, and
        // a platform torn down without a disconnect (the element tree does this on shutdown) still
        // revokes the subscription before its own destruction completes.
        void detach_view_changed(scroll_view_platform& platform)
        {
            if (platform.native != nullptr)
            {
                as_scroll_viewer(platform.native).ViewChanged(winrt::event_token{platform.view_changed_token});
            }
            platform.view_changed_token = 0;
            platform.scrolled_view = nullptr;
        }
    } // namespace

    scroll_view_platform::~scroll_view_platform()
    {
        detach_view_changed(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<scroll_view_platform> scroll_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<scroll_view_platform>();
        scroll_viewer viewer;
        // See the layout-seam decision in this file's header comment: the Canvas panel is the single
        // child every content-host partial hosts (content_page_handler's Canvas, layout_handler's Canvas),
        // just wrapped in the ScrollViewer's Content slot instead of a plain page/layout host.
        canvas panel;
        viewer.Content(panel);
        platform->native = maui::platform::windows::take<winui::UIElement>(viewer);
        return platform;
    }

    // Wire the scrolled write-back (the ScrollEventProxy.Connect twin): ScrollViewer.ViewChanged fires on
    // every offset change, intermediate (mid-drag/animation) or final.
    void scroll_view_handler::on_connect_handler(scroll_view_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        platform.scrolled_view = virtual_view();
        auto* self = &platform;
        platform.view_changed_token =
            as_scroll_viewer(platform.native)
                .ViewChanged([self](const winrt::Windows::Foundation::IInspectable&,
                                    const winui::Controls::ScrollViewerViewChangedEventArgs& args) {
                    const scroll_viewer viewer = as_scroll_viewer(self->native);
                    self->offset_x = viewer.HorizontalOffset();
                    self->offset_y = viewer.VerticalOffset();
                    if (self->scrolled_view == nullptr)
                    {
                        return;
                    }
                    // The platform write-back (ScrollEventProxy.Scrolled): the virtual offsets follow the
                    // native ones, raising the control's Scrolled event.
                    self->scrolled_view->set_horizontal_offset(self->offset_x);
                    self->scrolled_view->set_vertical_offset(self->offset_y);
                    if (!args.IsIntermediate())
                    {
                        self->scrolled_view->scroll_finished();
                    }
                })
                .value;
    }

    void scroll_view_handler::on_disconnect_handler(scroll_view_platform& platform)
    {
        detach_view_changed(platform);
    }

    void scroll_view_handler::set_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->hosted_content = virtual_view() != nullptr ? virtual_view()->content() : nullptr;
        if (platform->native == nullptr)
        {
            return;
        }
        // Single-content host: replace whatever is there rather than appending (content_page_handler's
        // exact recipe), so a re-set (the mapper re-runs on every Content change) cannot stack two
        // generations of content on top of each other.
        const canvas panel = as_panel(platform->native);
        panel.Children().Clear();
        if (const winui::UIElement element = native_of(platform->hosted_content))
        {
            panel.Children().Append(element);
        }
    }

    void scroll_view_handler::update_orientation()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->orientation = virtual_view()->orientation();
        // ScrollViewHandler.Windows.MapOrientation: pick ONE of the two bar-visibility values as the
        // shared `visibility` argument — horizontal's when Orientation is EXACTLY Horizontal, vertical's
        // for every other orientation (including Both/Vertical/Neither). Ported 1:1; see
        // apply_scroll_bar_visibility's comment for why this is faithful, not a simplification.
        const i_scroll_view& view = *virtual_view();
        const scroll_bar_visibility visibility = platform->orientation == scroll_orientation::horizontal
                                                     ? view.horizontal_scroll_bar_visibility()
                                                     : view.vertical_scroll_bar_visibility();
        apply_scroll_bar_visibility(as_scroll_viewer(platform->native), platform->orientation, visibility);
    }

    void scroll_view_handler::update_horizontal_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->horizontal_bar_visibility = virtual_view()->horizontal_scroll_bar_visibility();
        apply_scroll_bar_visibility(as_scroll_viewer(platform->native), virtual_view()->orientation(),
                                    platform->horizontal_bar_visibility);
    }

    void scroll_view_handler::update_vertical_scroll_bar_visibility()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || virtual_view() == nullptr)
        {
            return;
        }
        platform->vertical_bar_visibility = virtual_view()->vertical_scroll_bar_visibility();
        apply_scroll_bar_visibility(as_scroll_viewer(platform->native), virtual_view()->orientation(),
                                    platform->vertical_bar_visibility);
    }

    // C# MapRequestScrollTo: clamp the target to the native ScrollableWidth/Height, skip (just
    // acknowledging ScrollFinished) if already there, else ChangeView.
    void scroll_view_handler::scroll_to(const scroll_to_request& request)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || platform->native == nullptr || view == nullptr)
        {
            return;
        }
        platform->scroll_requests.push_back(request);

        const scroll_viewer viewer = as_scroll_viewer(platform->native);
        const double target_x = std::clamp(request.horizontal_offset, 0.0, viewer.ScrollableWidth());
        const double target_y = std::clamp(request.vertical_offset, 0.0, viewer.ScrollableHeight());

        if (target_y == viewer.VerticalOffset() && target_x == viewer.HorizontalOffset())
        {
            view->scroll_finished();
            return;
        }
        // The two offset arguments implicitly box into IReference<double> (the WinRT nullable-value
        // pattern, the same implicit boxing C++/WinRT projects for every ChangeView-style optional
        // parameter); the null zoomFactor leaves the current zoom untouched, matching C#'s `null` argument.
        viewer.ChangeView(target_x, target_y, nullptr, request.instant);
    }

    void scroll_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite. C# only tests `< 0` because its
        // cross-platform arrange never yields NaN; if one ever reaches XAML here it is an unrecoverable
        // stowed exception with no message and no stack (0xC000027B), so a skipped arrange is strictly
        // better than a dead process. A NaN arriving here is an upstream layout bug worth chasing, not
        // a value with a meaning.
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const scroll_viewer viewer = as_scroll_viewer(platform->native);
        canvas::SetLeft(viewer, frame.x);
        canvas::SetTop(viewer, frame.y);
        viewer.Width(frame.width);
        viewer.Height(frame.height);

        // LAYOUT SEAM (see this file's header comment): stamp the PANEL's own Width/Height to the larger
        // of the port's freshly-computed ContentSize and this viewport frame — the WinUI expression of
        // the MauiScrollView LayoutSubviews -> ContentSize push. `max(content, frame) - frame` is exactly
        // the same clamp the headless/Apple partials use for `available_x`/`available_y` in scroll_to, so
        // the native ScrollableWidth/Height that falls out of it agrees with every other backend.
        const auto* view = virtual_view();
        const maui::graphics::size content = view != nullptr ? view->content_size() : maui::graphics::size{};
        const double panel_width = std::isfinite(content.width) ? std::max(content.width, frame.width) : frame.width;
        const double panel_height =
            std::isfinite(content.height) ? std::max(content.height, frame.height) : frame.height;
        const canvas panel = as_panel(platform->native);
        panel.Width(panel_width);
        panel.Height(panel_height);
        // Clip is bounds-dependent (view_chrome_ops.cpp's apply_native_clip reads the just-set Width/
        // Height back); map_clip's own push (view_mapper.cpp) always runs before the first arrange, so
        // this re-invoke is what actually installs the clip once the scroll view has a real size. The
        // clip masks the VIEWPORT (the ScrollViewer `native` boxes), not the (larger, scrollable) content
        // panel above — matching WrapperView wrapping the ScrollView control itself, not its content.
        if (view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so all five controls behave identically;
    // see that header for why they are free functions taking the void* slot.
    void scroll_view_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void scroll_view_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void scroll_view_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void scroll_view_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void scroll_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
