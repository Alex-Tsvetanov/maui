#pragma once
// maui::networking::detail::connectivity_base  <=  the cross-platform half of
// Microsoft.Maui.Networking.ConnectivityImplementation (Connectivity.shared.cs): the
// listener-starting event accessors plus the change-dedupe cache.
//
// The FIRST ConnectivityChanged subscriber runs SetCurrent() (snapshots the current access +
// profiles) and then platform_start_listeners() BEFORE the handler is stored, so a throwing start
// (the netstandard mirror) leaves no subscription behind; the last removal stops the listeners.
// on_connectivity_changed() re-reads access + profiles and raises only when the access differs OR
// the profile sequence differs (C#'s `currentAccess != e.NetworkAccess || !currentProfiles
// .SequenceEqual(e.ConnectionProfiles)`), re-snapshotting before each raise. The C# raise is
// marshalled to the main thread via MainThread.BeginInvokeOnMainThread; the port leaves that to the
// platform partial (the apple .mm dispatches the listener callback onto the main queue before
// calling on_connectivity_changed(); the headless fake raises inline). The value getters
// (network_access / connection_profiles) stay pure virtual - they ARE the platform partial in C#.

#include <utility>
#include <vector>

#include "maui/core/event.hpp"
#include "maui/essentials/connectivity.hpp"

namespace maui::networking::detail
{
    class connectivity_base : public i_connectivity
    {
    public:
        maui::core::connection_token add_connectivity_changed(
            maui::core::move_only_function<void(const connectivity_changed_event_args&)> handler) override
        {
            if (subscribers_ == 0)
            {
                set_current(); // snapshot before the first listener starts
                platform_start_listeners();
            }
            ++subscribers_;
            return connectivity_changed_.connect(std::move(handler));
        }

        bool remove_connectivity_changed(maui::core::connection_token token) override
        {
            if (!connectivity_changed_.disconnect(token))
            {
                return false;
            }
            if (--subscribers_ == 0)
            {
                platform_stop_listeners();
            }
            return true;
        }

    protected:
        connectivity_base() = default;

        // Start/StopListeners (the platform partial).
        virtual void platform_start_listeners() = 0;
        virtual void platform_stop_listeners() = 0;

        // OnConnectivityChanged(): re-read, dedupe against the snapshot, raise (re-snapshotting).
        void on_connectivity_changed()
        {
            connectivity_changed_event_args args{network_access(), connection_profiles()};
            if (current_access_ != args.network_access || current_profiles_ != args.connection_profiles)
            {
                current_access_ = args.network_access;
                current_profiles_ = args.connection_profiles;
                connectivity_changed_.raise(args);
            }
        }

    private:
        // SetCurrent(): snapshot the present access + profiles (so the first change is compared
        // against the state at subscribe time).
        void set_current()
        {
            current_access_ = network_access();
            current_profiles_ = connection_profiles();
        }

        maui::core::event<connectivity_changed_event_args> connectivity_changed_;
        int subscribers_ = 0;
        enum network_access current_access_ = network_access::unknown;
        std::vector<connection_profile> current_profiles_;
    };
} // namespace maui::networking::detail
