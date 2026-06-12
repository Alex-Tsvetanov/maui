// maui::controls::shapes::ellipse — the default-handler self-registration (the shared
// shape_view_handler; the control itself is header-only). See ellipse.hpp.

#include "maui/controls/shapes/ellipse.hpp"

#include "maui/core/handler_registry.hpp"
#include "maui/core/shape_view_handler.hpp"

// Self-register the shared shape handler for ellipse (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::shapes::ellipse, maui::core::shape_view_handler)
