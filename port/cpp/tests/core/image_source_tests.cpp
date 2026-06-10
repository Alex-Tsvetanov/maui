// Image-source service tests (headless). Areas:
//   (1) image_source_loader's in-memory uri cache + CacheValidity expiry, driven by an INJECTED CLOCK so
//       expiry is deterministic (no wall-clock). The cache reuses bytes for a repeat load of the same uri
//       until the entry is older than the source's cache_validity(), after which it is a miss + re-fetch.
//   (2) the on-DISK uri cache layered under (1): a fetched payload persists under a cache directory keyed by
//       the uri, a disk hit short-circuits the fetch (even on a fresh loader with an empty in-memory cache),
//       and the same CacheValidity clock seam expires disk entries. Ports UriImageSourceService's disk
//       caching (DownloadAndCacheImageAsync + GetCachedFileName/CacheImage/GetCachedImage/IsImageCached).
//   (3) the ASYNC fetch seam: an injected uri_fetch that defers its byte sink (simulating an NSURLSession
//       dataTask completing later, off-thread) + a manual_dispatcher — the apply lands only once the fetch
//       reports AND the dispatcher is pumped, and a superseded load's result is dropped by the identity
//       recheck. Ports ImageSourcePartExtensions.UpdateSourceAsync's marshalled apply + applied-recheck.
//   (4) image_source_service_provider — the typed DI provider layered over the flat registry. Ported from
//       src/Core/tests/UnitTests/ImageSource/ImageSourceServiceTests.cs +
//       ImageSourceToImageSourceServiceTypeMappingTests.cs (resolution to concrete-over-interface, the
//       most-derived-interface walk, ambiguous-match throw, the GetRequiredImageSourceService throw).
#include "maui/core/image_source_service_provider.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

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
#include "maui/core/manual_dispatcher.hpp"
#include "maui/core/move_only_function.hpp"
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
    using maui::core::manual_dispatcher;
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

    // ---- on-disk cache (layered under the in-memory cache; ports DownloadAndCacheImageAsync's disk half) ----

    // A loader with the disk cache pointed at a unique temp directory + an INJECTED uri fetch whose call
    // count the test inspects (so a "disk hit short-circuits the fetch" can be proven). The fetch returns the
    // bytes the test stages, simulating the network (the bytes need not be a real image — the headless decode
    // marks any non-empty buffer loaded). Each loader instance has its OWN in-memory cache, so a second
    // loader over the same directory exercises the persistent disk layer in isolation.
    class loader_disk_cache : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // A cache dir unique ACROSS PROCESSES, removed in TearDown. `ctest -j N` runs every test in its
            // own process, so a per-process counter is NOT unique — two concurrently-running fixture tests
            // would share (and SetUp-delete!) the same directory. The TEST NAME is unique suite-wide (one
            // ctest invocation never runs the same test twice at once), and the random suffix additionally
            // isolates simultaneous ctest invocations (e.g. two checkouts/worktrees on one machine).
            std::random_device entropy;
            const auto* const info = ::testing::UnitTest::GetInstance()->current_test_info();
            dir_ = std::filesystem::path(::testing::TempDir()) /
                   ("maui_disk_cache_" + std::string(info->name()) + "_" + std::to_string(entropy()));
        }
        void TearDown() override
        {
            std::error_code ec;
            std::filesystem::remove_all(dir_, ec);
        }

        // Build a loader sharing the test's disk directory + clock; its injected fetch counts calls and
        // returns `payload_` (empty => a failed fetch). Returned by value into a unique_ptr the test owns.
        std::unique_ptr<image_source_loader> make_loader()
        {
            auto loader = std::make_unique<image_source_loader>();
            loader->set_clock([this] { return clock_; });
            loader->set_disk_cache_directory(dir_.string());
            loader->set_uri_fetch([this](const std::string& /*uri*/, const cancellation_token& /*token*/,
                                         image_source_loader::uri_bytes_sink sink) {
                ++fetch_calls_;
                sink(payload_); // synchronous (inline) — the async marshalling is covered separately
            });
            return loader;
        }

        // Load `uri` through `loader`, recording whether the decoded result was loaded.
        void load(image_source_loader& loader, const std::string& uri, std::chrono::milliseconds validity)
        {
            last_loaded_ = false;
            loader.update_source(image_source::from_uri(uri, /*caching*/ true, validity).get(),
                                 [this](const image_source_result& result) { last_loaded_ = result.loaded(); });
        }

        void advance(std::chrono::milliseconds delta)
        {
            clock_ += delta;
        }

        std::filesystem::path dir_;
        std::chrono::steady_clock::time_point clock_;
        image_bytes payload_{std::byte{1}, std::byte{2}, std::byte{3}};
        int fetch_calls_ = 0;
        bool last_loaded_ = false;
    };

    TEST_F(loader_disk_cache, fetched_bytes_are_persisted_to_disk)
    {
        const std::string uri = "https://example.test/a.png";
        auto loader = make_loader();

        load(*loader, uri, std::chrono::hours(1));
        EXPECT_TRUE(last_loaded_);
        EXPECT_EQ(fetch_calls_, 1);
        // The bytes landed in the disk cache at the crc64(uri)+ext path.
        EXPECT_TRUE(std::filesystem::exists(loader->disk_cache_path(uri)));
    }

    TEST_F(loader_disk_cache, fresh_disk_entry_short_circuits_the_fetch_on_a_new_loader)
    {
        const std::string uri = "https://example.test/b.png";

        // Loader #1 fetches + persists.
        auto first = make_loader();
        load(*first, uri, std::chrono::hours(1));
        ASSERT_TRUE(last_loaded_);
        ASSERT_EQ(fetch_calls_, 1);

        // Loader #2 (fresh, empty in-memory cache) over the SAME directory: the disk hit serves the bytes
        // without invoking the fetch (fetch_calls_ stays 1).
        auto second = make_loader();
        load(*second, uri, std::chrono::hours(1));
        EXPECT_TRUE(last_loaded_);
        EXPECT_EQ(fetch_calls_, 1) << "a fresh disk entry must short-circuit the network fetch";
    }

    TEST_F(loader_disk_cache, expired_disk_entry_is_a_miss_and_refetches)
    {
        const std::string uri = "https://example.test/c.png";

        auto first = make_loader();
        load(*first, uri, std::chrono::hours(1));
        ASSERT_EQ(fetch_calls_, 1);

        // A new loader after the validity window has passed: the disk entry is stale, so it is a miss and the
        // fetch runs again (fetch_calls_ increments).
        advance(std::chrono::hours(2)); // > 1h validity
        auto second = make_loader();
        load(*second, uri, std::chrono::hours(1));
        EXPECT_TRUE(last_loaded_);
        EXPECT_EQ(fetch_calls_, 2) << "an expired disk entry must re-fetch";
    }

    TEST_F(loader_disk_cache, a_failed_fetch_writes_nothing_to_disk)
    {
        const std::string uri = "https://example.test/d.png";
        payload_ = {}; // the network yields no bytes
        auto loader = make_loader();

        load(*loader, uri, std::chrono::hours(1));
        EXPECT_FALSE(last_loaded_);
        EXPECT_FALSE(std::filesystem::exists(loader->disk_cache_path(uri)));
    }

    // A disk hit repopulates the in-memory layer with the DISK entry's ORIGINAL timestamp (not the read
    // time), so the in-memory copy expires on the same schedule — its lifetime is not silently extended.
    TEST_F(loader_disk_cache, disk_hit_repopulates_in_memory_without_extending_lifetime)
    {
        const std::string uri = "https://example.test/e.png";

        // Loader #1 fetches + persists at T=0.
        auto first = make_loader();
        load(*first, uri, std::chrono::hours(1));
        ASSERT_EQ(fetch_calls_, 1);

        // At T+50min, a fresh loader reads the disk (still within 1h) and repopulates its in-memory entry —
        // stamped with the disk's original T=0, not T+50min.
        advance(std::chrono::minutes(50));
        auto second = make_loader();
        load(*second, uri, std::chrono::hours(1));
        ASSERT_TRUE(last_loaded_);
        ASSERT_EQ(fetch_calls_, 1) << "still a disk hit at T+50min";

        // At T+1h05min (past the 1h window from the ORIGINAL fetch), the SAME loader#2 reload must be a miss
        // on BOTH layers (the in-memory copy carries T=0, so it is expired too) → it re-fetches. Had the
        // in-memory copy been stamped at T+50min, it would still be "fresh" here and wrongly short-circuit.
        advance(std::chrono::minutes(15)); // now T+1h05min
        load(*second, uri, std::chrono::hours(1));
        EXPECT_TRUE(last_loaded_);
        EXPECT_EQ(fetch_calls_, 2) << "the repopulated in-memory entry must expire on the original schedule";
    }

    // ---- async fetch + dispatcher marshalling (ports UpdateSourceAsync's deferred apply + applied recheck) ----

    // A loader with a manual_dispatcher and a uri fetch that DEFERS its byte sink (stashes it for the test to
    // fire later, like an NSURLSession completion arriving off-thread). The apply is observed only after BOTH
    // the sink fires AND the dispatcher is pumped.
    class loader_async : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            loader_.set_dispatcher(dispatcher_);
            loader_.set_uri_fetch([this](const std::string& /*uri*/, const cancellation_token& token,
                                         image_source_loader::uri_bytes_sink sink) {
                pending_token_ = token;
                pending_sink_ = std::move(sink); // deferred — the test calls complete_fetch() to fire it
            });
        }

        // Kick a load; the apply records the (loaded) result + bumps applied_count_.
        void start(const std::shared_ptr<i_image_source>& source)
        {
            loader_.update_source(source.get(), [this](const image_source_result& result) {
                ++applied_count_;
                last_loaded_ = result.loaded();
            });
        }

        // Fire the deferred fetch with `bytes` (the "download finished" callback).
        void complete_fetch(image_bytes bytes)
        {
            ASSERT_TRUE(static_cast<bool>(pending_sink_));
            auto sink = std::move(pending_sink_);
            sink(std::move(bytes));
        }

        manual_dispatcher dispatcher_;
        image_source_loader loader_;
        cancellation_token pending_token_;
        image_source_loader::uri_bytes_sink pending_sink_;
        int applied_count_ = 0;
        bool last_loaded_ = false;
    };

    TEST_F(loader_async, apply_waits_for_both_the_fetch_and_the_dispatcher_pump)
    {
        const auto keep = image_source::from_uri("https://example.test/async.png", /*caching*/ false);
        start(keep);

        // The fetch is in flight: nothing applied yet, and the loader reports loading.
        EXPECT_EQ(applied_count_, 0);
        EXPECT_TRUE(loader_.is_loading());

        // The download completes (off-thread) but the apply is marshalled — still not applied until pumped.
        complete_fetch(image_bytes{std::byte{9}});
        EXPECT_EQ(applied_count_, 0);

        // Pumping the dispatcher runs the marshalled apply.
        EXPECT_EQ(dispatcher_.run_pending(), 1U);
        EXPECT_EQ(applied_count_, 1);
        EXPECT_TRUE(last_loaded_);
        EXPECT_FALSE(loader_.is_loading()); // completed
    }

    TEST_F(loader_async, a_superseded_async_load_is_dropped_by_the_identity_recheck)
    {
        const auto first = image_source::from_uri("https://example.test/first.png", /*caching*/ false);
        start(first);
        // The first fetch is in flight; capture its (now superseded) completion.
        auto first_sink = std::move(pending_sink_);

        // A SECOND load supersedes the first (begin_load cancels the first token; the second source becomes
        // current). pending_sink_/pending_token_ now belong to the second load.
        const auto second = image_source::from_uri("https://example.test/second.png", /*caching*/ false);
        start(second);
        EXPECT_FALSE(pending_token_.is_cancelled()); // the second load's token is live

        // The first (stale) download now completes + is pumped: its result is dropped (source no longer
        // current), so the apply does NOT fire for it.
        ASSERT_TRUE(static_cast<bool>(first_sink));
        auto stale = std::move(first_sink);
        stale(image_bytes{std::byte{1}});
        dispatcher_.run_pending();
        EXPECT_EQ(applied_count_, 0) << "a superseded load's apply must be dropped";

        // The second (current) download completes + is pumped: it applies.
        complete_fetch(image_bytes{std::byte{2}});
        dispatcher_.run_pending();
        EXPECT_EQ(applied_count_, 1);
        EXPECT_TRUE(last_loaded_);
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
