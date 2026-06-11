#pragma once
// maui::animations::lerp        <=  Microsoft.Maui.Animations.Lerp
// maui::animations::lerp_value  <=  Microsoft.Maui.Animations.AnimationLerpingExtensions.Lerp
//
// The linear-interpolation family backing lerping_animation. Two tightly-coupled pieces share this
// header (PROFILE §3 cluster rule), mirroring the C# pair Lerp.cs + AnimationLerpingExtensions.cs:
//   - the TYPED lerp free functions (C# extension methods -> free functions per PROFILE §5). They are
//     named lerp_value because a free function named `lerp` cannot coexist with the class `lerp` in
//     the same namespace (and would shadow std::lerp);
//   - the type-erased `lerp` registry lerping_animation resolves against. C# keys the registry by
//     System.Type and boxes values as object; the port keys by std::type_index (the values arrive as
//     std::any, whose dynamic type is only reachable through type_info — this is a boundary that
//     inherently needs erasure, PROFILE §7) and carries values as std::any.
//
// get(type) DEVIATION: C# GetLerp walks the BaseType chain so any unknown type lands on the
// typeof(object) entry (the half-way toggle GenericLerp). C++ has no base-type walk; get(type)
// returns the registered lerp or that same generic toggle fallback directly — the identical result.

#include <any>
#include <functional>
#include <typeindex>

#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::animations
{
    // ---- AnimationLerpingExtensions (typed) ----
    [[nodiscard]] double lerp_value(double start, double end, double progress);
    [[nodiscard]] float lerp_value(float start, float end, double progress);
    [[nodiscard]] maui::graphics::color lerp_value(const maui::graphics::color& start, const maui::graphics::color& end,
                                                   double progress);
    [[nodiscard]] maui::graphics::point lerp_value(const maui::graphics::point& start, const maui::graphics::point& end,
                                                   double progress);
    [[nodiscard]] maui::graphics::size lerp_value(const maui::graphics::size& start, const maui::graphics::size& end,
                                                  double progress);
    [[nodiscard]] maui::graphics::rect lerp_value(const maui::graphics::rect& start, const maui::graphics::rect& end,
                                                  double progress);
    [[nodiscard]] maui::core::thickness lerp_value(const maui::core::thickness& start, const maui::core::thickness& end,
                                                   double progress);

    // C# GenericLerp: toggle from start to end at the threshold (used for non-interpolatable types).
    template <class T>
    [[nodiscard]] T generic_lerp(const T& start, const T& end, double progress, double toggle_threshold = 0.5)
    {
        return progress < toggle_threshold ? start : end;
    }

    // ---- the registry (C# Lerp + Lerp.GetLerp) ----
    class lerp
    {
    public:
        // C# Lerp.LerpDelegate(object start, object end, double progress) -> object.
        using calculate_fn = std::function<std::any(const std::any& start, const std::any& end, double progress)>;

        // C#'s settable `Calculate` property — public, like the C# object-initializer usage.
        calculate_fn calculate;

        // C# Lerp.GetLerp(Type): the lerp registered for `type` (int/float/double/long/bool/unsigned,
        // color/point/size/rect/thickness are built in), else the generic half-way toggle fallback.
        [[nodiscard]] static const lerp& get(std::type_index type);
    };
} // namespace maui::animations
