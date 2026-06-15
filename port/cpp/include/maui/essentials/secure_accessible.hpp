#pragma once
// maui::storage::secure_accessible  <=  Security.SecAccessible
//
// The KeyChain accessibility classes Apple's Security.framework exposes for an item's
// kSecAttrAccessible attribute. SecureStorage's IPlatformSecureStorage (iOS/macOS) takes one of
// these for DefaultAccessible and the SetAsync(key, value, accessible) overload (the value baked
// into a new keychain record's kSecAttrAccessible at SecItemAdd time).
//
// This enum is ALWAYS AVAILABLE (not #if'd to Apple): the headless fake and the deferred Android
// backend both implement the same i_secure_storage contract and so must name the type to compile.
// The enum -> kSecAttrAccessible* mapping is Apple-only and lives in the keychain partial
// (src/platform/apple_shared/essentials_keychain.hpp), not here.
//
// Values mirror Security.SecAccessible 1:1 (member order and the leading Invalid sentinel
// included); the default SecureStorage uses is after_first_unlock.

namespace maui::storage
{
    // Security.SecAccessible.
    enum class secure_accessible
    {
        // SecAccessible.Invalid - the binding's "unset" sentinel; no kSecAttrAccessible* maps to it.
        invalid = 0,
        // kSecAttrAccessibleWhenUnlocked.
        when_unlocked,
        // kSecAttrAccessibleAfterFirstUnlock (the SecureStorage default).
        after_first_unlock,
        // kSecAttrAccessibleAlways (deprecated by Apple, kept for parity with the binding).
        always,
        // kSecAttrAccessibleWhenUnlockedThisDeviceOnly.
        when_unlocked_this_device_only,
        // kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly.
        after_first_unlock_this_device_only,
        // kSecAttrAccessibleAlwaysThisDeviceOnly (deprecated by Apple, kept for parity).
        always_this_device_only,
        // kSecAttrAccessibleWhenPasscodeSetThisDeviceOnly.
        when_passcode_set_this_device_only,
    };
} // namespace maui::storage
