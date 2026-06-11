// maui::controls::binding_base — mode/format/null-fallback policy + the target watch
// (binding_base.hpp). Ported from src/Controls/src/Core/BindingBase.cs (+ the binding.Apply(true)
// trigger inside BindableObject.SetValueActual, which the port hosts here as the target watch).
#include "maui/controls/bindings/binding_base.hpp"

#include <any>
#include <cctype>
#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/boxed_value.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    binding_base::~binding_base()
    {
        unwatch_target();
    }

    void binding_base::set_mode(maui::core::binding_mode value)
    {
        throw_if_applied();
        mode_ = value;
    }

    void binding_base::set_string_format(std::string value)
    {
        throw_if_applied();
        string_format_ = std::move(value);
    }

    void binding_base::set_target_null_value(std::any value)
    {
        throw_if_applied();
        target_null_value_ = std::move(value);
    }

    void binding_base::set_fallback_value(std::any value)
    {
        throw_if_applied();
        fallback_value_ = std::move(value);
    }

    void binding_base::apply(const maui::core::bindable_object::binding_context_box& context,
                             maui::core::bindable_object& target, std::string_view target_property,
                             bool from_binding_context_changed, maui::core::setter_specificity specificity)
    {
        (void)context;
        (void)from_binding_context_changed;
        (void)specificity;
        is_applied_ = true;
        // The watch must be live BEFORE the subclass pushes the first value: C# registers the binding
        // in context.Bindings before calling Apply, so even the initial push triggers Apply(true).
        watch_target(target, target_property);
    }

    void binding_base::apply(bool from_target)
    {
        (void)from_target;
        is_applied_ = true;
    }

    void binding_base::unapply(bool from_binding_context_changed)
    {
        is_applied_ = false;
        if (!from_binding_context_changed)
        {
            unwatch_target(); // a context-change reapply keeps watching the same target property
        }
    }

    std::any binding_base::get_source_value(std::any value, maui::core::type_tag target_type) const
    {
        (void)target_type;
        if (!value.has_value() && target_null_value_.has_value())
        {
            return target_null_value_;
        }
        if (!string_format_.empty())
        {
            const std::span<const std::any> args{&value, 1};
            if (auto formatted = try_format(string_format_, args))
            {
                return std::any{std::move(*formatted)};
            }
        }
        return value;
    }

    std::any binding_base::get_target_value(std::any value, maui::core::type_tag source_type) const
    {
        (void)source_type;
        return value;
    }

    void binding_base::set_relative_source_target_override(element* value)
    {
        relative_override_ = value;
        if (value != nullptr)
        {
            // element derives bindable_object, whose weak_token tracks its lifetime (§8).
            relative_override_alive_ = value->weak_token();
        }
        else
        {
            relative_override_alive_.reset();
        }
    }

    element* binding_base::relative_source_target_override() const
    {
        if (relative_override_ != nullptr && !relative_override_alive_.expired())
        {
            return relative_override_;
        }
        return nullptr;
    }

    std::optional<std::string> binding_base::try_format(std::string_view format, std::span<const std::any> args)
    {
        std::string out;
        out.reserve(format.size());
        std::size_t i = 0;
        while (i < format.size())
        {
            const char c = format[i];
            if (c == '{')
            {
                if (i + 1 < format.size() && format[i + 1] == '{')
                {
                    out.push_back('{');
                    i += 2;
                    continue;
                }
                const std::size_t close = format.find('}', i + 1);
                if (close == std::string_view::npos)
                {
                    return std::nullopt; // unclosed placeholder — C# FormatException
                }
                std::string_view body = format.substr(i + 1, close - i - 1);
                std::string_view spec;
                if (const std::size_t colon = body.find(':'); colon != std::string_view::npos)
                {
                    spec = body.substr(colon + 1);
                    body = body.substr(0, colon);
                }
                if (body.empty())
                {
                    return std::nullopt;
                }
                std::size_t index = 0;
                for (const char digit : body)
                {
                    if (std::isdigit(static_cast<unsigned char>(digit)) == 0)
                    {
                        return std::nullopt;
                    }
                    index = (index * 10) + static_cast<std::size_t>(digit - '0');
                }
                if (index >= args.size())
                {
                    return std::nullopt; // C# FormatException: index out of range
                }
                std::string rendered = maui::core::boxed_to_string(args[index]).value_or(std::string{});
                // The all-'0' zero-pad spec ("{0:000}"): pad an integral rendering to the spec width.
                if (!spec.empty() && spec.find_first_not_of('0') == std::string_view::npos)
                {
                    const bool negative = !rendered.empty() && rendered.front() == '-';
                    std::string digits = negative ? rendered.substr(1) : rendered;
                    if (!digits.empty() && digits.find_first_not_of("0123456789") == std::string::npos)
                    {
                        while (digits.size() < spec.size())
                        {
                            digits.insert(digits.begin(), '0');
                        }
                        rendered = negative ? "-" + digits : digits;
                    }
                }
                out += rendered;
                i = close + 1;
                continue;
            }
            if (c == '}')
            {
                if (i + 1 < format.size() && format[i + 1] == '}')
                {
                    out.push_back('}');
                    i += 2;
                    continue;
                }
                return std::nullopt;
            }
            out.push_back(c);
            ++i;
        }
        return out;
    }

    void binding_base::throw_if_applied() const
    {
        if (is_applied_)
        {
            throw std::runtime_error("binding_base: cannot change a binding while it's applied");
        }
    }

    void binding_base::watch_target(maui::core::bindable_object& target, std::string_view target_property)
    {
        if (watched_target_ == &target && watched_property_ == target_property && !watched_target_alive_.expired())
        {
            return; // already watching this property
        }
        unwatch_target();
        watched_target_ = &target;
        watched_target_alive_ = target.weak_token();
        watched_property_ = std::string{target_property};
        watch_token_ = target.property_changed.connect([this](std::string_view name) {
            if (name != watched_property_)
            {
                return;
            }
            // BindableObject._applying: a target change made WHILE a from-target apply is running
            // (e.g. the formatted forward push) must not re-trigger the from-target flow.
            if (applying_from_target_)
            {
                return;
            }
            applying_from_target_ = true;
            apply(/*from_target=*/true);
            applying_from_target_ = false;
        });
    }

    void binding_base::unwatch_target()
    {
        if (watched_target_ != nullptr && !watched_target_alive_.expired())
        {
            watched_target_->property_changed.disconnect(watch_token_);
        }
        watched_target_ = nullptr;
        watched_target_alive_.reset();
        watched_property_.clear();
        watch_token_ = 0;
    }
} // namespace maui::controls
