#pragma once
// maui::controls::image_brush  <=  Microsoft.Maui.Controls.ImageBrush (internal)
//
// A brush whose fill is an image source. Ported from src/Controls/src/Core/ImageBrush.cs: a Brush
// carrying an ImageSource (bindable, default null), IsEmpty == (ImageSource?.IsEmpty ?? true), and value
// Equals on the source reference. [ContentProperty] is ImageSource (loader metadata, not modeled).
//
// The brush→paint bridge maps it to maui::core::image_source_paint (Y3), which the per-backend
// apply_background resolves through the image service provider + renders as the view's backing layer. The
// source is held as a shared_ptr<core::i_image_source> (the brush owns it, PROFILE §8); image_source_paint
// references it by raw borrow, so the brush must outlive the converted paint (it does — the bridge clones
// per push and the brush stays on the owning view).
//
// Out-of-line definitions live in image_brush.cpp.

#include <memory>
#include <utility>

#include "maui/controls/brushes/brush.hpp"
#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    class image_brush final : public brush
    {
    public:
        image_brush() = default;
        explicit image_brush(std::shared_ptr<maui::core::i_image_source> image_source)
            : image_source_(std::move(image_source))
        {
        }

        // C# ImageBrush.ImageSource — the source to render as the fill (null when unset). The brush owns it.
        [[nodiscard]] const std::shared_ptr<maui::core::i_image_source>& image_source() const
        {
            return image_source_;
        }
        void set_image_source(std::shared_ptr<maui::core::i_image_source> value)
        {
            image_source_ = std::move(value);
        }

        // C# ImageBrush.IsEmpty — ImageSource is null, or the source reports empty.
        [[nodiscard]] bool is_empty() const override
        {
            return image_source_ == nullptr || image_source_->is_empty();
        }

        // C# ImageBrush.Equals — same ImageSource reference.
        [[nodiscard]] bool equals(const image_brush& other) const
        {
            return image_source_ == other.image_source_;
        }
        friend bool operator==(const image_brush& a, const image_brush& b)
        {
            return a.equals(b);
        }

    private:
        std::shared_ptr<maui::core::i_image_source> image_source_;
    };
} // namespace maui::controls
