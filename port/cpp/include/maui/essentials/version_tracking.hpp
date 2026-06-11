#pragma once
// maui::application_model::version_tracking                 <=  Microsoft.Maui.ApplicationModel.VersionTracking (static
// facade) maui::application_model::i_version_tracking               <= Microsoft.Maui.ApplicationModel.IVersionTracking
// maui::application_model::version_tracking_implementation  <=
// Microsoft.Maui.ApplicationModel.VersionTrackingImplementation
//
// Tracks the app's version/build history on the device. PURE managed - VersionTracking.shared.cs
// is the only C# file (no platform partials), so the whole feature lives in the cross-platform
// half: the implementation persists pipe-joined version/build trails through an injected
// i_preferences (container "{PackageName}.microsoft.maui.essentials.versiontracking") and reads
// the running app's version/build from an injected i_app_info (the headless test seam: fake both
// and the feature is fully exercised device-free). The C# internal class is public here (the port
// has no InternalsVisibleTo) - it is the injectable seam the tests construct directly; C#'s
// nullable Previous*/FirstInstalled* become std::optional.
//
// Construction TRACKS immediately (the C# ctor calls Track()); init_version_tracking() is the C#
// internal InitVersionTracking made public (re-runs the load+track pass - "usually only called
// once in production code, but multiple times in unit tests").

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/essentials/app_info.hpp"
#include "maui/essentials/preferences.hpp"

namespace maui::application_model
{
    class i_version_tracking
    {
    public:
        virtual ~i_version_tracking() = default;

        // Starts tracking version information (idempotent; the work happens once).
        virtual void track() = 0;

        [[nodiscard]] virtual bool is_first_launch_ever() const = 0;
        [[nodiscard]] virtual bool is_first_launch_for_current_version() const = 0;
        [[nodiscard]] virtual bool is_first_launch_for_current_build() const = 0;

        [[nodiscard]] virtual std::string current_version() const = 0;
        [[nodiscard]] virtual std::string current_build() const = 0;
        [[nodiscard]] virtual std::optional<std::string> previous_version() const = 0;
        [[nodiscard]] virtual std::optional<std::string> previous_build() const = 0;
        [[nodiscard]] virtual std::optional<std::string> first_installed_version() const = 0;
        [[nodiscard]] virtual std::optional<std::string> first_installed_build() const = 0;
        [[nodiscard]] virtual std::vector<std::string> version_history() const = 0;
        [[nodiscard]] virtual std::vector<std::string> build_history() const = 0;

        [[nodiscard]] virtual bool is_first_launch_for_version(std::string_view version) const = 0;
        [[nodiscard]] virtual bool is_first_launch_for_build(std::string_view build) const = 0;

    protected:
        i_version_tracking() = default;
        i_version_tracking(const i_version_tracking&) = default;
        i_version_tracking(i_version_tracking&&) = default;
        i_version_tracking& operator=(const i_version_tracking&) = default;
        i_version_tracking& operator=(i_version_tracking&&) = default;
    };

    // VersionTrackingImplementation: the concrete tracker over injected preferences + app info.
    class version_tracking_implementation final : public i_version_tracking
    {
    public:
        // The C# ctor (preferences, appInfo) - tracks immediately.
        version_tracking_implementation(std::shared_ptr<maui::storage::i_preferences> preferences,
                                        std::shared_ptr<i_app_info> app_info);

        void track() override;

        // InitVersionTracking (the C# internal reload seam): load the trails and track the
        // current version/build.
        void init_version_tracking();

        [[nodiscard]] bool is_first_launch_ever() const override
        {
            return is_first_launch_ever_;
        }
        [[nodiscard]] bool is_first_launch_for_current_version() const override
        {
            return is_first_launch_for_current_version_;
        }
        [[nodiscard]] bool is_first_launch_for_current_build() const override
        {
            return is_first_launch_for_current_build_;
        }

        [[nodiscard]] std::string current_version() const override;
        [[nodiscard]] std::string current_build() const override;
        [[nodiscard]] std::optional<std::string> previous_version() const override;
        [[nodiscard]] std::optional<std::string> previous_build() const override;
        [[nodiscard]] std::optional<std::string> first_installed_version() const override;
        [[nodiscard]] std::optional<std::string> first_installed_build() const override;
        [[nodiscard]] std::vector<std::string> version_history() const override;
        [[nodiscard]] std::vector<std::string> build_history() const override;

        [[nodiscard]] bool is_first_launch_for_version(std::string_view version) const override;
        [[nodiscard]] bool is_first_launch_for_build(std::string_view build) const override;

    private:
        [[nodiscard]] std::vector<std::string> read_history(std::string_view key) const;
        void write_history(std::string_view key, const std::vector<std::string>& history);
        [[nodiscard]] static std::optional<std::string> get_previous(const std::vector<std::string>& trail);

        std::shared_ptr<maui::storage::i_preferences> preferences_;
        std::shared_ptr<i_app_info> app_info_;
        std::string shared_name_; // GetPrivatePreferencesSharedName("versiontracking")
        bool tracked_ = false;    // C#'s `versionTrail != null` Track() gate
        bool is_first_launch_ever_ = false;
        bool is_first_launch_for_current_version_ = false;
        bool is_first_launch_for_current_build_ = false;
        std::vector<std::string> versions_;
        std::vector<std::string> builds_;
    };

    // The static facade over version_tracking::default_() (C# VersionTracking).
    class version_tracking final
    {
    public:
        version_tracking() = delete;

        static void track()
        {
            default_().track();
        }
        [[nodiscard]] static bool is_first_launch_ever()
        {
            return default_().is_first_launch_ever();
        }
        [[nodiscard]] static bool is_first_launch_for_current_version()
        {
            return default_().is_first_launch_for_current_version();
        }
        [[nodiscard]] static bool is_first_launch_for_current_build()
        {
            return default_().is_first_launch_for_current_build();
        }
        [[nodiscard]] static std::string current_version()
        {
            return default_().current_version();
        }
        [[nodiscard]] static std::string current_build()
        {
            return default_().current_build();
        }
        [[nodiscard]] static std::optional<std::string> previous_version()
        {
            return default_().previous_version();
        }
        [[nodiscard]] static std::optional<std::string> previous_build()
        {
            return default_().previous_build();
        }
        [[nodiscard]] static std::optional<std::string> first_installed_version()
        {
            return default_().first_installed_version();
        }
        [[nodiscard]] static std::optional<std::string> first_installed_build()
        {
            return default_().first_installed_build();
        }
        [[nodiscard]] static std::vector<std::string> version_history()
        {
            return default_().version_history();
        }
        [[nodiscard]] static std::vector<std::string> build_history()
        {
            return default_().build_history();
        }
        [[nodiscard]] static bool is_first_launch_for_version(std::string_view version)
        {
            return default_().is_first_launch_for_version(version);
        }
        [[nodiscard]] static bool is_first_launch_for_build(std::string_view build)
        {
            return default_().is_first_launch_for_build(build);
        }

        // VersionTracking.InitVersionTracking (internal in C#; the test reload seam) - no-ops when
        // the installed default is not a version_tracking_implementation, like the C# `as` cast.
        static void init_version_tracking();

        // VersionTracking.Default - lazily constructs the implementation over Preferences.Default
        // + AppInfo.Current - and SetDefault (the C# internal test seam made public; nullptr
        // resets to the lazy default).
        [[nodiscard]] static i_version_tracking& default_();
        static void set_default(std::shared_ptr<i_version_tracking> implementation);
    };
} // namespace maui::application_model
