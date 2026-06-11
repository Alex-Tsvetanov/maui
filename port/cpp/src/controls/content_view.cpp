// maui::controls::content_view — out-of-line part: the destructor (the vtable's key function, anchoring
// this TU; see the header) and the default-handler self-registration. The control is otherwise
// header-only over the merged templated_view base (content_view.hpp); its handler is the SAME
// content_page_handler ported from C#'s ContentViewHandler — content_page and content_view are siblings
// over one handler, exactly as both C# controls resolve to ContentViewHandler.

#include "maui/controls/content_view.hpp"

#include "maui/core/content_page_handler.hpp"
#include "maui/core/handler_registry.hpp"

namespace maui::controls
{
    content_view::~content_view() = default;
} // namespace maui::controls

// Self-register the default handler for content_view (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::content_view, maui::core::content_page_handler)
