#pragma once
// maui::core boxed-value helpers  <=  Microsoft.Maui.Controls.BindingExpressionHelper (TryConvert) +
//                                     System.Convert.ChangeType (the subset MAUI bindings rely on)
//
// The string-path binding engine (W1-02) moves values between properties of *different* static types
// through the port's boxed representation, std::any (the same box apply_setter already uses). C# does
// this with reflection + IConvertible; the port instead fixes the conversion lattice here, typed:
//   - exact type:            any holding T unboxes to T directly
//   - shared_ptr unwrap:     any holding shared_ptr<T> unboxes to T (the "object holds a boxed
//                            reference" analog); a null shared_ptr is the port's null
//   - null:                  an EMPTY std::any is the engine's null value. It unboxes only to a
//                            nullable T (a shared_ptr), mirroring C# TryConvert's "null converts to
//                            reference/nullable types only"
//   - numeric lattice:       any arithmetic source -> any arithmetic target (Convert.ChangeType)
//   - string -> number:      invariant parse via std::from_chars (charconv_compat on old libc++),
//                            with BindingExpressionHelper's two editing guards: a trailing decimal
//                            separator ("4.") and a "-0"-prefixed zero ("-0", "-0.0") do NOT convert
//                            to a floating target, so a user typing those isn't canonicalized mid-edit
//   - number -> string:      std::format("{}", value) (the invariant Convert.ToString analog)
// Anything else fails (returns nullopt) — the binding-failure path, never an exception.

#include <any>
#include <charconv>
#include <concepts>
#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "maui/detail/charconv_compat.hpp"

namespace maui::core
{
    namespace detail
    {
        template <class T> struct is_shared_ptr : std::false_type
        {
        };
        template <class U> struct is_shared_ptr<std::shared_ptr<U>> : std::true_type
        {
        };

        // The arithmetic types the numeric lattice probes (the Convert.ChangeType sources MAUI hits).
        template <class T, class Probe> bool probe_arithmetic(const std::any& value, std::optional<T>& out)
        {
            if (const auto* held = std::any_cast<Probe>(&value))
            {
                out = static_cast<T>(*held);
                return true;
            }
            return false;
        }

        template <class T> [[nodiscard]] std::optional<T> unbox_arithmetic(const std::any& value)
        {
            std::optional<T> out;
            if (probe_arithmetic<T, bool>(value, out) || probe_arithmetic<T, char>(value, out) ||
                probe_arithmetic<T, signed char>(value, out) || probe_arithmetic<T, unsigned char>(value, out) ||
                probe_arithmetic<T, short>(value, out) || probe_arithmetic<T, unsigned short>(value, out) ||
                probe_arithmetic<T, int>(value, out) || probe_arithmetic<T, unsigned int>(value, out) ||
                probe_arithmetic<T, long>(value, out) || probe_arithmetic<T, unsigned long>(value, out) ||
                probe_arithmetic<T, long long>(value, out) || probe_arithmetic<T, unsigned long long>(value, out) ||
                probe_arithmetic<T, float>(value, out) || probe_arithmetic<T, double>(value, out) ||
                probe_arithmetic<T, long double>(value, out))
            {
                return out;
            }
            return std::nullopt;
        }

        // string -> arithmetic, invariant culture, with the BindingExpressionHelper editing guards.
        template <class T> [[nodiscard]] std::optional<T> parse_arithmetic(std::string_view text)
        {
            if constexpr (std::floating_point<T>)
            {
                // "4." should not update a bound floating property (bugzilla 32871)...
                if (text.ends_with('.'))
                {
                    return std::nullopt;
                }
                T parsed{};
                const char* first = text.data();
                const char* last = first + text.size();
                auto [ptr, ec] = maui::detail::from_chars_general(first, last, parsed);
                if (ec != std::errc{} || ptr != last)
                {
                    return std::nullopt;
                }
                // ...and neither should "-0"/"-0.0" (the user will likely keep typing).
                if (text.starts_with("-0") && parsed == T{})
                {
                    return std::nullopt;
                }
                return parsed;
            }
            else
            {
                T parsed{};
                const char* first = text.data();
                const char* last = first + text.size();
                auto [ptr, ec] = std::from_chars(first, last, parsed);
                if (ec != std::errc{} || ptr != last)
                {
                    return std::nullopt;
                }
                return parsed;
            }
        }
    } // namespace detail

    [[nodiscard]] inline std::optional<std::string> boxed_to_string(const std::any& value);

    // Box a typed value into the engine's representation: a null shared_ptr boxes as the EMPTY any
    // (the engine's null), everything else as a copy of the value itself.
    template <class T> [[nodiscard]] std::any box_value(const T& value)
    {
        if constexpr (detail::is_shared_ptr<T>::value)
        {
            if (!value)
            {
                return {};
            }
        }
        return std::any{value};
    }

    // Unbox toward T through the conversion lattice documented above. nullopt = not convertible (the
    // C# TryConvert == false path).
    template <class T> [[nodiscard]] std::optional<T> try_unbox(const std::any& value)
    {
        if (!value.has_value())
        {
            // null: only a nullable T can absorb it.
            if constexpr (detail::is_shared_ptr<T>::value)
            {
                return T{};
            }
            else
            {
                return std::nullopt;
            }
        }
        if (const T* exact = std::any_cast<T>(&value))
        {
            return *exact;
        }
        // shared_ptr<T> unwrap (a value-like source boxed by reference — e.g. a string binding context).
        if constexpr (std::copy_constructible<T>)
        {
            if (const auto* shared = std::any_cast<std::shared_ptr<T>>(&value))
            {
                if (*shared)
                {
                    return **shared;
                }
                return std::nullopt;
            }
        }
        if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
        {
            if (auto numeric = detail::unbox_arithmetic<T>(value))
            {
                return numeric;
            }
            if (const auto* text = std::any_cast<std::string>(&value))
            {
                return detail::parse_arithmetic<T>(*text);
            }
            if (const auto* shared_text = std::any_cast<std::shared_ptr<std::string>>(&value))
            {
                if (*shared_text)
                {
                    return detail::parse_arithmetic<T>(**shared_text);
                }
            }
        }
        if constexpr (std::is_same_v<T, std::string>)
        {
            if (auto text = boxed_to_string(value))
            {
                return text;
            }
        }
        return std::nullopt;
    }

    // The value rendered as text (for StringFormat and number->string conversion): strings pass
    // through, arithmetic values render invariantly via std::format. nullopt for anything else.
    [[nodiscard]] inline std::optional<std::string> boxed_to_string(const std::any& value)
    {
        if (!value.has_value())
        {
            return std::nullopt;
        }
        if (const auto* text = std::any_cast<std::string>(&value))
        {
            return *text;
        }
        if (const auto* shared_text = std::any_cast<std::shared_ptr<std::string>>(&value))
        {
            return *shared_text ? std::optional<std::string>{**shared_text} : std::nullopt;
        }
        std::optional<std::string> out;
        const auto render = [&out, &value](auto probe) {
            using probe_t = decltype(probe);
            if (const auto* held = std::any_cast<probe_t>(&value))
            {
                out = std::format("{}", *held);
                return true;
            }
            return false;
        };
        if (render(bool{}) || render(char{}) || render(int{}) || render(unsigned{}) || render(long{}) ||
            render(static_cast<unsigned long>(0)) || render(static_cast<long long>(0)) ||
            render(static_cast<unsigned long long>(0)) || render(short{}) || render(static_cast<unsigned short>(0)) ||
            render(float{}) || render(double{}) || render(static_cast<long double>(0)))
        {
            return out;
        }
        return std::nullopt;
    }
} // namespace maui::core
