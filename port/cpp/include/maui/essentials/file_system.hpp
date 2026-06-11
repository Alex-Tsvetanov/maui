#pragma once
// maui::storage::file_system    <=  Microsoft.Maui.Storage.FileSystem (static facade)
// maui::storage::i_file_system  <=  Microsoft.Maui.Storage.IFileSystem
//
// Locations for the app's device folders plus read access to files shipped inside the app
// package. The C# Task<Stream> OpenAppPackageFileAsync is synchronous on every ported backend
// (Task.FromResult over File.OpenRead), so it becomes a plain std::ifstream return; a missing
// file throws std::runtime_error (the C# FileNotFoundException). Filenames are normalized like
// FileSystemUtils.NormalizePath ('\' -> '/'), so "Folder\\File.txt" finds "Folder/File.txt".
// FileBase / ReadOnlyFile / FileResult (the picker-result file model) are out of this unit's
// scope and not ported.
//
// Backends (suffix oracle): apple/macOS + ios REAL (FileSystem.ios.tvos.watchos.macos.cs -
// NSSearchPath caches/library directories; package files resolve under NSBundle.mainBundle
// (+ "Contents/Resources" on macOS)). Headless mirrors netstandard (throws) until faked - the
// fake exposes settable directories plus a settable app-package ROOT directory, the seam the
// package-file queries resolve against.

#include <fstream>
#include <memory>
#include <string>
#include <string_view>

namespace maui::storage
{
    class i_file_system
    {
    public:
        virtual ~i_file_system() = default;

        // CacheDirectory: temporary data, may be cleared by the OS at any time.
        [[nodiscard]] virtual std::string cache_directory() const = 0;
        // AppDataDirectory: persistent app data (backed up).
        [[nodiscard]] virtual std::string app_data_directory() const = 0;

        // OpenAppPackageFileAsync: a read stream over a file shipped in the app package
        // (throws std::runtime_error when the file does not exist).
        [[nodiscard]] virtual std::ifstream open_app_package_file(std::string_view filename) = 0;
        // AppPackageFileExistsAsync.
        [[nodiscard]] virtual bool app_package_file_exists(std::string_view filename) = 0;

    protected:
        i_file_system() = default;
        i_file_system(const i_file_system&) = default;
        i_file_system(i_file_system&&) = default;
        i_file_system& operator=(const i_file_system&) = default;
        i_file_system& operator=(i_file_system&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (FileSystemImplementation), one per backend under
        // src/platform/<backend>/essentials_file_system.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_file_system> make_file_system();

        // FileSystemUtils.NormalizePath: '\' separators become '/'.
        [[nodiscard]] std::string normalize_app_package_path(std::string_view filename);
    } // namespace detail

    // The static facade. Statics forward to current(), exactly like the C# static class.
    class file_system final
    {
    public:
        file_system() = delete;

        [[nodiscard]] static std::string cache_directory()
        {
            return current().cache_directory();
        }
        [[nodiscard]] static std::string app_data_directory()
        {
            return current().app_data_directory();
        }
        [[nodiscard]] static std::ifstream open_app_package_file(std::string_view filename)
        {
            return current().open_app_package_file(filename);
        }
        [[nodiscard]] static bool app_package_file_exists(std::string_view filename)
        {
            return current().app_package_file_exists(filename);
        }

        // FileSystem.Current (lazy platform default) + SetCurrent (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_file_system& current();
        static void set_current(std::shared_ptr<i_file_system> implementation);
    };
} // namespace maui::storage
