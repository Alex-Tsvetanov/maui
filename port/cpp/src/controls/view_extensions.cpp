// maui::controls view extensions — see include/maui/controls/view_extensions.hpp. Ported from
// src/Controls/src/Core/ViewExtensions.cs (the animation half; the AnimateToAsync core + the
// CancelAnimations handle sweep).
#include "maui/controls/view_extensions.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

#include "maui/animations/easing.hpp"
#include "maui/controls/animation.hpp"
#include "maui/controls/animation_extensions.hpp"
#include "maui/controls/element.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::controls
{
    namespace detail
    {
        void animate_to(element& target, double start, double end, const char* name,
                        maui::core::move_only_function<void(element&, double)> update, std::uint32_t length,
                        std::optional<maui::animations::easing> easing_function, animation_completion on_finished)
        {
            // C#: easing ??= Easing.Linear; new Animation(UpdateProperty, start, end, easing)
            //     .Commit(view, name, 16, length, finished: (f, a) => tcs.SetResult(a)).
            if (!easing_function)
            {
                easing_function = maui::animations::easing::linear();
            }
            element* anchor = &target;
            auto update_property = [anchor, update = std::move(update)](double value) { update(*anchor, value); };
            auto anim = std::make_shared<animation>(std::move(update_property), start, end, std::move(easing_function));
            anim->commit(target, name, 16, length, std::nullopt,
                         [on_finished = std::move(on_finished)](double /*final_value*/, bool canceled) {
                             if (on_finished)
                             {
                                 on_finished(canceled);
                             }
                         });
        }
    } // namespace detail

    void cancel_animations(element& target)
    {
        abort_animation(target, "layout_to");
        abort_animation(target, "translate_to");
        abort_animation(target, "rotate_to");
        abort_animation(target, "rotate_y_to");
        abort_animation(target, "rotate_x_to");
        abort_animation(target, "scale_to");
        abort_animation(target, "scale_x_to");
        abort_animation(target, "scale_y_to");
        abort_animation(target, "fade_to");
    }
} // namespace maui::controls
