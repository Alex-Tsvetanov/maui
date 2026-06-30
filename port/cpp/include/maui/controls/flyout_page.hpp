#pragma once
// maui::controls::flyout_page  <=  Microsoft.Maui.Controls.FlyoutPage
//
// A page managing two panes: a Flyout (menu/navigation, must carry a Title) and a Detail (the selected
// content), with IsPresented controlling whether the flyout pane is showing and FlyoutLayoutBehavior
// deciding popover-vs-split presentation. Ported from FlyoutPage.cs + FlyoutLayoutBehavior.cs; the
// handler seam is maui::core::i_flyout_view (IFlyoutView).
//
// Shape notes (deviations documented):
//   - The port's Page base is content_page (Title + the Appearing/Disappearing lifecycle), so
//     flyout_page derives it — the inherited content() surface is unused (a flyout page hosts its two
//     panes, not a Content; C# FlyoutPage : Page has no Content either).
//   - Flyout/Detail are NON-owning content_page* (the caller owns the pages, PROFILE §8). The C#
//     setter guards are kept AS EXCEPTIONS because the oracle tests assert them (the data_template
//     precedent): null after a value was set → std::invalid_argument (ArgumentNullException; the port
//     also rejects an initial null, where C# would NRE); a flyout without a Title and an
//     already-parented page → std::runtime_error (InvalidOperationException).
//   - IsPresented's C# defaultValueCreator (`DeviceInfo.Platform == DevicePlatform.macOS`) is evaluated
//     once in the constructor (the lazy creator collapsed; install a device_info mock first in tests,
//     exactly like the C# fixture). Setting it false while ShouldShowSplitMode() →
//     std::runtime_error (the C# propertyChanging guard).
//   - ShouldShowSplitMode reads device_info::idiom() and the MAIN display orientation
//     (device_display::main_display_info().orientation — C# uses Window.GetOrientation(), which falls
//     back to the display when the window has no size; the port has no per-window orientation yet).
//   - SendAppearing/SendDisappearing propagate into both panes FIRST, then fire this page's own event
//     (FlyoutPage.OnAppearing/OnDisappearing order); appearing also re-enables CanChangeIsPresented and
//     re-runs UpdateFlyoutLayoutBehavior.
//   - send_back_button_pressed ports the FlyoutPage-level BackButtonPressed event branch; the
//     Flyout/Detail Page.SendBackButtonPressed legs are deferred (content_page has no page-level
//     back-button hook — the navigation_page precedent). The per-page navigation events
//     (NavigatingFrom/NavigatedFrom/To) and DisconnectHandlers on a replaced detail are deferred too.
//   - IFlyoutPageController (DetailBounds/FlyoutBounds, the legacy renderer seam) is out of scope.

#include <functional>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/flyout_layout_behavior.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/flyout_behavior.hpp"
#include "maui/core/i_flyout_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    // C# Microsoft.Maui.Controls.BackButtonPressedEventArgs (Internals): the handled flag a subscriber
    // sets to consume the back press.
    struct back_button_pressed_event_args
    {
        bool handled = false;
    };

    class flyout_page : public content_page, public maui::core::i_flyout_view
    {
    public:
        flyout_page();
        ~flyout_page() override;
        flyout_page(const flyout_page&) = delete;
        flyout_page(flyout_page&&) = delete;
        flyout_page& operator=(const flyout_page&) = delete;
        flyout_page& operator=(flyout_page&&) = delete;

        // Shared bindable-property descriptors (one instance per type, like FlyoutPage.*Property).
        static const maui::core::bindable_property<bool>& is_presented_property();
        static const maui::core::bindable_property<bool>& is_gesture_enabled_property();
        static const maui::core::bindable_property<flyout_layout_behavior>& flyout_layout_behavior_property();

        // ---- events ----
        // C# FlyoutPage.IsPresentedChanged — raised after the IsPresented value actually changes.
        maui::core::event<> is_presented_changed;
        // C# IFlyoutPageController.BackButtonPressed — a subscriber can mark the press handled.
        maui::core::event<back_button_pressed_event_args&> back_button_pressed;

        // ---- Flyout / Detail (non-owning; the C# setter guards kept as exceptions — header note) ----
        [[nodiscard]] content_page* flyout() const
        {
            return flyout_;
        }
        void set_flyout(content_page* value);

        [[nodiscard]] content_page* detail() const
        {
            return detail_;
        }
        void set_detail(content_page* value);

        // ---- IsPresented ----
        [[nodiscard]] bool is_presented() const
        {
            return is_presented_.get();
        }
        // Throws std::runtime_error when hiding the flyout when split mode is active (the C#
        // OnIsPresentedPropertyChanging guard).
        void set_is_presented(bool value);

        // ---- FlyoutLayoutBehavior ----
        [[nodiscard]] flyout_layout_behavior layout_behavior() const
        {
            return flyout_layout_behavior_.get();
        }
        void set_layout_behavior(flyout_layout_behavior value)
        {
            flyout_layout_behavior_.set(value);
        }

        // ---- IsGestureEnabled ----
        [[nodiscard]] bool is_gesture_enabled() const
        {
            return is_gesture_enabled_.get();
        }
        void set_is_gesture_enabled(bool value)
        {
            is_gesture_enabled_.set(value);
        }

        // C# IFlyoutPageController.ShouldShowSplitMode: never on a phone; otherwise Split always, or
        // SplitOnLandscape/Default in landscape, or SplitOnPortrait in portrait.
        [[nodiscard]] bool should_show_split_mode() const;

        // C# FlyoutPage.ShouldShowToolbarButton: always on a phone; otherwise only while NOT in a
        // split presentation.
        [[nodiscard]] bool should_show_toolbar_button() const;

        // C# IFlyoutPageController.CanChangeIsPresented (re-enabled on appearing; cleared when a
        // non-default behavior forces split mode).
        [[nodiscard]] bool can_change_is_presented() const
        {
            return can_change_is_presented_;
        }

        // C# FlyoutPage.UpdateFlyoutLayoutBehavior: entering split mode presents the flyout and (for a
        // non-default behavior) locks IsPresented.
        void update_flyout_layout_behavior();

        // The FlyoutPage-level back-press branch (C# OnBackButtonPressed's BackButtonPressed event leg;
        // the Flyout/Detail SendBackButtonPressed legs are deferred — header note).
        bool send_back_button_pressed() const;

        // ---- page lifecycle (FlyoutPage.OnAppearing/OnDisappearing — panes first, then this page) ----
        void send_appearing() override;
        void send_disappearing() override;

        // FlyoutPage hosts its two PANES (flyout_/detail_), NOT the inherited content() surface — so the
        // base content_page::measure (which measures content_, null here) never reaches the panes and the
        // detail pane's content has zero desired size → it collapses (the slider/buttons render as a
        // zero-height sliver). Override to MEASURE both panes against the full constraint (each pane is a
        // content_page the split VC / drawer gives the whole column), mirroring tabbed_page::measure and the
        // arrange override below — without this the arrange-only path lays a never-measured tree out flat.
        maui::graphics::size measure(double width_constraint, double height_constraint) override;

        // FlyoutPage hosts its two PANES (flyout_/detail_), NOT the inherited content() surface — so the
        // base content_page::arrange (which arranges content_, null here) never reaches the panes and the
        // detail pane's content renders blank. Override to arrange both panes within this page's frame
        // (host-relative {0,0,w,h}: the native split VC positions the columns, and on a collapsed phone the
        // detail fills the screen, so each pane's content arranges from its column origin).
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

        // ---- i_flyout_view (the handler's seam) ----
        [[nodiscard]] maui::core::i_view* flyout_view() const override
        {
            return flyout_;
        }
        [[nodiscard]] maui::core::i_view* flyout_detail() const override
        {
            return detail_;
        }
        [[nodiscard]] bool flyout_is_presented() const override
        {
            return is_presented();
        }
        void set_flyout_is_presented(bool value) override
        {
            set_is_presented(value);
        }
        [[nodiscard]] maui::core::flyout_behavior flyout_behavior_value() const override
        {
            return should_show_split_mode() ? maui::core::flyout_behavior::locked : maui::core::flyout_behavior::flyout;
        }
        [[nodiscard]] double flyout_width() const override
        {
            return -1; // IFlyoutView.FlyoutWidth — platform default (the non-Android C# branch)
        }
        [[nodiscard]] bool flyout_is_gesture_enabled() const override
        {
            return is_gesture_enabled();
        }

    protected:
        // Flyout and Detail are this page's logical children (InternalChildren), so BindingContext +
        // Window inherit down to both panes.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (flyout_ != nullptr)
            {
                visit(*flyout_);
            }
            if (detail_ != nullptr)
            {
                visit(*detail_);
            }
        }

        // Routes the is_presented / flyout_layout_behavior property changes to their C# propertyChanged
        // callbacks (IsPresentedChanged; UpdateFlyoutLayoutBehavior + the handler's FlyoutBehavior
        // refresh) after the base raises the event / updates the handler.
        void on_property_changed(std::string_view name) override;

    private:
        // The shared body of the two pane setters (the C# Detail/Flyout setters differ only in the
        // flyout's Title guard, applied by the caller).
        void set_pane(content_page*& slot, content_page* value, const char* role);

        content_page* flyout_ = nullptr; // non-owning (the caller owns the panes)
        content_page* detail_ = nullptr;
        maui::core::property<bool> is_presented_{*this, is_presented_property()};
        maui::core::property<bool> is_gesture_enabled_{*this, is_gesture_enabled_property()};
        maui::core::property<flyout_layout_behavior> flyout_layout_behavior_{*this, flyout_layout_behavior_property()};
        bool can_change_is_presented_ = true; // IFlyoutPageController.CanChangeIsPresented
        // The DeviceDisplay.MainDisplayInfoChanged subscription (OnMainDisplayInfoChanged →
        // Handler.UpdateValue(FlyoutBehavior)); a token, disconnected in the destructor (§8).
        maui::core::connection_token display_changed_token_{};
    };
} // namespace maui::controls
