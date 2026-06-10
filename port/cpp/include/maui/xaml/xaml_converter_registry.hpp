#pragma once
// maui::xaml::xaml_converter_registry — string-literal → typed-value conversion table (M7 wave 1).
//
// C# counterpart: TypeConversionExtensions.ConvertTo (src/Controls/src/Core/Xaml/
// TypeConversionExtensions.cs) — given a target Type, it first asks the type's [TypeConverter]
// (reflection-discovered) and then falls back to the built-in invariant-culture conversions
// (Int32.Parse / Double.Parse / Boolean.Parse / the string "{}" escape …). The reflection-free port
// replaces both with one explicit table: register_converter<T>(fn) stores a `const std::string& → T`
// function type-erased under type_tag::of<T>(), and convert(target_type, text) dispatches to it. A
// property registration names its converter IMPLICITLY by its value type T — the
// xaml_property_registry records type_tag::of<T>() per property, so the loader converts an attribute
// string with exactly the converter T names (see xaml_property_registry::try_set_from_text).
//
// Error strategy (xaml_parse_exception.hpp): a LOOKUP miss is throw-free — convert() returns an EMPTY
// std::any when no converter is registered for the target type. A registered converter, however,
// THROWS xaml_parse_exception on a malformed literal — the net behavior of C#'s Parse raising
// FormatException, which ConvertTo catches and the visitor then throws as a XAML error.
//
// Scope (M7 wave 1): only the trivially-available built-ins ship here — std::string, double, int,
// bool, registered by register_standard_xaml_converters (xaml_standard_types.hpp). The MAUI value
// converters (color, thickness, grid-definition lists, image sources, enums, …) are unit U4's
// deliverable and slot into this same seam; tests cover the seam with fakes until then.

#include <any>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

#include "maui/core/type_tag.hpp"

namespace maui::xaml
{
    class xaml_converter_registry
    {
    public:
        // The type-erased stored form: parse `text` and return the value boxed as exactly T (the
        // std::any's held type is what xaml_property_registry's setters unbox).
        using converter = std::function<std::any(const std::string& text)>;

        // Register (or replace) the converter producing T. `convert` is any callable
        // `T(const std::string&)`; it should throw xaml_parse_exception on a malformed literal.
        template <class T, class F> void register_converter(F convert)
        {
            converters_[maui::core::type_tag::of<T>()] = [convert = std::move(convert)](const std::string& text) {
                return std::any{convert(text)};
            };
        }

        [[nodiscard]] bool has_converter(maui::core::type_tag target_type) const;

        // Convert `text` to the type identified by `target_type`. Returns an EMPTY std::any when no
        // converter is registered (throw-free miss); propagates the converter's xaml_parse_exception
        // on a malformed literal.
        [[nodiscard]] std::any convert(maui::core::type_tag target_type, const std::string& text) const;

    private:
        std::unordered_map<maui::core::type_tag, converter> converters_;
    };

    // The process-wide default registry (Meyers singleton), the converter sibling of
    // default_xaml_type_registry() — the M7 loader resolves from it when no explicit registry is
    // threaded through.
    [[nodiscard]] xaml_converter_registry& default_xaml_converter_registry();
} // namespace maui::xaml
