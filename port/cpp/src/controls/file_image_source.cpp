// maui::controls::image_source — out-of-line factories. See file_image_source.hpp. Mirror C#'s
// ImageSource.FromFile / FromUri / FromStream, returning each source through the i_image_source contract
// (the control holds it as a shared_ptr; the handler borrows it raw via i_image::source()).

#include "maui/controls/file_image_source.hpp"

#include <memory>
#include <string>
#include <utility>

#include "maui/controls/stream_image_source.hpp"
#include "maui/controls/uri_image_source.hpp"
#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    std::shared_ptr<maui::core::i_image_source> image_source::from_file(std::string path)
    {
        return std::make_shared<file_image_source>(std::move(path));
    }

    std::shared_ptr<maui::core::i_image_source> image_source::from_uri(std::string uri, bool caching_enabled)
    {
        return std::make_shared<uri_image_source>(std::move(uri), caching_enabled);
    }

    std::shared_ptr<maui::core::i_image_source> image_source::from_stream(stream_provider provider)
    {
        return std::make_shared<stream_image_source>(std::move(provider));
    }
} // namespace maui::controls
