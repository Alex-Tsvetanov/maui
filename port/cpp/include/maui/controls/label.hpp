#pragma once
// maui::controls::label  <=  Microsoft.Maui.Controls.Label
//
// A text-displaying control (the second concrete control). Display-only: no events, command, or inbound
// send_* channel — purely properties pushed to the native view. Ported from Label.cs. Same API shape as
// button: bare-noun interface getters + method accessors, each backed by a private property<T> whose
// change flows through view::on_property_changed to the handler.

#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/property.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class label : public view<maui::core::i_label>
    {
    public:
        // Shared bindable-property descriptors (one instance per type, like Label.*Property).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::core::font>& font_property();
        static const maui::core::bindable_property<double>& character_spacing_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& horizontal_text_alignment_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& vertical_text_alignment_property();
        static const maui::core::bindable_property<maui::core::text_decorations>& text_decorations_property();
        static const maui::core::bindable_property<double>& line_height_property();

        // ---- i_text / i_text_style ----
        [[nodiscard]] std::string_view text() const override
        {
            return text_.get();
        }
        [[nodiscard]] maui::graphics::color text_color() const override
        {
            return text_color_.get();
        }
        [[nodiscard]] maui::core::font font() const override
        {
            return font_.get();
        }
        [[nodiscard]] double character_spacing() const override
        {
            return character_spacing_.get();
        }

        // ---- i_text_alignment ----
        [[nodiscard]] maui::core::text_alignment horizontal_text_alignment() const override
        {
            return horizontal_text_alignment_.get();
        }
        [[nodiscard]] maui::core::text_alignment vertical_text_alignment() const override
        {
            return vertical_text_alignment_.get();
        }

        // ---- i_padding / i_label ----
        [[nodiscard]] maui::core::thickness padding() const override
        {
            return padding_.get();
        }
        [[nodiscard]] maui::core::text_decorations text_decorations() const override
        {
            return text_decorations_.get();
        }
        [[nodiscard]] double line_height() const override
        {
            return line_height_.get();
        }

        // ---- public setters (drive the handler via on_property_changed) ----
        void set_text(std::string value)
        {
            text_.set(std::move(value));
        }
        void set_text_color(maui::graphics::color value)
        {
            text_color_.set(value);
        }
        void set_font(maui::core::font value)
        {
            font_.set(std::move(value));
        }
        void set_character_spacing(double value)
        {
            character_spacing_.set(value);
        }
        void set_padding(maui::core::thickness value)
        {
            padding_.set(value);
        }
        void set_horizontal_text_alignment(maui::core::text_alignment value)
        {
            horizontal_text_alignment_.set(value);
        }
        void set_vertical_text_alignment(maui::core::text_alignment value)
        {
            vertical_text_alignment_.set(value);
        }
        void set_text_decorations(maui::core::text_decorations value)
        {
            text_decorations_.set(value);
        }
        void set_line_height(double value)
        {
            line_height_.set(value);
        }

    private:
        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::core::font> font_{*this, font_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
        maui::core::property<maui::core::text_alignment> horizontal_text_alignment_{
            *this, horizontal_text_alignment_property()};
        maui::core::property<maui::core::text_alignment> vertical_text_alignment_{*this,
                                                                                  vertical_text_alignment_property()};
        maui::core::property<maui::core::text_decorations> text_decorations_{*this, text_decorations_property()};
        maui::core::property<double> line_height_{*this, line_height_property()};
    };
} // namespace maui::controls
