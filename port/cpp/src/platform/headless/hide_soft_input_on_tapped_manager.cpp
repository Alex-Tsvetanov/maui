// hide_soft_input_on_tapped_manager — headless backend hook. The headless backend has no native windowing
// or soft keyboard, so the ONE backend-specific step (setup_native — the native gesture wiring) returns an
// empty cleanup token: nothing is ever armed, FeatureEnabled stays false (no page is tracked), and
// UpdatePage / UpdateFocusForView are observably no-ops while the shared cross-platform bookkeeping
// (src/core/hide_soft_input_on_tapped_manager.cpp) still compiles and runs. The real wiring is the iOS
// partial (src/platform/ios/hide_soft_input_on_tapped_manager.mm).

#include "maui/platform/ios/hide_soft_input_on_tapped_manager.hpp"

#include <functional>

namespace maui::platform::ios
{
    std::function<void()> hide_soft_input_on_tapped_manager::setup_native(maui::core::i_view& /*focused_view*/)
    {
        return {}; // Headless has no soft keyboard — nothing to wire.
    }
} // namespace maui::platform::ios
