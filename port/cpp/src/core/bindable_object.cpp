// maui::core::bindable_object — the change-notification base (bindable_object.hpp).
#include "maui/core/bindable_object.hpp"

#include <any>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "maui/core/binding_mode.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::core
{
    void bindable_object::on_property_changed(std::string_view name)
    {
        property_changed.raise(name);
    }

    void bindable_object::on_property_changing(std::string_view name)
    {
        property_changing.raise(name);
    }

    void bindable_object::register_property(std::string_view name, property_handle handle)
    {
        properties_.insert_or_assign(name, std::move(handle));
    }

    void bindable_object::unregister_property(std::string_view name)
    {
        properties_.erase(name);
    }

    void bindable_object::apply_setter(std::string_view name, const std::any& value, setter_specificity specificity)
    {
        if (auto it = properties_.find(name); it != properties_.end())
        {
            it->second.apply(value, specificity);
        }
    }

    void bindable_object::clear_setter(std::string_view name, setter_specificity specificity)
    {
        if (auto it = properties_.find(name); it != properties_.end())
        {
            it->second.clear(specificity);
        }
    }

    // --- runtime bindings (W1-02) -----------------------------------------------------------------
    namespace
    {
        // The one property name the channel recognizes without a registered handle: the binding
        // context itself (C#'s BindableObject.BindingContextProperty, name "BindingContext").
        constexpr std::string_view k_binding_context_name = "binding_context";
    } // namespace

    bool bindable_object::has_property(std::string_view name) const
    {
        return name == k_binding_context_name || properties_.contains(name);
    }

    bool bindable_object::is_property_set(std::string_view name) const
    {
        if (auto it = properties_.find(name); it != properties_.end() && it->second.is_set)
        {
            return it->second.is_set();
        }
        return false;
    }

    std::optional<std::any> bindable_object::try_get_value(std::string_view name) const
    {
        if (name == k_binding_context_name)
        {
            return binding_context_.boxed;
        }
        if (auto it = properties_.find(name); it != properties_.end() && it->second.get)
        {
            return it->second.get();
        }
        return std::nullopt;
    }

    std::shared_ptr<bindable_object> bindable_object::try_get_object(std::string_view name) const
    {
        if (name == k_binding_context_name)
        {
            return binding_context_.object;
        }
        if (auto it = properties_.find(name); it != properties_.end() && it->second.get_object)
        {
            return it->second.get_object();
        }
        return nullptr;
    }

    bool bindable_object::try_set_value(std::string_view name, const std::any& value, setter_specificity specificity)
    {
        if (auto it = properties_.find(name); it != properties_.end() && it->second.try_apply)
        {
            return it->second.try_apply(value, specificity);
        }
        return false;
    }

    std::optional<binding_mode> bindable_object::property_default_binding_mode(std::string_view name) const
    {
        if (auto it = properties_.find(name); it != properties_.end())
        {
            return it->second.default_binding_mode;
        }
        return std::nullopt;
    }

    std::optional<bool> bindable_object::property_is_read_only(std::string_view name) const
    {
        if (auto it = properties_.find(name); it != properties_.end())
        {
            return it->second.is_read_only;
        }
        return std::nullopt;
    }

    std::optional<std::any> bindable_object::property_default_value(std::string_view name) const
    {
        if (auto it = properties_.find(name); it != properties_.end() && it->second.get_default)
        {
            return it->second.get_default();
        }
        return std::nullopt;
    }

    std::optional<type_tag> bindable_object::property_type(std::string_view name) const
    {
        if (auto it = properties_.find(name); it != properties_.end())
        {
            return it->second.type;
        }
        return std::nullopt;
    }

    void bindable_object::demote_value_to_binding(std::string_view name)
    {
        if (auto it = properties_.find(name); it != properties_.end() && it->second.demote_to_binding)
        {
            it->second.demote_to_binding();
        }
    }
    // --- end runtime bindings (W1-02) -------------------------------------------------------------

    void bindable_object::on_binding_context_changed()
    {
        binding_context_changed.raise();
    }

    void bindable_object::set_binding_context_raw(binding_context_box context, bool is_explicit)
    {
        if (is_explicit)
        {
            binding_context_explicit_ = true;
        }
        else if (binding_context_explicit_)
        {
            return; // an inherited context never overrides an explicitly-set one
        }
        if (binding_context_.value == context.value)
        {
            return; // ReferenceEquals short-circuit (same shared object => no change)
        }
        binding_context_ = std::move(context);
        on_binding_context_changed();
    }
} // namespace maui::core
