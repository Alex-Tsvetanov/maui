#pragma once
// maui::storage::secure_storage    <=  Microsoft.Maui.Storage.SecureStorage (static facade)
// maui::storage::i_secure_storage  <=  Microsoft.Maui.Storage.ISecureStorage
//
// Securely stores simple key/value pairs. The C# Task surface becomes the library's callback
// convention: get_async delivers std::optional<std::string> (nullopt = "value not found", the C#
// null); set_async completes inline on every ported backend (the C# partials all return
// Task.FromResult / Task.CompletedTask) and throws inline on errors. Remove/RemoveAll are
// synchronous in C# already.
//
// Shared-partial validation (SecureStorage.shared.cs): GetAsync/SetAsync throw for a null/blank
// key (ArgumentNullException -> std::invalid_argument; a key is "blank" when empty or
// whitespace-only). The C# null-value SetAsync throw cannot arise (string_view is a value).
//
// Apple specifics not ported: IPlatformSecureStorage.DefaultAccessible (the SecAccessible knob) -
// the keychain partials hardcode C#'s default kSecAttrAccessibleAfterFirstUnlock; the
// SecAccessible-taking SetAsync overload collapses onto the plain one.
//
// Backends (suffix oracle): apple/macOS + ios REAL (SecureStorage.ios.tvos.watchos.macos.cs -
// keychain GenericPassword records; service = the SecureStorageImplementation.Alias
// "{PackageName}.microsoft.maui.essentials.preferences"). Headless mirrors netstandard (throws)
// until faked.

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/move_only_function.hpp"

namespace maui::storage
{
    // Receives the decrypted value (nullopt when the key was not found).
    using secure_value_callback = maui::core::move_only_function<void(const std::optional<std::string>&)>;

    class i_secure_storage
    {
    public:
        virtual ~i_secure_storage() = default;

        // GetAsync: decrypt and deliver the value for the key (nullopt = not found).
        virtual void get_async(std::string_view key, secure_value_callback on_complete) = 0;
        // SetAsync: encrypt and store the value (completes inline; throws inline on errors).
        virtual void set_async(std::string_view key, std::string_view value) = 0;
        // Remove: true when the key existed and was removed.
        virtual bool remove(std::string_view key) = 0;
        // RemoveAll: drop every stored pair.
        virtual void remove_all() = 0;

    protected:
        i_secure_storage() = default;
        i_secure_storage(const i_secure_storage&) = default;
        i_secure_storage(i_secure_storage&&) = default;
        i_secure_storage& operator=(const i_secure_storage&) = default;
        i_secure_storage& operator=(i_secure_storage&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (SecureStorageImplementation), one per backend under
        // src/platform/<backend>/essentials_secure_storage.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_secure_storage> make_secure_storage();

        // The shared-partial key validation (string.IsNullOrWhiteSpace(key) ->
        // ArgumentNullException); throws std::invalid_argument.
        void require_secure_storage_key(std::string_view key);
    } // namespace detail

    // The static facade over secure_storage::default_() (C# SecureStorage.Default).
    class secure_storage final
    {
    public:
        secure_storage() = delete;

        static void get_async(std::string_view key, secure_value_callback on_complete)
        {
            default_().get_async(key, std::move(on_complete));
        }
        static void set_async(std::string_view key, std::string_view value)
        {
            default_().set_async(key, value);
        }
        static bool remove(std::string_view key)
        {
            return default_().remove(key);
        }
        static void remove_all()
        {
            default_().remove_all();
        }

        // SecureStorage.Default (lazy platform default) + SetDefault (the C# internal test seam
        // made public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_secure_storage& default_();
        static void set_default(std::shared_ptr<i_secure_storage> implementation);
    };
} // namespace maui::storage
