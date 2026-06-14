#pragma once
// maui::networking::connectivity                       <=  Microsoft.Maui.Networking.Connectivity (static facade)
// maui::networking::i_connectivity                     <=  Microsoft.Maui.Networking.IConnectivity
// maui::networking::network_access                     <=  Microsoft.Maui.Networking.NetworkAccess
// maui::networking::connection_profile                 <=  Microsoft.Maui.Networking.ConnectionProfile
// maui::networking::connectivity_changed_event_args    <=  Microsoft.Maui.Networking.ConnectivityChangedEventArgs
//
// Monitors the device's network conditions: the current NetworkAccess, the active ConnectionProfiles,
// and a ConnectivityChanged event. Connectivity.shared.cs splits as: the value getters + the
// Start/StopListeners hooks are the platform partial (pure virtual here), while the listener-lifecycle
// event accessors and the change-dedupe cache are the shared ConnectivityImplementation logic
// (detail::connectivity_base). C#'s `ConnectivityChanged` event accessor maps to explicit add_/remove_
// pairs: the FIRST subscriber snapshots the current access+profiles (SetCurrent) and starts the
// platform listeners, the last remove stops them. OnConnectivityChanged re-reads access+profiles and
// raises only when access differs OR the profile sequence differs (C#'s SequenceEqual), re-snapshotting
// before each raise. The C# facade's `ConnectionProfiles.Distinct()` is applied in the static facade.
//
// Backends (suffix oracle): apple/macOS + ios REAL (Connectivity.ios.tvos.macos.cs - the port uses
// Network.framework's NWPathMonitor rather than the deprecated SCNetworkReachability the C# partial
// wraps; same NetworkAccess/ConnectionProfile contract). Headless mirrors netstandard
// (Connectivity.netstandard.watchos.cs throws) until the fake is configured.

#include <memory>
#include <vector>

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::networking
{
    // ConnectionProfile: the type of an active connection.
    enum class connection_profile
    {
        unknown = 0,
        bluetooth = 1,
        cellular = 2,
        ethernet = 3,
        wifi = 4,
    };

    // NetworkAccess: the state of the connection to the internet.
    enum class network_access
    {
        unknown = 0,
        none = 1,
        local = 2,
        constrained_internet = 3,
        internet = 4,
    };

    // ConnectivityChangedEventArgs: the access + profiles carried by a change.
    struct connectivity_changed_event_args
    {
        enum network_access network_access = network_access::unknown;
        std::vector<connection_profile> connection_profiles;
    };

    class i_connectivity
    {
    public:
        virtual ~i_connectivity() = default;

        // IConnectivity.NetworkAccess.
        [[nodiscard]] virtual enum network_access network_access() const = 0;
        // IConnectivity.ConnectionProfiles (the raw platform sequence; the facade applies Distinct).
        [[nodiscard]] virtual std::vector<connection_profile> connection_profiles() const = 0;

        // ConnectivityChanged event accessors (first add snapshots + starts the platform listeners,
        // last remove stops them - the shared ConnectivityImplementation semantics).
        virtual maui::core::connection_token add_connectivity_changed(
            maui::core::move_only_function<void(const connectivity_changed_event_args&)> handler) = 0;
        virtual bool remove_connectivity_changed(maui::core::connection_token token) = 0;

    protected:
        i_connectivity() = default;
        i_connectivity(const i_connectivity&) = default;
        i_connectivity(i_connectivity&&) = default;
        i_connectivity& operator=(const i_connectivity&) = default;
        i_connectivity& operator=(i_connectivity&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (ConnectivityImplementation), one per backend under
        // src/platform/<backend>/essentials_connectivity.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_connectivity> make_connectivity();

        // Connectivity.ConnectionProfiles applies .Distinct() (order-preserving, first occurrence).
        [[nodiscard]] std::vector<connection_profile> distinct_profiles(
            const std::vector<connection_profile>& profiles);
    } // namespace detail

    // The static facade over connectivity::current() (C# Connectivity.Current).
    class connectivity final
    {
    public:
        connectivity() = delete;

        [[nodiscard]] static enum network_access network_access()
        {
            return current().network_access();
        }
        // Connectivity.ConnectionProfiles => Current.ConnectionProfiles.Distinct().
        [[nodiscard]] static std::vector<connection_profile> connection_profiles()
        {
            return detail::distinct_profiles(current().connection_profiles());
        }
        static maui::core::connection_token add_connectivity_changed(
            maui::core::move_only_function<void(const connectivity_changed_event_args&)> handler)
        {
            return current().add_connectivity_changed(std::move(handler));
        }
        static bool remove_connectivity_changed(maui::core::connection_token token)
        {
            return current().remove_connectivity_changed(token);
        }

        // Connectivity.Current (lazy platform default) + SetCurrent (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_connectivity& current();
        static void set_current(std::shared_ptr<i_connectivity> implementation);
    };
} // namespace maui::networking
