#pragma once
// maui::core::property_path  <=  Microsoft.Maui.Controls.BindingExpression.ParsePath +
//                                BindingExpressionPart (the parsed shape only)
//
// The parsed form of a string binding path ("Customer.Name", "Model.Items[1]", ".", ".[0]"):
// an ordered list of parts, each either the self part ("."), a property name, or an indexer
// (the raw text between brackets). Parsing mirrors C# exactly:
//   - the whole path is trimmed; a leading '.' marks (and consumes) the self hop;
//   - parts split on '.', each trimmed; an empty part is malformed;
//   - "Name[index]" yields a property part followed by an indexer part; the bracket text is kept
//     raw (the indexer decides how to read it), the name before it is trimmed;
//   - a missing ']', empty brackets, or a trailing '.' throw (C# FormatException ->
//     std::invalid_argument here; the port folds C#'s Argument/Format split into one type).
// The first part is ALWAYS the self part (C# seeds parts with "."), so a walker can treat the
// source object itself as hop zero. The walking/subscribing engine lives in binding_expression.hpp.

#include <string>
#include <string_view>
#include <vector>

namespace maui::core
{
    class property_path
    {
    public:
        struct part
        {
            std::string content;     // property name, raw indexer text, or "." for the self part
            bool is_indexer = false; // "[...]" part — resolves through i_indexable
            bool is_self = false;    // the leading self hop (the source object itself)
        };

        // Parse `path` (throws std::invalid_argument on malformed syntax — C# FormatException).
        [[nodiscard]] static property_path parse(std::string_view path);

        [[nodiscard]] const std::vector<part>& parts() const
        {
            return parts_;
        }
        [[nodiscard]] const std::string& text() const
        {
            return text_;
        }

    private:
        property_path() = default;

        std::string text_;
        std::vector<part> parts_;
    };
} // namespace maui::core
