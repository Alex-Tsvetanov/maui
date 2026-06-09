// image_handler — headless platform recipe. Mirrors the mapped aspect (and the resolved source) into
// image_platform so tests can observe them. The Apple twin is src/platform/apple/image_handler.mm.

#include "maui/core/image_handler.hpp"

#include <memory>
#include <string>

#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/image_source_result.hpp"
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

    // IsOpaque / IsAnimationPlaying (headless mirrors). The Apple twin pushes these to the NSImageView.
    void image_handler::map_is_opaque(image_handler& handler, i_image& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->opaque = view.is_opaque();
        }
    }

    void image_handler::map_is_animation_playing(image_handler& handler, i_image& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->animation_playing = view.is_animation_playing();
        }
    }

    // ---- per-backend source primitives (the cross-platform map_source in image_handler.cpp routes here) ----

    // File fast-path (headless mirror): no native handle, so record kind="file" + the path, marked loaded.
    void image_handler::load_file_source_sync(image_platform& platform, const i_file_image_source& file_src)
    {
        platform.source_kind = "file";
        platform.source_file = std::string(file_src.file());
        platform.source_loaded = true;
    }

    // The async loader's apply (headless mirror): copy the result's kind + detail into the mirror. A
    // !loaded() result (empty/failed) clears it, mirroring SetImageSource(null).
    void image_handler::apply_loaded_result(image_platform& platform, const image_source_result& result)
    {
        if (!result.loaded())
        {
            clear_source_native(platform);
            return;
        }
        platform.source_kind = result.kind();
        platform.source_file = result.detail();
        platform.source_loaded = true;
    }

    // Clear the loaded image (headless mirror).
    void image_handler::clear_source_native(image_platform& platform)
    {
        platform.source_kind.clear();
        platform.source_file.clear();
        platform.source_loaded = false;
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
