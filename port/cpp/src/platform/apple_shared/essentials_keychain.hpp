#pragma once
// The Keychain secure-storage implementation, ONE Obj-C++ definition for BOTH Apple backends -
// the analog of the single SecureStorage.ios.tvos.watchos.macos.cs partial. Ported 1:1 from the
// C# KeyChain helper over the SecItem API:
//   * Records are kSecClassGenericPassword with account = key and service = the
//     SecureStorageImplementation.Alias ("{PackageName}.microsoft.maui.essentials.preferences").
//   * ValueForKey: SecItemCopyMatching (one record, return-data); not-found reads as nullopt.
//   * SetValueForKey: an existing record is removed first, then SecItemAdd with label = key and
//     kSecAttrAccessible = AfterFirstUnlock (the C# DefaultAccessible default - the SecAccessible
//     knob itself is not ported); errSecDuplicateItem retries remove+add once; any other failure
//     throws std::runtime_error("Error adding record: <code>").
//   * Remove: query-then-SecItemDelete, true only when the record existed.
//   * RemoveAll: SecItemDelete over class+service.
// Included by src/platform/{apple,ios}/essentials_secure_storage.mm (the .mm provides
// make_secure_storage). The shared key validation lives in detail::secure_storage_base.

#import <Foundation/Foundation.h>
#import <Security/Security.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "maui/essentials/preferences.hpp" // private_preferences_shared_name (the Alias)
#include "maui/essentials/secure_storage.hpp"

#include "src/essentials/detail/secure_storage_base.hpp"

namespace maui::storage::apple_shared
{
    namespace keychain_detail
    {
        inline NSString* to_ns_string(std::string_view value)
        {
            return [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
        }

        // ExistingRecordForKey: the account+service match dictionary.
        inline NSMutableDictionary* record_query(std::string_view key, NSString* service)
        {
            NSMutableDictionary* const query = [NSMutableDictionary dictionary];
            query[(__bridge NSString*)kSecClass] = (__bridge NSString*)kSecClassGenericPassword;
            query[(__bridge NSString*)kSecAttrAccount] = to_ns_string(key);
            query[(__bridge NSString*)kSecAttrService] = service;
            return query;
        }
    } // namespace keychain_detail

    class keychain_secure_storage final : public detail::secure_storage_base
    {
    public:
        keychain_secure_storage()
            // The Alias is computed lazily in C# (a static initialized on first use); here the
            // implementation itself is created lazily, so the ctor read is the same moment.
            : service_(keychain_detail::to_ns_string(detail::private_preferences_shared_name("preferences")))
        {
        }

    protected:
        void platform_get_async(std::string_view key, secure_value_callback on_complete) override
        {
            on_complete(value_for_key(key));
        }

        void platform_set_async(std::string_view key, std::string_view value) override
        {
            // C# SetValueForKey: an empty value only removes the record.
            if (value_for_key(key).has_value())
            {
                remove_record(key);
            }
            if (value.empty())
            {
                return;
            }

            OSStatus result = add_record(key, value);
            if (result == errSecDuplicateItem)
            {
                // "Duplicate item found. Attempting to remove and add again."
                remove_record(key);
                result = add_record(key, value);
            }
            if (result != errSecSuccess)
            {
                throw std::runtime_error("Error adding record: " + std::to_string(result));
            }
        }

        bool platform_remove(std::string_view key) override
        {
            if (!value_for_key(key).has_value())
            {
                return false;
            }
            remove_record(key);
            return true;
        }

        void platform_remove_all() override
        {
            NSMutableDictionary* const query = [NSMutableDictionary dictionary];
            query[(__bridge NSString*)kSecClass] = (__bridge NSString*)kSecClassGenericPassword;
            query[(__bridge NSString*)kSecAttrService] = service_;
            SecItemDelete((__bridge CFDictionaryRef)query);
        }

    private:
        [[nodiscard]] std::optional<std::string> value_for_key(std::string_view key) const
        {
            NSMutableDictionary* const query = keychain_detail::record_query(key, service_);
            query[(__bridge NSString*)kSecReturnData] = @YES;
            query[(__bridge NSString*)kSecMatchLimit] = (__bridge NSString*)kSecMatchLimitOne;

            CFTypeRef result = nullptr;
            const OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef)query, &result);
            if (status != errSecSuccess || result == nullptr)
            {
                return std::nullopt;
            }
            NSData* const data = CFBridgingRelease(result);
            NSString* const text = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            const char* const utf8 = [text UTF8String];
            return utf8 != nullptr ? std::optional<std::string>(utf8) : std::optional<std::string>(std::string());
        }

        [[nodiscard]] OSStatus add_record(std::string_view key, std::string_view value) const
        {
            // CreateRecordForNewKeyValue: account/service/label/accessible/value-data.
            NSMutableDictionary* const record = keychain_detail::record_query(key, service_);
            record[(__bridge NSString*)kSecAttrLabel] = keychain_detail::to_ns_string(key);
            record[(__bridge NSString*)kSecAttrAccessible] =
                (__bridge NSString*)kSecAttrAccessibleAfterFirstUnlock; // the C# DefaultAccessible
            record[(__bridge NSString*)kSecValueData] = [NSData dataWithBytes:value.data() length:value.size()];
            return SecItemAdd((__bridge CFDictionaryRef)record, nullptr);
        }

        void remove_record(std::string_view key) const
        {
            const OSStatus result =
                SecItemDelete((__bridge CFDictionaryRef)keychain_detail::record_query(key, service_));
            if (result != errSecSuccess && result != errSecItemNotFound)
            {
                throw std::runtime_error("Error removing record: " + std::to_string(result));
            }
        }

        NSString* service_;
    };
} // namespace maui::storage::apple_shared
