#pragma once
// The Connectivity implementation, ONE Obj-C++ definition for BOTH Apple backends - the analog of
// the single Connectivity.ios.tvos.macos.cs partial. The C# original wraps the now-deprecated
// SCNetworkReachability (Connectivity.ios.tvos.macos.reachability.cs); the port instead uses
// Network.framework's nw_path_monitor, the modern Apple API for exactly this, while preserving the
// NetworkAccess / ConnectionProfile contract:
//   * network_access(): a synchronous nw_path snapshot via a momentary monitor -
//     nw_path_status_satisfied => Internet (constrained => ConstrainedInternet), satisfiable =>
//     None (the connection could be established but isn't), otherwise None. Mirrors the C# getter's
//     Internet-or-None result, plus the ConstrainedInternet state the modern API exposes.
//   * connection_profiles(): the interface types the current path uses
//     (nw_path_uses_interface_type) -> WiFi / Cellular / Ethernet (wired) / Bluetooth, in that probe
//     order; empty when the path is not satisfied (the C# `GetActiveConnectionType` yield set).
//   * Start/StopListeners: a long-lived nw_path_monitor whose update handler is dispatched on the
//     MAIN queue (the C# MainThread.BeginInvokeOnMainThread marshalling) and calls
//     on_connectivity_changed() - the shared dedupe in connectivity_base does the rest. The path is
//     also cached from the monitor so the getters reflect the live path while listening.
// Included by src/platform/{apple,ios}/essentials_connectivity.mm (the .mm provides
// make_connectivity).

#import <Network/Network.h>
#import <dispatch/dispatch.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "maui/essentials/connectivity.hpp"

#include "src/essentials/detail/connectivity_base.hpp"

namespace maui::networking::apple_shared
{
    namespace path_detail
    {
        // nw_path -> NetworkAccess.
        inline enum network_access access_from_path(nw_path_t path)
        {
            if (path == nullptr)
            {
                return network_access::none;
            }
            switch (nw_path_get_status(path))
            {
                case nw_path_status_satisfied:
                    return nw_path_is_constrained(path) ? network_access::constrained_internet
                                                        : network_access::internet;
                case nw_path_status_satisfiable:
                case nw_path_status_unsatisfied:
                case nw_path_status_invalid:
                default:
                    return network_access::none;
            }
        }

        // nw_path -> the interface types it uses (GetActiveConnectionType's set).
        inline std::vector<connection_profile> profiles_from_path(nw_path_t path)
        {
            std::vector<connection_profile> result;
            if (path == nullptr || nw_path_get_status(path) != nw_path_status_satisfied)
            {
                return result;
            }
            if (nw_path_uses_interface_type(path, nw_interface_type_wifi))
            {
                result.push_back(connection_profile::wifi);
            }
            if (nw_path_uses_interface_type(path, nw_interface_type_cellular))
            {
                result.push_back(connection_profile::cellular);
            }
            if (nw_path_uses_interface_type(path, nw_interface_type_wired))
            {
                result.push_back(connection_profile::ethernet);
            }
            if (nw_path_uses_interface_type(path, nw_interface_type_other) && result.empty())
            {
                // An "other" loopback/VPN/virtual path with no concrete medium reports Unknown,
                // mirroring the C# default branch of GetActiveConnectionType.
                result.push_back(connection_profile::unknown);
            }
            return result;
        }
    } // namespace path_detail

    // ARC manages every Network.framework object here: nw_path_t / nw_path_monitor_t conform to
    // OS_OBJECT_USE_OBJC, so __strong members and block captures are retained/released automatically
    // (these .mm files compile with -fobjc-arc) - no manual nw_retain/nw_release.
    class path_monitor_connectivity final : public detail::connectivity_base
    {
    public:
        ~path_monitor_connectivity() override
        {
            stop();
        }

        [[nodiscard]] enum network_access network_access() const override
        {
            return path_detail::access_from_path(current_path());
        }

        [[nodiscard]] std::vector<connection_profile> connection_profiles() const override
        {
            return path_detail::profiles_from_path(current_path());
        }

    protected:
        // StartListeners: a main-queue nw_path_monitor whose updates cache the path and re-raise
        // through the shared dedupe. The cached path keeps the getters live while listening.
        void platform_start_listeners() override
        {
            monitor_ = nw_path_monitor_create();
            nw_path_monitor_set_queue(monitor_, dispatch_get_main_queue());
            // The block holds the alive flag, not `this`: an update already queued when the
            // implementation is destroyed (set_current swap) re-checks the flag rather than touching
            // the dead object.
            const std::shared_ptr<std::atomic<bool>> alive = alive_;
            path_monitor_connectivity* const self = this;
            nw_path_monitor_set_update_handler(monitor_, ^(nw_path_t path) {
              if (!*alive)
              {
                  return;
              }
              self->cached_path_ = path; // __strong member: ARC retains
              self->on_connectivity_changed();
            });
            nw_path_monitor_start(monitor_);
        }

        void platform_stop_listeners() override
        {
            stop();
        }

    private:
        void stop()
        {
            *alive_ = false;
            if (monitor_ != nullptr)
            {
                nw_path_monitor_cancel(monitor_);
                monitor_ = nullptr; // ARC releases
            }
            cached_path_ = nullptr;                             // ARC releases
            alive_ = std::make_shared<std::atomic<bool>>(true); // fresh flag for a later restart
        }

        // The path the getters read: the live cached path while listening, otherwise a momentary
        // synchronous probe.
        [[nodiscard]] nw_path_t current_path() const
        {
            return cached_path_ != nullptr ? cached_path_ : probe_path();
        }

        // A one-shot path snapshot: spin up a monitor on a private queue, capture the first update,
        // cancel. The captured path is read back on the calling thread; a std::mutex guards the
        // shared slot so a late handler invocation on the probe queue (after a wait timeout, before
        // cancel takes effect) cannot race the read. The captured path is ARC-retained.
        [[nodiscard]] static nw_path_t probe_path()
        {
            nw_path_monitor_t monitor = nw_path_monitor_create();
            dispatch_queue_t queue = dispatch_queue_create("maui.connectivity.probe", DISPATCH_QUEUE_SERIAL);
            nw_path_monitor_set_queue(monitor, queue);
            dispatch_semaphore_t done = dispatch_semaphore_create(0);
            const auto guard = std::make_shared<std::mutex>();
            __block nw_path_t captured = nullptr;
            nw_path_monitor_set_update_handler(monitor, ^(nw_path_t path) {
              const std::lock_guard<std::mutex> lock(*guard);
              if (captured == nullptr)
              {
                  captured = path; // ARC retains into the __block local
                  dispatch_semaphore_signal(done);
              }
            });
            nw_path_monitor_start(monitor);
            dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(2 * NSEC_PER_SEC)));
            nw_path_monitor_cancel(monitor);
            const std::lock_guard<std::mutex> lock(*guard);
            return captured;
        }

        nw_path_monitor_t monitor_ = nullptr;
        nw_path_t cached_path_ = nullptr;
        std::shared_ptr<std::atomic<bool>> alive_ = std::make_shared<std::atomic<bool>>(true);
    };
} // namespace maui::networking::apple_shared
