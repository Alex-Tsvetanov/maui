// maui::controls::multi_binding — combine inner bindings through a multi converter (multi_binding.hpp).
// Ported from src/Controls/src/Core/MultiBinding.cs: the hidden ProxyElement with one dynamically
// created bindable property per inner binding, the `_applying` re-entrancy guard, the ConvertBack
// distribution rules, and the ManualValueSetter specificity for recomputed pushes (the C# comment:
// it keeps TwoWay bindings updating after ConvertBack).
#include "maui/controls/bindings/multi_binding.hpp"

#include <algorithm>
#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/bindings/binding_base.hpp"
#include "maui/controls/bindings/binding_diagnostics.hpp"
#include "maui/controls/bindings/i_multi_value_converter.hpp"
#include "maui/controls/element.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/property.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    namespace
    {
        // Equality over the boxed slot values: empty==empty, exact-type scalar compares, otherwise
        // "changed" (C# Equals over object — the port can only compare what it can probe).
        template <class T> [[nodiscard]] bool probe_equal(const std::any& left, const std::any& right, bool& equal)
        {
            const T* l = std::any_cast<T>(&left);
            if (l == nullptr)
            {
                return false;
            }
            const T* r = std::any_cast<T>(&right);
            equal = r != nullptr && *l == *r;
            return true;
        }

        [[nodiscard]] bool boxed_equal(const std::any& left, const std::any& right)
        {
            if (!left.has_value() || !right.has_value())
            {
                return left.has_value() == right.has_value();
            }
            bool equal = false;
            if (probe_equal<std::string>(left, right, equal) || probe_equal<bool>(left, right, equal) ||
                probe_equal<int>(left, right, equal) || probe_equal<unsigned int>(left, right, equal) ||
                probe_equal<long>(left, right, equal) || probe_equal<unsigned long>(left, right, equal) ||
                probe_equal<long long>(left, right, equal) || probe_equal<unsigned long long>(left, right, equal) ||
                probe_equal<double>(left, right, equal) || probe_equal<float>(left, right, equal) ||
                probe_equal<do_nothing_value>(left, right, equal) || probe_equal<unset_value>(left, right, equal))
            {
                return equal;
            }
            return false;
        }

        // The best type_tag the port can give ConvertBack for a slot's current value (C# uses
        // values[i]?.GetType() ?? typeof(object); 'object' becomes type_tag::of<void>()).
        [[nodiscard]] maui::core::type_tag boxed_type(const std::any& value)
        {
            if (std::any_cast<std::string>(&value) != nullptr)
            {
                return maui::core::type_tag::of<std::string>();
            }
            if (std::any_cast<double>(&value) != nullptr)
            {
                return maui::core::type_tag::of<double>();
            }
            if (std::any_cast<int>(&value) != nullptr)
            {
                return maui::core::type_tag::of<int>();
            }
            if (std::any_cast<bool>(&value) != nullptr)
            {
                return maui::core::type_tag::of<bool>();
            }
            return maui::core::type_tag::of<void>();
        }

        struct applying_guard
        {
            explicit applying_guard(bool& flag) : flag_(flag)
            {
                flag_ = true;
            }
            applying_guard(const applying_guard&) = delete;
            applying_guard(applying_guard&&) = delete;
            applying_guard& operator=(const applying_guard&) = delete;
            applying_guard& operator=(applying_guard&&) = delete;
            ~applying_guard()
            {
                flag_ = false;
            }

        private:
            bool& flag_;
        };
    } // namespace

    // The hidden ProxyElement: one boxed-value property per inner binding ("mb-proxy{i}"), created
    // with the multi-binding's realized mode as DefaultBindingMode (so inner Default bindings realize
    // to it) and a changed callback that recomputes the combined value.
    class multi_binding::proxy_element final : public element
    {
    public:
        struct boxed_slot
        {
            std::any value;
            // custom_boxed (boxed_value.hpp): a slot absorbs and yields ANY boxed value verbatim, so
            // the inner bindings' typed pushes flow through it untouched.
            [[nodiscard]] std::any to_boxed() const
            {
                return value;
            }
            [[nodiscard]] static boxed_slot from_boxed(const std::any& boxed)
            {
                return boxed_slot{boxed};
            }
            bool operator==(const boxed_slot& other) const
            {
                return boxed_equal(value, other.value);
            }
        };

        struct slot
        {
            slot(proxy_element& owner, std::string slot_name, maui::core::binding_mode default_mode,
                 std::function<void()> on_changed)
                : name(std::move(slot_name)),
                  descriptor(name, boxed_slot{},
                             {.property_changed = [callback = std::move(on_changed)](maui::core::bindable_object&,
                                                                                     const boxed_slot&,
                                                                                     const boxed_slot&) { callback(); },
                              .default_binding_mode = default_mode}),
                  value(owner, descriptor)
            {
            }
            std::string name; // declared first: the descriptor borrows it as the property name
            maui::core::bindable_property<boxed_slot> descriptor;
            maui::core::property<boxed_slot> value;
        };

        void add_slot(std::string name, maui::core::binding_mode default_mode, std::function<void()> on_changed)
        {
            slots_.push_back(std::make_unique<slot>(*this, std::move(name), default_mode, std::move(on_changed)));
        }
        [[nodiscard]] const std::vector<std::unique_ptr<slot>>& slots() const
        {
            return slots_;
        }
        void set_slot_value(std::size_t index, std::any value)
        {
            // C#: _proxyObject.SetValue(_bpProxies[i], values[i]) — a plain (manual) set.
            slots_[index]->value.set(boxed_slot{std::move(value)}, maui::core::setter_specificity::manual_value_setter);
        }

    private:
        std::vector<std::unique_ptr<slot>> slots_;
    };

    multi_binding::multi_binding() = default;

    multi_binding::~multi_binding() = default;

    void multi_binding::set_converter(std::shared_ptr<i_multi_value_converter> value)
    {
        throw_if_applied();
        converter_ = std::move(value);
    }

    void multi_binding::set_converter_parameter(std::any value)
    {
        throw_if_applied();
        converter_parameter_ = std::move(value);
    }

    void multi_binding::add_binding(std::shared_ptr<binding_base> value)
    {
        throw_if_applied();
        if (!value)
        {
            throw std::invalid_argument("multi_binding: binding is null");
        }
        bindings_.push_back(std::move(value));
    }

    void multi_binding::set_bindings(std::vector<std::shared_ptr<binding_base>> value)
    {
        throw_if_applied();
        bindings_ = std::move(value);
    }

    std::shared_ptr<binding_base> multi_binding::clone() const
    {
        auto clone = std::make_shared<multi_binding>();
        clone->converter_ = converter_;
        clone->converter_parameter_ = converter_parameter_;
        for (const std::shared_ptr<binding_base>& inner : bindings_)
        {
            clone->bindings_.push_back(inner->clone());
        }
        clone->set_mode(mode());
        if (!string_format().empty())
        {
            clone->set_string_format(string_format());
        }
        clone->set_target_null_value(target_null_value());
        clone->set_fallback_value(fallback_value());
        return clone;
    }

    void multi_binding::apply(const maui::core::bindable_object::binding_context_box& context,
                              maui::core::bindable_object& target, std::string_view target_property,
                              bool from_binding_context_changed, maui::core::setter_specificity specificity)
    {
        if (bindings_.empty())
        {
            throw std::runtime_error("multi_binding: Bindings is empty");
        }
        if (!converter_ && string_format().empty())
        {
            throw std::runtime_error("multi_binding: cannot apply because both Converter and StringFormat are null");
        }
        binding_base::apply(context, target, target_property, from_binding_context_changed, specificity);

        if (target_ != &target)
        {
            target_ = &target;
            target_alive_ = target.weak_token();
            target_property_ = std::string{target_property};
            specificity_ = specificity;

            if (!proxy_)
            {
                proxy_ = std::make_unique<proxy_element>();
                const applying_guard guard{applying_};
                // C#: the proxy properties' DefaultBindingMode is the multi's mode resolved against
                // the target property (no read-only downgrade here, exactly like MultiBinding.Apply).
                maui::core::binding_mode default_mode = mode();
                if (default_mode == maui::core::binding_mode::default_mode)
                {
                    default_mode = target.property_default_binding_mode(target_property)
                                       .value_or(maui::core::binding_mode::one_way);
                }
                // C# parents the proxy under the target and hands inner bindings the target element
                // as RelativeSourceTargetOverride; a NESTED multi receives the outer's override here
                // (its own `target` is the outer proxy), so relative walks start at the REAL element.
                element* target_element = relative_source_target_override();
                if (target_element == nullptr)
                {
                    target_element = dynamic_cast<element*>(&target);
                }
                for (std::size_t i = 0; i < bindings_.size(); ++i)
                {
                    bindings_[i]->set_relative_source_target_override(target_element);
                    const std::string slot_name = "mb-proxy" + std::to_string(i);
                    proxy_->add_slot(slot_name, default_mode, [this] { on_proxy_changed(); });
                    proxy_->set_binding(slot_name, bindings_[i]);
                }
            }
        }
        proxy_->set_binding_context_box(context);

        if (realized_mode() == maui::core::binding_mode::one_way_to_source)
        {
            return;
        }
        push_to_target(specificity);
    }

    void multi_binding::apply(bool from_target)
    {
        if (applying_)
        {
            return;
        }
        binding_base::apply(from_target);
        if (target_ == nullptr || target_alive_.expired() || !proxy_)
        {
            return;
        }
        const maui::core::binding_mode realized = realized_mode();
        if (realized == maui::core::binding_mode::one_time)
        {
            return;
        }
        if (from_target && realized == maui::core::binding_mode::one_way)
        {
            return;
        }
        if (!from_target && realized == maui::core::binding_mode::one_way_to_source)
        {
            return;
        }
        if (!from_target)
        {
            // "ManualValueSetter specificity ensures TwoWay bindings continue updating after
            // ConvertBack" (the C# comment, verbatim).
            push_to_target(maui::core::setter_specificity::manual_value_setter);
        }
        else
        {
            apply_back_to_proxies();
        }
    }

    void multi_binding::unapply(bool from_binding_context_changed)
    {
        if (!from_binding_context_changed)
        {
            if (proxy_)
            {
                for (const auto& slot : proxy_->slots())
                {
                    proxy_->remove_binding(slot->name);
                }
            }
            proxy_.reset();
            target_ = nullptr;
            target_alive_.reset();
        }
        binding_base::unapply(from_binding_context_changed);
    }

    maui::core::binding_mode multi_binding::realized_mode() const
    {
        maui::core::binding_mode realized = mode();
        if (realized == maui::core::binding_mode::default_mode && target_ != nullptr)
        {
            realized =
                target_->property_default_binding_mode(target_property_).value_or(maui::core::binding_mode::one_way);
        }
        if (realized == maui::core::binding_mode::default_mode)
        {
            realized = maui::core::binding_mode::one_way;
        }
        if (realized == maui::core::binding_mode::two_way && target_ != nullptr &&
            target_->property_is_read_only(target_property_).value_or(false))
        {
            realized = maui::core::binding_mode::one_way_to_source;
        }
        return realized;
    }

    std::any multi_binding::compute_source_value(maui::core::type_tag target_type) const
    {
        std::vector<std::any> values;
        values.reserve(proxy_->slots().size());
        for (const auto& slot : proxy_->slots())
        {
            values.push_back(slot->value.get().value);
        }

        std::any value;
        if (converter_)
        {
            value = converter_->convert(values, target_type, converter_parameter_);
        }
        else if (!string_format().empty())
        {
            // Converter-less composite formatting ("{0} - {1} - {2}") returns directly (no re-format).
            if (auto formatted = try_format(string_format(), values))
            {
                return std::any{std::move(*formatted)};
            }
            value = std::any{std::move(values)};
        }
        if (is_do_nothing(value))
        {
            return value; // never format/replace the sentinel
        }
        if (is_unset_value(value))
        {
            return fallback_value();
        }
        return binding_base::get_source_value(std::move(value), target_type);
    }

    void multi_binding::push_to_target(maui::core::setter_specificity specificity)
    {
        const maui::core::type_tag target_type =
            target_->property_type(target_property_).value_or(maui::core::type_tag::of<void>());
        const std::any value = compute_source_value(target_type);
        if (is_do_nothing(value))
        {
            return;
        }
        const applying_guard guard{applying_};
        if (!target_->try_set_value(target_property_, value, specificity))
        {
            send_binding_failure("multi_binding: value cannot be converted to the type of target property '" +
                                 target_property_ + "'");
        }
    }

    void multi_binding::apply_back_to_proxies()
    {
        const applying_guard guard{applying_};
        if (!converter_)
        {
            return; // C#: GetTargetValue's base answer isn't an object[] — nothing to distribute
        }
        const std::any target_value = target_->try_get_value(target_property_).value_or(std::any{});
        std::vector<maui::core::type_tag> types;
        types.reserve(proxy_->slots().size());
        for (const auto& slot : proxy_->slots())
        {
            types.push_back(boxed_type(slot->value.get().value));
        }
        const std::optional<std::vector<std::any>> values =
            converter_->convert_back(target_value, types, converter_parameter_);
        if (!values)
        {
            return; // converter failed — no source updates at all
        }
        const std::size_t count = std::min(proxy_->slots().size(), values->size());
        for (std::size_t i = 0; i < count; ++i)
        {
            const std::any& value = (*values)[i];
            if (is_do_nothing(value) || is_unset_value(value))
            {
                continue; // skip this source binding
            }
            proxy_->set_slot_value(i, value);
        }
    }

    void multi_binding::on_proxy_changed()
    {
        if (!applying_)
        {
            apply(/*from_target=*/false);
        }
    }
} // namespace maui::controls
