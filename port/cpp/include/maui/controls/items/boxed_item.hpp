#pragma once
// maui::controls::boxed_item  <=  the C# `object` item of the Items layer (ItemsSource elements,
// SelectedItem/SelectedItems entries, EmptyView/Header/Footer payloads).
//
// The port's reflection-free object stand-in for the items machinery: a typed box (shared_ptr<void> +
// type_tag — the same shape as bindable_object::binding_context_box, which it wraps so a boxed item
// can become a templated cell's BindingContext directly) plus the two behaviors the C# object model
// supplies through virtual dispatch, captured as plain function pointers when the static type is
// still known (boxed_item::of<T>):
//   - equals: C# object.Equals — value equality when T has operator== (strings/numbers/records),
//     reference (shared_ptr) identity otherwise. Two boxes of different types are never equal;
//     two boxes sharing one object always are.
//   - text:   C# object.ToString — string-convertible Ts render themselves, arithmetic Ts format via
//     std::to_string, anything else has no reflection-free display form and renders empty
//     (documented; the oracle's default cells show item.ToString()).
//
// of(std::shared_ptr<T>) shares the caller's object (reference semantics — selection compares the
// same instance the source holds); of(T) copies into a fresh shared_ptr (value semantics — selection
// then relies on T's operator==, exactly the C# string/value-type behavior).

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class boxed_item
    {
    public:
        // The null item (C# null).
        boxed_item() = default;

        // Box a copy of `value` (value semantics; see header).
        template <class T> [[nodiscard]] static boxed_item of(T value)
        {
            return of(std::make_shared<T>(std::move(value)));
        }

        // Box a shared object (reference semantics; a null pointer boxes the null item).
        template <class T> [[nodiscard]] static boxed_item of(std::shared_ptr<T> value)
        {
            boxed_item item;
            if (!value)
            {
                return item;
            }
            item.box_.value = std::static_pointer_cast<void>(value);
            item.box_.type = maui::core::type_tag::of<T>();
            item.box_.boxed = value;
            if constexpr (std::is_base_of_v<maui::core::bindable_object, T>)
            {
                item.box_.object = value;
            }
            if constexpr (std::equality_comparable<T>)
            {
                item.equals_ = [](const void* left, const void* right) {
                    return *static_cast<const T*>(left) == *static_cast<const T*>(right);
                };
            }
            if constexpr (std::is_convertible_v<T, std::string>)
            {
                item.text_ = [](const void* value_ptr) { return std::string{*static_cast<const T*>(value_ptr)}; };
            }
            else if constexpr (std::is_arithmetic_v<T>)
            {
                item.text_ = [](const void* value_ptr) { return std::to_string(*static_cast<const T*>(value_ptr)); };
            }
            return item;
        }

        [[nodiscard]] bool has_value() const
        {
            return static_cast<bool>(box_.value);
        }

        // The value as T (type-checked through the type_tag; null on mismatch or the null item).
        template <class T> [[nodiscard]] std::shared_ptr<T> as() const
        {
            if (!box_.value || box_.type != maui::core::type_tag::of<T>())
            {
                return nullptr;
            }
            return std::static_pointer_cast<T>(box_.value);
        }

        // C# object.Equals (see header). Null equals null; null never equals a value.
        [[nodiscard]] bool equals(const boxed_item& other) const
        {
            if (!box_.value && !other.box_.value)
            {
                return true;
            }
            if (!box_.value || !other.box_.value)
            {
                return false;
            }
            if (box_.value == other.box_.value)
            {
                return true; // the same object (reference equality)
            }
            if (box_.type == other.box_.type && equals_ != nullptr)
            {
                return equals_(box_.value.get(), other.box_.value.get());
            }
            return false;
        }
        friend bool operator==(const boxed_item& left, const boxed_item& right)
        {
            return left.equals(right);
        }

        // C# object.ToString stand-in (see header; empty for non-displayable types and the null item).
        [[nodiscard]] std::string text() const
        {
            return (box_.value && text_ != nullptr) ? text_(box_.value.get()) : std::string{};
        }

        // The item as a BindingContext box (templated content's context = the item) — the same shape
        // data_template_selector keys its recycle cache on.
        [[nodiscard]] const maui::core::bindable_object::binding_context_box& context_box() const
        {
            return box_;
        }

        // The item viewed as a bindable_object (an EmptyView/Header/Footer that IS a view is hosted
        // directly, like C#'s `EmptyView is View` checks); null when the boxed type is not one.
        [[nodiscard]] const std::shared_ptr<maui::core::bindable_object>& as_bindable() const
        {
            return box_.object;
        }

    private:
        maui::core::bindable_object::binding_context_box box_;
        bool (*equals_)(const void*, const void*) = nullptr;
        std::string (*text_)(const void*) = nullptr;
    };
} // namespace maui::controls
