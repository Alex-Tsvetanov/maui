// content_page_handler — WinUI 3 platform recipe: a Microsoft.UI.Xaml.Controls.Canvas hosting the page's
// single content element. The real-native twin of the headless partial.
//
// Why a Canvas and not a ContentPresenter (which is the closer conceptual match): the port arranges the
// page's content itself and hands it an absolute-in-parent frame, so the host must NOT impose its own
// layout — the same reason layout_handler uses a Canvas. A ContentPresenter would re-measure and
// re-position the child and fight the cross-platform arrange.

#include "maui/core/content_page_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <memory>

#include "maui/core/i_content_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using canvas = winui::Controls::Canvas;

    canvas as_host(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<canvas>();
    }

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
} // namespace

namespace maui::core
{
    content_page_platform::~content_page_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<content_page_platform> content_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<content_page_platform>();
        platform->native = maui::platform::windows::take<winui::UIElement>(canvas{});
        return platform;
    }

    void content_page_handler::set_content()
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
        // Single-content host: replace whatever is there rather than appending, so a re-set (the mapper
        // re-runs on every Content change) cannot stack two generations of content on top of each other.
        const canvas host = as_host(platform->native);
        host.Children().Clear();
        if (const winui::UIElement element = native_of(platform->hosted_content))
        {
            host.Children().Append(element);
        }
    }

    maui::graphics::size content_page_handler::get_desired_size(double /*width_constraint*/,
                                                                double /*height_constraint*/) const
    {
        // The control measures its content within its padding, not the handler — same on every backend.
        return {0, 0};
    }

    void content_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const canvas host = as_host(platform->native);
        canvas::SetLeft(host, frame.x);
        canvas::SetTop(host, frame.y);
        host.Width(frame.width);
        host.Height(frame.height);
    }

    // --- platform configuration: the iOSSpecific Page knobs. Windows has no status bar and no home
    // indicator, so these stay counters exactly as on headless — C# maps them on iOS only.
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

    void content_page_handler::on_connect_handler(content_page_platform& /*platform*/)
    {
    }

    void content_page_handler::on_disconnect_handler(content_page_platform& /*platform*/)
    {
    }
} // namespace maui::core
