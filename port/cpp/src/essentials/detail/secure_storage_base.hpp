#pragma once
// maui::storage::detail::secure_storage_base  <=  the cross-platform half of
// Microsoft.Maui.Storage.SecureStorageImplementation (SecureStorage.shared.cs): GetAsync/SetAsync
// validate the key (null/whitespace -> ArgumentNullException, std::invalid_argument here) before
// reaching the platform hooks; Remove/RemoveAll forward straight through, exactly like the C#
// partial-method split.

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/essentials/secure_storage.hpp"

namespace maui::storage::detail
{
    class secure_storage_base : public i_secure_storage
    {
    public:
        void get_async(std::string_view key, secure_value_callback on_complete) final
        {
            require_secure_storage_key(key);
            platform_get_async(key, std::move(on_complete));
        }

        void set_async(std::string_view key, std::string_view value) final
        {
            require_secure_storage_key(key);
            platform_set_async(key, value);
        }

        bool remove(std::string_view key) final
        {
            return platform_remove(key);
        }

        void remove_all() final
        {
            platform_remove_all();
        }

    protected:
        secure_storage_base() = default;

        // PlatformGetAsync / PlatformSetAsync / PlatformRemove / PlatformRemoveAll.
        virtual void platform_get_async(std::string_view key, secure_value_callback on_complete) = 0;
        virtual void platform_set_async(std::string_view key, std::string_view value) = 0;
        virtual bool platform_remove(std::string_view key) = 0;
        virtual void platform_remove_all() = 0;
    };
} // namespace maui::storage::detail
