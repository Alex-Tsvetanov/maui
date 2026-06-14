#pragma once
// maui::controls::image_cell  <=  Microsoft.Maui.Controls.ImageCell
//
// A text_cell that also carries an image. Ported from src/Controls/src/Core/Cells/ImageCell.cs: it adds
// the ImageSource bindable property over the full text_cell surface (Text/Detail/colors + the command).
//
// DEVIATION (documented): C#'s ImageCell flows its BindingContext into the (BindableObject) ImageSource
// and cancels the source's pending load on Disappearing. The port's image sources are PLAIN interfaces
// (i_image_source, not bindable_object — see i_image_source.hpp's first-cut note), so neither the
// context propagation nor the Cancel()-on-disappear has an analog here; the bindable ImageSource itself
// is faithfully modeled (a shared, owned source).

#include <memory>
#include <utility>

#include "maui/controls/cells/text_cell.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    class image_cell : public text_cell
    {
    public:
        image_cell()
        {
            this->set_style_target_type<image_cell, text_cell>();
        }

        // Shared bindable-property descriptor (ImageCell.ImageSourceProperty).
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>&
        image_source_property();

        [[nodiscard]] std::shared_ptr<maui::core::i_image_source> image_source() const
        {
            return image_source_.get();
        }
        void set_image_source(std::shared_ptr<maui::core::i_image_source> value)
        {
            image_source_.set(std::move(value));
        }

    private:
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> image_source_{*this, image_source_property()};
    };
} // namespace maui::controls
