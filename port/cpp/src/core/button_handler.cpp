// button_handler — cross-platform part: the shared mapper tables + ctor (ButtonHandler.cs). The
// platform recipe (create/connect/disconnect/map_text/measure) lives in the per-backend partial.

#include "maui/core/button_handler.hpp"

#include <utility> // std::move (the loading callback handed to the loader)

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_mapper.hpp"

namespace maui::core
{
    // Text + text appearance, keyed by i_text_button (C# TextButtonMapper<ITextButton>).
    property_mapper<i_text_button, button_handler>& button_handler::text_mapper()
    {
        static property_mapper<i_text_button, button_handler> table{
            {"text", &button_handler::map_text},
            {"text_color", &button_handler::map_text_color},
            {"font", &button_handler::map_font},
            {"character_spacing", &button_handler::map_character_spacing},
        };
        return table;
    }

    // The cross-platform ImageButtonMapper (C# ButtonHandler.ImageButtonMapper, [Source] → MapImageSource)
    // plus the controls-side ContentLayout remap (Button.Mapper.cs MapContentLayout). Keyed on
    // i_text_button — the narrow seam (Button is not an i_image): map_image_source reads view.image_source().
    property_mapper<i_text_button, button_handler>& button_handler::image_mapper()
    {
        static property_mapper<i_text_button, button_handler> table{
            {"source", &button_handler::map_image_source},
            {"content_layout", &button_handler::map_content_layout},
        };
        return table;
    }

    // The button's own mapper (padding + the i_button_stroke border), chained onto the shared view_mapper
    // (the generic IView properties), the text mapper, and the image mapper — mirroring C#
    // ButtonHandler.Mapper(TextButtonMapper, ImageButtonMapper, ViewHandler.ViewMapper). The chain is
    // ordered so the generic IView keys run first (keys() walks the chain in reverse), then the image keys,
    // then the text keys, then the button's own keys; no keys collide across the four mappers.
    property_mapper<i_button, button_handler>& button_handler::mapper()
    {
        static property_mapper<i_button, button_handler> table = [] {
            property_mapper<i_button, button_handler> mapped{
                {"padding", &button_handler::map_padding},
                {"stroke_color", &button_handler::map_stroke_color},
                {"stroke_thickness", &button_handler::map_stroke_thickness},
                {"corner_radius", &button_handler::map_corner_radius},
            };
            // Reverse-order iteration in keys() means the LAST chained mapper's keys come first, so listing
            // image_mapper then text_mapper then view_mapper yields: view (generic IView) keys, then text
            // keys, then image keys (C# lists TextButtonMapper, ImageButtonMapper, ViewMapper — same set).
            mapped.set_chained({&image_mapper(), &text_mapper(), &view_mapper()});
            return mapped;
        }();
        return table;
    }

    // Cross-platform source routing — image_handler::map_source's twin over the button platform primitives
    // (ButtonHandler.MapImageSource → MapImageSourceAsync → ImageSourceLoader.UpdateImageSourceAsync): the
    // file fast-path is synchronous; every other source (uri/stream/font) goes through the handler-owned
    // loader (async, with the source-identity recheck); a null/empty source cancels + clears.
    void button_handler::map_image_source(button_handler& handler, i_text_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Record the current ContentLayout (position + spacing) on the mirror BEFORE the load, so the
        // per-backend icon primitive (android apply_button_icon) places the compound drawable correctly —
        // "source" runs before "content_layout" in the mapper table, so the icon primitive can't rely on
        // map_content_layout having run yet.
        platform->content_layout = view.content_layout_spec();

        // Push the in-flight loading state back to the control so UpdateIsLoading re-pushes ContentLayout
        // on completion (C# Button.cs:499-505 — IImageSourcePart.UpdateIsLoading). Mirror image_handler.
        i_text_button* const view_ptr = &view;
        auto on_loading = [view_ptr](bool is_loading) { view_ptr->update_is_loading(is_loading); };

        i_image_source* const src = view.image_source();
        if (src == nullptr || src->is_empty())
        {
            handler.image_source_loader_.update_source(nullptr, nullptr, std::move(on_loading));
            clear_source_native(*platform);
            return;
        }

        if (const auto* file_src = dynamic_cast<const i_file_image_source*>(src))
        {
            handler.image_source_loader_.update_source(nullptr, nullptr);
            load_file_source_sync(*platform, *file_src);
            view.update_is_loading(false);
            return;
        }

        handler.image_source_loader_.update_source(
            src, [platform](const image_source_result& result) { apply_loaded_result(*platform, result); },
            std::move(on_loading));
    }

    // ContentLayout (Button.Mapper.cs MapContentLayout → UpdateContentLayout): the port stores + pushes it
    // but defers the text+image composition (no container infra), so the cross-platform mapper just records
    // a push for tests to observe. A real UpdateContentLayout (image positioning + spacing) lands when the
    // container subsystem does (see button_handler.hpp / the iOS partial header).
    void button_handler::map_content_layout(button_handler& handler, i_text_button& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            ++platform->content_layout_push_count;
            platform->content_layout = view.content_layout_spec(); // mirror for the android icon primitive
        }
    }

    // No button-specific commands beyond the (currently empty) view command set. The type must be
    // qualified inside the body: the method name `command_mapper` shadows the `command_mapper` template.
    maui::core::command_mapper<i_button, button_handler>& button_handler::command_mapper()
    {
        static maui::core::command_mapper<i_button, button_handler> table{};
        return table;
    }

    button_handler::button_handler() : view_handler(&mapper(), &command_mapper())
    {
        // Per-backend loader wiring (the image_handler convention; headless leaves the defaults).
        configure_loader(image_source_loader_);
    }
} // namespace maui::core
