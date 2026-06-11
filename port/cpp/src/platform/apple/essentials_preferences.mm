// preferences - Apple (AppKit / macOS) platform partial: the shared NSUserDefaults
// implementation (essentials_user_defaults.hpp, the Preferences.ios.tvos.watchos.macos.cs twin).
// Compiled as Objective-C++ with ARC for the apple backend.

#include <memory>

#include "maui/essentials/preferences.hpp"

#include "src/platform/apple_shared/essentials_user_defaults.hpp"

namespace maui::storage::detail
{
    std::shared_ptr<i_preferences> make_preferences()
    {
        return std::make_shared<apple_shared::user_defaults_preferences>();
    }
} // namespace maui::storage::detail
