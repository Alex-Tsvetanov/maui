// file_system - iOS (UIKit) platform partial. Ported 1:1 from
// FileSystem.ios.tvos.watchos.macos.cs: cache/app-data resolve through NSSearchPath (the user
// domain's Caches / Library directories), and app-package files resolve DIRECTLY under
// NSBundle.mainBundle.bundlePath (no "Contents/Resources" - that segment is the
// MACCATALYST || MACOS branch), with '\' separators normalized. A missing package file throws
// std::runtime_error (the C# FileNotFoundException). Compiled as Objective-C++ with ARC for the
// ios backend.

#import <Foundation/Foundation.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "maui/essentials/file_system.hpp"

namespace maui::storage
{
    namespace
    {
        std::string search_path_directory(NSSearchPathDirectory directory)
        {
            NSArray<NSString*>* const dirs = NSSearchPathForDirectoriesInDomains(directory, NSUserDomainMask, YES);
            if (dirs.count == 0)
            {
                return {}; // "this should never happen..." (the C# null return)
            }
            const char* const utf8 = [dirs[0] UTF8String];
            return utf8 != nullptr ? std::string(utf8) : std::string();
        }

        std::filesystem::path app_package_file_path(std::string_view filename)
        {
            const char* const bundle_path = [[[NSBundle mainBundle] bundlePath] UTF8String];
            const std::filesystem::path root(bundle_path != nullptr ? bundle_path : "");
            return root / detail::normalize_app_package_path(filename);
        }

        class ios_file_system final : public i_file_system
        {
        public:
            [[nodiscard]] std::string cache_directory() const override
            {
                return search_path_directory(NSCachesDirectory);
            }

            [[nodiscard]] std::string app_data_directory() const override
            {
                return search_path_directory(NSLibraryDirectory);
            }

            [[nodiscard]] std::ifstream open_app_package_file(std::string_view filename) override
            {
                const std::filesystem::path file = app_package_file_path(filename);
                if (!std::filesystem::exists(file))
                {
                    throw std::runtime_error("Could not find app package file '" + std::string(filename) + "'.");
                }
                return std::ifstream(file, std::ios::binary);
            }

            [[nodiscard]] bool app_package_file_exists(std::string_view filename) override
            {
                return std::filesystem::exists(app_package_file_path(filename));
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_file_system> make_file_system()
        {
            return std::make_shared<ios_file_system>();
        }
    } // namespace detail
} // namespace maui::storage
