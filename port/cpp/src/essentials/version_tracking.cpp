// version_tracking - PURE managed (VersionTracking.shared.cs has no platform partials): the
// trail algorithm over an injected i_preferences + i_app_info, plus the facade's lazy default
// (VersionTracking.Default = new VersionTrackingImplementation(Preferences.Default,
// AppInfo.Current)). Histories persist pipe-joined under the
// "{PackageName}.microsoft.maui.essentials.versiontracking" container.

#include "maui/essentials/version_tracking.hpp"
#include "maui/essentials/app_info.hpp"
#include "maui/essentials/preferences.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace maui::application_model
{
    namespace
    {
        constexpr std::string_view versions_key = "VersionTracking.Versions";
        constexpr std::string_view builds_key = "VersionTracking.Builds";

        // string.Join("|", history).
        std::string join_history(const std::vector<std::string>& history)
        {
            std::string result;
            for (const std::string& entry : history)
            {
                if (!result.empty())
                {
                    result += '|';
                }
                result += entry;
            }
            return result;
        }

        // Split('|', RemoveEmptyEntries).
        std::vector<std::string> split_history(std::string_view joined)
        {
            std::vector<std::string> result;
            std::size_t start = 0;
            while (start <= joined.size())
            {
                const std::size_t separator = joined.find('|', start);
                const std::size_t end = separator == std::string_view::npos ? joined.size() : separator;
                if (end > start)
                {
                    result.emplace_back(joined.substr(start, end - start));
                }
                if (separator == std::string_view::npos)
                {
                    break;
                }
                start = separator + 1;
            }
            return result;
        }
    } // namespace

    version_tracking_implementation::version_tracking_implementation(
        std::shared_ptr<maui::storage::i_preferences> preferences, std::shared_ptr<i_app_info> app_info)
        : preferences_(std::move(preferences)), app_info_(std::move(app_info))
    {
        // static readonly sharedName = Preferences.GetPrivatePreferencesSharedName("versiontracking"),
        // computed from the injected app info here (the C# static reads AppInfo.Current).
        shared_name_ = app_info_->package_name();
        shared_name_ += ".microsoft.maui.essentials.versiontracking";

        track(); // the C# ctor tracks immediately
    }

    void version_tracking_implementation::track()
    {
        if (tracked_)
        {
            return; // C#'s `versionTrail != null` early-out
        }
        init_version_tracking();
    }

    void version_tracking_implementation::init_version_tracking()
    {
        tracked_ = true;

        is_first_launch_ever_ = !preferences_->contains_key(versions_key, shared_name_) ||
                                !preferences_->contains_key(builds_key, shared_name_);
        if (is_first_launch_ever_)
        {
            versions_.clear();
            builds_.clear();
        }
        else
        {
            versions_ = read_history(versions_key);
            builds_ = read_history(builds_key);
        }

        const std::string current_version_value = current_version();
        const std::string last_installed_version = versions_.empty() ? std::string() : versions_.back();
        is_first_launch_for_current_version_ = std::ranges::find(versions_, current_version_value) == versions_.end() ||
                                               current_version_value != last_installed_version;
        if (is_first_launch_for_current_version_)
        {
            // Avoid duplicates and move the current version to the end of the list.
            std::erase(versions_, current_version_value);
            versions_.push_back(current_version_value);
        }

        const std::string current_build_value = current_build();
        const std::string last_installed_build = builds_.empty() ? std::string() : builds_.back();
        is_first_launch_for_current_build_ = std::ranges::find(builds_, current_build_value) == builds_.end() ||
                                             current_build_value != last_installed_build;
        if (is_first_launch_for_current_build_)
        {
            std::erase(builds_, current_build_value);
            builds_.push_back(current_build_value);
        }

        if (is_first_launch_for_current_version_ || is_first_launch_for_current_build_)
        {
            write_history(versions_key, versions_);
            write_history(builds_key, builds_);
        }
    }

    std::string version_tracking_implementation::current_version() const
    {
        return app_info_->version_string();
    }

    std::string version_tracking_implementation::current_build() const
    {
        return app_info_->build_string();
    }

    std::optional<std::string> version_tracking_implementation::previous_version() const
    {
        return get_previous(versions_);
    }

    std::optional<std::string> version_tracking_implementation::previous_build() const
    {
        return get_previous(builds_);
    }

    std::optional<std::string> version_tracking_implementation::first_installed_version() const
    {
        return versions_.empty() ? std::nullopt : std::optional<std::string>(versions_.front());
    }

    std::optional<std::string> version_tracking_implementation::first_installed_build() const
    {
        return builds_.empty() ? std::nullopt : std::optional<std::string>(builds_.front());
    }

    std::vector<std::string> version_tracking_implementation::version_history() const
    {
        return versions_;
    }

    std::vector<std::string> version_tracking_implementation::build_history() const
    {
        return builds_;
    }

    bool version_tracking_implementation::is_first_launch_for_version(std::string_view version) const
    {
        return current_version() == version && is_first_launch_for_current_version_;
    }

    bool version_tracking_implementation::is_first_launch_for_build(std::string_view build) const
    {
        return current_build() == build && is_first_launch_for_current_build_;
    }

    std::vector<std::string> version_tracking_implementation::read_history(std::string_view key) const
    {
        const std::optional<std::string> joined = preferences_->get_string(key, std::nullopt, shared_name_);
        return joined.has_value() ? split_history(*joined) : std::vector<std::string>();
    }

    void version_tracking_implementation::write_history(std::string_view key, const std::vector<std::string>& history)
    {
        preferences_->set_string(key, join_history(history), shared_name_);
    }

    std::optional<std::string> version_tracking_implementation::get_previous(const std::vector<std::string>& trail)
    {
        return trail.size() >= 2 ? std::optional<std::string>(trail[trail.size() - 2]) : std::nullopt;
    }

    namespace
    {
        std::shared_ptr<i_version_tracking>& version_tracking_storage()
        {
            static std::shared_ptr<i_version_tracking> storage;
            return storage;
        }
    } // namespace

    i_version_tracking& version_tracking::default_()
    {
        auto& storage = version_tracking_storage();
        if (storage == nullptr)
        {
            storage = std::make_shared<version_tracking_implementation>(maui::storage::preferences::default_shared(),
                                                                        app_info::current_shared());
        }
        return *storage;
    }

    void version_tracking::set_default(std::shared_ptr<i_version_tracking> implementation)
    {
        version_tracking_storage() = std::move(implementation);
    }

    void version_tracking::init_version_tracking()
    {
        // (Default as VersionTrackingImplementation)?.InitVersionTracking().
        if (auto* implementation = dynamic_cast<version_tracking_implementation*>(&default_()))
        {
            implementation->init_version_tracking();
        }
    }
} // namespace maui::application_model
