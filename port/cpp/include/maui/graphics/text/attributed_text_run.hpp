#pragma once
// maui::graphics::text::attributed_text_run  <=  Microsoft.Maui.Graphics.Text.AttributedTextRun
//   (+ IAttributedTextRun, AttributedTextRunExtensions.cs, AttributedTextRunComparer.cs)
//
// One attributed range of a text: [start, start + length) plus its attribute bag. Deliberate
// deviation (recorded in port/STATUS.md): C#'s IAttributedTextRun interface is collapsed into this
// concrete type — AttributedTextRun is its only first-party implementation and run lists want value
// semantics. The C# extension methods (GetEnd / Intersects / IntersectsExactly /
// CalculatedIntersections) become members; the list-level Optimize and the comparer are the free
// function / functor below. C#'s debug-only ToString() is omitted.
//
// Out-of-line definitions live in src/graphics/text/attributed_text_run.cpp.

#include <vector>

#include "maui/graphics/text/text_attributes.hpp"

namespace maui::graphics::text
{
    class attributed_text_run
    {
    public:
        // C# AttributedTextRun(int start, int length, ITextAttributes attributes).
        attributed_text_run(int start, int length, text_attributes attributes);

        // C# IAttributedTextRun.Start / .Length / .Attributes.
        [[nodiscard]] int start() const;
        [[nodiscard]] int length() const;
        [[nodiscard]] const text_attributes& attributes() const;

        // C# AttributedTextRunExtensions.GetEnd — Start + Length.
        [[nodiscard]] int get_end() const;

        // C# AttributedTextRunExtensions.Intersects(first, second) / (first, start, length).
        [[nodiscard]] bool intersects(const attributed_text_run& second) const;
        [[nodiscard]] bool intersects(int start, int length) const;

        // C# AttributedTextRunExtensions.IntersectsExactly — identical start AND length.
        [[nodiscard]] bool intersects_exactly(const attributed_text_run& second) const;
        [[nodiscard]] bool intersects_exactly(int start, int length) const;

        // C# AttributedTextRunExtensions.CalculatedIntersections — split two overlapping runs into
        // 1-3 disjoint runs; the overlap carries the union of both attribute bags (second wins).
        [[nodiscard]] std::vector<attributed_text_run> calculated_intersections(
            const attributed_text_run& second) const;

        friend bool operator==(const attributed_text_run& a, const attributed_text_run& b)
        {
            return a.start_ == b.start_ && a.length_ == b.length_ && a.attributes_ == b.attributes_;
        }
        friend bool operator!=(const attributed_text_run& a, const attributed_text_run& b)
        {
            return !(a == b);
        }

    private:
        int start_;
        int length_;
        text_attributes attributes_;
    };

    // C# AttributedTextRunComparer — orders by start, then by length (here as a strict-weak "less"
    // usable with std::sort, equivalent to the C# -1/0/1 comparison).
    struct attributed_text_run_comparer
    {
        [[nodiscard]] bool operator()(const attributed_text_run& first, const attributed_text_run& second) const
        {
            return first.start() < second.start() ||
                   (first.start() == second.start() && first.length() < second.length());
        }
    };

    // C# AttributedTextRunExtensions.Optimize(List<IAttributedTextRun>, int textLength) — clamp the
    // runs to the text bounds (dropping the now-empty ones), sort, then join/split overlaps.
    void optimize_runs(std::vector<attributed_text_run>& runs, int text_length);
} // namespace maui::graphics::text
