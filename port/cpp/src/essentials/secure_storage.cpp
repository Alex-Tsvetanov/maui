// The cross-platform half of the secure-storage facade: the lazily-created implementation slot
// behind SecureStorage.Default / SetDefault, plus the shared-partial key validation
// (SecureStorage.shared.cs GetAsync/SetAsync: a null/whitespace key throws). The implementation
// itself is the per-backend partial (src/platform/<backend>/essentials_secure_storage.{cpp,mm})
// via detail::make_secure_storage().

#include "maui/essentials/secure_storage.hpp"

#include <algorithm>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace maui::storage
{
    namespace
    {
        std::shared_ptr<i_secure_storage>& secure_storage_storage()
        {
            static std::shared_ptr<i_secure_storage> storage;
            return storage;
        }
    } // namespace

    i_secure_storage& secure_storage::default_()
    {
        auto& storage = secure_storage_storage();
        if (storage == nullptr)
        {
            storage = detail::make_secure_storage();
        }
        return *storage;
    }

    void secure_storage::set_default(std::shared_ptr<i_secure_storage> implementation)
    {
        secure_storage_storage() = std::move(implementation);
    }

    namespace detail
    {
        void require_secure_storage_key(std::string_view key)
        {
            const bool blank =
                std::ranges::all_of(key, [](unsigned char character) { return std::isspace(character) != 0; });
            if (blank)
            {
                // string.IsNullOrWhiteSpace(key) -> ArgumentNullException(nameof(key)).
                throw std::invalid_argument("key");
            }
        }
    } // namespace detail
} // namespace maui::storage
