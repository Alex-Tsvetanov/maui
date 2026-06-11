// file_system - headless platform partial: the backend's lazy default is the controllable fake
// (essentials_appmodel_fakes.hpp), which unconfigured mirrors FileSystem's netstandard partial. The
// real-device twins are src/platform/{apple,ios}/essentials_file_system.mm.

#include "maui/essentials/file_system.hpp"

#include <memory>

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace maui::storage::detail
{
    std::shared_ptr<i_file_system> make_file_system()
    {
        return std::make_shared<headless_file_system>();
    }
} // namespace maui::storage::detail
