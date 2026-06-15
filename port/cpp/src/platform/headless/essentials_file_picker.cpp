// file_picker - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_fakes.hpp), which unconfigured mirrors FilePicker's netstandard partial (PlatformPickAsync
// throws). The real-device twins are src/platform/{apple,ios}/essentials_file_picker.mm (a service seam
// - the document-picker UI is not drivable in the spawned sim process).
//
// The predefined file_picker_file_type statics (images/png/jpeg/videos/pdf) are defined here as EMPTY
// registries - the netstandard mirror, where Platform*FileType() throws NotImplementedInReferenceAssembly;
// here value() throws feature_not_supported because no platform entry is configured (no try_get hit).

#include "maui/essentials/file_picker.hpp"

#include <memory>

#include "src/platform/headless/essentials_fakes.hpp"

namespace maui::storage
{
    // netstandard mirror: empty registries -> value() throws on the current platform.
    const file_picker_file_type file_picker_file_type::images{};
    const file_picker_file_type file_picker_file_type::png{};
    const file_picker_file_type file_picker_file_type::jpeg{};
    const file_picker_file_type file_picker_file_type::videos{};
    const file_picker_file_type file_picker_file_type::pdf{};

    namespace detail
    {
        std::shared_ptr<i_file_picker> make_file_picker()
        {
            return std::make_shared<headless_file_picker>();
        }
    } // namespace detail
} // namespace maui::storage
