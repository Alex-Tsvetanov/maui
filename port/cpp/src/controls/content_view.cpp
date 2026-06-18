// maui::controls::content_view — out-of-line part: the destructor (the vtable's key function, anchoring
// this TU; see the header) and the default-handler self-registration. The control is otherwise
// header-only over the merged templated_view base (content_view.hpp); its handler is the SAME
// content_page_handler ported from C#'s ContentViewHandler — content_page and content_view are siblings
// over one handler, exactly as both C# controls resolve to ContentViewHandler.

#include "maui/controls/content_view.hpp"

#include "maui/core/bindable_property.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/safe_area_edges.hpp"
#include "maui/core/safe_area_regions.hpp"
#include "maui/core/thickness.hpp"

namespace maui::controls
{
    content_view::~content_view() = default;

    const maui::core::bindable_property<maui::core::safe_area_edges>& content_view::safe_area_edges_property()
    {
        // C# ContentView.SafeAreaEdgesProperty default is SafeAreaEdges.None.
        static const maui::core::bindable_property<maui::core::safe_area_edges> descriptor{
            "safe_area_edges", maui::core::safe_area_edges::none()};
        return descriptor;
    }

    void content_view::set_safe_area_insets(const maui::core::thickness& value)
    {
        (void)value;
    }

    maui::core::safe_area_regions content_view::get_safe_area_regions_for_edge(int edge) const
    {
        const auto r = safe_area_edges_.get().edge(edge);
        if (r == maui::core::safe_area_regions::default_value)
        {
            return maui::core::safe_area_regions::none;
        }
        return r;
    }
} // namespace maui::controls

// Self-register the default handler for content_view (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::content_view, maui::core::content_page_handler)
