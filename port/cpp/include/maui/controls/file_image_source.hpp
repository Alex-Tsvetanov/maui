#pragma once
// maui::controls::file_image_source  <=  Microsoft.Maui.Controls.FileImageSource
//
// A concrete image source backed by a file path. Ported from src/Controls/src/Core/FileImageSource.cs +
// the IFileImageSource contract: it owns a std::string path; is_empty() mirrors C#'s
// string.IsNullOrEmpty(File) and file() returns the stored path.
//
// FIRST CUT: this is the only concrete source modeled (uri / stream / font sources are deferred). The path
// is loaded synchronously by image_handler::map_source (the async loader / caching are deferred).
//
// image_source is a small factory namespace-struct: image_source::from_file(path) mints a shared source.
// Ownership: the image control owns the returned shared_ptr; i_image::source() hands back a raw borrow.

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    class file_image_source : public maui::core::i_file_image_source
    {
    public:
        explicit file_image_source(std::string file) : file_(std::move(file))
        {
        }

        // C# FileImageSource.IsEmpty => string.IsNullOrEmpty(File).
        [[nodiscard]] bool is_empty() const override
        {
            return file_.empty();
        }
        [[nodiscard]] std::string_view file() const override
        {
            return file_;
        }

    private:
        std::string file_;
    };

    // Factory entry point (mirrors C#'s ImageSource.FromFile). A free-standing struct rather than a free
    // function so future source kinds (FromUri / FromStream) can join the same `image_source::` surface.
    struct image_source
    {
        [[nodiscard]] static std::shared_ptr<maui::core::i_image_source> from_file(std::string path);
    };
} // namespace maui::controls
