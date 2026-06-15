#pragma once
// maui::storage::file_picker            <=  Microsoft.Maui.Storage.FilePicker (static facade)
// maui::storage::i_file_picker          <=  Microsoft.Maui.Storage.IFilePicker
// maui::storage::file_picker_file_type  <=  Microsoft.Maui.Storage.FilePickerFileType
// maui::storage::pick_options           <=  Microsoft.Maui.Storage.PickOptions
//
// Lets the user pick one or more files from device storage. The C# Task<FileResult?> /
// Task<IEnumerable<FileResult>?> surface becomes the library's callback convention: pick_async hands
// an optional<file_result> (empty = the user cancelled, the C# null), pick_multiple_async a
// vector<file_result> (empty = cancelled - the C# contract returns an empty collection, never null).
// PickOptions {FileTypes, PickerTitle} are both optional; the default options (FileTypes=null) let
// every file type be selected.
//
// SERVICE SEAM / DEVIATION: the real document-picker UI is NOT drivable in the spawned simulator gtest
// process (no presenting view controller / no window / no run loop), so file_picker is a SERVICE SEAM,
// exactly like media_picker. The apple/ios partials are present (the ios partial builds a
// UIDocumentPickerViewController in Open mode and presents over the current view controller; the macos
// partial would present an NSOpenPanel) but the picking itself runs only inside a real app - the
// partials raise a documented feature_not_supported when there is no presenting context. The headless
// fake is the test path: it returns a canned optional/vector configured per pick kind.
//
// Backends (suffix oracle): ios REAL (FilePicker.ios.cs - UIDocumentPickerViewController; the same
// partial services MacCatalyst). macOS has NO FilePicker partial in MAUI (only the Android/iOS/Windows/
// Tizen partials exist), so on the apple/macOS backend the picker is a service seam with no real path.
// Headless mirrors netstandard (FilePicker.netstandard.watchos.tvos.cs throws) until the fake is
// configured.
//
// NOTE on FileSystemUtils.EnsurePhysicalFileResultsAsync: the C# ios partial copies external-provider
// files locally before handing back FileResults. The port has no FileSystemUtils analogue yet; because
// the ios partial here is a service seam that never reaches the conversion (it throws before
// presenting succeeds), that copy step is deferred and noted rather than ported.

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/core/move_only_function.hpp"
#include "maui/essentials/device_info.hpp"
#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_result.hpp"

namespace maui::storage
{
    using maui::devices::device_platform;

    // FilePickerFileType: a platform-keyed registry of allowed file-type identifiers (Android MIME
    // types, iOS UTType constants, Windows extensions). value() resolves the entry for the current
    // platform (DeviceInfo.Current.Platform) and throws PlatformNotSupportedException - folded to
    // feature_not_supported here - when the current platform has no entry. The predefined statics
    // (images/png/jpeg/videos/pdf) are populated per platform by the platform partial; headless leaves
    // them empty (the netstandard mirror throws on value()).
    class file_picker_file_type
    {
    public:
        // PlatformImageFileType etc. - the predefined types, populated per platform at library load
        // (the platform partial's Platform*FileType()).
        static const file_picker_file_type images;
        static const file_picker_file_type png;
        static const file_picker_file_type jpeg;
        static const file_picker_file_type videos;
        static const file_picker_file_type pdf;

        // FilePickerFileType() - the protected ctor: an empty registry.
        file_picker_file_type() = default;

        // One platform -> allowed-types entry. device_platform is a string-backed value type with
        // operator== but no std::hash, so the registry is an association list looked up linearly (it
        // holds at most a handful of platform entries - the Dictionary's small-N twin).
        struct platform_entry
        {
            device_platform platform;
            std::vector<std::string> types;
        };

        // FilePickerFileType(IDictionary<DevicePlatform, IEnumerable<string>>).
        explicit file_picker_file_type(std::vector<platform_entry> file_types) : file_types_(std::move(file_types))
        {
        }

        // FilePickerFileType.Value: the allowed types for the current platform; throws when the
        // current platform has no configured entry (the C# PlatformNotSupportedException fold).
        [[nodiscard]] std::vector<std::string> value() const
        {
            return get_platform_file_type(maui::devices::device_info::platform());
        }

        // GetPlatformFileType(platform) without the throw: the configured types for a specific
        // platform, or std::nullopt when the platform has no entry (callers - the ios partial - decide
        // the fallback; value() folds the missing case into the feature_not_supported throw).
        [[nodiscard]] std::optional<std::vector<std::string>> try_get(const device_platform& platform) const
        {
            const auto found = std::find_if(file_types_.begin(), file_types_.end(),
                                            [&](const platform_entry& entry) { return entry.platform == platform; });
            if (found == file_types_.end())
            {
                return std::nullopt;
            }
            return found->types;
        }

    private:
        [[nodiscard]] std::vector<std::string> get_platform_file_type(const device_platform& platform) const
        {
            if (auto found = try_get(platform); found.has_value())
            {
                return *std::move(found);
            }
            throw maui::application_model::feature_not_supported("This platform does not support this file type.");
        }

        std::vector<platform_entry> file_types_;
    };

    // PickOptions: the (optional) picker knobs. file_types null => all file types selectable;
    // picker_title is the Android-only picker title (ignored elsewhere). The default has file_types
    // null, matching PickOptions.Default.
    class pick_options
    {
    public:
        // PickOptions.PickerTitle (Android-only; not guaranteed to be shown).
        std::string picker_title;

        // PickOptions.FileTypes (null/empty => all types). std::nullopt is the C# null.
        std::optional<file_picker_file_type> file_types;
    };

    using file_result_callback = maui::core::move_only_function<void(const std::optional<file_result>&)>;
    using file_results_callback = maui::core::move_only_function<void(const std::vector<file_result>&)>;

    class i_file_picker
    {
    public:
        virtual ~i_file_picker() = default;

        // IFilePicker.PickAsync: a single file, or empty when the user cancelled (the C# null).
        virtual void pick_async(const pick_options& options, file_result_callback on_complete) = 0;
        // IFilePicker.PickMultipleAsync: zero or more files; empty when cancelled (never null in C#).
        virtual void pick_multiple_async(const pick_options& options, file_results_callback on_complete) = 0;

    protected:
        i_file_picker() = default;
        i_file_picker(const i_file_picker&) = default;
        i_file_picker(i_file_picker&&) = default;
        i_file_picker& operator=(const i_file_picker&) = default;
        i_file_picker& operator=(i_file_picker&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (FilePickerImplementation), one per backend under
        // src/platform/<backend>/essentials_file_picker.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_file_picker> make_file_picker();
    } // namespace detail

    // The static facade over file_picker::default_() (C# FilePicker.Default). The pick overloads
    // default the options to PickOptions{} (FileTypes=null - the C# PickOptions.Default).
    class file_picker final
    {
    public:
        file_picker() = delete;

        static void pick_async(file_result_callback on_complete)
        {
            default_().pick_async(pick_options{}, std::move(on_complete));
        }
        static void pick_async(const pick_options& options, file_result_callback on_complete)
        {
            default_().pick_async(options, std::move(on_complete));
        }
        static void pick_multiple_async(file_results_callback on_complete)
        {
            default_().pick_multiple_async(pick_options{}, std::move(on_complete));
        }
        static void pick_multiple_async(const pick_options& options, file_results_callback on_complete)
        {
            default_().pick_multiple_async(options, std::move(on_complete));
        }

        // FilePicker.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_file_picker& default_();
        static void set_default(std::shared_ptr<i_file_picker> implementation);
    };
} // namespace maui::storage
