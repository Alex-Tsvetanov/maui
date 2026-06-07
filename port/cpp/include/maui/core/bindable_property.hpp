#pragma once
// maui::core::bindable_property  <=  Microsoft.Maui.Controls.BindableProperty
//
// The (type-erased) descriptor for a bindable property: name, default value, and the optional
// callbacks (property-changed/changing, coerce, validate, default-value creator). Ported from
// src/Controls/src/Core/BindableProperty.cs. Like the C# original, the descriptor is type-erased
// (values flow as boxed std::any); the type parameter only appears at create<T>() time, which bakes
// in a T-specific equality function and adapts the typed callbacks to the boxed signatures.
//
// A property is identity-keyed by its address in bindable_object's value store, so it is non-copyable
// and non-movable — declare one as a `static const` (or a class member) and reference it. create<T>()
// returns a prvalue, so `static const bindable_property p = bindable_property::create<T>(...)` works
// via guaranteed copy elision despite the type being immovable.
//
// M1 scope: name/default/read-only + the five callbacks. Binding-specific bits (BindingMode,
// TryConvert, ReturnType, attached/read-only keys, dependencies) are deferred to M5.

#include <any>
#include <functional>
#include <string>
#include <utility>

namespace maui::core
{
    class bindable_object;

    class bindable_property
    {
    public:
        // Type-erased callback signatures (boxed std::any values), mirroring the C# delegates. The
        // default-value creator takes a const owner (it runs lazily from the const get_value path).
        using changed_delegate = std::function<void(bindable_object &, const std::any &, const std::any &)>;
        using changing_delegate = std::function<void(bindable_object &, const std::any &, const std::any &)>;
        using coerce_delegate = std::function<std::any(bindable_object &, const std::any &)>;
        using validate_delegate = std::function<bool(bindable_object &, const std::any &)>;
        using default_value_creator_delegate = std::function<std::any(const bindable_object &)>;
        using equality = bool (*)(const std::any &, const std::any &);

        // Typed creation options (the C# Create() optional parameters).
        template <class T> struct options
        {
            std::function<void(bindable_object &, const T &, const T &)> property_changed;
            std::function<void(bindable_object &, const T &, const T &)> property_changing;
            std::function<T(bindable_object &, const T &)> coerce_value;
            std::function<bool(bindable_object &, const T &)> validate_value;
            std::function<T(const bindable_object &)> default_value_creator;
            bool is_read_only = false;
        };

        template <class T>
        static bindable_property create(std::string name, T default_value = T{}, options<T> opts = {});

        bindable_property(const bindable_property &) = delete;
        bindable_property(bindable_property &&) = delete;
        bindable_property &operator=(const bindable_property &) = delete;
        bindable_property &operator=(bindable_property &&) = delete;
        ~bindable_property() = default;

        [[nodiscard]] const std::string &name() const;
        [[nodiscard]] bool is_read_only() const;
        [[nodiscard]] const std::any &default_value() const;
        [[nodiscard]] bool has_default_value_creator() const;
        // Default value for this owner: the creator's result if set, otherwise the static default.
        [[nodiscard]] std::any get_default_value(const bindable_object &owner) const;
        // Type-aware equality of two boxed values (both empty == equal; one empty != the other).
        [[nodiscard]] bool values_equal(const std::any &a, const std::any &b) const;

        [[nodiscard]] const changed_delegate &on_changed() const;
        [[nodiscard]] const changing_delegate &on_changing() const;
        [[nodiscard]] const coerce_delegate &on_coerce() const;
        [[nodiscard]] const validate_delegate &on_validate() const;

    private:
        bindable_property(std::string name, std::any default_value, equality equals, changed_delegate changed,
                          changing_delegate changing, coerce_delegate coerce, validate_delegate validate,
                          default_value_creator_delegate creator, bool read_only);

        std::string name_;
        std::any default_value_;
        equality equals_;
        changed_delegate changed_;
        changing_delegate changing_;
        coerce_delegate coerce_;
        validate_delegate validate_;
        default_value_creator_delegate default_value_creator_;
        bool is_read_only_;
    };

    template <class T> bindable_property bindable_property::create(std::string name, T default_value, options<T> opts)
    {
        equality const equals = [](const std::any &a, const std::any &b) {
            return std::any_cast<const T &>(a) == std::any_cast<const T &>(b);
        };

        changed_delegate changed;
        if (opts.property_changed)
        {
            changed = [callback = std::move(opts.property_changed)](bindable_object &owner, const std::any &old_value,
                                                                    const std::any &new_value) {
                callback(owner, std::any_cast<const T &>(old_value), std::any_cast<const T &>(new_value));
            };
        }
        changing_delegate changing;
        if (opts.property_changing)
        {
            changing = [callback = std::move(opts.property_changing)](bindable_object &owner, const std::any &old_value,
                                                                      const std::any &new_value) {
                callback(owner, std::any_cast<const T &>(old_value), std::any_cast<const T &>(new_value));
            };
        }
        coerce_delegate coerce;
        if (opts.coerce_value)
        {
            coerce = [callback = std::move(opts.coerce_value)](bindable_object &owner,
                                                               const std::any &value) -> std::any {
                return std::any(callback(owner, std::any_cast<const T &>(value)));
            };
        }
        validate_delegate validate;
        if (opts.validate_value)
        {
            validate = [callback = std::move(opts.validate_value)](bindable_object &owner,
                                                                   const std::any &value) -> bool {
                return callback(owner, std::any_cast<const T &>(value));
            };
        }
        default_value_creator_delegate creator;
        if (opts.default_value_creator)
        {
            creator = [callback = std::move(opts.default_value_creator)](const bindable_object &owner) -> std::any {
                return std::any(callback(owner));
            };
        }

        return bindable_property(std::move(name), std::any(std::move(default_value)), equals, std::move(changed),
                                 std::move(changing), std::move(coerce), std::move(validate), std::move(creator),
                                 opts.is_read_only);
    }
} // namespace maui::core
