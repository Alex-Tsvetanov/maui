#pragma once
// maui::graphics::text::i_attributed_text  <=  Microsoft.Maui.Graphics.Text.IAttributedText
//   (+ AbstractAttributedText.cs's Optimal flag)
//
// A text string plus its list of attributed runs — the contract i_canvas::draw_text consumes.
// An abstract class (PROFILE §11 — runtime polymorphism: the canvas draws any implementation).
// C#'s AbstractAttributedText layer only adds the Optimal flag (checked by the Optimize extension
// through a downcast); the port folds optimal() into this interface with the same false default —
// observationally identical, one indirection less.

#include <string>
#include <vector>

#include "maui/graphics/text/attributed_text_run.hpp"

namespace maui::graphics::text
{
    class i_attributed_text
    {
    public:
        virtual ~i_attributed_text() = default;

        // C# IAttributedText.Text.
        [[nodiscard]] virtual const std::string& text() const = 0;

        // C# IAttributedText.Runs.
        [[nodiscard]] virtual const std::vector<attributed_text_run>& runs() const = 0;

        // C# AbstractAttributedText.Optimal — true when the runs are already optimized (sorted,
        // disjoint); Optimize() then returns the text unchanged.
        [[nodiscard]] virtual bool optimal() const
        {
            return false;
        }

    protected:
        i_attributed_text() = default;
        i_attributed_text(const i_attributed_text&) = default;
        i_attributed_text(i_attributed_text&&) = default;
        i_attributed_text& operator=(const i_attributed_text&) = default;
        i_attributed_text& operator=(i_attributed_text&&) = default;
    };
} // namespace maui::graphics::text
