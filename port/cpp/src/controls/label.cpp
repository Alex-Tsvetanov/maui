// maui::controls::label — out-of-line definitions: the shared bindable-property descriptors and the
// default-handler self-registration. See label.hpp.

#include "maui/controls/label.hpp"

#include "maui/controls/element.hpp" // detach_logical_child on the formatted_text child

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "maui/controls/formatted_string.hpp"
#include "maui/controls/span.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/label_run.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    const maui::core::bindable_property<std::string>& label::text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"text", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& label::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& label::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& label::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& label::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& label::horizontal_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "horizontal_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& label::vertical_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "vertical_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_decorations>& label::text_decorations_property()
    {
        static const maui::core::bindable_property<maui::core::text_decorations> descriptor{
            "text_decorations", maui::core::text_decorations::none};
        return descriptor;
    }

    const maui::core::bindable_property<double>& label::line_height_property()
    {
        static const maui::core::bindable_property<double> descriptor{"line_height", -1.0};
        return descriptor;
    }

    void label::set_formatted_text(std::shared_ptr<formatted_string> value)
    {
        if (formatted_text_ == value)
        {
            return;
        }
        // §8 + Label.FormattedTextProperty.propertyChanging: drop the OLD subscription and unparent the old
        // formatted_string BEFORE releasing it, so its `changed` signal can never reach this label after it
        // stops owning it.
        formatted_text_token_.reset();
        if (formatted_text_)
        {
            element::detach_logical_child(*formatted_text_);
        }

        formatted_text_ = std::move(value);

        if (formatted_text_)
        {
            // Label.FormattedTextProperty.propertyChanged: parent it (flows BindingContext down —
            // SetInheritedBindingContext), subscribe its single `changed` signal (the collapsed
            // SpansCollectionChanged + OnFormattedTextChanged), and clear Text (the two are exclusive).
            attach_logical_child(*formatted_text_);
            formatted_text_token_ =
                maui::core::connect_scoped(formatted_text_->changed, [this] { rebuild_formatted_text_runs(); });
            text_.clear();
        }

        rebuild_formatted_text_runs();
    }

    void label::rebuild_formatted_text_runs()
    {
        // Port of FormattedStringExtensions.ToNSAttributedString: resolve each span into a label_run against
        // this label's defaults (font / TextColor / CharacterSpacing / LineHeight). A null span text is
        // skipped (C# `if (span.Text == null) continue;`); the port's span text is never null (default "").
        formatted_text_runs_.clear();
        if (formatted_text_)
        {
            const maui::core::font default_font = font();
            const double default_font_size = default_font.size();
            for (const auto& span_ptr : formatted_text_->spans())
            {
                if (!span_ptr)
                {
                    continue;
                }
                const span& s = *span_ptr;
                maui::core::label_run run;
                run.text = std::string(s.text());
                run.run_font = s.get_effective_font(default_font_size, default_font);
                // span.TextColor ?? defaultColor (the label's TextColor); nullopt = no color attribute.
                if (s.is_text_color_set())
                {
                    run.text_color = s.text_color();
                }
                else if (text_color_.is_set())
                {
                    run.text_color = text_color();
                }
                if (s.is_background_color_set())
                {
                    run.background_color = s.background_color();
                }
                // CharacterSpacing: span when set else the label default, clamped >= 0 (Math.Max(0, ...)).
                const double spacing = s.is_character_spacing_set() ? s.character_spacing() : character_spacing();
                run.character_spacing = std::max(0.0, spacing);
                // TextDecorations only when the span set them (C# `if (span.IsSet(TextDecorationsProperty))`).
                if (s.is_text_decorations_set())
                {
                    run.decorations = s.text_decorations();
                }
                // LineHeight: span when >= 0 else the label default (still gated >= 0 by the handler).
                run.line_height = s.line_height() >= 0 ? s.line_height() : line_height();
                formatted_text_runs_.push_back(std::move(run));
            }
        }
        // Re-map: on_property_changed routes to handler->update_value("formatted_text") (the mapper key).
        this->on_property_changed("formatted_text");
    }
} // namespace maui::controls

// Self-register the default handler for label (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::label, maui::core::label_handler)
