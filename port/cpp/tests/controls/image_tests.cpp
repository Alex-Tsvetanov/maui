// Tests for the image control + its headless handler seam — a display-only control. Setting aspect (and a
// source) flows virtual→native into the headless image_platform mirror (image_aspect / source_kind /
// source_file / source_loaded). A FILE source loads synchronously; uri/stream sources load through the
// handler-owned image_source_loader (the async path — driven deterministically via a manual_dispatcher).
#include "maui/controls/image.hpp"

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <ios>
#include <memory>
#include <string>

#include "maui/controls/file_image_source.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/cancellation_token.hpp"
#include "maui/core/file_image_source_service.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_stream_image_source.hpp"
#include "maui/core/image_handler.hpp"
#include "maui/core/image_source_service_registry.hpp"
#include "maui/core/image_source_services.hpp"
#include "maui/core/manual_dispatcher.hpp"
#include "maui/core/stream_image_source_service.hpp"
#include "maui/core/uri_image_source_service.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::image;
    using maui::controls::image_source;
    using maui::core::aspect;
    using maui::core::cancellation_token;
    using maui::core::i_element_handler;
    using maui::core::i_file_image_source;
    using maui::core::i_image;
    using maui::core::image_bytes;
    using maui::core::image_handler;
    using maui::core::manual_dispatcher;

    // A stream source that yields a fixed number of bytes (the value doesn't matter to the headless mirror;
    // the byte COUNT distinguishes which source applied via the "<bytes:N>" detail).
    std::shared_ptr<maui::core::i_image_source> make_stream_source(std::size_t byte_count)
    {
        return image_source::from_stream(
            [byte_count](const cancellation_token&) { return image_bytes(byte_count, std::byte{0x7F}); });
    }

    TEST(image, aspect_defaults_to_aspect_fit_and_is_settable)
    {
        image control;
        EXPECT_EQ(control.aspect(), aspect::aspect_fit);
        control.set_aspect(aspect::fill);
        EXPECT_EQ(control.aspect(), aspect::fill);
    }

    TEST(image, usable_through_interface_reference)
    {
        image control;
        control.set_aspect(aspect::center);
        i_image& as_image = control;
        EXPECT_EQ(as_image.aspect(), aspect::center);
    }

    TEST(image_seam, attaching_handler_maps_initial_aspect)
    {
        image control;
        control.set_aspect(aspect::aspect_fill);
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_EQ(handler->virtual_view(), &control);
        EXPECT_EQ(handler->typed_platform_view()->image_aspect, aspect::aspect_fill);
    }

    TEST(image_seam, setting_aspect_maps_to_platform)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_aspect(aspect::fill);
        EXPECT_EQ(platform->image_aspect, aspect::fill);

        control.set_aspect(aspect::center);
        EXPECT_EQ(platform->image_aspect, aspect::center);
    }

    TEST(image_seam, handler_resolved_from_default_registry)
    {
        // image -> image_handler is self-registered in image.cpp (MAUI_REGISTER_HANDLER).
        std::shared_ptr<i_element_handler> const handler =
            maui::core::default_handler_registry().create_handler<image>();
        ASSERT_NE(handler, nullptr);
        auto* resolved = dynamic_cast<image_handler*>(handler.get());
        ASSERT_NE(resolved, nullptr);

        image control;
        control.set_aspect(aspect::fill);
        control.set_handler(handler);
        EXPECT_EQ(resolved->typed_platform_view()->image_aspect, aspect::fill);
    }

    // ---- source (file source, synchronous load; headless mirror) ----

    TEST(image, source_defaults_to_null_and_owns_a_set_source)
    {
        image control;
        EXPECT_EQ(control.source(), nullptr);

        control.set_source(image_source::from_file("/tmp/picture.png"));
        auto* src = control.source(); // raw borrow into the owned shared_ptr
        ASSERT_NE(src, nullptr);
        EXPECT_FALSE(src->is_empty());
        const auto* file_src = dynamic_cast<const i_file_image_source*>(src);
        ASSERT_NE(file_src, nullptr);
        EXPECT_EQ(file_src->file(), "/tmp/picture.png");
    }

    TEST(image, factory_makes_an_empty_source_for_an_empty_path)
    {
        auto src = image_source::from_file("");
        ASSERT_NE(src, nullptr);
        EXPECT_TRUE(src->is_empty());
    }

    TEST(image_seam, attaching_handler_loads_initial_source)
    {
        image control;
        control.set_source(image_source::from_file("/tmp/initial.png"));
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);

        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_file, "/tmp/initial.png");
    }

    TEST(image_seam, setting_file_source_maps_to_platform)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // No source yet -> nothing loaded.
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_TRUE(platform->source_file.empty());

        control.set_source(image_source::from_file("/tmp/a.png"));
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_file, "/tmp/a.png");

        // Replacing with a different source re-loads.
        control.set_source(image_source::from_file("/tmp/b.png"));
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_file, "/tmp/b.png");
    }

    TEST(image_seam, empty_source_does_not_load)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(image_source::from_file(""));
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_TRUE(platform->source_file.empty());
    }

    TEST(image_seam, clearing_source_back_to_null_clears_the_load)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(image_source::from_file("/tmp/c.png"));
        ASSERT_TRUE(platform->source_loaded);

        control.set_source(nullptr);
        EXPECT_FALSE(platform->source_loaded);
        EXPECT_TRUE(platform->source_file.empty());
    }

    // The file fast-path also records the kind, distinguishing it from the async (uri/stream) loads.
    TEST(image_seam, file_source_records_kind_file)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        control.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(image_source::from_file("/tmp/k.png"));
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "file");
    }

    // ---- (a) stream source loaded through the loader (async, pumped via manual_dispatcher) ----

    TEST(image_seam, stream_source_loads_through_loader)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        manual_dispatcher disp;
        control.set_handler(handler);
        handler->source_loader().set_dispatcher(disp);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        control.set_source(make_stream_source(4));
        // The apply is marshalled onto the dispatcher: nothing is loaded until it is pumped.
        EXPECT_FALSE(platform->source_loaded);

        const std::size_t ran = disp.run_pending();
        EXPECT_GE(ran, 1U);
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "stream");
        EXPECT_EQ(platform->source_file, "<bytes:4>"); // detail = the decoded byte count
    }

    // ---- (b) begin_load cancels the previous in-flight load (identity recheck) ----

    TEST(image_seam, begin_load_cancels_the_previous_in_flight_load)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        manual_dispatcher disp;
        control.set_handler(handler);
        handler->source_loader().set_dispatcher(disp);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Set source A (3 bytes) then source B (5 bytes) BEFORE pumping: both applies are queued, but A's
        // load was cancelled + superseded by B (begin_load cancels the previous token and the source no
        // longer matches), so only B applies after the pump.
        control.set_source(make_stream_source(3));
        control.set_source(make_stream_source(5));
        EXPECT_FALSE(platform->source_loaded); // neither applied yet

        disp.run_pending();

        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "stream");
        EXPECT_EQ(platform->source_file, "<bytes:5>"); // B applied; A's apply was skipped by the recheck
    }

    // ---- (c) the registry resolves file / uri / stream sources to their services ----

    TEST(image_source_registry, resolves_each_source_kind_to_its_service)
    {
        maui::core::image_source_service_registry registry;
        maui::core::register_default_image_source_services(registry);

        const auto file_src = image_source::from_file("/tmp/r.png");
        const auto uri_src = image_source::from_uri("file:///tmp/r.png");
        const auto stream_src = make_stream_source(2);

        const auto file_service = registry.resolve(*file_src);
        const auto uri_service = registry.resolve(*uri_src);
        const auto stream_service = registry.resolve(*stream_src);

        ASSERT_NE(file_service, nullptr);
        ASSERT_NE(uri_service, nullptr);
        ASSERT_NE(stream_service, nullptr);

        // Each source kind resolves to its matching concrete service.
        EXPECT_NE(dynamic_cast<maui::core::file_image_source_service*>(file_service.get()), nullptr);
        EXPECT_NE(dynamic_cast<maui::core::uri_image_source_service*>(uri_service.get()), nullptr);
        EXPECT_NE(dynamic_cast<maui::core::stream_image_source_service*>(stream_service.get()), nullptr);
    }

    // The loader's in-memory uri cache reuses the fetched bytes for a repeat load of the same uri. Both
    // loads of a `file://` uri resolve through the loader and apply the decoded result.
    TEST(image_seam, uri_source_loads_through_loader_and_caches)
    {
        image control;
        auto handler = std::make_shared<image_handler>();
        manual_dispatcher disp;
        control.set_handler(handler);
        handler->source_loader().set_dispatcher(disp);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Write a small temp file the file:// uri can read (a few bytes — decode just mirrors the count).
        const std::string path = std::string(::testing::TempDir()) + "maui_uri_cache_test.bin";
        {
            std::ofstream out(path, std::ios::binary);
            out << "abcd"; // 4 bytes
        }
        const std::string uri = "file://" + path;

        control.set_source(image_source::from_uri(uri));
        disp.run_pending();
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "uri");
        EXPECT_EQ(platform->source_file, uri); // detail = the uri string

        // Delete the file, then load the SAME uri again: the in-memory cache still has the bytes, so it
        // still loads (a non-cached fetch would now fail).
        std::remove(path.c_str());
        control.set_source(image_source::from_uri(uri));
        disp.run_pending();
        EXPECT_TRUE(platform->source_loaded);
        EXPECT_EQ(platform->source_kind, "uri");
    }
} // namespace
