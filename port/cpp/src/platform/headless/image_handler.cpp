// image_handler — headless platform recipe. Mirrors the mapped aspect (and the resolved file source) into
// image_platform so tests can observe them. The Apple twin is src/platform/apple/image_handler.mm.

#include "maui/core/image_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Backend-defined (the Apple twin CFReleases `native`). image_platform holds only trivially-
    // destructible members, so the headless body just clears the unused native slot — an explicit,
    // user-provided body (not `= default`) so the destructor mirrors the Apple RAII shape without
    // tripping performance-trivially-destructible on this trivial-member struct.
    image_platform::~image_platform()
    {
        native = nullptr;
    }

    std::unique_ptr<image_platform> image_handler::create_platform_view()
    {
        return std::make_unique<image_platform>();
    }

    void image_handler::map_aspect(image_handler& handler, i_image& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->image_aspect = view.aspect();
        }
    }

    // Synchronous file load (headless mirror). C#'s MapSource fires an async loader; this cut resolves the
    // source on the calling thread. A null or empty source clears the image (source_loaded=false); a file
    // source with a non-empty path records that path and marks it loaded. The non-file source kinds
    // (uri/stream/font) + async/caching are deferred — only i_file_image_source is handled.
    void image_handler::map_source(image_handler& handler, i_image& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }

        const i_image_source* const src = view.source();
        if (src == nullptr || src->is_empty())
        {
            platform->source_file.clear();
            platform->source_loaded = false;
            return;
        }

        if (const auto* file_src = dynamic_cast<const i_file_image_source*>(src))
        {
            platform->source_file = std::string(file_src->file());
            platform->source_loaded = true;
            return;
        }

        // A non-empty source we don't know how to load (deferred source kind): clear rather than guess.
        platform->source_file.clear();
        platform->source_loaded = false;
    }

    maui::graphics::size image_handler::get_desired_size(double /*width_constraint*/,
                                                         double /*height_constraint*/) const
    {
        // No image bytes are loaded this cut, so there is no intrinsic content size to report.
        return {0, 0};
    }

    void image_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
