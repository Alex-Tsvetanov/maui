// maui::graphics::text::attributed_text_run — out-of-line definitions. See attributed_text_run.hpp.
// Ported from src/Graphics/src/Graphics/Text/{AttributedTextRun,AttributedTextRunExtensions,
// AttributedTextRunComparer}.cs. The intersection split and the list-level Optimize mirror the C#
// index manipulation move for move (including the in-loop RemoveAt/InsertRange dance).

#include "maui/graphics/text/attributed_text_run.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "maui/graphics/text/text_attributes.hpp"

namespace maui::graphics::text
{
    attributed_text_run::attributed_text_run(int start, int length, text_attributes attributes)
        : start_(start), length_(length), attributes_(std::move(attributes))
    {
    }

    int attributed_text_run::start() const
    {
        return start_;
    }

    int attributed_text_run::length() const
    {
        return length_;
    }

    const text_attributes& attributed_text_run::attributes() const
    {
        return attributes_;
    }

    int attributed_text_run::get_end() const
    {
        // C# GetEnd: Start + Length.
        return start_ + length_;
    }

    bool attributed_text_run::intersects(const attributed_text_run& second) const
    {
        // C# Intersects(first, second): first.Start < second.GetEnd() && first.GetEnd() > second.Start.
        return start_ < second.get_end() && get_end() > second.start_;
    }

    bool attributed_text_run::intersects(int start, int length) const
    {
        const int end = start + length;
        return start_ < end && get_end() > start;
    }

    bool attributed_text_run::intersects_exactly(const attributed_text_run& second) const
    {
        return start_ == second.start_ && length_ == second.length_;
    }

    bool attributed_text_run::intersects_exactly(int start, int length) const
    {
        return start_ == start && length_ == length;
    }

    std::vector<attributed_text_run> attributed_text_run::calculated_intersections(
        const attributed_text_run& second) const
    {
        // C# CalculatedIntersections, branch for branch. "combined" is first ∪ second (second wins).
        const attributed_text_run& first = *this;
        std::vector<attributed_text_run> intersections;
        const text_attributes combined(first.attributes_, second.attributes_);

        if (first.start_ == second.start_)
        {
            if (first.length_ == second.length_)
            {
                intersections.emplace_back(first.start_, first.length_, combined);
            }
            else if (first.length_ > second.length_)
            {
                const int start1 = first.start_;
                const int length1 = std::min(first.length_, second.length_);
                const int start2 = start1 + length1;
                const int length2 = std::max(first.length_, second.length_) - length1;

                intersections.emplace_back(start1, length1, combined);
                intersections.emplace_back(start2, length2, first.attributes_);
            }
            else
            {
                const int start1 = first.start_;
                const int length1 = std::min(first.length_, second.length_);
                const int start2 = start1 + length1;
                const int length2 = std::max(first.length_, second.length_) - length1;

                intersections.emplace_back(start1, length1, combined);
                intersections.emplace_back(start2, length2, second.attributes_);
            }
        }
        else if (first.get_end() == second.get_end())
        {
            if (first.start_ < second.start_)
            {
                const int start1 = first.start_;
                const int length1 = second.start_ - first.start_;
                const int start2 = start1 + length1;
                const int length2 = std::max(first.length_, second.length_) - length1;

                intersections.emplace_back(start1, length1, first.attributes_);
                intersections.emplace_back(start2, length2, combined);
            }
            else
            {
                const int start1 = second.start_;
                const int length1 = first.start_ - second.start_;
                const int start2 = start1 + length1;
                const int length2 = std::max(first.length_, second.length_) - length1;

                intersections.emplace_back(start1, length1, second.attributes_);
                intersections.emplace_back(start2, length2, combined);
            }
        }
        else
        {
            if (first.start_ < second.start_)
            {
                const int start1 = first.start_;
                const int length1 = second.start_ - first.start_;
                const int start2 = start1 + length1;
                const int length2 = second.length_;
                const int start3 = start2 + length2;
                const int length3 = std::max(first.length_, second.length_) - (length1 + length2);

                intersections.emplace_back(start1, length1, first.attributes_);
                intersections.emplace_back(start2, length2, combined);
                intersections.emplace_back(start3, length3,
                                           first.get_end() > second.get_end() ? first.attributes_ : second.attributes_);
            }
            else
            {
                const int start1 = second.start_;
                const int length1 = first.start_ - second.start_;
                const int start2 = start1 + length1;
                const int length2 = first.length_;
                const int start3 = start2 + length2;
                const int length3 = std::max(first.length_, second.length_) - (length1 + length2);

                intersections.emplace_back(start1, length1, second.attributes_);
                intersections.emplace_back(start2, length2, combined);
                intersections.emplace_back(start3, length3,
                                           first.get_end() > second.get_end() ? first.attributes_ : second.attributes_);
            }
        }

        return intersections;
    }

    void optimize_runs(std::vector<attributed_text_run>& runs, int text_length)
    {
        // C# AttributedTextRunExtensions.Optimize — pass 1: clamp every run to [0, textLength),
        // dropping the runs that clamp to nothing.
        for (std::ptrdiff_t i = 0; i < std::ssize(runs); i++)
        {
            const attributed_text_run& run = runs[static_cast<std::size_t>(i)];
            const int end = run.get_end();

            if (run.start() < 0 || end > text_length)
            {
                const int start = std::max(run.start(), 0);
                const int max_length = text_length - start;
                const int length = std::min(run.length(), max_length);
                if (length > 0)
                {
                    runs[static_cast<std::size_t>(i)] = attributed_text_run(start, length, run.attributes());
                }
                else
                {
                    runs.erase(runs.begin() + i);
                    i--;
                }
            }
        }

        std::ranges::sort(runs, attributed_text_run_comparer{});

        // Pass 2: join the runs that overlap (exact overlaps merge attribute bags; partial overlaps
        // split through CalculatedIntersections, then re-sort) — the C# loop ported move for move.
        std::optional<attributed_text_run> previous;
        for (std::ptrdiff_t i = 0; i < std::ssize(runs); i++)
        {
            attributed_text_run run = runs[static_cast<std::size_t>(i)];

            if (previous.has_value())
            {
                if (previous->intersects_exactly(run))
                {
                    const text_attributes combined(previous->attributes(), run.attributes());
                    run = attributed_text_run(run.start(), run.length(), combined);
                    runs[static_cast<std::size_t>(i - 1)] = run;
                    runs.erase(runs.begin() + i);
                    i--;
                }
                else if (previous->intersects(run))
                {
                    const std::vector<attributed_text_run> intersections = previous->calculated_intersections(run);
                    runs.erase(runs.begin() + i); // RemoveAt(i--): drop the current run...
                    i--;
                    runs.erase(runs.begin() + i); // RemoveAt(i): ...then the previous one
                    runs.insert(runs.begin() + i, intersections.begin(), intersections.end());
                    i++;
                    run = runs[static_cast<std::size_t>(i)];
                    std::ranges::sort(runs, attributed_text_run_comparer{});
                }
            }

            previous = run;
        }
    }
} // namespace maui::graphics::text
