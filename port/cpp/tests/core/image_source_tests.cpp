// Image-source service tests (headless). Two areas:
//   (1) image_source_loader's in-memory uri cache + CacheValidity expiry, driven by an INJECTED CLOCK so
//       expiry is deterministic (no wall-clock). The cache reuses bytes for a repeat load of the same uri
//       until the entry is older than the source's cache_validity(), after which it is a miss + re-fetch.
//   (2) image_source_service_provider — the typed DI provider layered over the flat registry. Ported from
//       src/Core/tests/UnitTests/ImageSource/ImageSourceServiceTests.cs +
//       ImageSourceToImageSourceServiceTypeMappingTests.cs (resolution to concrete-over-interface, the
//       most-derived-interface walk, ambiguous-match throw, the GetRequiredImageSourceService throw).
#include "maui/core/image_source_service_provider.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <ios>
#include <memory>
#include <stdexcept>
#include <string>

#include "maui/controls/file_image_source.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/file_image_source_service.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_image_source_service.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/image_source_service_registry.hpp"
#include "maui/core/image_source_services.hpp"
#include "maui/core/stream_image_source_service.hpp"
#include "maui/core/uri_image_source_service.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image_source;
    using maui::core::cancellation_token;
    using maui::core::file_image_source_service;
    using maui::core::i_image_source;
    using maui::core::i_image_source_service;
    using maui::core::image_bytes;
    using maui::core::image_source_loader;
    using maui::core::image_source_result;
    using maui::core::image_source_service_provider;
    using maui::core::image_source_service_registry;
    using maui::core::register_default_image_source_services;
    using maui::core::stream_image_source_service;
    using maui::core::uri_image_source_service;

    // A loader test fixture: an inline (no-dispatcher) loader whose clock is a member the test advances.
    class loader_cache : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            loader_.set_clock([this] { return clock_; });
        }

        // Apply that records how many bytes the decoded result mirrored (the headless "uri" detail is the
        // uri; "loaded" tells us whether a fetch produced anything). We capture the loaded flag only.
        void load(const std::shared_ptr<i_image_source>& source)
        {
            last_loaded_ = false;
            loader_.update_source(source.get(),
                                  [this](const image_source_result& result) { last_loaded_ = result.loaded(); });
        }

        void advance(std::chrono::milliseconds delta)
        {
            clock_ += delta;
        }

        image_source_loader loader_;
        std::chrono::steady_clock::time_point clock_;
        bool last_loaded_ = false;
    };

    // Write `contents` to `path` (binary).
    void write_file(const std::string& path, const std::string& contents)
    {
        std::ofstream out(path, std::ios::binary);
        out << contents;
    }

    std::string temp_path(const char* name)
    {
        return std::string(::testing::TempDir()) + name;
    }

    TEST_F(loader_cache, repeat_load_within_validity_reuses_cached_bytes)
    {
        const std::string path = temp_path("maui_ttl_reuse.bin");
        write_file(path, "abcd");
        const std::string uri = "file://" + path;

        load(image_source::from_uri(uri, /*caching*/ true, std::chrono::hours(1)));
        EXPECT_TRUE(last_loaded_);

        // Delete the file, then reload the SAME uri within the validity window: the cache still has the
        // bytes, so it loads without re-reading disk.
        std::remove(path.c_str());
        advance(std::chrono::minutes(30)); // still < 1h validity
        load(image_source::from_uri(uri, true, std::chrono::hours(1)));
        EXPECT_TRUE(last_loaded_);
    }

    TEST_F(loader_cache, entry_past_cache_validity_is_a_miss)
    {
        const std::string path = temp_path("maui_ttl_expire.bin");
        write_file(path, "abcd");
        const std::string uri = "file://" + path;

        load(image_source::from_uri(uri, true, std::chrono::hours(1)));
        EXPECT_TRUE(last_loaded_);

        // Delete the file, advance PAST the validity window, reload: the cached entry has expired, so it is
        // a miss; the re-fetch now fails (file gone) → nothing loaded.
        std::remove(path.c_str());
        advance(std::chrono::hours(2)); // > 1h validity
        load(image_source::from_uri(uri, true, std::chrono::hours(1)));
        EXPECT_FALSE(last_loaded_);
    }

    TEST_F(loader_cache, expired_entry_is_refreshed_when_the_source_is_still_available)
    {
        const std::string path = temp_path("maui_ttl_refresh.bin");
        write_file(path, "abcd");
        const std::string uri = "file://" + path;

        load(image_source::from_uri(uri, true, std::chrono::hours(1)));
        EXPECT_TRUE(last_loaded_);

        // Past expiry but the file still exists: the entry is re-fetched + re-stamped, so it loads again,
        // and a subsequent load within the NEW window reuses the refreshed entry even after deletion.
        advance(std::chrono::hours(2));
        load(image_source::from_uri(uri, true, std::chrono::hours(1)));
        EXPECT_TRUE(last_loaded_);

        std::remove(path.c_str());
        advance(std::chrono::minutes(10)); // within the refreshed window
        load(image_source::from_uri(uri, true, std::chrono::hours(1)));
        EXPECT_TRUE(last_loaded_);
    }

    TEST_F(loader_cache, caching_disabled_never_serves_from_cache)
    {
        const std::string path = temp_path("maui_ttl_nocache.bin");
        write_file(path, "abcd");
        const std::string uri = "file://" + path;

        load(image_source::from_uri(uri, /*caching*/ false));
        EXPECT_TRUE(last_loaded_);

        // With caching off, the bytes are never cached; deleting the file makes the next load fail.
        std::remove(path.c_str());
        load(image_source::from_uri(uri, false));
        EXPECT_FALSE(last_loaded_);
    }

    // ---- image_source_service_provider (the typed DI provider over the registry) ----

    // Two stub source interfaces + their services (ports the IFirst/ISecond setup of
    // ImageSourceServiceTests.cs). Each concrete source implements exactly one interface (our resolution
    // model). The services are distinguishable by dynamic_cast.
    struct i_first_image_source : maui::core::i_image_source
    {
    };
    struct i_second_image_source : maui::core::i_image_source
    {
    };
    struct first_image_source final : i_first_image_source
    {
        [[nodiscard]] bool is_empty() const override
        {
            return false;
        }
    };
    struct second_image_source final : i_second_image_source
    {
        [[nodiscard]] bool is_empty() const override
        {
            return false;
        }
    };
    struct first_service final : i_image_source_service
    {
        void load(i_image_source& /*source*/, const cancellation_token& /*token*/, completion on_result) override
        {
            on_result(image_source_result{});
        }
    };
    struct second_service final : i_image_source_service
    {
        void load(i_image_source& /*source*/, const cancellation_token& /*token*/, completion on_result) override
        {
            on_result(image_source_result{});
        }
    };

    // ResolvesCorrectServiceInstance: each source kind resolves to its own registered service instance.
    TEST(image_source_service_provider, resolves_correct_service_per_source_kind)
    {
        image_source_service_provider provider;
        provider.register_service<i_first_image_source, first_service>();
        provider.register_service<i_second_image_source, second_service>();

        const first_image_source first;
        const second_image_source second;

        const auto for_first = provider.get_required_image_source_service(first);
        const auto for_second = provider.get_required_image_source_service(second);

        EXPECT_NE(dynamic_cast<first_service*>(for_first.get()), nullptr);
        EXPECT_NE(dynamic_cast<second_service*>(for_second.get()), nullptr);
    }

    // A repeated resolve returns the SAME cached service instance (C#'s _serviceCache; our registry caches).
    TEST(image_source_service_provider, resolves_same_service_instance_across_calls)
    {
        image_source_service_provider provider;
        provider.register_service<i_first_image_source, first_service>();

        const first_image_source first;
        EXPECT_EQ(provider.get_image_source_service(first), provider.get_image_source_service(first));
    }

    // get_image_source_service returns nullptr when nothing is registered; the "required" variant throws.
    TEST(image_source_service_provider, unregistered_source_returns_null_and_required_throws)
    {
        image_source_service_provider provider;
        const first_image_source first;

        EXPECT_EQ(provider.get_image_source_service(first), nullptr);
        EXPECT_THROW((void)provider.get_required_image_source_service(first), std::runtime_error);
    }

    // The provider can layer over an existing registry (e.g. the default one) without re-registering —
    // HostBuilderImageSourceTests.CanRetrieveFileUsingInterfaceImageSource analog for the built-in file/uri/
    // stream services.
    TEST(image_source_service_provider, layers_over_an_existing_registry)
    {
        image_source_service_registry registry;
        register_default_image_source_services(registry);
        const image_source_service_provider provider(registry);

        const auto file_src = image_source::from_file("/tmp/p.png");
        const auto uri_src = image_source::from_uri("file:///tmp/p.png");
        const auto stream_src =
            image_source::from_stream([](const cancellation_token&) { return image_bytes(2, std::byte{0}); });

        EXPECT_NE(dynamic_cast<file_image_source_service*>(provider.get_required_image_source_service(*file_src).get()),
                  nullptr);
        EXPECT_NE(dynamic_cast<uri_image_source_service*>(provider.get_required_image_source_service(*uri_src).get()),
                  nullptr);
        EXPECT_NE(
            dynamic_cast<stream_image_source_service*>(provider.get_required_image_source_service(*stream_src).get()),
            nullptr);
    }
} // namespace
