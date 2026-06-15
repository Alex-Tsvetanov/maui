// label_handler — headless platform recipe. Mirrors the mapped properties into label_platform so tests
// can observe them. The Apple twin is src/platform/apple/label_handler.mm.

#include "maui/core/label_handler.hpp"

#include <cmath>
#include <cstddef>
#include <memory>
#include <string>

#include "maui/core/dimension.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/thickness.hpp"
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

    void label_handler::map_vertical_text_alignment(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->vertical_alignment = view.vertical_text_alignment();
        }
    }

    void label_handler::map_character_spacing(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->character_spacing = view.character_spacing();
        }
    }

    void label_handler::map_text_decorations(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->decorations = view.text_decorations();
        }
    }

    void label_handler::map_formatted_text(label_handler& handler, i_label& view)
    {
        // Headless: mirror the resolved attributed runs so tests can assert per-span attributes flowed to
        // the platform mirror (the Apple/iOS twin builds an NSAttributedString instead). Empty runs leave
        // the plain `text` mirror in place — the FormattedText / Text exclusivity is enforced by the label.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->formatted_text_runs = view.formatted_text_runs();
        }
    }

    void label_handler::map_line_height(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->line_height = view.line_height();
        }
    }

    void label_handler::map_padding(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->padding = view.padding();
        }
    }

    // LabelHandler.MapLineBreakMode / MapMaxLines: headless keeps the raw view values (the native backends
    // resolve them into UILabel.lineBreakMode + numberOfLines via SetLineBreakMode). Both mirror both
    // fields so a test observing either key sees the current LineBreakMode + MaxLines pair.
    void label_handler::map_line_break_mode(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->line_break_mode_value = view.line_break_mode();
            platform->max_lines = view.max_lines();
        }
    }

    void label_handler::map_max_lines(label_handler& handler, i_label& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->line_break_mode_value = view.line_break_mode();
            platform->max_lines = view.max_lines();
        }
    }

    maui::graphics::size label_handler::get_desired_size(double width_constraint, double /*height_constraint*/) const
    {
        // Headless metric (~7pt per character, fixed line height). It mirrors the two cross-platform
        // measurement branches the native backends realize: (a) the explicit-Width PreferredMaxLayoutWidth
        // branch — when the virtual view carries an explicit Width, the text wraps to that width over
        // multiple lines (LabelHandler.iOS.GetDesiredSize clamps the width constraint to VirtualView.Width
        // and sets PreferredMaxLayoutWidth); and (b) MauiLabel.SizeThatFits, which subtracts the TextInsets
        // before measuring and adds them back, so Padding inflates the desired size.
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        constexpr double per_char = 7.0;
        constexpr double line = 16.0;
        const maui::core::thickness& pad = platform->padding;
        const double text_width = static_cast<double>(platform->text.size()) * per_char;

        // The wrap width is the explicit virtual Width when set (clamped by the width constraint), else the
        // width constraint itself (infinite for an unconstrained measure). The content area excludes the
        // horizontal insets, matching MauiLabel.SizeThatFits' adjustedWidth.
        const double virtual_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        double wrap_width = dimension::is_explicit_set(virtual_width) ? virtual_width : width_constraint;
        if (dimension::is_explicit_set(virtual_width) && std::isfinite(width_constraint) &&
            width_constraint < wrap_width)
        {
            wrap_width = width_constraint;
        }

        double content_width = text_width;
        double lines = 1.0;
        if (std::isfinite(wrap_width) && wrap_width > 0)
        {
            const double available = wrap_width - pad.left - pad.right;
            if (available > 0 && text_width > available)
            {
                lines = std::ceil(text_width / available);
                content_width = available;
            }
        }
        const double width = content_width + pad.left + pad.right;
        const double height = (lines * line) + pad.top + pad.bottom;
        return {width, height};
    }

    void label_handler::platform_arrange(const maui::graphics::rect& /*frame*/)
    {
        // Headless: no native layout to apply.
    }
} // namespace maui::core
