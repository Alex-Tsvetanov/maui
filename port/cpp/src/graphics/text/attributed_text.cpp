// maui::graphics::text::attributed_text — out-of-line definitions. See attributed_text.hpp. Ported
// from src/Graphics/src/Graphics/Text/{AttributedText,AttributedTextBlock,AttributedTextExtensions}.cs.
// create_paragraphs reproduces System.IO.StringReader.ReadLine's line-break handling (\n, \r, \r\n)
// AND the C# quirk of advancing the run-offset cursor by length + 1 regardless of the actual
// terminator width (a \r\n shifts the following paragraphs' run offsets by one in C# too).

#include "maui/graphics/text/attributed_text.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/graphics/text/attributed_text_run.hpp"
#include "maui/graphics/text/i_attributed_text.hpp"
#include "maui/graphics/text/text_attributes.hpp"

namespace maui::graphics::text
{
    namespace
    {
        // System.IO.StringReader.ReadLine over an in-memory string: returns the next line without
        // its terminator (consuming \r\n as one break), nullopt once the string is exhausted.
        std::optional<std::string> read_line(const std::string& value, std::size_t& pos)
        {
            if (pos >= value.size())
            {
                return std::nullopt;
            }
            std::size_t i = pos;
            while (i < value.size() && value[i] != '\r' && value[i] != '\n')
            {
                i++;
            }
            std::string line = value.substr(pos, i - pos);
            if (i < value.size())
            {
                if (value[i] == '\r' && i + 1 < value.size() && value[i + 1] == '\n')
                {
                    i += 2;
                }
                else
                {
                    i += 1;
                }
            }
            pos = i;
            return line;
        }
    } // namespace

    attributed_text::attributed_text(std::string text, std::vector<attributed_text_run> runs, bool optimal)
        : text_(std::move(text)), runs_(std::move(runs)), optimal_(optimal)
    {
    }

    const std::string& attributed_text::text() const
    {
        return text_;
    }

    const std::vector<attributed_text_run>& attributed_text::runs() const
    {
        return runs_;
    }

    bool attributed_text::optimal() const
    {
        return optimal_;
    }

    attributed_text_block::attributed_text_block(std::string text, std::optional<text_attributes> attributes)
        : text_(std::move(text)), attributes_(std::move(attributes))
    {
    }

    const std::string& attributed_text_block::text() const
    {
        return text_;
    }

    const std::optional<text_attributes>& attributed_text_block::attributes() const
    {
        return attributes_;
    }

    attributed_text optimize(const i_attributed_text& text)
    {
        // C# Optimize: an already-optimal text is returned unchanged (here: copied into the
        // standard attributed_text); otherwise redistribute the runs over the full range.
        if (text.optimal())
        {
            return {text.text(), text.runs(), true};
        }

        const int start = 0;
        const int attribute_index = 0;
        const auto length = static_cast<int>(text.text().length());
        std::vector<attributed_text_run> runs;
        create_paragraph_run(text, start, length, runs, attribute_index);
        return {text.text(), std::move(runs), true};
    }

    std::vector<attributed_text> create_paragraphs(const i_attributed_text& text)
    {
        std::vector<attributed_text> paragraphs;

        int start = 0;
        int attribute_index = 0;

        std::size_t pos = 0;
        while (const std::optional<std::string> line = read_line(text.text(), pos))
        {
            const auto length = static_cast<int>(line->length());

            std::vector<attributed_text_run> runs;
            attribute_index = create_paragraph_run(text, start, length, runs, attribute_index);

            paragraphs.emplace_back(*line, std::move(runs));

            start += length + 1;
        }

        return paragraphs;
    }

    int create_paragraph_run(const i_attributed_text& text, int start, int length,
                             std::vector<attributed_text_run>& runs, int start_index_for_search)
    {
        // C# CreateParagraphRun, branch for branch.
        const std::vector<attributed_text_run>& source_runs = text.runs();

        // If the text doesn't have any runs, then we can simply return.
        if (source_runs.empty())
        {
            return 0;
        }

        // If we've already reached the end of the runs, we can simply return.
        if (!std::cmp_less(start_index_for_search, source_runs.size()))
        {
            return start_index_for_search;
        }

        const int end = start + length;
        int index = start_index_for_search;

        // C# loops do-while; the guard above guarantees the first iteration, so a while is
        // equivalent (and keeps the linters happy).
        while (std::cmp_less(index, source_runs.size()))
        {
            const attributed_text_run& run = source_runs[static_cast<std::size_t>(index)];

            // If the run is after the end index, then we can go ahead and return.
            if (end < run.start())
            {
                return index;
            }

            if (run.intersects(start, length))
            {
                if (start == run.start())
                {
                    const int paragraph_start = run.start() - start;
                    const int paragraph_length = std::min(run.length(), length);
                    runs.emplace_back(paragraph_start, paragraph_length, run.attributes());

                    // Same length: the next run (if any) applies to the next paragraph.
                    if (run.length() == length)
                    {
                        return index + 1;
                    }

                    // Longer than the line: this run's attributes also apply to the next paragraph.
                    if (run.length() > length)
                    {
                        return index;
                    }

                    // Shorter than the line: the next run may still apply to this line — continue.
                }
                else if (end == run.get_end())
                {
                    const int paragraph_start = std::max(run.start() - start, 0);
                    const int paragraph_length = std::min(run.length(), end - paragraph_start);
                    runs.emplace_back(paragraph_start, paragraph_length, run.attributes());

                    // Run and line share an end: the next run (if any) is for the next paragraph.
                    return index + 1;
                }
                else
                {
                    const int paragraph_start = std::max(run.start() - start, 0);
                    const int paragraph_length = std::min(run.length(), length - paragraph_start);
                    runs.emplace_back(paragraph_start, paragraph_length, run.attributes());
                }
            }

            index++;
        }

        return index;
    }

    std::vector<attributed_text_block> create_blocks(const i_attributed_text& text)
    {
        std::vector<attributed_text_block> blocks;

        std::size_t start = 0;
        const std::size_t end = text.text().length();

        for (const attributed_text_run& run : text.runs())
        {
            if (std::cmp_less(start, run.start()))
            {
                // The unattributed gap before this run.
                const auto no_attr_length = static_cast<std::size_t>(run.start()) - start;
                blocks.emplace_back(text.text().substr(start, no_attr_length), std::nullopt);
                start = static_cast<std::size_t>(run.start());
            }

            const int length = run.length();
            if (length > 0)
            {
                blocks.emplace_back(text.text().substr(start, static_cast<std::size_t>(length)), run.attributes());
                start = static_cast<std::size_t>(run.get_end());
            }
        }

        if (start < end)
        {
            blocks.emplace_back(text.text().substr(start), std::nullopt);
        }

        return blocks;
    }
} // namespace maui::graphics::text
