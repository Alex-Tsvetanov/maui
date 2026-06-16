#pragma once
// maui::platform::ios::hide_soft_input_on_tapped_manager
//   <=  Microsoft.Maui.Controls.HideSoftInputOnTappedChangedManager
//       (src/Controls/src/Core/ContentPage/HideSoftInputOnTappedChanged/
//        HideSoftInputOnTappedChangedManager.{cs,Platform.cs,iOS.cs})
//
// Drives ContentPage.HideSoftInputOnTapped on iOS: when the feature is enabled (any tracked page has the
// flag set AND has navigated to) and a text-input view becomes focused, a UITapGestureRecognizer
// (resign_first_responder_touch_gesture_recognizer) is attached to the input's window. Tapping the page
// OUTSIDE a text-input control / table resigns the focused input's first responder, hiding the soft
// keyboard. The recognizer cleans itself up on editing-end / window removal.
//
// The C# class is a scoped DI service shared between every ContentPage + every InputView of a window
// (resolved via Handler.GetService<>). The port keeps ONE manager per content_page_handler (no DI
// container at this layer); the content_page mapper routes UpdatePage onto its handler's manager, and the
// focus path (UpdateFocusForView) is driven by the same manager. The cross-platform tracking surface
// (UpdatePage / UpdateFocusForView / FeatureEnabled) is the ANDROID||IOS-shared .Platform.cs; the
// native SetupHideSoftInputOnTapped(UIView) is the iOS-only twin. Mirroring the keyboard_auto_manager /
// gesture_platform_manager split, the method bodies are backend-defined: the iOS .mm does the real
// UIKit wiring, the headless .cpp compiles a no-op stub so the headless build links and runs.
//
// Ownership (PROFILE §8): the tracked pages + the focused view are NON-owning raw back-references (the
// caller owns those control objects, which outlive this manager — it lives inside their page handler);
// the manager's lifecycle is bounded by the focus / navigation (disappearing) events it hooks, guarded
// by FeatureEnabled. The genuine memory hazard is on the NATIVE side and is handled there: the gesture
// recognizer holds WEAK refs to its UIView/UIWindow under ARC and tears down via a cleanup token (see
// resign_first_responder_touch_gesture_recognizer.mm). The watching-for-taps cleanup is a move-only
// std::function (C#'s ActionDisposable analog) held here and run on the next focus change / teardown.

#include <functional>
#include <vector>

#include "maui/core/event.hpp" // scoped_connection — the per-page disappearing (NavigatedFrom) hook

namespace maui::controls
{
    class content_page;
} // namespace maui::controls

namespace maui::core
{
    class i_view;
} // namespace maui::core

namespace maui::platform::ios
{
    // hide_soft_input_on_tapped_manager  <=  HideSoftInputOnTappedChangedManager.
    class hide_soft_input_on_tapped_manager
    {
    public:
        hide_soft_input_on_tapped_manager() = default;
        // Backend-defined dtor: the iOS twin disposes the active tap-recognizer cleanup (DisconnectFrom
        // Platform); headless's is a no-op. Declared here, defined per backend so the move-only cleanup
        // member is torn down in the TU that knows how (apple/headless symmetry with the *_platform dtors).
        ~hide_soft_input_on_tapped_manager();
        hide_soft_input_on_tapped_manager(const hide_soft_input_on_tapped_manager&) = delete;
        hide_soft_input_on_tapped_manager(hide_soft_input_on_tapped_manager&&) = delete;
        hide_soft_input_on_tapped_manager& operator=(const hide_soft_input_on_tapped_manager&) = delete;
        hide_soft_input_on_tapped_manager& operator=(hide_soft_input_on_tapped_manager&&) = delete;

        // C# HideSoftInputOnTappedChangedManager.UpdatePage: track / untrack `page`. When the page has the
        // flag set AND has navigated to (has_appeared), add it (once) and hook its disappearing event (the
        // NavigatedFrom analog) so it is removed when it leaves; otherwise remove it. Re-runs the focus
        // setup afterwards. Cross-platform (the ANDROID||IOS-shared .Platform.cs); the only backend-specific
        // step is the native gesture wiring inside setup_native (a no-op off-device).
        void update_page(maui::controls::content_page& page);

        // C# HideSoftInputOnTappedChangedManager.UpdateFocusForView: re-point the tracked focused input to
        // `view` when it became focused (or clear it when the tracked view lost focus), then — if the
        // feature is enabled and the view is focused — (re)wire the tap gesture via setup_native. The
        // cleanup token is held internally (mirroring _watchingForTaps; the C# IDisposable return is
        // internal bookkeeping the port keeps as a member rather than handing back).
        void update_focus_for_view(maui::core::i_view& view);

        // C# HideSoftInputOnTappedChangedManager.FeatureEnabled: any tracked page with the flag set that
        // has navigated to. (Always false off-device: setup_native never wires anything, so nothing is
        // tracked — but the predicate itself is shared.)
        [[nodiscard]] bool feature_enabled() const;

    private:
        // C# SetupHideSoftInputOnTapped(): re-evaluate the focus wiring for the currently-tracked focused
        // view (called after UpdatePage and after a focus change). Cross-platform.
        void setup_hide_soft_input_on_tapped();

        // C# DisconnectFromPlatform(): dispose + clear the active tap-recognizer cleanup token. Cross-
        // platform (the cleanup token is a backend-produced std::function, run uniformly here). noexcept so
        // the destructor that calls it cannot let an exception escape (the cleanup closures never throw).
        void disconnect_from_platform() noexcept;

        // C# UpdatePage's local RemovePage / OnPageNavigatedFrom: drop `page` from the tracked set (tearing
        // down its disappearing hook) and re-run the focus setup. Cross-platform.
        void remove_page(maui::controls::content_page& page);

        // The ONE backend-specific step (C# SetupHideSoftInputOnTapped(UIView) / the .iOS.cs twin): wire the
        // resign-first-responder tap gesture onto the focused input's native window and return the cleanup
        // token (an empty std::function when nothing was wired — no native view / no window). The iOS .mm
        // does the real UIKit work via resign_first_responder_touch_gesture_recognizer; the headless + apple
        // backends return an empty token (no soft keyboard), so update_page / update_focus_for_view are
        // observably no-ops off-device while the cross-platform bookkeeping above is shared verbatim. STATIC:
        // it reads only the passed-in focused view (no manager state), so every backend's definition is too.
        [[nodiscard]] static std::function<void()> setup_native(maui::core::i_view& focused_view);

        // A tracked page (C# _contentPages entry) plus its disappearing (NavigatedFrom) subscription, so
        // the page is removed when it navigates away and the hook is torn down on remove (C#'s
        // page.NavigatedFrom -= OnPageNavigatedFrom). NON-owning page back-ref.
        struct tracked_page
        {
            maui::controls::content_page* page = nullptr;
            maui::core::scoped_connection disappearing_token; // removes the page on navigation-away
        };

        // The tracked pages (C# _contentPages) and the currently-focused input (C# _focusedView, a weak
        // ref there — a non-owning raw pointer here; cleared on unfocus / navigation). NON-owning.
        std::vector<tracked_page> content_pages_;
        maui::core::i_view* focused_view_ = nullptr;

        // C# _watchingForTaps (the ActionDisposable from UpdateFocusForView): the cleanup that disposes
        // the native gesture token. Move-only std::function; run + cleared by disconnect_from_platform.
        std::function<void()> watching_for_taps_;
    };

    // C# InputView.MapIsFocused (InputView.Platform.cs, #if ANDROID||IOS): an InputView's IsFocused change
    // routes to handler.GetService<HideSoftInputOnTappedChangedManager>().UpdateFocusForView(iv). The port
    // has no DI scope, so the manager lives on the containing content_page's handler: walk `view`'s logical-
    // parent chain to the owning content_page, resolve its handler's soft_input_manager(), and forward the
    // focus change. A no-op when the view is not under a content_page or the page has no attached handler
    // (mirrors C#'s `handler?. ... ?.` null-conditional). The input handlers' is_focused mapper calls this.
    void route_input_view_focus(maui::core::i_view& view);
} // namespace maui::platform::ios
