#pragma once
// maui::xaml::xaml_static_registry — the {x:Static} member table (M7 wave 2).
//
// C# counterpart: StaticExtension.ProvideValue (src/Controls/src/Xaml/MarkupExtensions/
// StaticExtension.cs) resolves "[prefix:]typeName.memberName" via IXamlTypeResolver and then walks
// the type hierarchy by REFLECTION for a static property / field (constants and enum members
// included). C++23 has no reflection (PROFILE §6), so the port replaces the whole walk with an
// explicit table: "Type.Member" → a value-producing function, registered up front. The function is
// invoked per use (C# invokes the property getter per ProvideValue), returning the value boxed as the
// exact type the property registry's setters unbox.
//
// register_standard_markup_extensions (markup_extensions.hpp) seeds the table with the port's
// obvious statics — the 147 named colors as "Colors.<Name>" (Microsoft.Maui.Graphics.Colors, the
// dominant x:Static target in MAUI markup). Apps register their own statics the same way
// (the explicit-registration analog of C#'s "any public static member" reach).
//
// Error strategy (xaml_parse_exception.hpp): lookups are throw-free — find() returns nullptr on a
// miss; the StaticExtension turns a miss into the thrown xaml_parse_exception ("No static member
// found for {Member}"), mirroring C#.

#include <any>
#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace maui::xaml
{
    class xaml_static_registry
    {
    public:
        // Produces the member's CURRENT value, boxed (C# invokes the static getter / reads the field).
        using member_fn = std::function<std::any()>;

        // The process-wide registry (function-local static, like markup_extension_registry).
        [[nodiscard]] static xaml_static_registry& instance();

        // Register `member` ("Colors.Red", "MyStatics.AppName") to produce `value`. Later
        // registrations replace earlier ones (test seam).
        void register_member(std::string member, member_fn value);

        // The producer for `member`, or nullptr when unknown (throw-free miss).
        [[nodiscard]] const member_fn* find(std::string_view member) const;

    private:
        std::map<std::string, member_fn, std::less<>> members_;
    };
} // namespace maui::xaml
