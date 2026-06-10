#pragma once
// maui::controls::dynamic_resource  <=  Microsoft.Maui.Controls.Internals.DynamicResource
//
// A reference to a resource by key, NOT the resource's value — the marker the {DynamicResource Key}
// markup extension provides (DynamicResourceExtension.ProvideValue returns one), which the XAML
// loader's apply step special-cases into element::set_dynamic_resource instead of a plain property
// set (ApplyPropertiesVisitor.TrySetDynamicResource; Setter.Apply does the same for styles). Ported
// from src/Controls/src/Core/Internals/DynamicResource.cs: a single get-only Key. Header-only — a
// two-member value type with nothing out-of-line (PROFILE §3).

#include <string>
#include <utility>

namespace maui::controls
{
    class dynamic_resource
    {
    public:
        explicit dynamic_resource(std::string key) : key_(std::move(key))
        {
        }

        [[nodiscard]] const std::string& key() const // DynamicResource.Key
        {
            return key_;
        }

    private:
        std::string key_;
    };
} // namespace maui::controls
