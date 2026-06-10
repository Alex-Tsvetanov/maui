#pragma once
// maui::xaml::xaml_parse_exception  <=  Microsoft.Maui.Controls.Xaml.XamlParseException
//
// The XAML subsystem's ONE error channel, ported from src/Controls/src/Core/XamlParseException.cs.
// (M7 wave 1 briefly shipped two flavors — a message-only class here and a line-info class in
// xaml_parser.hpp; unified here in wave 2 so the registries, the parser, and the markup extensions
// all throw the same definition and the ODR stays clean.)
//
// ERROR-CHANNEL DECISION (M7 wave 1). The C# loader splits errors in two:
//   - its lookup PRIMITIVES are throw-free — XamlParser.GetElementType returns null plus an `out
//     XamlParseException`, and ApplyPropertiesVisitor.TrySetPropertyValue returns false plus an out
//     exception;
//   - the LOADER then THROWS the XamlParseException (CreateValuesVisitor / ApplyPropertiesVisitor
//     `throw xpe`) unless a hydration ExceptionHandler intercepts it.
// The port mirrors that split exactly: every registry lookup in this library is throw-free (nullptr /
// false / an empty std::any on a miss), and the M7 loader converts a miss into a thrown
// xaml_parse_exception. An exception (not a result type) is chosen because exceptions are this
// codebase's established channel for boundary failures (service_registry::get_required_service and the
// graphics color/grid_length parsers all throw std-exceptions), and deriving std::runtime_error keeps
// it catchable alongside them.
//
// In-library throw sites: a REGISTERED converter rejecting a malformed literal (the net behavior of
// C#'s `Double.Parse` raising FormatException, which TypeConversionExtensions.ConvertTo catches and
// surfaces for the visitor to throw as a XAML error), the xaml_parser's malformed-markup errors, and
// the markup extensions' factory/ProvideValue failures (StaticResourceExtension & co. throw
// XamlParseException directly — markup_extensions.hpp).
//
// C#'s XamlParseException carries IXmlLineInfo (line/position) and prefixes it into the message
// (FormatMessage: "Position {line}:{pos}. {message}"). The parser — the layer that owns positions —
// constructs with line info (1-based; -1 = none) so what() carries the C#-formatted shape; the
// registries and markup extensions have no XML position and construct with the message alone.

#include <format>
#include <stdexcept>
#include <string>

namespace maui::xaml
{
    class xaml_parse_exception : public std::runtime_error
    {
    public:
        explicit xaml_parse_exception(const std::string& message, int line_number = -1, int line_position = -1)
            : std::runtime_error(format_message(message, line_number, line_position)), unformatted_message_(message),
              line_number_(line_number), line_position_(line_position)
        {
        }

        [[nodiscard]] const std::string& unformatted_message() const
        {
            return unformatted_message_;
        }
        [[nodiscard]] int line_number() const
        {
            return line_number_;
        }
        [[nodiscard]] int line_position() const
        {
            return line_position_;
        }
        [[nodiscard]] bool has_line_info() const
        {
            return line_number_ >= 0 && line_position_ >= 0;
        }

    private:
        // XamlParseException.FormatMessage: "Position {line}:{pos}. {message}" when line info exists.
        [[nodiscard]] static std::string format_message(const std::string& message, int line_number, int line_position)
        {
            if (line_number < 0 || line_position < 0)
            {
                return message;
            }
            return std::format("Position {}:{}. {}", line_number, line_position, message);
        }

        std::string unformatted_message_;
        int line_number_;
        int line_position_;
    };
} // namespace maui::xaml
