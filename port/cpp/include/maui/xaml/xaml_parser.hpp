#pragma once
// maui::xaml::xaml_parser  <=  Microsoft.Maui.Controls.Xaml.XamlParser (the parse half)
//
// XAML markup → the maui::xaml node tree (xaml_node.hpp). Mirrors XamlParser.cs's reader loop
// (ParseXaml / ParseXamlElementFor / ReadNode / ParseXamlAttributes / GetValueNode /
// PrefixesToIgnore) plus the XamlLoader.Load/Create root-node construction, re-expressed over a
// pugixml DOM walk (C# pulls from an XmlReader; the decisions and their order are identical — see
// xaml_parser.cpp). The reflection-driven other half of XamlParser.cs (GetElementType + the xmlns →
// type registries) is the concurrent M7 registries unit; visitors/hydration are wave 2.
//
// Also here, as in the C# file set:
//   - type_arguments_parser         <=  Microsoft.Maui.Controls.Xaml.TypeArgumentsParser
//   - the markup-extension STRING TOKENIZATION primitives (match_markup / get_next_piece /
//     parse_markup_name)            <=  Microsoft.Maui.Controls.Xaml.MarkupExpressionParser
//     The "{…}" DETECTION lives in the parser (GetValueNode → markup_node). Extension RESOLUTION
//     (MarkupExtensionParser.Parse + ProvideValue, and the resolution-coupled ParseProperty /
//     ParsePropertyExpression which recurse through IExpressionParser) is wave 2.

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/xaml/xaml_node.hpp"
// xaml_parse_exception (<= XamlParseException) lives in its own header — the parser constructs it
// WITH line info ("Position {line}:{position}. {message}"); the registries construct message-only.
#include "maui/xaml/xaml_parse_exception.hpp"

namespace maui::xaml
{
    // ---- parse_options ----
    // The DeviceInfo.Platform seam for XamlParser.PrefixesToIgnore: an xmlns declaration carrying a
    // `targetPlatform=` clause makes its prefix ignorable when the platform does NOT match.
    // nullopt mirrors C#'s "DeviceInfo unavailable" catch — every targetPlatform'd prefix is then
    // ignorable. (The "Windows" / "WinUI" backward-compatibility match is ported too.)
    struct parse_options
    {
        std::optional<std::string> target_platform;
    };

    // ---- type_arguments_parser  <=  Microsoft.Maui.Controls.Xaml.TypeArgumentsParser ----
    // Parses an x:TypeArguments expression ("x:String" / "sys:Int32, sys:String" / nested generics
    // "ns:List(x:String)") against the in-scope xmlns. Throws xaml_parse_exception for an
    // undeclared prefix ("No xmlns declaration for prefix '…'.") or an invalid type name (C# stores
    // a null XmlType and fails later — the port fails eagerly; documented deviation).
    class type_arguments_parser
    {
    public:
        type_arguments_parser() = delete;

        [[nodiscard]] static std::vector<xml_type> parse_expression(std::string_view expression,
                                                                    const xml_namespace_resolver& resolver,
                                                                    int line_number = -1, int line_position = -1);
        [[nodiscard]] static xml_type parse_single(std::string_view expression, const xml_namespace_resolver& resolver,
                                                   int line_number = -1, int line_position = -1);
    };

    // ---- xaml_parser  <=  Microsoft.Maui.Controls.Xaml.XamlParser (static class) ----
    class xaml_parser
    {
    public:
        xaml_parser() = delete;

        // XamlLoader.Load/Create's reader loop + XamlParser.ParseXaml: parse a whole XAML document
        // into its node tree. Throws xaml_parse_exception on malformed XML (line info from the
        // offending offset) and on the parser-level errors (duplicate property element, duplicate
        // x:Arguments, multiple DataTemplate children, undeclared prefixes, …).
        //
        // Line info on the nodes mirrors XmlReader's 1-based line/column of the element-name /
        // text start. Deviations (pugixml offers no per-attribute or end-tag offsets): nodes built
        // from ATTRIBUTE values carry the owning element's position, and a multi-value list_node
        // carries the property element's start position instead of its end tag's.
        [[nodiscard]] static std::shared_ptr<root_node> parse(std::string_view xaml, const parse_options& options = {});

        // XamlParser.ParsePropertyName: maps x2006/x2009-namespace attributes onto the canonical
        // x:* directive names (xml_name::x_key() etc.); unknown x-namespace attributes map to
        // xml_name::empty() (the caller skips them); everything else passes through.
        [[nodiscard]] static xml_name parse_property_name(const xml_name& name);
    };

    // ---- markup-extension string tokenization  <=  MarkupExpressionParser (tokenization half) ----

    // MarkupExpressionParser.MatchMarkup: extracts the extension name of "{Name …}". On failure
    // `matched` is false and `end` carries the C# diagnostic out-codes (1: too short, 2: no leading
    // '{', 3: all blank, 6: unterminated); on success `end` is the index just past the name.
    struct markup_match
    {
        bool matched = false;
        std::string match;
        std::size_t end = 0;
    };
    [[nodiscard]] markup_match match_markup(std::string_view expression);

    // MarkupExpressionParser.GetNextPiece: consumes one value token (quote-aware, '\'-escape-aware,
    // trailing-whitespace-trimmed) up to the next '}' ',' or '='. `next` is that delimiter;
    // `remaining` is the suffix AFTER it — a view into the caller's buffer (keep it alive).
    // Throws xaml_parse_exception: "Unterminated quoted string" / "Empty value string in markup
    // expression" / "Unexpected end of markup expression".
    struct markup_piece
    {
        std::string piece;
        char next = '\0';
        std::string_view remaining;
    };
    [[nodiscard]] markup_piece get_next_piece(std::string_view remaining);

    // MarkupExpressionParser.ParseName: "prefix:name" → (prefix, name), "name" → ("", name).
    // Throws std::invalid_argument for more than one ':' (C# ArgumentException).
    [[nodiscard]] std::pair<std::string, std::string> parse_markup_name(std::string_view name);
} // namespace maui::xaml
