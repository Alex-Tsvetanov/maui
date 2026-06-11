#pragma once
// maui::animations::i_animator  <=  Microsoft.Maui.Animations.IAnimator
//
// Objects implementing i_animator can act as parent objects for animations: an animation holds a
// weak back-reference to its animator parent (animation::set_parent) and detaches itself through
// remove_animation when it finishes (animation::remove_from_parent). Ported from
// src/Core/src/Animations/IAnimator.cs. Nothing in the ported surface implements it yet (same as the
// C# oracle, where only Animation.Parent consumes it) — the seam exists for platform animators.

#include <memory>

namespace maui::animations
{
    class animation;

    class i_animator
    {
    public:
        virtual ~i_animator() = default;

        // C# IAnimator.AddAnimation: add an animation to this element.
        virtual void add_animation(const std::shared_ptr<animation>& animation_to_add) = 0;
        // C# IAnimator.RemoveAnimation: remove an animation from this element.
        virtual void remove_animation(animation& animation_to_remove) = 0;

    protected:
        i_animator() = default;
        i_animator(const i_animator&) = default;
        i_animator(i_animator&&) = default;
        i_animator& operator=(const i_animator&) = default;
        i_animator& operator=(i_animator&&) = default;
    };
} // namespace maui::animations
