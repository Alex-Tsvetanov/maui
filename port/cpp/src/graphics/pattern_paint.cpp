// maui::graphics::pattern_paint  <=  Microsoft.Maui.Graphics.PatternPaint
// Out-of-line definitions (see pattern_paint.hpp). Behavior derived from
// src/Graphics/src/Graphics/PatternPaint.cs. The destructor is out-of-line so the unique_ptr<paint_pattern>
// sees the complete type (paint_pattern is only forward-declared in the header).

#include "maui/graphics/pattern_paint.hpp"

#include <memory>

#include "maui/graphics/i_pattern.hpp"
#include "maui/graphics/paint_pattern.hpp"

namespace maui::graphics
{
    // Defined out-of-line (not = default in the header) so the unique_ptr<paint_pattern> sees the complete
    // type for its destructor in every TU — the header only forward-declares paint_pattern.
    pattern_paint::pattern_paint() = default;
    pattern_paint::~pattern_paint() = default;

    void pattern_paint::set_pattern(i_pattern* value)
    {
        // C#: _pattern = value; if (!(_pattern is PaintPattern)) _pattern = new PaintPattern(_pattern) { Paint = this
        // };
        owned_wrapper_.reset();

        if (value == nullptr)
        {
            pattern_ = nullptr;
            return;
        }

        if (auto* const already = dynamic_cast<paint_pattern*>(value))
        {
            // Already a PaintPattern — referenced as-is (the caller owns it); C# leaves it untouched
            // (it does NOT reset its Paint back-reference).
            pattern_ = already;
            return;
        }

        // Wrap the value in an owned paint_pattern whose Paint back-reference is this paint.
        owned_wrapper_ = std::make_unique<paint_pattern>(value);
        owned_wrapper_->set_paint(this);
        pattern_ = owned_wrapper_.get();
    }
} // namespace maui::graphics
