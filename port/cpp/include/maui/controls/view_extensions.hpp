#pragma once
// maui::controls view extensions  <=  Microsoft.Maui.Controls.ViewExtensions (the animation surface)
//
// The animatable property helpers over maui::controls::view<…>: fade_to / scale_to (+x/y) /
// rotate_to (+x/y, relative) / translate_to / layout_to and cancel_animations. Ported from
// src/Controls/src/Core/ViewExtensions.cs. C#'s `Task<bool>` completion becomes the port's async
// stand-in — a completion CALLBACK (cf. the image loader's apply_callback): it is invoked exactly
// once with `canceled` (true when the animation was aborted by cancel_animations / a same-name
// restart, or force-finished because the system disabled animations; false when it ran out
// naturally). The C# *Async suffixes drop with the Tasks; the obsolete synonyms are not ported.
//
// The functions are templates over the concrete view<ViewInterface> (the transform setters live on
// the view template, not on i_view); each delegates to the non-template detail::animate_to core.
// LIFETIME: C# guards the update closure with a WeakReference. The port instead anchors every
// running animation in the element itself (element::animation_state) — destroying the view
// deterministically detaches its animations from the manager, so the captured view reference can
// never be invoked afterwards (the completion callback is then dropped unfired; documented
// deviation, see element_animations.hpp).

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

#include "maui/animations/easing.hpp"
#include "maui/controls/animation.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::controls
{
    // The Task<bool> stand-in: completion(canceled).
    using animation_completion = maui::core::move_only_function<void(bool)>;

    namespace detail
    {
        // C# ViewExtensions.AnimateToAsync: animate update(target, value) across [start, end] under
        // the animation handle `name`. Defined in view_extensions.cpp.
        void animate_to(element& target, double start, double end, const char* name,
                        maui::core::move_only_function<void(element&, double)> update, std::uint32_t length,
                        std::optional<maui::animations::easing> easing_function, animation_completion on_finished);
    } // namespace detail

    // C# CancelAnimations: abort every ViewExtensions animation on the element.
    void cancel_animations(element& target);

    // C# FadeToAsync: animate Opacity to `opacity`.
    template <class ViewInterface>
    void fade_to(view<ViewInterface>& target, double opacity, std::uint32_t length = 250,
                 std::optional<maui::animations::easing> easing_function = {}, animation_completion on_finished = {})
    {
        detail::animate_to(
            target, target.opacity(), opacity, "fade_to",
            [](element& e, double value) { static_cast<view<ViewInterface>&>(e).set_opacity(value); }, length,
            std::move(easing_function), std::move(on_finished));
    }

    // C# RotateToAsync: animate Rotation to `rotation`.
    template <class ViewInterface>
    void rotate_to(view<ViewInterface>& target, double rotation, std::uint32_t length = 250,
                   std::optional<maui::animations::easing> easing_function = {}, animation_completion on_finished = {})
    {
        detail::animate_to(
            target, target.rotation(), rotation, "rotate_to",
            [](element& e, double value) { static_cast<view<ViewInterface>&>(e).set_rotation(value); }, length,
            std::move(easing_function), std::move(on_finished));
    }

    // C# RelRotateToAsync: rotate BY drotation from the current rotation.
    template <class ViewInterface>
    void rel_rotate_to(view<ViewInterface>& target, double drotation, std::uint32_t length = 250,
                       std::optional<maui::animations::easing> easing_function = {},
                       animation_completion on_finished = {})
    {
        rotate_to(target, target.rotation() + drotation, length, std::move(easing_function), std::move(on_finished));
    }

    // C# RotateXToAsync: animate RotationX to `rotation`.
    template <class ViewInterface>
    void rotate_x_to(view<ViewInterface>& target, double rotation, std::uint32_t length = 250,
                     std::optional<maui::animations::easing> easing_function = {},
                     animation_completion on_finished = {})
    {
        detail::animate_to(
            target, target.rotation_x(), rotation, "rotate_x_to",
            [](element& e, double value) { static_cast<view<ViewInterface>&>(e).set_rotation_x(value); }, length,
            std::move(easing_function), std::move(on_finished));
    }

    // C# RotateYToAsync: animate RotationY to `rotation`.
    template <class ViewInterface>
    void rotate_y_to(view<ViewInterface>& target, double rotation, std::uint32_t length = 250,
                     std::optional<maui::animations::easing> easing_function = {},
                     animation_completion on_finished = {})
    {
        detail::animate_to(
            target, target.rotation_y(), rotation, "rotate_y_to",
            [](element& e, double value) { static_cast<view<ViewInterface>&>(e).set_rotation_y(value); }, length,
            std::move(easing_function), std::move(on_finished));
    }

    // C# ScaleToAsync: animate Scale to `scale`.
    template <class ViewInterface>
    void scale_to(view<ViewInterface>& target, double scale, std::uint32_t length = 250,
                  std::optional<maui::animations::easing> easing_function = {}, animation_completion on_finished = {})
    {
        detail::animate_to(
            target, target.scale(), scale, "scale_to",
            [](element& e, double value) { static_cast<view<ViewInterface>&>(e).set_scale(value); }, length,
            std::move(easing_function), std::move(on_finished));
    }

    // C# RelScaleToAsync: scale BY dscale from the current scale.
    template <class ViewInterface>
    void rel_scale_to(view<ViewInterface>& target, double dscale, std::uint32_t length = 250,
                      std::optional<maui::animations::easing> easing_function = {},
                      animation_completion on_finished = {})
    {
        scale_to(target, target.scale() + dscale, length, std::move(easing_function), std::move(on_finished));
    }

    // C# ScaleXToAsync: animate ScaleX to `scale`.
    template <class ViewInterface>
    void scale_x_to(view<ViewInterface>& target, double scale, std::uint32_t length = 250,
                    std::optional<maui::animations::easing> easing_function = {}, animation_completion on_finished = {})
    {
        detail::animate_to(
            target, target.scale_x(), scale, "scale_x_to",
            [](element& e, double value) { static_cast<view<ViewInterface>&>(e).set_scale_x(value); }, length,
            std::move(easing_function), std::move(on_finished));
    }

    // C# ScaleYToAsync: animate ScaleY to `scale`.
    template <class ViewInterface>
    void scale_y_to(view<ViewInterface>& target, double scale, std::uint32_t length = 250,
                    std::optional<maui::animations::easing> easing_function = {}, animation_completion on_finished = {})
    {
        detail::animate_to(
            target, target.scale_y(), scale, "scale_y_to",
            [](element& e, double value) { static_cast<view<ViewInterface>&>(e).set_scale_y(value); }, length,
            std::move(easing_function), std::move(on_finished));
    }

    // C# TranslateToAsync: animate TranslationX/Y concurrently (two child animations under one
    // composite, exactly the C# structure).
    template <class ViewInterface>
    void translate_to(view<ViewInterface>& target, double x, double y, std::uint32_t length = 250,
                      std::optional<maui::animations::easing> easing_function = {},
                      animation_completion on_finished = {})
    {
        const maui::animations::easing ease =
            easing_function ? std::move(*easing_function) : maui::animations::easing::linear();
        element* anchor = &target;
        auto translate_x = [anchor](double value) {
            static_cast<view<ViewInterface>&>(*anchor).set_translation_x(value);
        };
        auto translate_y = [anchor](double value) {
            static_cast<view<ViewInterface>&>(*anchor).set_translation_y(value);
        };
        auto composite = std::make_shared<animation>();
        composite->add(0, 1, std::make_shared<animation>(std::move(translate_x), target.translation_x(), x, ease));
        composite->add(0, 1, std::make_shared<animation>(std::move(translate_y), target.translation_y(), y, ease));
        composite->commit(target, "translate_to", 16, length, std::nullopt,
                          [on_finished = std::move(on_finished)](double /*final_value*/, bool canceled) {
                              if (on_finished)
                              {
                                  on_finished(canceled);
                              }
                          });
    }

    // C# LayoutToAsync (obsolete in C#, kept for the surface): ease the arranged bounds from the
    // current frame to `bounds`.
    template <class ViewInterface>
    void layout_to(view<ViewInterface>& target, const maui::graphics::rect& bounds, std::uint32_t length = 250,
                   std::optional<maui::animations::easing> easing_function = {}, animation_completion on_finished = {})
    {
        const maui::graphics::rect start = target.frame();
        detail::animate_to(
            target, 0, 1, "layout_to",
            [start, bounds](element& e, double progress) {
                const maui::graphics::rect frame{start.x + ((bounds.x - start.x) * progress),
                                                 start.y + ((bounds.y - start.y) * progress),
                                                 start.width + ((bounds.width - start.width) * progress),
                                                 start.height + ((bounds.height - start.height) * progress)};
                static_cast<view<ViewInterface>&>(e).arrange(frame);
            },
            length, std::move(easing_function), std::move(on_finished));
    }
} // namespace maui::controls
