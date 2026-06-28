#pragma once
// maui/ui.hpp — the curated consumer-facing facade for the framework (the `maui::ui` surface).
//
// A single umbrella that (a) pulls in the common control/value headers a consumer needs and (b) re-exports
// the names they actually spell, so app code says `ui::content_page` / `ui::colors::blue` / `ui::thickness`
// instead of the deep `maui::controls::` / `maui::graphics::` / `maui::core::` namespaces. This header
// defines NO new types itself — every type lives in its own header (PROFILE §3); it only `using`-aliases
// existing entities (zero runtime cost, zero ODR risk). It is the deliberate public facade called for in
// PUBLIC_API_DESIGN.md (§3-I), not a re-export shim.
//
// Naming: full snake_case names (ui::vertical_stack_layout) are aliased here as TYPES. The TERSE builder
// names (ui::label(...), ui::button(...), and later ui::vstack(...)) are FUNCTIONS that return an owning
// view_ref<T> (maui/ui/builder.hpp); they are deliberately NOT type aliases so the name stays free for the
// builder. The handle types ui::view_ref<T> / ui::weak_ref<T> live in maui/ui/view_ref.hpp.

// This is a curated facade: every include below is RE-EXPORTED, so a consumer that includes "maui/ui.hpp"
// is treated by include-what-you-use / clang-tidy's misc-include-cleaner as having the re-exported symbols
// directly available (the whole point of the umbrella). The pragma is the standard facade annotation — it
// is not a lint suppression.
// IWYU pragma: begin_exports
#include "maui/controls/application.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/event.hpp"
#include "maui/core/observable.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/ui/app.hpp"
#include "maui/ui/bind.hpp"
#include "maui/ui/builder.hpp"
#include "maui/ui/view_ref.hpp"
// IWYU pragma: end_exports

namespace maui::ui
{
    // ---- app base ---- (maui::ui::app is defined in maui/ui/app.hpp)

    // ---- controls / layouts / pages: full snake_case names (1:1 with maui::controls) ----
    // Note: button / label / entry / grid / vstack / hstack are intentionally NOT aliased — those names
    // are reserved for the owning-builder FUNCTIONS (P4). Their types remain maui::controls::* meanwhile.
    using maui::controls::application;
    using maui::controls::content_page;
    using maui::controls::horizontal_stack_layout;
    using maui::controls::vertical_stack_layout;
    using maui::controls::window;

    // ---- graphics ----
    using maui::graphics::color;
    namespace colors = maui::graphics::colors;

    // ---- core value types + signal primitives ----
    using maui::core::binding_mode;
    using maui::core::connect_scoped;
    using maui::core::event;
    using maui::core::observable; // the one-member bindable VM property (ui::observable<T>)
    using maui::core::scoped_connection;
    using maui::core::thickness;
} // namespace maui::ui
