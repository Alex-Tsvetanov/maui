#pragma once
// maui::controls::state_trigger_base        <=  Microsoft.Maui.Controls.StateTriggerBase
// maui::controls::state_trigger             <=  Microsoft.Maui.Controls.StateTrigger
// maui::controls::compare_state_trigger<T>  <=  Microsoft.Maui.Controls.CompareStateTrigger
// maui::controls::device_state_trigger      <=  Microsoft.Maui.Controls.DeviceStateTrigger
// maui::controls::orientation_state_trigger <=  Microsoft.Maui.Controls.OrientationStateTrigger
// maui::controls::adaptive_trigger          <=  Microsoft.Maui.Controls.AdaptiveTrigger
//
// State triggers activate visual states AUTOMATICALLY: each visual_state can carry triggers
// (VisualState.StateTriggers), and whenever a trigger's is_active flips, the owning group re-evaluates
// which state should be current (VisualStateGroup.UpdateStateTriggers → GoToState). The trigger cluster
// shares this header (one family, like trigger.hpp) with state_trigger.cpp for the out-of-line bodies.
//
// Lifecycle (StateTriggerBase): set_active(bool) records IsActive (raising is_active_changed on a
// change) and ALWAYS pokes the owning group's re-evaluation — even when the value did not change,
// matching C#'s SetActive. send_attached/send_detached bracket the platform subscriptions
// (OnAttached/OnDetached); C# drives them from VisualElement.OnWindowChanged → InvalidateStateTriggers
// (attach = entered a window, detach = left it), which the port mirrors through the element's
// loaded/unloaded events (visual_state_manager subscribes them when the groups are stored on a view via
// set_visual_state_groups).
//
// The C# back-reference chain (trigger → VisualState → VisualStateGroup → VisualElement) is collapsed
// into two owner hooks set by the visual_state_manager when it wires the groups: the host element (for
// adaptive_trigger's Window lookup) and an update callback (the group's UpdateStateTriggers). Both are
// NON-owning — the manager is a member of the host view, so the hooks never outlive it.
//
// Typed substitutions (reflection-free, documented):
//   - compare_state_trigger<T> replaces CompareStateTrigger's object Property/Value pair + the
//     Convert.ChangeType coercion (a stringly-XAML artifact) with a typed pair compared by operator==.
//     A default-constructed trigger is ACTIVE (C# compares null == null → true).
//   - device_state_trigger / orientation_state_trigger read the ported essentials facades
//     (maui::devices::device_info / device_display) — the same seam the C# triggers use.
//   - adaptive_trigger treats the window's Dimension.Unset (NaN) geometry like C#'s -1 "unknown".

#include <string>
#include <string_view>
#include <utility>

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/essentials/device_display.hpp" // display_orientation (OrientationStateTrigger.Orientation)

namespace maui::controls
{
    class element; // forward — the host the manager wires (the VisualElement back-ref)
    class window;  // forward — adaptive_trigger watches the host's containing window

    class state_trigger_base
    {
    public:
        state_trigger_base(const state_trigger_base&) = delete;
        state_trigger_base(state_trigger_base&&) = delete;
        state_trigger_base& operator=(const state_trigger_base&) = delete;
        state_trigger_base& operator=(state_trigger_base&&) = delete;
        virtual ~state_trigger_base() = default;

        // StateTriggerBase.IsActive — whether this trigger currently selects its state.
        [[nodiscard]] bool is_active() const
        {
            return is_active_;
        }
        // StateTriggerBase.IsAttached — whether the trigger is live (its host element is in a window).
        [[nodiscard]] bool is_attached() const
        {
            return is_attached_;
        }
        // StateTriggerBase.IsActiveChanged — raised when is_active actually changes.
        maui::core::event<> is_active_changed;

    protected:
        state_trigger_base() = default;

        // StateTriggerBase.SetActive: record IsActive (raising is_active_changed on a change) and ALWAYS
        // ask the owning group to re-evaluate its active trigger (VisualState?.VisualStateGroup?
        // .UpdateStateTriggers() — unconditionally called in C# too).
        void set_active(bool active);

        // OnAttached/OnDetached — subclasses subscribe/unsubscribe their platform feeds here.
        virtual void on_attached()
        {
        }
        virtual void on_detached()
        {
        }

        // The host element (the VisualState→VisualStateGroup→VisualElement walk, pre-collapsed by the
        // manager's wiring). Null until the groups are stored on a view (set_visual_state_groups).
        [[nodiscard]] element* attached_element() const
        {
            return visual_element_;
        }

    private:
        friend class visual_state_manager; // wires the owner hooks and drives the attach lifecycle

        // SendAttached/SendDetached — idempotent attach/detach brackets (IsAttached guards).
        void send_attached();
        void send_detached();
        // The manager's wiring: the host element + the owning group's UpdateStateTriggers. Cleared with
        // (nullptr, {}) when the groups are replaced.
        void set_owner(element* visual_element, maui::core::move_only_function<void()> update_state_triggers);

        element* visual_element_ = nullptr; // NON-owning host (the manager is a member of it)
        maui::core::move_only_function<void()> update_state_triggers_;
        bool is_active_ = false;
        bool is_attached_ = false;
    };

    // StateTrigger: activates its state when the developer sets is_active. The C# class shadows the base
    // read-only IsActive with a settable bindable; the port keeps the base getter and adds the setter.
    class state_trigger final : public state_trigger_base
    {
    public:
        // StateTrigger.IsActive setter (OnIsActiveChanged → UpdateState). Works before attach too —
        // the recorded activity is picked up when the groups are stored / re-evaluated.
        void set_is_active(bool value)
        {
            if (requested_ == value)
            {
                return;
            }
            requested_ = value;
            update_state();
        }

    protected:
        void on_attached() override
        {
            update_state();
        }

    private:
        void update_state()
        {
            set_active(requested_);
        }

        bool requested_ = false;
    };

    // CompareStateTrigger: active while `property` equals `value`. The typed port compares with
    // operator== (C#'s Convert.ChangeType/enum coercion exists only for stringly-typed XAML input).
    // NOTE a default-constructed trigger is active, mirroring C#'s null == null comparison.
    template <class T> class compare_state_trigger final : public state_trigger_base
    {
    public:
        compare_state_trigger()
        {
            update_state(); // the C# constructor runs UpdateState (both sides unset compare equal)
        }

        // CompareStateTrigger.Property — the (snapshot) value under test. Re-evaluates on every set.
        void set_property(T value)
        {
            property_ = std::move(value);
            update_state();
        }
        // CompareStateTrigger.Value — the comparand. Re-evaluates on every set.
        void set_value(T value)
        {
            value_ = std::move(value);
            update_state();
        }

    protected:
        void on_attached() override
        {
            update_state();
        }

    private:
        void update_state()
        {
            set_active(property_ == value_);
        }

        T property_{};
        T value_{};
    };

    // DeviceStateTrigger: active while the app runs on the named platform ("Android", "iOS", …) —
    // compared via maui::devices::device_platform, with C#'s legacy "UWP" → WinUI alias.
    class device_state_trigger final : public state_trigger_base
    {
    public:
        // DeviceStateTrigger.Device (OnDeviceChanged → UpdateState). An empty name never activates.
        void set_device(std::string value);
        [[nodiscard]] std::string_view device() const
        {
            return device_;
        }

    protected:
        void on_attached() override
        {
            update_state();
        }

    private:
        void update_state();

        std::string device_;
    };

    // OrientationStateTrigger: active while the main display's orientation matches. Subscribes to
    // DeviceDisplay.MainDisplayInfoChanged while attached; Unknown never activates.
    class orientation_state_trigger final : public state_trigger_base
    {
    public:
        orientation_state_trigger()
        {
            update_state(); // the C# constructor runs UpdateState
        }
        ~orientation_state_trigger() override;

        // OrientationStateTrigger.Orientation (OnOrientationChanged → UpdateState).
        void set_orientation(maui::devices::display_orientation value);
        [[nodiscard]] maui::devices::display_orientation orientation() const
        {
            return orientation_;
        }

    protected:
        void on_attached() override;
        void on_detached() override;

    private:
        void update_state();
        void unsubscribe();

        maui::devices::display_orientation orientation_ = maui::devices::display_orientation::unknown;
        maui::core::connection_token display_token_ = 0; // the MainDisplayInfoChanged subscription
    };

    // AdaptiveTrigger: active while the host's window is at least min_window_width × min_window_height.
    // Watches window.size_changed while attached; -1 (the default) means "no constraint on this axis",
    // and an unknown window size (no window, Dimension.Unset, or -1) leaves the activity unchanged.
    class adaptive_trigger final : public state_trigger_base
    {
    public:
        ~adaptive_trigger() override;

        // AdaptiveTrigger.MinWindowWidth / MinWindowHeight (OnMinWindowDimensionChanged → UpdateState).
        void set_min_window_width(double value);
        void set_min_window_height(double value);
        [[nodiscard]] double min_window_width() const
        {
            return min_window_width_;
        }
        [[nodiscard]] double min_window_height() const
        {
            return min_window_height_;
        }

    protected:
        void on_attached() override; // AttachEvents + UpdateState(knownAttached: true)
        void on_detached() override; // DetachEvents

    private:
        void attach_events();
        void detach_events();
        void update_state(bool known_attached = false);

        double min_window_width_ = -1;
        double min_window_height_ = -1;
        window* window_ = nullptr; // NON-owning — the watched window (must outlive the attachment)
        maui::core::scoped_connection size_token_;
    };
} // namespace maui::controls
