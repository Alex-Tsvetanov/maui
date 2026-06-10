// haptic_feedback - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors HapticFeedback's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_haptic_feedback.mm.

#include "maui/essentials/haptic_feedback.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::devices::detail
{
    std::shared_ptr<i_haptic_feedback> make_haptic_feedback()
    {
        return std::make_shared<headless_haptic_feedback>();
    }
} // namespace maui::devices::detail
