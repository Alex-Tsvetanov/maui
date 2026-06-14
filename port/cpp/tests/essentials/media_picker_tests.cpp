// media_picker on the headless backend. MAUI has no MediaPicker UnitTests (it is device-only UI);
// this suite covers the cross-platform contract via the service-seam fake: the netstandard mirror
// (every member throws), the configured fake returning a canned file_result per pick kind, the
// cancellation (empty result) contract, and the CapturePhoto/CaptureVideo IsCaptureSupported gate.
// It also covers the file_result FileBase-derived accessors (FileName from the path leaf,
// ContentType from the extension).

#include <memory>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_result.hpp"
#include "maui/essentials/media_picker.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::media;
    using maui::application_model::feature_not_supported;
    using maui::storage::file_result;

    class media_picker_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            media_picker::set_default(nullptr);
        }
        void TearDown() override
        {
            media_picker::set_default(nullptr);
        }
    };

    TEST_F(media_picker_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW((void)media_picker::is_capture_supported(), feature_not_supported);
        EXPECT_THROW(media_picker::pick_photo_async([](const std::optional<file_result>&) {}), feature_not_supported);
        EXPECT_THROW(media_picker::pick_photos_async([](const std::vector<file_result>&) {}), feature_not_supported);
    }

    TEST_F(media_picker_test, configured_fake_returns_canned_single_result)
    {
        auto fake = std::make_shared<headless_media_picker>();
        fake->set_is_capture_supported(true);
        fake->set_single_result(file_result("/tmp/photo.png"));
        media_picker::set_default(fake);

        std::optional<file_result> result;
        media_picker::pick_photo_async([&](const std::optional<file_result>& r) { result = r; });
        ASSERT_TRUE(result.has_value());
        const file_result picked = result.value_or(file_result{});
        EXPECT_EQ(picked.full_path(), "/tmp/photo.png");
        EXPECT_EQ(picked.file_name(), "photo.png");
        EXPECT_EQ(picked.content_type(), "image/png");
        EXPECT_EQ(fake->last_kind(), headless_media_picker::kind::pick_photo);
    }

    TEST_F(media_picker_test, cancelled_pick_returns_empty)
    {
        auto fake = std::make_shared<headless_media_picker>();
        fake->set_single_result(std::nullopt); // user cancelled
        media_picker::set_default(fake);

        std::optional<file_result> result = file_result("placeholder");
        media_picker::pick_video_async([&](const std::optional<file_result>& r) { result = r; });
        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(fake->last_kind(), headless_media_picker::kind::pick_video);
    }

    TEST_F(media_picker_test, pick_photos_returns_multi_result)
    {
        auto fake = std::make_shared<headless_media_picker>();
        fake->set_multi_result({file_result("/tmp/a.jpg"), file_result("/tmp/b.mov")});
        media_picker::set_default(fake);

        std::vector<file_result> results;
        media_picker_options options;
        options.selection_limit = 0; // no limit
        media_picker::pick_photos_async(options, [&](const std::vector<file_result>& r) { results = r; });
        ASSERT_EQ(results.size(), 2U);
        EXPECT_EQ(results[0].content_type(), "image/jpeg");
        EXPECT_EQ(results[1].content_type(), "video/quicktime");
        const std::optional<media_picker_options> recorded = fake->last_options();
        ASSERT_TRUE(recorded.has_value());
        EXPECT_EQ(recorded.value_or(media_picker_options{}).selection_limit, 0);
    }

    TEST_F(media_picker_test, capture_gated_on_is_capture_supported)
    {
        auto fake = std::make_shared<headless_media_picker>();
        fake->set_is_capture_supported(false);
        fake->set_single_result(file_result("/tmp/x.png"));
        media_picker::set_default(fake);

        EXPECT_THROW(media_picker::capture_photo_async([](const std::optional<file_result>&) {}),
                     feature_not_supported);
        EXPECT_THROW(media_picker::capture_video_async([](const std::optional<file_result>&) {}),
                     feature_not_supported);

        fake->set_is_capture_supported(true);
        std::optional<file_result> result;
        media_picker::capture_photo_async([&](const std::optional<file_result>& r) { result = r; });
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(fake->last_kind(), headless_media_picker::kind::capture_photo);
    }

    TEST_F(media_picker_test, options_compression_quality_clamps)
    {
        media_picker_options options;
        options.set_compression_quality(150);
        EXPECT_EQ(options.compression_quality(), 100);
        options.set_compression_quality(-5);
        EXPECT_EQ(options.compression_quality(), 0);
        options.set_compression_quality(60);
        EXPECT_EQ(options.compression_quality(), 60);
    }

    // file_result content-type fallback for an unknown extension.
    TEST_F(media_picker_test, file_result_unknown_extension_falls_back_to_octet_stream)
    {
        const file_result result("/tmp/data.bin");
        EXPECT_EQ(result.content_type(), "application/octet-stream");
        EXPECT_EQ(result.file_name(), "data.bin");
    }

    TEST_F(media_picker_test, file_result_explicit_overrides_win)
    {
        file_result result("/tmp/photo.png");
        result.set_file_name("renamed.png");
        result.set_content_type("image/custom");
        EXPECT_EQ(result.file_name(), "renamed.png");
        EXPECT_EQ(result.content_type(), "image/custom");
    }
} // namespace
