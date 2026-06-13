// connectivity - iOS (UIKit) platform partial: the shared Network.framework implementation
// (essentials_connectivity_path_monitor.hpp, the Connectivity.ios.tvos.macos.cs twin). Compiled as
// Objective-C++ with ARC for the ios backend. Identical to the apple partial - NWPathMonitor is the
// same Network.framework API on both, so both backends share the one path_monitor_connectivity.

#include <memory>

#include "maui/essentials/connectivity.hpp"

#include "src/platform/apple_shared/essentials_connectivity_path_monitor.hpp"

namespace maui::networking::detail
{
    std::shared_ptr<i_connectivity> make_connectivity()
    {
        return std::make_shared<apple_shared::path_monitor_connectivity>();
    }
} // namespace maui::networking::detail
