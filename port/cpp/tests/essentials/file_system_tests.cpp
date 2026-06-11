// file_system on the headless backend: the unconfigured fake mirrors FileSystem's netstandard
// partial (every member throws - Essentials.UnitTests FileSystem_Tests), and the configured fake
// runs the DeviceTests FileSystem_Tests behavior suite against a staged app-package ROOT
// directory (valid cache/app-data paths, package files load by relative path - both separator
// styles - and a missing package file throws).

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_system.hpp"

#include "src/platform/headless/essentials_appmodel_fakes.hpp"

namespace
{
    using namespace maui::storage;
    using maui::application_model::feature_not_supported;

    constexpr std::string_view bundle_file_contents = "This file was in the app bundle.";

    class file_system_test : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            file_system::set_current(nullptr);
            // Unique per test process: ctest runs each case in parallel from the same binary, so a
            // shared root would race SetUp's remove_all against a sibling's reads.
            root_ = std::filesystem::temp_directory_path() /
                    (std::string("maui_file_system_tests_") +
                     ::testing::UnitTest::GetInstance()->current_test_info()->name());
            std::filesystem::remove_all(root_);
            std::filesystem::create_directories(root_ / "Folder");
            write_file(root_ / "AppBundleFile.txt");
            write_file(root_ / "AppBundleFile_NoExtension");
            write_file(root_ / "Folder" / "AppBundleFile_Nested.txt");
        }

        void TearDown() override
        {
            file_system::set_current(nullptr);
            std::filesystem::remove_all(root_);
        }

        std::shared_ptr<headless_file_system> install_configured()
        {
            auto fake = std::make_shared<headless_file_system>();
            fake->set_cache_directory((root_ / "cache").string());
            fake->set_app_data_directory((root_ / "data").string());
            fake->set_app_package_root(root_.string());
            file_system::set_current(fake);
            return fake;
        }

        static std::string read_all(std::ifstream stream)
        {
            std::ostringstream contents;
            contents << stream.rdbuf();
            return contents.str();
        }

    private:
        static void write_file(const std::filesystem::path& path)
        {
            std::ofstream file(path, std::ios::binary);
            file << bundle_file_contents;
        }

        std::filesystem::path root_;
    };

    // FileSystem_Fail_On_NetStandard / OpenAppPackageFileAsync_Fail_On_NetStandard.
    TEST_F(file_system_test, netstandard_mirror_throws_until_configured)
    {
        EXPECT_THROW((void)file_system::app_data_directory(), feature_not_supported);
        EXPECT_THROW((void)file_system::cache_directory(), feature_not_supported);
        EXPECT_THROW((void)file_system::open_app_package_file("filename.txt"), feature_not_supported);
        EXPECT_THROW((void)file_system::app_package_file_exists("filename.txt"), feature_not_supported);
    }

    // CacheDirectory_Is_Valid / AppDataDirectory_Is_Valid.
    TEST_F(file_system_test, directories_are_valid)
    {
        install_configured();
        EXPECT_FALSE(file_system::cache_directory().empty());
        EXPECT_FALSE(file_system::app_data_directory().empty());
    }

    // OpenAppPackageFileAsync_Can_Load_File (both separator styles reach the nested file).
    TEST_F(file_system_test, open_app_package_file_can_load_file)
    {
        install_configured();
        for (const std::string_view filename : {"AppBundleFile.txt", "AppBundleFile_NoExtension",
                                                "Folder/AppBundleFile_Nested.txt", "Folder\\AppBundleFile_Nested.txt"})
        {
            std::ifstream stream = file_system::open_app_package_file(filename);
            EXPECT_TRUE(stream.is_open());
            EXPECT_EQ(read_all(std::move(stream)), bundle_file_contents);
        }
    }

    // OpenAppPackageFileAsync_Throws_If_File_Is_Not_Found.
    TEST_F(file_system_test, open_app_package_file_throws_if_not_found)
    {
        install_configured();
        EXPECT_THROW((void)file_system::open_app_package_file("MissingFile.txt"), std::runtime_error);
    }

    // AppPackageFileExistsAsync.
    TEST_F(file_system_test, app_package_file_exists)
    {
        install_configured();
        EXPECT_TRUE(file_system::app_package_file_exists("AppBundleFile.txt"));
        EXPECT_TRUE(file_system::app_package_file_exists("Folder\\AppBundleFile_Nested.txt"));
        EXPECT_FALSE(file_system::app_package_file_exists("MissingFile.txt"));
    }

    // FileSystemUtils.NormalizePath.
    TEST_F(file_system_test, normalize_app_package_path)
    {
        EXPECT_EQ(detail::normalize_app_package_path("Folder\\File.txt"), "Folder/File.txt");
        EXPECT_EQ(detail::normalize_app_package_path("Folder/File.txt"), "Folder/File.txt");
        EXPECT_EQ(detail::normalize_app_package_path(""), "");
    }
} // namespace
