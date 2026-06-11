// secure_storage - Apple (AppKit / macOS) platform partial: the shared Keychain implementation
// (essentials_keychain.hpp, the SecureStorage.ios.tvos.watchos.macos.cs twin). Compiled as
// Objective-C++ with ARC for the apple backend.

#include <memory>

#include "maui/essentials/secure_storage.hpp"

#include "src/platform/apple_shared/essentials_keychain.hpp"

namespace maui::storage::detail
{
    std::shared_ptr<i_secure_storage> make_secure_storage()
    {
        return std::make_shared<apple_shared::keychain_secure_storage>();
    }
} // namespace maui::storage::detail
