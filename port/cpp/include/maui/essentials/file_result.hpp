#pragma once
// maui::storage::file_result  <=  Microsoft.Maui.Storage.FileResult / FileBase
//
// The result of a file/media pick: a full path plus the derived file name and content type. Ported
// from the FileBase.shared.cs surface used by MediaPicker/FilePicker - FullPath is the source of
// truth, FileName defaults to the path's leaf, and ContentType defaults to a MIME type guessed from
// the extension (the port maps the common image/video extensions; everything else falls back to
// application/octet-stream, FileBase.DefaultContentType). A caller may override file_name /
// content_type explicitly. OpenReadAsync becomes open_read_async, reading the file bytes (the
// port's stream stand-in). This is the media_picker payload; FilePicker itself is not ported here.

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/move_only_function.hpp"

namespace maui::storage
{
    class file_result
    {
    public:
        file_result() = default;
        explicit file_result(std::string full_path) : full_path_(std::move(full_path))
        {
        }
        file_result(std::string full_path, std::string content_type)
            : full_path_(std::move(full_path)), content_type_(std::move(content_type))
        {
        }

        // FileBase.FullPath.
        [[nodiscard]] const std::string& full_path() const
        {
            return full_path_;
        }
        void set_full_path(std::string value)
        {
            full_path_ = std::move(value);
        }

        // FileBase.FileName: the explicit override, else the path's leaf.
        [[nodiscard]] std::string file_name() const;
        void set_file_name(std::string value)
        {
            file_name_ = std::move(value);
        }

        // FileBase.ContentType: the explicit override, else a MIME type from the extension, else
        // application/octet-stream.
        [[nodiscard]] std::string content_type() const;
        void set_content_type(std::string value)
        {
            content_type_ = std::move(value);
        }

        // FileBase.OpenReadAsync(): the file's bytes (the port's stream stand-in). Reads the file at
        // full_path() synchronously and hands the buffer to the callback.
        using read_callback = maui::core::move_only_function<void(const std::vector<std::byte>&)>;
        void open_read_async(read_callback on_complete) const;

    private:
        std::string full_path_;
        std::string file_name_;    // empty = derive from path
        std::string content_type_; // empty = derive from extension
    };

    namespace detail
    {
        // PlatformGetContentType: the extension -> MIME map for the common image/video types
        // (lowercased extension WITHOUT the dot); empty for anything unrecognised.
        [[nodiscard]] std::string content_type_from_extension(std::string_view extension);
    } // namespace detail
} // namespace maui::storage
