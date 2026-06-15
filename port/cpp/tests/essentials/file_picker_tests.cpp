// file_picker on the headless backend. MAUI has no FilePicker UnitTests (it is device-only UI); this
// suite covers the cross-platform contract via the service-seam fake: the netstandard mirror
// (PlatformPickAsync throws), the configured fake returning a canned optional<file_result> (PickAsync)
// / vector<file_result> (PickMultipleAsync), the cancellation contracts (PickAsync -> empty optional,
// PickMultipleAsync -> empty vector, never null), the PickOptions recording (picker_title + file_types
// pass-through), and the PickOptions.Default file_types == null (all types selectable). It also covers
// the FilePickerFileType platform registry (Value resolves the current platform's entry, throws when
// the platform has no entry - the netstandard mirror).

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "maui/essentials/device_info.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_picker.hpp"
#include "maui/essentials/file_result.hpp"

#include "src/platform/headless/essentials_fakes.hpp"

namespace
{
    using namespace maui::storage;
    using maui::application_model::feature_not_supported;
    using maui::devices::device_platform;

    class file_picker_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            file_picker::set_default(nullptr);
            maui::devices::device_info::set_current(nullptr);
        }
        void TearDown() override
        {
            // Reset unconditionally so a test that pins device_info::current() (the FilePickerFileType
            // platform-resolution tests) cannot leak a stale platform into later tests if it returns
            // early on a failed assertion.
            file_picker::set_default(nullptr);
            maui::devices::device_info::set_current(nullptr);
        }
    };

    TEST_F(file_picker_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW(file_picker::pick_async([](const std::optional<file_result>&) {}), feature_not_supported);
        EXPECT_THROW(file_picker::pick_multiple_async([](const std::vector<file_result>&) {}), feature_not_supported);
    }

    TEST_F(file_picker_test, configured_fake_returns_single_result)
    {
        auto fake = std::make_shared<headless_file_picker>();
        fake->set_single_result(file_result("/tmp/report.png"));
        file_picker::set_default(fake);

        std::optional<file_result> result;
        file_picker::pick_async([&](const std::optional<file_result>& r) { result = r; });
        ASSERT_TRUE(result.has_value());
        const file_result picked = result.value_or(file_result{});
        EXPECT_EQ(picked.full_path(), "/tmp/report.png");
        EXPECT_EQ(picked.file_name(), "report.png");
        EXPECT_EQ(picked.content_type(), "image/png"); // FileBase MIME map (image/video only in the port)
        // An extension outside the ported map falls back to FileBase.DefaultContentType.
        const file_result document("/tmp/doc.pdf");
        EXPECT_EQ(document.content_type(), "application/octet-stream");
        EXPECT_EQ(fake->last_kind(), headless_file_picker::kind::pick_single);
    }

    TEST_F(file_picker_test, cancelled_pick_returns_empty_optional)
    {
        auto fake = std::make_shared<headless_file_picker>();
        fake->set_single_result(std::nullopt); // user cancelled -> the C# PickAsync null
        file_picker::set_default(fake);

        std::optional<file_result> result = file_result("placeholder");
        file_picker::pick_async([&](const std::optional<file_result>& r) { result = r; });
        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(fake->last_kind(), headless_file_picker::kind::pick_single);
    }

    TEST_F(file_picker_test, pick_multiple_returns_multi_result)
    {
        auto fake = std::make_shared<headless_file_picker>();
        fake->set_multi_result({file_result("/tmp/a.png"), file_result("/tmp/b.jpg")});
        file_picker::set_default(fake);

        std::vector<file_result> results;
        file_picker::pick_multiple_async([&](const std::vector<file_result>& r) { results = r; });
        ASSERT_EQ(results.size(), 2U);
        EXPECT_EQ(results[0].content_type(), "image/png");
        EXPECT_EQ(results[1].content_type(), "image/jpeg");
        EXPECT_EQ(fake->last_kind(), headless_file_picker::kind::pick_multiple);
    }

    TEST_F(file_picker_test, cancelled_pick_multiple_returns_empty_vector_never_null)
    {
        auto fake = std::make_shared<headless_file_picker>();
        fake->set_multi_result({}); // user cancelled -> the C# empty collection (never null)
        file_picker::set_default(fake);

        std::vector<file_result> results{file_result("placeholder")};
        bool callback_invoked = false;
        file_picker::pick_multiple_async([&](const std::vector<file_result>& r) {
            callback_invoked = true;
            results = r;
        });
        EXPECT_TRUE(callback_invoked);
        EXPECT_TRUE(results.empty());
        EXPECT_EQ(fake->last_kind(), headless_file_picker::kind::pick_multiple);
    }

    TEST_F(file_picker_test, options_picker_title_and_file_types_pass_through)
    {
        auto fake = std::make_shared<headless_file_picker>();
        fake->set_single_result(file_result("/tmp/doc.pdf"));
        file_picker::set_default(fake);

        pick_options options;
        options.picker_title = "Choose a document";
        const file_picker_file_type custom({{.platform = device_platform::ios(), .types = {"com.adobe.pdf"}},
                                            {.platform = device_platform::mac_catalyst(), .types = {"com.adobe.pdf"}}});
        options.file_types = custom;

        file_picker::pick_async(options, [](const std::optional<file_result>&) {});

        const std::optional<pick_options> recorded = fake->last_options();
        ASSERT_TRUE(recorded.has_value());
        const pick_options opts = recorded.value_or(pick_options{});
        EXPECT_EQ(opts.picker_title, "Choose a document");
        ASSERT_TRUE(opts.file_types.has_value());
        const std::optional<std::vector<std::string>> ios_types =
            opts.file_types.value_or(file_picker_file_type{}).try_get(device_platform::ios());
        ASSERT_TRUE(ios_types.has_value());
        const std::vector<std::string> ios_types_v = ios_types.value_or(std::vector<std::string>{});
        ASSERT_EQ(ios_types_v.size(), 1U);
        EXPECT_EQ(ios_types_v.front(), "com.adobe.pdf");
    }

    // PickOptions.Default: FileTypes is null (all file types selectable) and no picker title.
    TEST_F(file_picker_test, default_options_have_null_file_types)
    {
        auto fake = std::make_shared<headless_file_picker>();
        fake->set_single_result(std::nullopt);
        file_picker::set_default(fake);

        file_picker::pick_async([](const std::optional<file_result>&) {}); // the default-options overload
        const std::optional<pick_options> recorded = fake->last_options();
        ASSERT_TRUE(recorded.has_value());
        const pick_options opts = recorded.value_or(pick_options{});
        EXPECT_FALSE(opts.file_types.has_value()); // null => all types
        EXPECT_TRUE(opts.picker_title.empty());
    }

    // FilePickerFileType.Value resolves the entry for the current platform; a registry with no entry
    // for the current platform throws (the C# PlatformNotSupportedException fold).
    TEST_F(file_picker_test, file_type_value_resolves_current_platform_or_throws)
    {
        auto info = std::make_shared<maui::devices::headless_device_info>();
        info->set_platform(device_platform::ios());
        maui::devices::device_info::set_current(info);

        const file_picker_file_type with_ios({{.platform = device_platform::ios(), .types = {"public.image"}}});
        const std::vector<std::string> resolved = with_ios.value();
        ASSERT_EQ(resolved.size(), 1U);
        EXPECT_EQ(resolved.front(), "public.image");

        const file_picker_file_type android_only({{.platform = device_platform::android(), .types = {"image/*"}}});
        EXPECT_THROW((void)android_only.value(), feature_not_supported);
    }

    // The predefined statics are the netstandard mirror on headless: empty registries -> Value throws.
    TEST_F(file_picker_test, predefined_file_types_are_netstandard_mirror)
    {
        auto info = std::make_shared<maui::devices::headless_device_info>();
        info->set_platform(device_platform::ios());
        maui::devices::device_info::set_current(info);

        EXPECT_THROW((void)file_picker_file_type::images.value(), feature_not_supported);
        EXPECT_THROW((void)file_picker_file_type::pdf.value(), feature_not_supported);
    }
} // namespace
