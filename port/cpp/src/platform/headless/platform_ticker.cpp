// The headless backend's platform ticker  <=  PlatformTicker.Standard.cs (which is just the base
// Ticker). The deterministic manual_ticker over the supplied dispatcher: with the headless
// manual_dispatcher, tests pump ticks via dispatcher.advance(...). See
// include/maui/animations/platform_ticker.hpp.
#include "maui/animations/platform_ticker.hpp"

#include <memory>

#include "maui/animations/manual_ticker.hpp"

namespace maui::animations
{
    std::shared_ptr<ticker> create_platform_ticker(maui::core::i_dispatcher& dispatcher)
    {
        return std::make_shared<manual_ticker>(dispatcher);
    }
} // namespace maui::animations
