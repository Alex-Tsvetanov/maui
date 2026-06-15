#pragma once
// The headless backend's app-model + storage essentials implementations - one controllable FAKE
// per feature, the in-memory twins of the C# *.netstandard.*.cs partials (the W1-17 sibling of
// essentials_fakes.hpp, which holds the devices+sensors family).
//
// Contract every fake follows (the family rule from essentials_fakes.hpp):
//   * UNCONFIGURED, it mirrors the netstandard partial byte-for-byte: members that throw
//     NotImplementedInReferenceAssemblyException there throw feature_not_supported here; members
//     that return defaults there (app_info requested theme/layout direction) return the same
//     defaults.
//   * The set_*/configure test setters turn it into a working in-memory device: preferences
//     become a real container map, secure storage a real string map, the launcher/browser record
//     what they were asked to open, and simulate_* methods drive the shared raise paths.
//   * Everything is inline and synchronous - async callbacks complete inline on the caller's
//     thread.
//
// The per-feature factories (detail::make_*) in src/platform/headless/essentials_<feature>.cpp
// return these fakes as the backend's lazy defaults; tests can also instantiate them directly
// and install them through the facades' set_current/set_default seams.

#include <chrono>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "maui/core/app_theme.hpp"
#include "maui/essentials/app_actions.hpp"
#include "maui/essentials/app_info.hpp"
#include "maui/essentials/browser.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_system.hpp"
#include "maui/essentials/launcher.hpp"
#include "maui/essentials/main_thread.hpp"
#include "maui/essentials/permissions.hpp"
#include "maui/essentials/preferences.hpp"
#include "maui/essentials/secure_accessible.hpp"
#include "maui/essentials/secure_storage.hpp"

#include "src/essentials/detail/app_actions_base.hpp"
#include "src/essentials/detail/secure_storage_base.hpp"

namespace maui::storage
{
    namespace headless_detail
    {
        // The netstandard partials' throw, shared by every unconfigured fake member.
        [[noreturn]] inline void throw_not_implemented()
        {
            throw maui::application_model::feature_not_supported(
                "This feature is not implemented on the headless backend until the fake is configured "
                "(the netstandard-partial mirror).");
        }
    } // namespace headless_detail

    // PreferencesImplementation (netstandard): every member throws - until configure() flips the
    // fake into a working in-memory container map (the NSUserDefaults twin). Values are stored
    // typed; a type-mismatched read reports the default (the fake's simplification of the
    // platform stores' coercion rules).
    class headless_preferences final : public i_preferences
    {
    public:
        void configure()
        {
            configured_ = true;
        }

        [[nodiscard]] bool contains_key(std::string_view key, std::string_view shared_name) const override
        {
            require_configured();
            const auto container = containers_.find(std::string(shared_name));
            return container != containers_.end() && container->second.contains(std::string(key));
        }

        void remove(std::string_view key, std::string_view shared_name) override
        {
            require_configured();
            const auto container = containers_.find(std::string(shared_name));
            if (container != containers_.end())
            {
                container->second.erase(std::string(key));
            }
        }

        void clear(std::string_view shared_name) override
        {
            require_configured();
            containers_.erase(std::string(shared_name));
        }

        [[nodiscard]] std::optional<std::string> get_string(std::string_view key,
                                                            std::optional<std::string> default_value,
                                                            std::string_view shared_name) const override
        {
            return get<std::string>(key, std::move(default_value), shared_name);
        }
        void set_string(std::string_view key, const std::optional<std::string>& value,
                        std::string_view shared_name) override
        {
            require_configured();
            if (!value.has_value())
            {
                // The C# null-value Set removes the key.
                remove(key, shared_name);
                return;
            }
            set<std::string>(key, *value, shared_name);
        }

        [[nodiscard]] bool get_bool(std::string_view key, bool default_value,
                                    std::string_view shared_name) const override
        {
            return *get<bool>(key, default_value, shared_name);
        }
        void set_bool(std::string_view key, bool value, std::string_view shared_name) override
        {
            require_configured();
            set<bool>(key, value, shared_name);
        }

        [[nodiscard]] std::int32_t get_int(std::string_view key, std::int32_t default_value,
                                           std::string_view shared_name) const override
        {
            return *get<std::int32_t>(key, default_value, shared_name);
        }
        void set_int(std::string_view key, std::int32_t value, std::string_view shared_name) override
        {
            require_configured();
            set<std::int32_t>(key, value, shared_name);
        }

        [[nodiscard]] std::int64_t get_long(std::string_view key, std::int64_t default_value,
                                            std::string_view shared_name) const override
        {
            return *get<std::int64_t>(key, default_value, shared_name);
        }
        void set_long(std::string_view key, std::int64_t value, std::string_view shared_name) override
        {
            require_configured();
            set<std::int64_t>(key, value, shared_name);
        }

        [[nodiscard]] float get_float(std::string_view key, float default_value,
                                      std::string_view shared_name) const override
        {
            return *get<float>(key, default_value, shared_name);
        }
        void set_float(std::string_view key, float value, std::string_view shared_name) override
        {
            require_configured();
            set<float>(key, value, shared_name);
        }

        [[nodiscard]] double get_double(std::string_view key, double default_value,
                                        std::string_view shared_name) const override
        {
            return *get<double>(key, default_value, shared_name);
        }
        void set_double(std::string_view key, double value, std::string_view shared_name) override
        {
            require_configured();
            set<double>(key, value, shared_name);
        }

        [[nodiscard]] date_time get_date_time(std::string_view key, date_time default_value,
                                              std::string_view shared_name) const override
        {
            // Stored as the int64 tick encoding (the C# ToBinary-string shape, minus the Kind).
            const std::optional<std::int64_t> ticks = get<std::int64_t>(key, std::nullopt, shared_name);
            return ticks.has_value() ? date_time(std::chrono::duration_cast<date_time::duration>(tick_duration(*ticks)))
                                     : default_value;
        }
        void set_date_time(std::string_view key, date_time value, std::string_view shared_name) override
        {
            require_configured();
            set<std::int64_t>(key, std::chrono::duration_cast<tick_duration>(value.time_since_epoch()).count(),
                              shared_name);
        }

    private:
        using tick_duration = std::chrono::duration<std::int64_t, std::ratio<1, 10000000>>; // 100 ns ticks
        using stored_value = std::variant<std::string, bool, std::int32_t, std::int64_t, float, double>;

        void require_configured() const
        {
            if (!configured_)
            {
                headless_detail::throw_not_implemented();
            }
        }

        template <class T>
        [[nodiscard]] std::optional<T> get(std::string_view key, std::optional<T> default_value,
                                           std::string_view shared_name) const
        {
            require_configured();
            const auto container = containers_.find(std::string(shared_name));
            if (container == containers_.end())
            {
                return default_value;
            }
            const auto entry = container->second.find(std::string(key));
            if (entry == container->second.end() || !std::holds_alternative<T>(entry->second))
            {
                return default_value;
            }
            return std::get<T>(entry->second);
        }

        template <class T> void set(std::string_view key, T value, std::string_view shared_name)
        {
            containers_[std::string(shared_name)][std::string(key)] = stored_value(std::move(value));
        }

        bool configured_ = false;
        std::map<std::string, std::map<std::string, stored_value>> containers_;
    };

    // SecureStorageImplementation (netstandard): every platform hook throws - until configure()
    // flips the fake into a working in-memory keychain twin (the shared key validation in
    // secure_storage_base runs for real either way). DefaultAccessible is mirrored as a plain field
    // (the C# get/set property); the fake has no keychain so the SecAccessible never reaches a real
    // record - it only records, per stored key, which accessible the set used, so the
    // IPlatformSecureStorage paths can be exercised headless.
    class headless_secure_storage final : public detail::secure_storage_base
    {
    public:
        void configure()
        {
            configured_ = true;
        }

        // Test seam (no C# analog): the accessible the value at `key` was last stored with, so the
        // headless suite can verify set_async vs set_async_with_accessible without a real keychain.
        [[nodiscard]] std::optional<secure_accessible> accessible_for(std::string_view key) const
        {
            const auto entry = accessibles_.find(std::string(key));
            return entry == accessibles_.end() ? std::nullopt : std::optional<secure_accessible>(entry->second);
        }

    protected:
        void platform_get_async(std::string_view key, secure_value_callback on_complete) override
        {
            require_configured();
            const auto entry = values_.find(std::string(key));
            on_complete(entry == values_.end() ? std::nullopt : std::optional<std::string>(entry->second));
        }

        void platform_set_async(std::string_view key, std::string_view value) override
        {
            // C# PlatformSetAsync -> SetAsync(key, value, DefaultAccessible).
            store(key, value, accessible_);
        }

        void platform_set_async_with_accessible(std::string_view key, std::string_view value,
                                                secure_accessible accessible) override
        {
            store(key, value, accessible);
        }

        [[nodiscard]] secure_accessible platform_get_default_accessible() const override
        {
            return accessible_;
        }

        void platform_set_default_accessible(secure_accessible accessible) override
        {
            accessible_ = accessible;
        }

        bool platform_remove(std::string_view key) override
        {
            require_configured();
            accessibles_.erase(std::string(key));
            return values_.erase(std::string(key)) > 0;
        }

        void platform_remove_all() override
        {
            require_configured();
            values_.clear();
            accessibles_.clear();
        }

    private:
        void store(std::string_view key, std::string_view value, secure_accessible accessible)
        {
            require_configured();
            values_[std::string(key)] = std::string(value);
            accessibles_[std::string(key)] = accessible;
        }

        void require_configured() const
        {
            if (!configured_)
            {
                headless_detail::throw_not_implemented();
            }
        }

        bool configured_ = false;
        // C# `DefaultAccessible { get; set; } = SecAccessible.AfterFirstUnlock;`.
        secure_accessible accessible_ = secure_accessible::after_first_unlock;
        std::map<std::string, std::string> values_;
        std::map<std::string, secure_accessible> accessibles_;
    };

    // FileSystemImplementation (netstandard): every member throws - until the directories / the
    // app-package ROOT are staged (the package-file seam the queries resolve against).
    class headless_file_system final : public i_file_system
    {
    public:
        [[nodiscard]] std::string cache_directory() const override
        {
            return require(cache_directory_);
        }
        [[nodiscard]] std::string app_data_directory() const override
        {
            return require(app_data_directory_);
        }

        [[nodiscard]] std::ifstream open_app_package_file(std::string_view filename) override
        {
            const std::filesystem::path file = package_file_path(filename);
            if (!std::filesystem::exists(file))
            {
                // The C# FileNotFoundException from File.OpenRead.
                throw std::runtime_error("Could not find app package file '" + std::string(filename) + "'.");
            }
            return std::ifstream(file, std::ios::binary);
        }

        [[nodiscard]] bool app_package_file_exists(std::string_view filename) override
        {
            return std::filesystem::exists(package_file_path(filename));
        }

        void set_cache_directory(std::string value)
        {
            cache_directory_ = std::move(value);
        }
        void set_app_data_directory(std::string value)
        {
            app_data_directory_ = std::move(value);
        }
        void set_app_package_root(std::string value)
        {
            app_package_root_ = std::move(value);
        }

    private:
        [[nodiscard]] static const std::string& require(const std::optional<std::string>& value)
        {
            if (!value.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            return *value;
        }

        [[nodiscard]] std::filesystem::path package_file_path(std::string_view filename) const
        {
            return std::filesystem::path(require(app_package_root_)) / detail::normalize_app_package_path(filename);
        }

        std::optional<std::string> cache_directory_;
        std::optional<std::string> app_data_directory_;
        std::optional<std::string> app_package_root_;
    };
} // namespace maui::storage

namespace maui::application_model
{
    namespace headless_detail = maui::storage::headless_detail;

    // AppInfoImplementation (netstandard): package/name/version/build/settings/packaging-model
    // throw; requested theme is Unspecified and layout direction Unknown (returned, not thrown) -
    // until configured.
    class headless_app_info final : public i_app_info
    {
    public:
        [[nodiscard]] std::string package_name() const override
        {
            return require(package_name_);
        }
        [[nodiscard]] std::string name() const override
        {
            return require(name_);
        }
        [[nodiscard]] std::string version_string() const override
        {
            return require(version_string_);
        }
        [[nodiscard]] std::string build_string() const override
        {
            return require(build_string_);
        }

        void show_settings_ui() override
        {
            if (!settings_ui_supported_)
            {
                headless_detail::throw_not_implemented();
            }
            ++settings_ui_shown_count_;
        }

        [[nodiscard]] maui::core::app_theme requested_theme() const override
        {
            return requested_theme_;
        }
        [[nodiscard]] app_packaging_model packaging_model() const override
        {
            if (!packaging_model_.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            return *packaging_model_;
        }
        [[nodiscard]] layout_direction requested_layout_direction() const override
        {
            return requested_layout_direction_;
        }

        void set_package_name(std::string value)
        {
            package_name_ = std::move(value);
        }
        void set_name(std::string value)
        {
            name_ = std::move(value);
        }
        void set_version_string(std::string value)
        {
            version_string_ = std::move(value);
        }
        void set_build_string(std::string value)
        {
            build_string_ = std::move(value);
        }
        void set_show_settings_ui_supported(bool value)
        {
            settings_ui_supported_ = value;
        }
        void set_requested_theme(maui::core::app_theme value)
        {
            requested_theme_ = value;
        }
        void set_packaging_model(app_packaging_model value)
        {
            packaging_model_ = value;
        }
        void set_requested_layout_direction(layout_direction value)
        {
            requested_layout_direction_ = value;
        }

        [[nodiscard]] int settings_ui_shown_count() const
        {
            return settings_ui_shown_count_;
        }

    private:
        [[nodiscard]] static const std::string& require(const std::optional<std::string>& value)
        {
            if (!value.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            return *value;
        }

        std::optional<std::string> package_name_;
        std::optional<std::string> name_;
        std::optional<std::string> version_string_;
        std::optional<std::string> build_string_;
        std::optional<app_packaging_model> packaging_model_;
        maui::core::app_theme requested_theme_ = maui::core::app_theme::unspecified;
        layout_direction requested_layout_direction_ = layout_direction::unknown;
        bool settings_ui_supported_ = false;
        int settings_ui_shown_count_ = 0;
    };

    // The MainThread netstandard partial: both members throw until a custom implementation is
    // installed. Configured (set_is_main_thread), the fake QUEUES platform begin-invokes so tests
    // can assert the facade's inline-vs-posted gate; run_pending() plays the queue back.
    class headless_main_thread final : public i_main_thread
    {
    public:
        [[nodiscard]] bool is_main_thread() const override
        {
            if (!is_main_thread_.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            return *is_main_thread_;
        }

        void begin_invoke_on_main_thread(main_thread_action action) override
        {
            if (!is_main_thread_.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            pending_.push_back(std::move(action));
        }

        void set_is_main_thread(bool value)
        {
            is_main_thread_ = value;
        }

        [[nodiscard]] std::size_t pending_count() const
        {
            return pending_.size();
        }

        // Drain the queue (the headless stand-in for the main loop turning).
        void run_pending()
        {
            while (!pending_.empty())
            {
                main_thread_action action = std::move(pending_.front());
                pending_.pop_front();
                action();
            }
        }

    private:
        std::optional<bool> is_main_thread_;
        std::deque<main_thread_action> pending_;
    };

    // LauncherImplementation (netstandard): every member throws - until configured. Configured,
    // can_open answers the staged flag, open records the URI and reports the staged result, and
    // try_open composes them exactly like the platform partials (no open attempt when the scheme
    // cannot be opened).
    class headless_launcher final : public i_launcher
    {
    public:
        void can_open_async(std::string_view uri, launch_callback on_complete) override
        {
            require_configured();
            last_queried_uri_ = std::string(uri);
            on_complete(can_open_);
        }

        void open_async(std::string_view uri, launch_callback on_complete) override
        {
            require_configured();
            opened_uris_.emplace_back(uri);
            on_complete(open_result_);
        }

        void try_open_async(std::string_view uri, launch_callback on_complete) override
        {
            require_configured();
            if (!can_open_)
            {
                last_queried_uri_ = std::string(uri);
                on_complete(false);
                return;
            }
            open_async(uri, std::move(on_complete));
        }

        void set_can_open(bool value)
        {
            configured_ = true;
            can_open_ = value;
        }
        void set_open_result(bool value)
        {
            configured_ = true;
            open_result_ = value;
        }

        [[nodiscard]] const std::vector<std::string>& opened_uris() const
        {
            return opened_uris_;
        }
        [[nodiscard]] const std::optional<std::string>& last_queried_uri() const
        {
            return last_queried_uri_;
        }

    private:
        void require_configured() const
        {
            if (!configured_)
            {
                headless_detail::throw_not_implemented();
            }
        }

        bool configured_ = false;
        bool can_open_ = true;
        bool open_result_ = true;
        std::vector<std::string> opened_uris_;
        std::optional<std::string> last_queried_uri_;
    };

    // BrowserImplementation (netstandard): open throws - until configured; then it records the
    // uri + options and reports the staged result.
    class headless_browser final : public i_browser
    {
    public:
        void open_async(std::string_view uri, const browser_launch_options& options,
                        launch_callback on_complete) override
        {
            if (!configured_)
            {
                headless_detail::throw_not_implemented();
            }
            last_uri_ = std::string(uri);
            last_options_ = options;
            on_complete(open_result_);
        }

        void set_open_result(bool value)
        {
            configured_ = true;
            open_result_ = value;
        }

        [[nodiscard]] const std::optional<std::string>& last_uri() const
        {
            return last_uri_;
        }
        [[nodiscard]] const std::optional<browser_launch_options>& last_options() const
        {
            return last_options_;
        }

    private:
        bool configured_ = false;
        bool open_result_ = true;
        std::optional<std::string> last_uri_;
        std::optional<browser_launch_options> last_options_;
    };

    // AppActionsImplementation (netstandard): every member (including IsSupported) throws - until
    // configured. supported=false mirrors the C# ios gate (get/set throw feature_not_supported);
    // supported=true stores the actions in-memory and simulate_activated drives the shared event.
    class headless_app_actions final : public detail::app_actions_base
    {
    public:
        [[nodiscard]] bool is_supported() const override
        {
            if (!supported_.has_value())
            {
                headless_detail::throw_not_implemented();
            }
            return *supported_;
        }

        void get_async(app_actions_callback on_complete) override
        {
            require_supported();
            on_complete(actions_);
        }

        void set_async(const std::vector<app_action>& actions) override
        {
            require_supported();
            actions_ = actions;
        }

        void set_is_supported(bool value)
        {
            supported_ = value;
        }

        // A shortcut activation arriving from the platform (PerformActionForShortcutItem's role).
        void simulate_activated(const app_action& action)
        {
            raise_app_action_activated(action);
        }

    private:
        void require_supported() const
        {
            if (!is_supported())
            {
                // The C# ios partial's `if (!IsSupported) throw new FeatureNotSupportedException()`.
                throw maui::application_model::feature_not_supported();
            }
        }

        std::optional<bool> supported_;
        std::vector<app_action> actions_;
    };

    // The Permissions netstandard partial: every member throws - until statuses are staged. The
    // fake-grantable seam: set_status stages check_status (and the request fallback),
    // set_request_result stages a distinct request answer, set_undeclared makes ensure_declared
    // throw permission_error (the missing-manifest-entry path), set_should_show_rationale stages
    // the Android-only rationale flag.
    class headless_permission_backend final : public i_permission_backend
    {
    public:
        void check_status(permission_kind kind, permission_status_callback on_complete) override
        {
            on_complete(require_status(kind));
        }

        void request(permission_kind kind, permission_status_callback on_complete) override
        {
            last_requested_ = kind;
            const auto request_entry = request_results_.find(kind);
            if (request_entry != request_results_.end())
            {
                on_complete(request_entry->second);
                return;
            }
            on_complete(require_status(kind));
        }

        void ensure_declared(permission_kind kind) override
        {
            if (undeclared_.contains(kind))
            {
                throw permission_error("You must declare the permission in your application manifest.");
            }
            if (!statuses_.contains(kind))
            {
                headless_detail::throw_not_implemented();
            }
        }

        [[nodiscard]] bool should_show_rationale(permission_kind kind) override
        {
            if (!statuses_.contains(kind))
            {
                headless_detail::throw_not_implemented();
            }
            return rationale_.contains(kind);
        }

        void set_status(permission_kind kind, permission_status status)
        {
            statuses_[kind] = status;
        }
        void set_request_result(permission_kind kind, permission_status status)
        {
            request_results_[kind] = status;
        }
        void set_undeclared(permission_kind kind)
        {
            undeclared_.insert(kind);
        }
        void set_should_show_rationale(permission_kind kind)
        {
            rationale_.insert(kind);
        }

        [[nodiscard]] std::optional<permission_kind> last_requested() const
        {
            return last_requested_;
        }

    private:
        [[nodiscard]] permission_status require_status(permission_kind kind) const
        {
            const auto entry = statuses_.find(kind);
            if (entry == statuses_.end())
            {
                headless_detail::throw_not_implemented();
            }
            return entry->second;
        }

        std::map<permission_kind, permission_status> statuses_;
        std::map<permission_kind, permission_status> request_results_;
        std::set<permission_kind> undeclared_;
        std::set<permission_kind> rationale_;
        std::optional<permission_kind> last_requested_;
    };
} // namespace maui::application_model
