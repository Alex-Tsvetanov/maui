#pragma once
// maui::application_model::data_transfer::share                       <=
// Microsoft.Maui.ApplicationModel.DataTransfer.Share (static facade) maui::application_model::data_transfer::i_share <=
// Microsoft.Maui.ApplicationModel.DataTransfer.IShare maui::application_model::data_transfer::share_text_request <=
// ...DataTransfer.ShareTextRequest maui::application_model::data_transfer::share_file_request          <=
// ...DataTransfer.ShareFileRequest maui::application_model::data_transfer::share_multiple_files_request<=
// ...DataTransfer.ShareMultipleFilesRequest maui::application_model::data_transfer::share_file                  <=
// ...DataTransfer.ShareFile
//
// Shares text / a URI / files to other apps. The C# `Task RequestAsync(...)` becomes the library's
// callback convention (a completion signal; backends complete inline / on presentation). The shared
// half (ShareImplementation) carries the request validation, ported 1:1:
//   * text request: throws std::invalid_argument when BOTH Text and Uri are empty (the C#
//     ArgumentException - "include at least one of them").
//   * single-file request: throws std::invalid_argument when File is unset (no path).
//   * multiple-files request: throws std::invalid_argument when the file list is empty or any entry
//     is unset.
// (C#'s ArgumentNullException for a null request object has no port analog - a request is a value, not
// a nullable reference.)
//
// Share is a UI seam: macOS presents NSSharingServicePicker, iOS presents UIActivityViewController,
// both anchored to the current window/view controller. Neither is drivable in the spawned simulator
// gtest process (no key window) - the same constraint browser/contacts hit. So the platform Request is
// a DOCUMENTED service seam: the request shaping + presentation wiring exist, but the on-simulator
// suite cannot drive the picker; the headless fake is the behavioral test path (it records the request).
//
// PresentationSourceBounds (the iPad popover source rect) is carried as a rect_f. ShareFile keeps the
// full path + optional content type (the port has no FileBase; the only fields the platform Request
// reads).
//
// Backends (suffix oracle): apple/macOS REAL (Share.macos.cs - NSSharingServicePicker), ios REAL
// (Share.ios.cs - UIActivityViewController). Headless mirrors netstandard (throws until faked; the fake
// records the request).

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/core/move_only_function.hpp"
#include "maui/graphics/rect_f.hpp"

namespace maui::application_model::data_transfer
{
    // A bare completion signal for request_async (the Task with no result).
    using share_completion_callback = maui::core::move_only_function<void()>;

    // ShareFile: a file to share (full path + optional MIME content type).
    class share_file
    {
    public:
        explicit share_file(std::string full_path) : full_path_(std::move(full_path))
        {
        }
        share_file(std::string full_path, std::string content_type)
            : full_path_(std::move(full_path)), content_type_(std::move(content_type))
        {
        }

        [[nodiscard]] const std::string& full_path() const
        {
            return full_path_;
        }
        [[nodiscard]] const std::string& content_type() const
        {
            return content_type_;
        }

    private:
        std::string full_path_;
        std::string content_type_;
    };

    // ShareRequestBase: the title + iOS popover source rect every request carries.
    struct share_request_base
    {
        std::string title;
        maui::graphics::rect_f presentation_source_bounds = maui::graphics::rect_f::zero;
    };

    // ShareTextRequest: text / subject / uri to share.
    struct share_text_request : share_request_base
    {
        share_text_request() = default;
        explicit share_text_request(std::string text_value)
        {
            text = std::move(text_value);
        }
        share_text_request(std::string text_value, std::string title_value)
        {
            text = std::move(text_value);
            title = std::move(title_value);
        }

        std::string text;
        std::string subject; // Android-only in practice
        std::string uri;
    };

    // ShareFileRequest: a single file to share.
    struct share_file_request : share_request_base
    {
        share_file_request() = default;
        explicit share_file_request(share_file shared_file) : file(std::move(shared_file))
        {
        }
        share_file_request(std::string title_value, share_file shared_file) : file(std::move(shared_file))
        {
            title = std::move(title_value);
        }

        std::optional<share_file> file;
    };

    // ShareMultipleFilesRequest: several files to share.
    struct share_multiple_files_request : share_request_base
    {
        share_multiple_files_request() = default;
        explicit share_multiple_files_request(std::vector<share_file> shared_files) : files(std::move(shared_files))
        {
        }
        share_multiple_files_request(std::string title_value, std::vector<share_file> shared_files)
            : files(std::move(shared_files))
        {
            title = std::move(title_value);
        }

        std::vector<share_file> files;
    };

    class i_share
    {
    public:
        virtual ~i_share() = default;

        // RequestAsync(ShareTextRequest) - the platform presentation half (PlatformRequestAsync).
        virtual void request_async(const share_text_request& request, share_completion_callback on_complete) = 0;
        // RequestAsync(ShareFileRequest).
        virtual void request_async(const share_file_request& request, share_completion_callback on_complete) = 0;
        // RequestAsync(ShareMultipleFilesRequest).
        virtual void request_async(const share_multiple_files_request& request,
                                   share_completion_callback on_complete) = 0;

    protected:
        i_share() = default;
        i_share(const i_share&) = default;
        i_share(i_share&&) = default;
        i_share& operator=(const i_share&) = default;
        i_share& operator=(i_share&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (ShareImplementation), one per backend under
        // src/platform/<backend>/essentials_share.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_share> make_share();

        // The shared request validators (ShareImplementation.RequestAsync) - throw
        // std::invalid_argument exactly where the C# throws ArgumentException, BEFORE the platform
        // request. Provided so every backend (and the facade) validates identically.
        void validate_share_request(const share_text_request& request);
        void validate_share_request(const share_file_request& request);
        void validate_share_request(const share_multiple_files_request& request);
    } // namespace detail

    // The static facade over share::default_() (C# Share). The string overloads wrap a
    // ShareTextRequest (ShareExtensions).
    class share final
    {
    public:
        share() = delete;

        // RequestAsync(text).
        static void request_async(std::string text, share_completion_callback on_complete)
        {
            request_async(share_text_request{std::move(text)}, std::move(on_complete));
        }
        // RequestAsync(text, title).
        static void request_async(std::string text, std::string title, share_completion_callback on_complete)
        {
            request_async(share_text_request{std::move(text), std::move(title)}, std::move(on_complete));
        }
        static void request_async(const share_text_request& request, share_completion_callback on_complete)
        {
            detail::validate_share_request(request);
            default_().request_async(request, std::move(on_complete));
        }
        static void request_async(const share_file_request& request, share_completion_callback on_complete)
        {
            detail::validate_share_request(request);
            default_().request_async(request, std::move(on_complete));
        }
        static void request_async(const share_multiple_files_request& request, share_completion_callback on_complete)
        {
            detail::validate_share_request(request);
            default_().request_async(request, std::move(on_complete));
        }

        // Share.Default (lazy platform default) + SetDefault (the C# internal test seam made public;
        // nullptr resets to the lazy platform default).
        [[nodiscard]] static i_share& default_();
        static void set_default(std::shared_ptr<i_share> implementation);
    };
} // namespace maui::application_model::data_transfer
