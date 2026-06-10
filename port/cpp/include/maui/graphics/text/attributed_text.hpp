#pragma once
// maui::graphics::text::attributed_text  <=  Microsoft.Maui.Graphics.Text.AttributedText
//   (+ AttributedTextBlock.cs — a tight cluster: blocks only exist as CreateBlocks' output —
//    + the free functions from AttributedTextExtensions.cs)
//
// The standard immutable i_attributed_text: a string, its runs, and the Optimal flag. The C#
// extension methods Optimize / CreateParagraphs / CreateParagraphRun / CreateBlocks are the free
// functions below (they operate on the i_attributed_text contract, not just this type).
//
// Documented-deferred (recorded in port/STATUS.md): the XML wire format
// (XmlAttributedTextReader/Writer.cs, XmlAttributedTextNames.cs) and MutableAttributedText.cs —
// nothing in the canvas core consumes them; port them with the first real reader/writer call site.
//
// Out-of-line definitions live in src/graphics/text/attributed_text.cpp.

#include <optional>
#include <string>
#include <vector>

#include "maui/graphics/text/attributed_text_run.hpp"
#include "maui/graphics/text/i_attributed_text.hpp"
#include "maui/graphics/text/text_attributes.hpp"

namespace maui::graphics::text
{
    class attributed_text final : public i_attributed_text
    {
    public:
        // C# AttributedText(string text, IReadOnlyList<IAttributedTextRun> runs, bool optimal = false).
        attributed_text(std::string text, std::vector<attributed_text_run> runs, bool optimal = false);

        [[nodiscard]] const std::string& text() const override;
        [[nodiscard]] const std::vector<attributed_text_run>& runs() const override;
        [[nodiscard]] bool optimal() const override;

    private:
        std::string text_;
        std::vector<attributed_text_run> runs_;
        bool optimal_;
    };

    // C# AttributedTextBlock — a contiguous piece of text plus the attributes covering it (nullopt
    // for the unattributed gaps), produced by create_blocks. ToString() (debug-only) is omitted.
    class attributed_text_block
    {
    public:
        attributed_text_block(std::string text, std::optional<text_attributes> attributes);

        [[nodiscard]] const std::string& text() const;
        [[nodiscard]] const std::optional<text_attributes>& attributes() const;

    private:
        std::string text_;
        std::optional<text_attributes> attributes_;
    };

    // ---- AttributedTextExtensions (C# null-text inputs cannot arise: text is a value here) ----

    // C# Optimize — an already-optimal text passes through unchanged (copied — the port returns a
    // value, C# returns the same reference); otherwise the runs are redistributed via
    // create_paragraph_run over the whole range and the result is flagged optimal.
    [[nodiscard]] attributed_text optimize(const i_attributed_text& text);

    // C# CreateParagraphs — split on line breaks (StringReader.ReadLine semantics: \n, \r, \r\n),
    // distributing each run onto the paragraphs it covers (run offsets become paragraph-relative).
    [[nodiscard]] std::vector<attributed_text> create_paragraphs(const i_attributed_text& text);

    // C# CreateParagraphRun — append the runs of text covering [start, start+length) to `runs`
    // (offsets rebased; see the C# original for the boundary cases) and return the index where the
    // search for the NEXT paragraph should resume.
    int create_paragraph_run(const i_attributed_text& text, int start, int length,
                             std::vector<attributed_text_run>& runs, int start_index_for_search = 0);

    // C# CreateBlocks — cut the text into contiguous blocks: attributed pieces (one per run) with
    // unattributed gap blocks between them.
    [[nodiscard]] std::vector<attributed_text_block> create_blocks(const i_attributed_text& text);
} // namespace maui::graphics::text
