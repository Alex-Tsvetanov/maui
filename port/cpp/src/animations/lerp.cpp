// maui::animations::lerp — see include/maui/animations/lerp.hpp. Ported from
// src/Core/src/Animations/Lerp.cs + AnimationLerpingExtensions.cs.
#include "maui/animations/lerp.hpp"

#include <any>
#include <cstdint>
#include <typeindex>
#include <unordered_map>

#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::animations
{
    double lerp_value(double start, double end, double progress)
    {
        return ((end - start) * progress) + start;
    }

    float lerp_value(float start, float end, double progress)
    {
        return static_cast<float>((end - start) * progress) + start;
    }

    maui::graphics::color lerp_value(const maui::graphics::color& start, const maui::graphics::color& end,
                                     double progress)
    {
        return {lerp_value(start.red, end.red, progress), lerp_value(start.green, end.green, progress),
                lerp_value(start.blue, end.blue, progress), lerp_value(start.alpha, end.alpha, progress)};
    }

    maui::graphics::point lerp_value(const maui::graphics::point& start, const maui::graphics::point& end,
                                     double progress)
    {
        return {lerp_value(start.x, end.x, progress), lerp_value(start.y, end.y, progress)};
    }

    maui::graphics::size lerp_value(const maui::graphics::size& start, const maui::graphics::size& end, double progress)
    {
        return {lerp_value(start.width, end.width, progress), lerp_value(start.height, end.height, progress)};
    }

    maui::graphics::rect lerp_value(const maui::graphics::rect& start, const maui::graphics::rect& end, double progress)
    {
        return {lerp_value(start.x, end.x, progress), lerp_value(start.y, end.y, progress),
                lerp_value(start.width, end.width, progress), lerp_value(start.height, end.height, progress)};
    }

    maui::core::thickness lerp_value(const maui::core::thickness& start, const maui::core::thickness& end,
                                     double progress)
    {
        return {lerp_value(start.left, end.left, progress), lerp_value(start.top, end.top, progress),
                lerp_value(start.right, end.right, progress), lerp_value(start.bottom, end.bottom, progress)};
    }

    namespace
    {
        // A registry entry for an arithmetic T, matching the C# per-type entries (the cast pattern
        // `(T)((end - start) * progress) + start`).
        template <class T> lerp make_arithmetic_lerp()
        {
            return {.calculate = [](const std::any& start, const std::any& end, double progress) -> std::any {
                const T s = std::any_cast<T>(start);
                const T e = std::any_cast<T>(end);
                return static_cast<T>(static_cast<T>(static_cast<double>(e - s) * progress) + s);
            }};
        }

        // A registry entry that routes through the typed lerp_value overload for T.
        template <class T> lerp make_value_lerp()
        {
            return {.calculate = [](const std::any& start, const std::any& end, double progress) -> std::any {
                return lerp_value(std::any_cast<T>(start), std::any_cast<T>(end), progress);
            }};
        }

        const std::unordered_map<std::type_index, lerp>& lerps()
        {
            static const std::unordered_map<std::type_index, lerp> registry = [] {
                std::unordered_map<std::type_index, lerp> map;
                map.emplace(typeid(int), make_arithmetic_lerp<int>());
                map.emplace(typeid(short), make_arithmetic_lerp<short>());
                map.emplace(typeid(std::uint8_t), make_arithmetic_lerp<std::uint8_t>());
                map.emplace(typeid(long long), make_arithmetic_lerp<long long>());
                map.emplace(typeid(unsigned), make_arithmetic_lerp<unsigned>());
                map.emplace(typeid(float), make_value_lerp<float>());
                map.emplace(typeid(double), make_value_lerp<double>());
                map.emplace(typeid(bool), lerp{.calculate = [](const std::any& start, const std::any& end,
                                                               double progress) -> std::any {
                                return generic_lerp(std::any_cast<bool>(start), std::any_cast<bool>(end), progress);
                            }});
                map.emplace(typeid(maui::graphics::color), make_value_lerp<maui::graphics::color>());
                map.emplace(typeid(maui::graphics::point), make_value_lerp<maui::graphics::point>());
                map.emplace(typeid(maui::graphics::size), make_value_lerp<maui::graphics::size>());
                map.emplace(typeid(maui::graphics::rect), make_value_lerp<maui::graphics::rect>());
                map.emplace(typeid(maui::core::thickness), make_value_lerp<maui::core::thickness>());
                return map;
            }();
            return registry;
        }

        // The typeof(object) entry every unknown type falls back to in C# (the base-type walk always
        // terminates on it): toggle from start to end half-way through.
        const lerp& generic_fallback()
        {
            static const lerp instance{
                .calculate = [](const std::any& start, const std::any& end, double progress) -> std::any {
                    return progress < 0.5 ? start : end;
                }};
            return instance;
        }
    } // namespace

    const lerp& lerp::get(std::type_index type)
    {
        const auto found = lerps().find(type);
        if (found != lerps().end())
        {
            return found->second;
        }
        return generic_fallback();
    }
} // namespace maui::animations
