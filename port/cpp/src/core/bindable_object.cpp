// maui::core::bindable_object — the change-notification base (bindable_object.hpp).
#include "maui/core/bindable_object.hpp"

#include <any>
#include <string_view>
#include <utility>

#include "maui/core/setter_specificity.hpp"

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
