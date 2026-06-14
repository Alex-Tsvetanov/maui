#pragma once
// maui::controls::entry_cell  <=  Microsoft.Maui.Controls.EntryCell
//
// A cell with a fixed Label and a single-line text Entry field. Ported from
// src/Controls/src/Core/Cells/EntryCell.cs. Implements ITextAlignment.
//
// Surface: Text (bindable, TwoWay), Label / Placeholder (bindable strings), LabelColor (bindable
// color), HorizontalTextAlignment / VerticalTextAlignment (bindable, ITextAlignment), Completed event
// + SendCompleted (the native return-key seam). Color collapse (port convention): nullable Color →
// color value. DEVIATION (documented, as the `entry` control): the Keyboard property is not ported (no
// Keyboard type in the port yet).

#include <string>
#include <utility>

#include "maui/controls/cells/cell.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_text_alignment.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class entry_cell : public cell, public maui::core::i_text_alignment
    {
    public:
        entry_cell()
        {
            this->set_style_target_type<entry_cell>();
        }

        // Shared bindable-property descriptors (one instance per type, like EntryCell.*Property). The two
        // alignment descriptors mirror TextAlignmentElement's shared HorizontalTextAlignment /
        // VerticalTextAlignment (default start; the port uses per-type descriptors named the same).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<std::string>& label_property();
        static const maui::core::bindable_property<std::string>& placeholder_property();
        static const maui::core::bindable_property<maui::graphics::color>& label_color_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& horizontal_text_alignment_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& vertical_text_alignment_property();

        // EntryCell.Completed — raised by SendCompleted (the native return-key action).
        maui::core::event<> completed;
        void send_completed()
        {
            completed.raise();
        }

        // ---- Text (TwoWay) / Label / Placeholder / LabelColor ----
        [[nodiscard]] std::string text() const
        {
            return text_.get();
        }
        void set_text(std::string value)
        {
            text_.set(std::move(value));
        }
        // The inbound native edit: writes through at the from-handler specificity (TwoWay default).
        void set_text_from_handler(std::string value)
        {
            text_.set(std::move(value), maui::core::setter_specificity::from_handler);
        }
        [[nodiscard]] std::string label() const
        {
            return label_.get();
        }
        void set_label(std::string value)
        {
            label_.set(std::move(value));
        }
        [[nodiscard]] std::string placeholder() const
        {
            return placeholder_.get();
        }
        void set_placeholder(std::string value)
        {
            placeholder_.set(std::move(value));
        }
        [[nodiscard]] maui::graphics::color label_color() const
        {
            return label_color_.get();
        }
        void set_label_color(maui::graphics::color value)
        {
            label_color_.set(value);
        }

        // ---- i_text_alignment ----
        [[nodiscard]] maui::core::text_alignment horizontal_text_alignment() const override
        {
            return horizontal_text_alignment_.get();
        }
        void set_horizontal_text_alignment(maui::core::text_alignment value)
        {
            horizontal_text_alignment_.set(value);
        }
        [[nodiscard]] maui::core::text_alignment vertical_text_alignment() const override
        {
            return vertical_text_alignment_.get();
        }
        void set_vertical_text_alignment(maui::core::text_alignment value)
        {
            vertical_text_alignment_.set(value);
        }

    private:
        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<std::string> label_{*this, label_property()};
        maui::core::property<std::string> placeholder_{*this, placeholder_property()};
        maui::core::property<maui::graphics::color> label_color_{*this, label_color_property()};
        maui::core::property<maui::core::text_alignment> horizontal_text_alignment_{
            *this, horizontal_text_alignment_property()};
        maui::core::property<maui::core::text_alignment> vertical_text_alignment_{*this,
                                                                                  vertical_text_alignment_property()};
    };
} // namespace maui::controls
