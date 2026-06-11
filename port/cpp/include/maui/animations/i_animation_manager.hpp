#pragma once
// maui::animations::i_animation_manager  <=  Microsoft.Maui.Animations.IAnimationManager
//
// The contract responsible for controlling animations: registered animations are ticked from the
// manager's i_ticker until they finish. Ported from src/Core/src/Animations/IAnimationManager.cs.
// Ownership (PROFILE §8): the manager holds shared_ptr to the animations it ticks (the GC analog of
// C#'s List<Animation>); add() therefore takes shared ownership, while remove() identifies the
// animation by reference (C# removes by instance identity). The manager itself is shared_ptr-owned
// (it is handed out via the service registry, and committed animations hold a weak back-reference).

#include <memory>

namespace maui::animations
{
    class animation;
    class i_ticker;

    class i_animation_manager
    {
    public:
        virtual ~i_animation_manager() = default;

        // C# IAnimationManager.Ticker: the ticker used to time the animations.
        [[nodiscard]] virtual i_ticker& ticker() const = 0;

        // C# IAnimationManager.SpeedModifier: a factor the animations' elapsed time is multiplied by.
        [[nodiscard]] virtual double speed_modifier() const = 0;
        virtual void set_speed_modifier(double value) = 0;

        // C# IAnimationManager.AutoStartTicker: whether add() starts the ticker automatically.
        [[nodiscard]] virtual bool auto_start_ticker() const = 0;
        virtual void set_auto_start_ticker(bool value) = 0;

        // C# IAnimationManager.Add / Remove.
        virtual void add(std::shared_ptr<animation> animation_to_add) = 0;
        virtual void remove(animation& animation_to_remove) = 0;

    protected:
        i_animation_manager() = default;
        i_animation_manager(const i_animation_manager&) = default;
        i_animation_manager(i_animation_manager&&) = default;
        i_animation_manager& operator=(const i_animation_manager&) = default;
        i_animation_manager& operator=(i_animation_manager&&) = default;
    };
} // namespace maui::animations
