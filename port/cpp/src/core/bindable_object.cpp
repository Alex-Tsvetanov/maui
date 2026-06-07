// maui::core::bindable_object — value precedence + change notification (bindable_object.hpp).
#include "maui/core/bindable_object.hpp"

#include <any>
#include <string_view>
#include <utility>

#include "maui/core/bindable_property.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::core
{
    std::any bindable_object::get_value(const bindable_property &property) const
    {
        // A creator must run (and cache) lazily, so its context is materialized on first read.
        if (property.has_default_value_creator())
        {
            return get_or_create_context(property).values.value();
        }
        const context *ctx = get_context(property);
        return ctx != nullptr ? ctx->values.value() : property.default_value();
    }

    void bindable_object::set_value(const bindable_property &property, std::any value)
    {
        set_value(property, std::move(value), setter_specificity::manual_value_setter);
    }

    void bindable_object::set_value(const bindable_property &property, std::any value, setter_specificity specificity)
    {
        if (property.is_read_only())
        {
            return; // C# logs a warning and ignores
        }
        set_value_core(property, std::move(value), specificity);
    }

    void bindable_object::clear_value(const bindable_property &property)
    {
        clear_value(property, setter_specificity::manual_value_setter);
    }

    void bindable_object::clear_value(const bindable_property &property, setter_specificity specificity)
    {
        if (property.is_read_only())
        {
            return;
        }
        clear_value_core(property, specificity);
    }

    void bindable_object::on_property_changed(std::string_view name)
    {
        property_changed.raise(name);
    }

    void bindable_object::on_property_changing(std::string_view name)
    {
        property_changing.raise(name);
    }

    bindable_object::context &bindable_object::get_or_create_context(const bindable_property &property) const
    {
        if (auto existing = properties_.find(&property); existing != properties_.end())
        {
            return existing->second;
        }
        context ctx;
        ctx.property = &property;
        ctx.values.set(setter_specificity::default_value, property.get_default_value(*this));
        auto inserted = properties_.emplace(&property, std::move(ctx)).first;
        return inserted->second;
    }

    bindable_object::context *bindable_object::get_context(const bindable_property &property) const
    {
        auto existing = properties_.find(&property);
        return existing != properties_.end() ? &existing->second : nullptr;
    }

    void bindable_object::set_value_core(const bindable_property &property, std::any value,
                                         setter_specificity specificity)
    {
        if (const auto &validate = property.on_validate(); validate && !validate(*this, value))
        {
            return; // invalid value, ignore (C# logs a warning)
        }
        if (const auto &coerce = property.on_coerce(); coerce)
        {
            value = coerce(*this, value);
        }

        context &ctx = get_or_create_context(property);

        // Re-entrant set of the same property (e.g. from a change handler) is queued and applied
        // after the current set completes, mirroring C#'s DelayedSetters.
        if (ctx.is_being_set)
        {
            ctx.delayed.push(set_request{.property = &property, .value = std::move(value), .specificity = specificity});
            return;
        }

        ctx.is_being_set = true;
        set_value_actual(property, ctx, std::move(value), specificity, false);
        while (!ctx.delayed.empty())
        {
            set_request request = std::move(ctx.delayed.front());
            ctx.delayed.pop();
            set_value_actual(*request.property, ctx, std::move(request.value), request.specificity, false);
        }
        ctx.is_being_set = false;
    }

    void bindable_object::set_value_actual(const bindable_property &property, context &ctx, std::any value,
                                           setter_specificity specificity, bool silent)
    {
        auto const current = ctx.values.specificity_and_value();
        std::any const original = current.second;
        setter_specificity original_specificity = current.first;

        // If the current top value came from the handler, a non-handler set overrides it.
        if (specificity != setter_specificity::from_handler && original_specificity == setter_specificity::from_handler)
        {
            ctx.values.remove(setter_specificity::from_handler);
            original_specificity = ctx.values.specificity();
        }

        // A value below the current specificity is kept (silently) so it can be restored later.
        if (specificity < original_specificity)
        {
            ctx.values.set(specificity, std::move(value));
            return;
        }

        bool const same_value = property.values_equal(value, original);
        if (!silent && !same_value)
        {
            if (const auto &changing = property.on_changing(); changing)
            {
                changing(*this, original, value);
            }
            on_property_changing(property.name());
        }

        ctx.values.set(specificity, value);

        if (!silent && !same_value)
        {
            on_property_changed(property.name());
            if (const auto &changed = property.on_changed(); changed)
            {
                changed(*this, original, value);
            }
        }
    }

    void bindable_object::clear_value_core(const bindable_property &property, setter_specificity specificity)
    {
        context *ctx = get_context(property);
        if (ctx == nullptr)
        {
            return;
        }

        auto current = ctx->values.specificity_and_value();
        std::any const original = current.second;
        if (current.first == setter_specificity::from_handler)
        {
            ctx->values.remove(setter_specificity::from_handler);
        }

        std::any const new_value = ctx->values.cleared_value(specificity);
        bool const changed = !property.values_equal(original, new_value);
        if (changed)
        {
            if (const auto &changing = property.on_changing(); changing)
            {
                changing(*this, original, new_value);
            }
            on_property_changing(property.name());
        }

        ctx->values.remove(specificity);

        // CoerceValue can carry side effects that must still run on clear (C# does this).
        if (const auto &coerce = property.on_coerce(); coerce)
        {
            coerce(*this, new_value);
        }

        if (changed)
        {
            on_property_changed(property.name());
            if (const auto &changed_callback = property.on_changed(); changed_callback)
            {
                changed_callback(*this, original, new_value);
            }
        }
    }
} // namespace maui::core
