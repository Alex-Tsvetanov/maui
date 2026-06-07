// maui::controls::image_source — out-of-line factory. See file_image_source.hpp. Mirrors C#'s
// ImageSource.FromFile, returning the source through the i_image_source contract (the control holds it as
// a shared_ptr; the handler borrows it raw via i_image::source()).

#include "maui/controls/file_image_source.hpp"

#include <memory>
#include <string>
#include <utility>

#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    std::shared_ptr<maui::core::i_image_source> image_source::from_file(std::string path)
    {
        return std::make_shared<file_image_source>(std::move(path));
    }
} // namespace maui::controls
