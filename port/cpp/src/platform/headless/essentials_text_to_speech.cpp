// text_to_speech - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors TextToSpeech's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_text_to_speech.mm.

#include "maui/essentials/text_to_speech.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::media::detail
{
    std::shared_ptr<i_text_to_speech> make_text_to_speech()
    {
        return std::make_shared<headless_text_to_speech>();
    }
} // namespace maui::media::detail
