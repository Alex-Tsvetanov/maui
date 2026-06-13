// connectivity on the headless backend. Ports Connectivity_Tests.cs (the netstandard throws -
// including the listener start hidden behind the ConnectivityChanged add) and exercises the shared
// ConnectivityImplementation logic via the fake: the listener lifecycle behind add/remove, the
// facade's ConnectionProfiles.Distinct(), and OnConnectivityChanged's change-dedupe (raise only
// when access or the profile sequence actually changed, per C#'s SequenceEqual compare).

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/connectivity.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::networking;
    using maui::application_model::feature_not_supported;

    class connectivity_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            connectivity::set_current(nullptr);
        }
        void TearDown() override
        {
            connectivity::set_current(nullptr);
        }
    };

    // Connectivity_Tests: NetworkAccess / ConnectionProfiles / ConnectivityChanged on netstandard
    // all throw NotImplementedInReferenceAssemblyException (mapped to feature_not_supported).
    TEST_F(connectivity_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW((void)connectivity::network_access(), feature_not_supported);
        EXPECT_THROW((void)connectivity::connection_profiles(), feature_not_supported);
        EXPECT_THROW((void)connectivity::add_connectivity_changed([](const connectivity_changed_event_args&) {}),
                     feature_not_supported);
    }

    TEST_F(connectivity_test, configured_fake_reads_through_facade)
    {
        auto fake = std::make_shared<headless_connectivity>();
        fake->set_network_access(network_access::internet);
        fake->set_connection_profiles({connection_profile::wifi, connection_profile::cellular});
        connectivity::set_current(fake);

        EXPECT_EQ(connectivity::network_access(), network_access::internet);
        EXPECT_EQ(connectivity::connection_profiles(),
                  (std::vector<connection_profile>{connection_profile::wifi, connection_profile::cellular}));
    }

    // Connectivity.ConnectionProfiles applies LINQ Distinct(): order-preserving, dedupe later dups.
    TEST_F(connectivity_test, connection_profiles_are_distinct)
    {
        auto fake = std::make_shared<headless_connectivity>();
        fake->set_network_access(network_access::internet);
        fake->set_connection_profiles({connection_profile::wifi, connection_profile::wifi, connection_profile::cellular,
                                       connection_profile::wifi});
        connectivity::set_current(fake);

        EXPECT_EQ(connectivity::connection_profiles(),
                  (std::vector<connection_profile>{connection_profile::wifi, connection_profile::cellular}));
    }

    TEST_F(connectivity_test, connectivity_changed_lifecycle_and_dedupe)
    {
        auto fake = std::make_shared<headless_connectivity>();
        fake->set_network_access(network_access::internet);
        fake->set_connection_profiles({connection_profile::wifi});
        connectivity::set_current(fake);

        int raises = 0;
        connectivity_changed_event_args last{};
        EXPECT_FALSE(fake->is_listening());
        const auto token = connectivity::add_connectivity_changed([&](const connectivity_changed_event_args& args) {
            ++raises;
            last = args;
        });
        EXPECT_TRUE(fake->is_listening());

        // Unchanged values (the add snapshotted internet/[wifi]) -> deduped.
        fake->simulate_connectivity_changed();
        EXPECT_EQ(raises, 0);

        // Access changes -> raise.
        fake->set_network_access(network_access::none);
        fake->set_connection_profiles({});
        fake->simulate_connectivity_changed();
        EXPECT_EQ(raises, 1);
        EXPECT_EQ(last.network_access, network_access::none);
        EXPECT_TRUE(last.connection_profiles.empty());

        // Re-raise with the same state -> deduped (the args were re-snapshotted).
        fake->simulate_connectivity_changed();
        EXPECT_EQ(raises, 1);

        // Only the profile sequence changes -> raise (C#'s SequenceEqual compare).
        fake->set_connection_profiles({connection_profile::cellular});
        fake->simulate_connectivity_changed();
        EXPECT_EQ(raises, 2);
        EXPECT_EQ(last.connection_profiles, (std::vector<connection_profile>{connection_profile::cellular}));

        EXPECT_TRUE(connectivity::remove_connectivity_changed(token));
        EXPECT_FALSE(fake->is_listening());
    }
} // namespace
