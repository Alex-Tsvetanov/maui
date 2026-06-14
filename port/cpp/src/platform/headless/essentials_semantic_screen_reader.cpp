// semantic_screen_reader - headless platform partial: the backend's lazy default is the controllable
// fake (essentials_comms_fakes.hpp), which unconfigured mirrors SemanticScreenReader's netstandard
// partial. The real-device twin is src/platform/ios/essentials_semantic_screen_reader.mm; macOS is
// NOT SUPPORTED (src/platform/apple/essentials_semantic_screen_reader.mm throws).

#include "maui/essentials/semantic_screen_reader.hpp"

#include <memory>

#include "src/platform/headless/essentials_comms_fakes.hpp"

namespace maui::accessibility::detail
{
    std::shared_ptr<i_semantic_screen_reader> make_semantic_screen_reader()
    {
        return std::make_shared<headless_semantic_screen_reader>();
    }
} // namespace maui::accessibility::detail
