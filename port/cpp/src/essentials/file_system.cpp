// The cross-platform half of the file_system facade: the lazily-created implementation slot
// behind FileSystem.Current / SetCurrent, plus FileSystemUtils.NormalizePath ('\' -> '/'). The
// implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_file_system.{cpp,mm}) via detail::make_file_system().

#include "maui/essentials/file_system.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace maui::storage
{
    namespace
    {
        std::shared_ptr<i_file_system>& file_system_storage()
        {
            static std::shared_ptr<i_file_system> storage;
            return storage;
        }
    } // namespace

    i_file_system& file_system::current()
    {
        auto& storage = file_system_storage();
        if (storage == nullptr)
        {
            storage = detail::make_file_system();
        }
        return *storage;
    }

    void file_system::set_current(std::shared_ptr<i_file_system> implementation)
    {
        file_system_storage() = std::move(implementation);
    }

    namespace detail
    {
        std::string normalize_app_package_path(std::string_view filename)
        {
            std::string result(filename);
            std::ranges::replace(result, '\\', '/');
            return result;
        }
    } // namespace detail
} // namespace maui::storage
