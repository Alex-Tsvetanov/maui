#pragma once
// maui::xaml::name_scope  <=  Microsoft.Maui.Controls.Internals.NameScope (+ INameScope)
// maui::xaml::name_scope_ref  <=  Microsoft.Maui.Controls.Xaml.NameScopeRef (XamlNode.cs)
//
// The x:Name registry of one XAML namescope: RegisterName / FindByName / UnregisterName, ported from
// src/Controls/src/Core/Internals/NameScope.cs (the interface + class collapse into one concrete type;
// the port has no second INameScope implementation). Stored values are the hydrated objects the
// RegisterXNames visitor sees — C#'s Dictionary<string, object> maps onto std::any (a control is a
// shared_ptr<bindable_object>, an x:String resource a std::string), with the typed find_by_name_as<T>
// helper unboxing the common control case. Keys compare ordinal (StringComparer.Ordinal ==
// std::string's operator<). The scope OWNS what it stores the same way C#'s GC-rooted dictionary does:
// a shared_ptr registered here keeps the control alive alongside the xaml_object_graph (no cycle —
// controls never point back at a scope).
//
// PLACEMENT DEVIATION (documented): C# attaches a scope to each BindableObject through the attached
// NameScope.NameScopeProperty and Element.FindByName walks RealParent until it finds one. The port's
// bindable_object has no attached-property store (PROFILE §7 removed the central bag), so scopes hang
// off the XAML NODES during the load (element_node::scope_ref) and the loader RESULT carries the root
// scope for caller-side find_by_name (xaml_loader.hpp). Per-element FindByName and the
// transientNamescope VSM workaround wait for an element-side scope slot.
//
// name_scope_ref is C#'s extra indirection: every node of a scope shares ONE ref object, so replacing
// ref->scope (CreateValuesVisitor does this when a load_into root already owns a scope) retargets the
// whole scope's nodes at once.

#include <any>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"

namespace maui::xaml
{
    class name_scope
    {
    public:
        // INameScope.RegisterName: throws std::invalid_argument (C# ArgumentException(paramName:
        // "name")) when the name is already registered — the RegisterXNames visitor converts that
        // into the user-facing xaml_parse_exception, exactly like C#.
        void register_name(std::string name, std::any scoped_element);

        // INameScope.FindByName: the stored value, or nullptr when unregistered (C# returns null).
        // The pointer is valid until the scope mutates.
        [[nodiscard]] const std::any* find_by_name(std::string_view name) const;

        // The common typed lookup: the registered control as TControl, or nullptr when the name is
        // unregistered, not a control, or a different control type (FindByName<T> as Element.cs's
        // generic extension uses it).
        template <class TControl> [[nodiscard]] std::shared_ptr<TControl> find_by_name_as(std::string_view name) const
        {
            const std::any* value = find_by_name(name);
            if (value == nullptr)
            {
                return nullptr;
            }
            const auto* object = std::any_cast<std::shared_ptr<maui::core::bindable_object>>(value);
            return object != nullptr ? std::dynamic_pointer_cast<TControl>(*object) : nullptr;
        }

        // INameScope.UnregisterName: throws std::invalid_argument on an empty or unregistered name
        // (C# ArgumentException; the null-name overload cannot arise — std::string is never null).
        void unregister_name(std::string_view name);

    private:
        std::map<std::string, std::any, std::less<>> names_;
    };

    // ---- name_scope_ref  <=  Microsoft.Maui.Controls.Xaml.NameScopeRef ----
    class name_scope_ref
    {
    public:
        std::shared_ptr<name_scope> scope; // NameScopeRef.NameScope (mutable on purpose — see above)
    };
} // namespace maui::xaml
