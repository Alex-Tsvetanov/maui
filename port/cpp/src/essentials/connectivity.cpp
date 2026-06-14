// The cross-platform half of the connectivity facade: the lazily-created implementation slot behind
// Connectivity.Current / Connectivity.SetCurrent, plus the ConnectionProfiles `.Distinct()` the C#
// facade applies. The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_connectivity.{cpp,mm}), reached through
// detail::make_connectivity() - the C# `currentImplementation ??= new ConnectivityImplementation()`.

#include "maui/essentials/connectivity.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace maui::networking
{
    namespace
    {
        std::shared_ptr<i_connectivity>& connectivity_storage()
        {
            static std::shared_ptr<i_connectivity> storage;
            return storage;
        }
    } // namespace

    namespace detail
    {
        std::vector<connection_profile> distinct_profiles(const std::vector<connection_profile>& profiles)
        {
            // LINQ Distinct(): preserve order, drop later duplicates. The set is tiny (<= 5 enum
            // values), so a linear membership scan is cheaper than a hash set.
            std::vector<connection_profile> result;
            for (const connection_profile profile : profiles)
            {
                bool seen = false;
                for (const connection_profile kept : result)
                {
                    if (kept == profile)
                    {
                        seen = true;
                        break;
                    }
                }
                if (!seen)
                {
                    result.push_back(profile);
                }
            }
            return result;
        }
    } // namespace detail

    i_connectivity& connectivity::current()
    {
        auto& storage = connectivity_storage();
        if (storage == nullptr)
        {
            storage = detail::make_connectivity();
        }
        return *storage;
    }

    void connectivity::set_current(std::shared_ptr<i_connectivity> implementation)
    {
        connectivity_storage() = std::move(implementation);
    }
} // namespace maui::networking
