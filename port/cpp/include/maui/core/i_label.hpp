#pragma once
// maui::core::i_label  <=  Microsoft.Maui.ILabel
//
// The virtual-view contract for a text label. Ported from src/Core/src/Core/ILabel.cs
// (ILabel : IView, IText, ITextAlignment, IPadding). text_decorations()'s return type is qualified
// because the method name would otherwise hide the type (as with i_text_style::font()).
//
// formatted_text_runs() is the port's extension carrying the label's rich per-span text down to the
// handler (Label.FormattedText, MISSING from the original audit — gap-closure G1). It is NOT on C#'s
// ILabel (the C# handler reads Label.FormattedText directly + the platform ToNSAttributedString); the
// reflection-free port hands the handler the already-resolved runs through this contract method instead.

#include <vector>

#include "maui/core/i_padding.hpp"
#include "maui/core/i_text.hpp"
#include "maui/core/i_text_alignment.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/label_run.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_decorations.hpp"

namespace maui::core
{
    class i_label : public i_view, public i_text, public i_text_alignment, public i_padding
    {
    public:
        [[nodiscard]] virtual maui::core::text_decorations text_decorations() const = 0;
        [[nodiscard]] virtual double line_height() const = 0;

        // Label.LineBreakMode (wrap / truncation; default WordWrap) and Label.MaxLines (max visible lines;
        // default -1 = "unset"). The two interact at the platform: truncation modes force a single line, and
        // an unset MaxLines under TailTruncation also means one line — see SetLineBreakMode (the iOS/AppKit
        // handlers fold both into one platform refresh, just as MapLineBreakMode/MapMaxLines both call it).
        [[nodiscard]] virtual maui::core::line_break_mode line_break_mode() const = 0;
        [[nodiscard]] virtual int max_lines() const = 0;

        // The label's FormattedText, RESOLVED into a flat list of attributed runs (Label.FormattedText →
        // ToNSAttributedString). Empty means "no formatted text" — the handler then maps the plain text()
        // (Label.cs: a non-null Text clears FormattedText, and a non-null FormattedText clears Text, so the
        // two are mutually exclusive). The concrete control owns the runs; this is a read borrow. See
        // label_run.hpp for the per-run resolution (the IImageSource-style controls→core boundary).
        [[nodiscard]] virtual const std::vector<maui::core::label_run>& formatted_text_runs() const = 0;
    };
} // namespace maui::core
