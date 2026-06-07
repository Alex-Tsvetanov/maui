// label_handler — headless platform recipe. Mirrors the mapped properties into label_platform so tests
// can observe them. The Apple twin is src/platform/apple/label_handler.mm.

#include "maui/core/label_handler.hpp"

#include <cstddef>
#include <memory>
#include <string>

#include "maui/core/i_label.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    label_platform::~label_platform() = default;

    std::unique_ptr<label_platform> label_handler::create_platform_view()
    {
        return std::make_unique<label_platform>();
    }

    void label_handler::map_text(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text = std::string(view.text());
        }
    }

    void label_handler::map_text_color(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_color = view.text_color();
        }
    }

    void label_handler::map_font(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->text_font = view.font();
        }
    }

    void label_handler::map_horizontal_text_alignment(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->horizontal_alignment = view.horizontal_text_alignment();
        }
    }

    maui::graphics::size label_handler::get_desired_size(double /*width_constraint*/,
                                                         double /*height_constraint*/) const
    {
        // Headless placeholder metric (~7pt per character, fixed line height).
        const auto* platform = typed_platform_view();
        const double width = platform != nullptr ? static_cast<double>(platform->text.size()) * 7.0 : 0.0;
        return {width, 16.0};
    }

    void label_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
