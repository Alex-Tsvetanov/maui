#pragma once
// maui::xaml::xaml_parse_exception  <=  Microsoft.Maui.Controls.Xaml.XamlParseException
//
// The XAML subsystem's error channel, ported from src/Controls/src/Core/XamlParseException.cs.
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
// The one in-library throw site: a REGISTERED converter rejecting a malformed literal throws
// xaml_parse_exception — the net behavior of C#'s `Double.Parse` raising FormatException, which
// TypeConversionExtensions.ConvertTo catches and surfaces for the visitor to throw as a XAML error.
//
// C#'s XamlParseException also carries IXmlLineInfo (line/position) and prefixes it into the message
// (FormatMessage: "Position {line}:{pos}. {message}"). The registries have no XML position, so this
// port carries the message only; the M7 loader — the layer that owns positions — formats them into the
// message it throws with, preserving the C# message shape without widening this type.

#include <stdexcept>
#include <string>

namespace maui::xaml
{
    class xaml_parse_exception : public std::runtime_error
    {
    public:
        explicit xaml_parse_exception(const std::string& message) : std::runtime_error(message)
        {
        }
    };
} // namespace maui::xaml
