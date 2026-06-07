// maui::core::bindable_property — the non-template parts (bindable_property.hpp).
#include "maui/core/bindable_property.hpp"

#include <any>
#include <string>
#include <utility>

namespace maui::core
{
    bindable_property::bindable_property(std::string name, std::any default_value, equality equals,
                                         changed_delegate changed, changing_delegate changing, coerce_delegate coerce,
                                         validate_delegate validate, default_value_creator_delegate creator,
                                         bool read_only)
        : name_(std::move(name)), default_value_(std::move(default_value)), equals_(equals),
          changed_(std::move(changed)), changing_(std::move(changing)), coerce_(std::move(coerce)),
          validate_(std::move(validate)), default_value_creator_(std::move(creator)), is_read_only_(read_only)
    {
    }

    const std::string &bindable_property::name() const
    {
        return name_;
    }
    bool bindable_property::is_read_only() const
    {
        return is_read_only_;
    }
    const std::any &bindable_property::default_value() const
    {
        return default_value_;
    }
    bool bindable_property::has_default_value_creator() const
    {
        return static_cast<bool>(default_value_creator_);
    }
    std::any bindable_property::get_default_value(const bindable_object &owner) const
    {
        return default_value_creator_ ? default_value_creator_(owner) : default_value_;
    }
    bool bindable_property::values_equal(const std::any &a, const std::any &b) const
    {
        if (a.has_value() != b.has_value())
        {
            return false;
        }
        if (!a.has_value())
        {
            return true;
        }
        return equals_(a, b);
    }
    const bindable_property::changed_delegate &bindable_property::on_changed() const
    {
        return changed_;
    }
    const bindable_property::changing_delegate &bindable_property::on_changing() const
    {
        return changing_;
    }
    const bindable_property::coerce_delegate &bindable_property::on_coerce() const
    {
        return coerce_;
    }
    const bindable_property::validate_delegate &bindable_property::on_validate() const
    {
        return validate_;
    }
} // namespace maui::core
