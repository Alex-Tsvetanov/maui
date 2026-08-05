// gesture_platform_manager — WinUI 3 (Windows) platform partial. Ported from
// src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.Windows.cs (line refs below are
// into that file).
//
// >>> THE STRUCTURAL FACT THAT SHAPES THIS WHOLE FILE <<<
// Unlike UIKit/AppKit — where one MAUI recognizer maps to one native UIGestureRecognizer, so the
// apple/ios partials keep a per-recognizer attachment table — Windows has NO per-recognizer native
// object. MAUI subscribes XAML ROUTED EVENTS on the single FrameworkElement (Tapped / DoubleTapped /
// RightTapped, the five Pointer*, ManipulationStarted|Delta|Completed + PointerCanceled, and the drag &
// drop pairs), and each handler then iterates `view.GestureRecognizers` and fans out.
// `UpdatingGestureRecognizers()` (:948-1063) recomputes ALL of those subscriptions from the WHOLE
// collection every time it runs, after first calling `ClearContainerEventHandlers()` (:315-403).
//
// So on this backend `native_attach(r)` and `native_detach(r)` mean the same thing: "the collection
// changed -> re-run one idempotent whole-collection subscription sync". Per-recognizer type dispatch
// happens INSIDE the routed-event callbacks. There is deliberately no N-natives-for-N-recognizers table.
// Because the cross-platform `load_recognizers()`
// (src/controls/gestures/gesture_platform_manager.cpp:56-78) calls native_attach once per ADDED
// recognizer and native_detach once per REMOVED one, the sync runs repeatedly per load — it is cheap (a
// few predicate scans) and idempotent (clear-then-resubscribe), exactly like the oracle.
//
// >>> WHICH COLLECTION THE SYNC AND THE FAN-OUT READ <<<
// `recognizers_` — the LIVE gesture_recognizer_collection, i.e. C#'s `view.GestureRecognizers`. NOT
// `attached_`: load_recognizers pushes into attached_ BEFORE native_attach and calls native_detach
// BEFORE erasing, so attached_ is stale at detach time (it still holds the recognizer being removed).
// `recognizers_` is accurate at both moments (the add sweep runs after the collection gained the item;
// the remove sweep only fires for items the collection no longer contains) — which is precisely what the
// oracle reads. The ONE place that must never touch it is native_detach_all(): view<> declares
// `gesture_manager_` BEFORE `gesture_recognizers_` (view.hpp:1225-1226), so at ~view time the collection
// is destroyed FIRST and `recognizers_` dangles by the time ~gesture_platform_manager runs. Teardown
// therefore works purely off the stored native state.
//
// Every FAN-OUT walks a SNAPSHOT, never the live vector — because each callback runs synchronous user
// code that may mutate the collection (or drop the view) mid-walk. That is the oracle's own rule, not a
// port invention: `GetGesturesFor<T>` copies before it yields
// (`foreach (… in new List<IGestureRecognizer>(gestures))`, EnumerableExtensions.cs:49-63), and EVERY
// fan-out in the Windows partial goes through it (:453, :469, :492, :617, :646, :776, :853, :869, :892).
// The port's snapshot holds shared_ptrs, so C#'s GC guarantee — a recognizer removed mid-walk still
// receives this event, and stays alive while it does — holds here too. `has_any_gesture` /
// `first_gesture` deliberately do NOT copy: they mirror `FirstGestureOrDefault`
// (EnumerableExtensions.cs:68-82), which walks the live collection, and they call no user code.
//
// >>> DEVIATIONS FROM THE ORACLE, ALL DELIBERATE AND NARROW <<<
//  1. Container-vs-Control split. C# holds `Control = _handler.PlatformView` and `Container =
//     _handler.ContainerView ?? _handler.PlatformView` (:55-64) and uses the split only for
//     `PreventGestureBubbling` (:92-93, :982-987, :1006-1010) — a "TODO MAUI"-flagged hack mirroring the
//     handlers that mark a tap handled to stop bubbling. No port handler opts into a container
//     (view_handler.hpp's `needs_container_v` is opt-in and no windows handler sets it) and the port has
//     no PreventGestureBubbling seam, so container and control coincide and both resolve to
//     `handler_->native_view()`. The two `else` branches that subscribe HandleTapped/HandleDoubleTapped
//     on the CONTROL therefore have no port equivalent.
//     // TODO: verify against src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.Windows.cs:55-64
//  2. Child-element gesture fan-out. Every `(view as IGestureController)?.GetChildElements(...)` branch
//     (:767-777, :961-964, :989-990) is skipped: the port has no IGestureController.GetChildElements and
//     no composite/child gesture recognizers, so there is nothing to enumerate.
//     // TODO: verify against src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.Windows.cs:961-990
//  3. Position reporting. C# hands each Send* a `relativeTo => GetPosition(relativeTo, e)` CALLBACK
//     (:736-746) so the consumer picks the reference element. The port's send_* take a plain
//     `std::optional<point>`, so the position is resolved ONCE here, relative to the subscribed
//     FrameworkElement — the same view-relative convention the ios bridge uses
//     (`[recognizer locationInView:recognizer.view]`).
//  4. PlatformArgs. Every C# Send* also carries a Platform*EventArgs wrapper exposing the raw WinUI args.
//     The port's recognizer surface has no PlatformArgs member, so those are dropped; the two places the
//     oracle READS BACK a platform flag (`args.PlatformArgs?.Handled` at :172, :193, :252) degrade to the
//     port's own `handled()` where one exists, and to "not handled" where it does not.
//  5. Multi-window pointer debouncing. `HasMultipleWindows()` / `IsPointerEventRelevantToCurrentElement()`
//     (:653-687) are DEAD CODE in the oracle — nothing calls either — so nothing is ported.
//  6. Two operator-precedence bugs in the oracle's drag&drop guards are NOT reproduced literally; see the
//     block comments in handle_drag_starting and handle_drag_leave. Both are ported to the behaviour the
//     oracle's own comments/intent describe, because a literal port is provably non-functional (an empty
//     DataPackage with no AllowedOperations cannot start a Windows drag at all).
//  7. The POINTER path reads a different collection in the oracle. Every other query here reads
//     `view.GestureRecognizers`, but the three pointer sites read `ElementGestureRecognizers` (:617,
//     :646, :1013), i.e. `Element.GetCompositeGestureRecognizers()` (:85-86). The composite collection is
//     `GestureRecognizers` MIRRORED (View.cs:97-166) plus ONE framework-injected member: the internal
//     `PointerGestureRecognizer` that `PointerGestureRecognizer.SetupForPointerOverVSM`
//     (PointerGestureRecognizer.cs:283-312) adds when — and only when — the element declares a
//     `PointerOver` visual state, so hover can drive the VSM without the developer declaring a
//     recognizer. The port has NO composite collection and no such injection (`common_states::pointer_over`
//     exists in visual_state_manager.hpp but nothing drives it), so the composite collection and
//     `recognizers_` are the same set BY CONSTRUCTION and the pointer sites are correct as written.
//     Consequence to fix ELSEWHERE, not here: a port view with a PointerOver visual state and no explicit
//     pointer recognizer never subscribes the pointer events, so that state never lights up. Closing it
//     means porting SetupForPointerOverVSM into the view/VSM layer; this backend then needs no change,
//     because the injected recognizer would arrive through `recognizers_`.
//     // TODO: verify against src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.Windows.cs:85-86
//
// Ownership (PROFILE §8): every lambda captures the manager `this`, whose destructor runs
// native_detach_all() and removes every subscription installed here — a routed event can therefore never
// START on a freed manager. (A callback ALREADY RUNNING whose user code destroys the view is a residual
// this file cannot close: the raw `manager_ptr` the lambda captured dies under it. The fan-out snapshot
// keeps the RECOGNIZERS alive across such a callback, which is as far as this seam reaches.)
// The drag payload is owned at PROCESS scope for the life of the drag session, not by the source view —
// see active_drag_slot() for why the oracle's ownership cannot be spelled with a raw address.

#include "maui/controls/gestures/gesture_platform_manager.hpp"

#include <winrt/Microsoft.UI.Input.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/data_package.hpp"
#include "maui/controls/data_package_operation.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/drag_gesture_recognizer.hpp"
#include "maui/controls/gestures/drop_gesture_recognizer.hpp"
#include "maui/controls/gestures/gesture_recognizer.hpp"
#include "maui/controls/gestures/gesture_recognizer_collection.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/graphics/point.hpp"
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace, and inside namespace maui::*
    // that name WINS over a file-scope alias (see button_handler.cpp's note on the collision).
    namespace winui = winrt::Microsoft::UI::Xaml;
    namespace winput = winrt::Microsoft::UI::Input;
    namespace wdt = winrt::Windows::ApplicationModel::DataTransfer;
    using winrt::Windows::Foundation::IInspectable;

    // :37 — the key the DataPackage travels under, byte-for-byte (a MAUI-started drag dropped on a port
    // view, or the reverse, has to agree on it).
    constexpr std::wstring_view k_data_package_key = L"_XFPropertes_DONTUSE";

    [[nodiscard]] maui::graphics::point to_point(const winrt::Windows::Foundation::Point& value)
    {
        return {static_cast<double>(value.X), static_cast<double>(value.Y)};
    }

    // C#'s `args.Data.Text.StartsWith("http", StringComparison.OrdinalIgnoreCase)` (:265).
    [[nodiscard]] bool starts_with_http(std::string_view text)
    {
        constexpr std::string_view k_prefix = "http";
        if (text.size() < k_prefix.size())
        {
            return false;
        }
        return std::ranges::equal(text.substr(0, k_prefix.size()), k_prefix, [](char lhs, char rhs) {
            const char folded = (lhs >= 'A' && lhs <= 'Z') ? static_cast<char>(lhs - 'A' + 'a') : lhs;
            return folded == rhs;
        });
    }

    // Which routed tap event a fan-out came from — the port's spelling of the oracle's
    // `e is DoubleTappedRoutedEventArgs` / `e is RightTappedRoutedEventArgs` type tests (:789, :818, :834).
    enum class tap_kind : std::uint8_t
    {
        tapped,
        double_tapped,
        right_tapped,
    };
} // namespace

namespace maui::controls
{
    // The manager's backend state (the forward-declared gesture_native_state, complete only in this TU —
    // which is why the ctor/dtor live here too: this is where the owning unique_ptr's deleter instantiates).
    //
    // NOT a per-recognizer attachment table (see the file header): one subscribed element, one token per
    // routed event, plus the gesture-tracking fields the oracle keeps as instance state.
    struct gesture_native_state
    {
        // The element every subscription below lives on (C#'s `_container`, :23). Stored rather than
        // re-read from handler_->native_view() so teardown always unsubscribes from the element it
        // actually subscribed to, even after the handler was swapped or dropped.
        winui::FrameworkElement target{nullptr};

        // ---- subscriptions (C#'s SubscriptionFlags, :1103-1115) ----
        bool tap_subscribed = false;          // ContainerTapAndRightTabEventSubscribed
        bool double_tap_subscribed = false;   // ContainerDoubleTapEventSubscribed
        bool pointer_subscribed = false;      // ContainerPgrPointerEventsSubscribed
        bool manipulation_subscribed = false; // ContainerManipulationAndPointerEventsSubscribed
        bool drag_subscribed = false;         // ContainerDragEventsSubscribed
        bool drop_subscribed = false;         // ContainerDropEventsSubscribed

        winrt::event_token tapped{};
        winrt::event_token right_tapped{};
        winrt::event_token double_tapped{};
        // The TextBox variants (:968-972, :994-1002): a TextBox's control template marks Tapped and
        // DoubleTapped handled, so a plain subscription is never invoked — the oracle registers through
        // AddHandler(handledEventsToo: true) instead, and must keep the handler OBJECT to RemoveHandler it
        // (C#'s `_tappedEventHandler` / `_doubleTappedEventHandler`, :26-27). Same idiom as
        // button_handler.cpp's pointer_sink, and the same reason.
        winui::Input::TappedEventHandler tapped_handler{nullptr};
        winui::Input::DoubleTappedEventHandler double_tapped_handler{nullptr};

        winrt::event_token pointer_entered{};
        winrt::event_token pointer_exited{};
        winrt::event_token pointer_moved{};
        winrt::event_token pointer_pressed{};
        winrt::event_token pointer_released{};

        winrt::event_token manipulation_started{};
        winrt::event_token manipulation_delta{};
        winrt::event_token manipulation_completed{};
        winrt::event_token pointer_canceled{};

        winrt::event_token drag_starting{};
        winrt::event_token drop_completed{};
        winrt::event_token drag_over{};
        winrt::event_token drop{};
        winrt::event_token drag_leave{};

        // HandleDragAndDropGesturePropertyChanged (:1086-1093): CanDrag / AllowDrop flipping AFTER the
        // collection settled must re-run the drag&drop install, because UpdateDragAndDropGestureRecognizers
        // only subscribes when the flag is already true (:923, :937). The port equivalent of C#'s
        // `dragGesture.PropertyChanged +=` (:920-921, :934-935); the recognizer pointer rides along so the
        // subscription can still be removed after the recognizer left the collection.
        gesture_recognizer* drag_watched = nullptr;
        maui::core::connection_token drag_watch_token = 0;
        gesture_recognizer* drop_watched = nullptr;
        maui::core::connection_token drop_watch_token = 0;

        // ---- gesture tracking (C#'s instance fields, :22, :32-36) ----
        std::vector<std::uint32_t> fingers;  // _fingers
        bool is_panning = false;             // _isPanning
        bool is_swiping = false;             // _isSwiping
        bool is_pinching = false;            // _isPinching
        bool was_pan_started_sent = false;   // _wasPanGestureStartedSent
        bool was_pinch_started_sent = false; // _wasPinchGestureStartedSent

        // The id of the drag STARTED here, while it is still in flight (0 = none). The package itself is
        // owned by active_drag_slot(), NOT by this view — see that function for why. Kept per-manager so
        // DropCompleted only releases the session THIS view started.
        std::uint64_t drag_payload_id = 0;
        // The borrow handed to drag_event_args when the property is absent — a drag that originated
        // outside this app. C# passes its null straight through with `package!` (:1098-1100, whose comment
        // asserts it can never be null); the port hands out an empty package rather than dereference null.
        data_package foreign_payload;
    };

    // The friend seam declared at gesture_platform_manager.hpp:71. Its documented first user is the ios
    // arbitration delegate, but the mechanism — "reach the manager's live handler_/sender_ FRESH on every
    // native callback" — is exactly what this backend needs: every C# handler here re-reads `Element` and
    // `_handler.VirtualView` on each callback rather than caching them.
    struct gesture_arbitration_access
    {
        [[nodiscard]] static maui::core::i_view_handler* handler(const gesture_platform_manager& manager)
        {
            return manager.handler_;
        }
        [[nodiscard]] static element* sender(const gesture_platform_manager& manager)
        {
            return manager.sender_;
        }
        [[nodiscard]] static gesture_recognizer_collection* recognizers(const gesture_platform_manager& manager)
        {
            return manager.recognizers_;
        }
        [[nodiscard]] static gesture_native_state* state(const gesture_platform_manager& manager)
        {
            return manager.native_state_.get();
        }
        [[nodiscard]] static gesture_native_state& ensure_state(gesture_platform_manager& manager)
        {
            if (!manager.native_state_)
            {
                manager.native_state_ = std::make_unique<gesture_native_state>();
            }
            return *manager.native_state_;
        }
    };

    namespace
    {
        using access = gesture_arbitration_access;

        // ---- collection queries (C#'s GestureRecognizerExtensions) --------------------------------

        // `view.GestureRecognizers.GetGesturesFor<T>()` — fan out over the collection in collection order
        // (see the file header on why this collection and not attached_).
        //
        // SNAPSHOT FIRST, then walk. `callback` runs synchronous user code — a handler is free to add or
        // remove a recognizer, or to navigate away and destroy the whole view — and every such mutation
        // reallocates or erases inside the live `std::vector`, so a walk of `items()` would be reading a
        // dangling iterator by the next step. Copying is the ORACLE's behaviour, not a port-side guard:
        // `GetGesturesFor` iterates `new List<IGestureRecognizer>(gestures)` (EnumerableExtensions.cs:56),
        // so C# also fires a recognizer that was removed part-way through the same fan-out. Holding
        // shared_ptrs (not raw pointers) reproduces the other half of that guarantee — the removed
        // recognizer is still ALIVE when its turn comes, which in C# is the GC's doing.
        template <class Recognizer, class Fn>
        void for_each_gesture(const gesture_platform_manager& manager, Fn&& callback)
        {
            gesture_recognizer_collection* const recognizers = access::recognizers(manager);
            if (recognizers == nullptr)
            {
                return;
            }
            std::vector<std::shared_ptr<Recognizer>> snapshot;
            snapshot.reserve(recognizers->items().size());
            for (const auto& recognizer : recognizers->items())
            {
                if (auto typed = std::dynamic_pointer_cast<Recognizer>(recognizer); typed != nullptr)
                {
                    snapshot.push_back(std::move(typed));
                }
            }
            for (const std::shared_ptr<Recognizer>& recognizer : snapshot)
            {
                callback(*recognizer);
            }
        }

        // `gestures.HasAnyGesturesFor<T>(predicate)`.
        template <class Recognizer, class Predicate>
        [[nodiscard]] bool has_any_gesture(const gesture_platform_manager& manager, Predicate&& predicate)
        {
            gesture_recognizer_collection* const recognizers = access::recognizers(manager);
            if (recognizers == nullptr)
            {
                return false;
            }
            return std::ranges::any_of(recognizers->items(), [&predicate](const auto& recognizer) {
                auto* const typed = dynamic_cast<Recognizer*>(recognizer.get());
                return typed != nullptr && predicate(*typed);
            });
        }

        template <class Recognizer> [[nodiscard]] bool has_any_gesture(const gesture_platform_manager& manager)
        {
            return has_any_gesture<Recognizer>(manager, [](const Recognizer&) { return true; });
        }

        // `gestures.FirstGestureOrDefault<T>()`.
        template <class Recognizer> [[nodiscard]] Recognizer* first_gesture(const gesture_platform_manager& manager)
        {
            gesture_recognizer_collection* const recognizers = access::recognizers(manager);
            if (recognizers == nullptr)
            {
                return nullptr;
            }
            for (const auto& recognizer : recognizers->items())
            {
                if (auto* const typed = dynamic_cast<Recognizer*>(recognizer.get()); typed != nullptr)
                {
                    return typed;
                }
            }
            return nullptr;
        }

        // C#'s `Element as View` guard, resolved through the handler so IsEnabled / Width / Height are read
        // as freshly as the oracle reads them.
        [[nodiscard]] maui::core::i_view* virtual_view_of(const gesture_platform_manager& manager)
        {
            maui::core::i_view_handler* const handler = access::handler(manager);
            return handler != nullptr ? handler->virtual_view() : nullptr;
        }

        // ---- teardown (ClearContainerEventHandlers, :315-403) --------------------------------------

        void forget_drag_drop_property_watches(gesture_native_state& state)
        {
            if (state.drag_watched != nullptr && state.drag_watch_token != 0)
            {
                state.drag_watched->property_changed.disconnect(state.drag_watch_token);
            }
            state.drag_watched = nullptr;
            state.drag_watch_token = 0;
            if (state.drop_watched != nullptr && state.drop_watch_token != 0)
            {
                state.drop_watched->property_changed.disconnect(state.drop_watch_token);
            }
            state.drop_watched = nullptr;
            state.drop_watch_token = 0;
        }

        // Remove EVERY subscription this manager installed. Reads only `state`, never the recognizer
        // collection — which is what makes it safe to call from ~gesture_platform_manager (file header).
        void clear_subscriptions(gesture_native_state& state)
        {
            if (const winui::FrameworkElement target = state.target; target != nullptr)
            {
                if (state.drag_subscribed)
                {
                    target.CanDrag(false);
                    target.DragStarting(state.drag_starting);
                    target.DropCompleted(state.drop_completed);
                }
                if (state.drop_subscribed)
                {
                    target.AllowDrop(false);
                    target.DragOver(state.drag_over);
                    target.Drop(state.drop);
                    target.DragLeave(state.drag_leave);
                }
                if (state.tap_subscribed)
                {
                    if (state.tapped_handler != nullptr)
                    {
                        target.RemoveHandler(winui::UIElement::TappedEvent(), winrt::box_value(state.tapped_handler));
                    }
                    else
                    {
                        target.Tapped(state.tapped);
                    }
                    target.RightTapped(state.right_tapped);
                }
                if (state.double_tap_subscribed)
                {
                    if (state.double_tapped_handler != nullptr)
                    {
                        target.RemoveHandler(winui::UIElement::DoubleTappedEvent(),
                                             winrt::box_value(state.double_tapped_handler));
                    }
                    else
                    {
                        target.DoubleTapped(state.double_tapped);
                    }
                }
                if (state.pointer_subscribed)
                {
                    target.PointerEntered(state.pointer_entered);
                    target.PointerExited(state.pointer_exited);
                    target.PointerMoved(state.pointer_moved);
                    target.PointerPressed(state.pointer_pressed);
                    target.PointerReleased(state.pointer_released);
                }
                if (state.manipulation_subscribed)
                {
                    target.ManipulationDelta(state.manipulation_delta);
                    target.ManipulationStarted(state.manipulation_started);
                    target.ManipulationCompleted(state.manipulation_completed);
                    target.PointerCanceled(state.pointer_canceled);
                }
            }

            state.drag_subscribed = false;
            state.drop_subscribed = false;
            state.tap_subscribed = false;
            state.double_tap_subscribed = false;
            state.pointer_subscribed = false;
            state.manipulation_subscribed = false;
            state.tapped_handler = nullptr;
            state.double_tapped_handler = nullptr;
            forget_drag_drop_property_watches(state);
        }

        // ---- drag & drop ---------------------------------------------------------------------------

        // C#'s `(global::Windows...DataPackageOperation)(int)dragEventArgs.AcceptedOperation` (:174-175,
        // :195-196): the port enum and the WinRT enum share UWP's values by construction
        // (data_package_operation.hpp's header note), so the conversion is the numeric identity — spelled
        // through the underlying integer exactly like the oracle.
        [[nodiscard]] wdt::DataPackageOperation to_winrt_operation(data_package_operation value)
        {
            return static_cast<wdt::DataPackageOperation>(static_cast<std::uint32_t>(value));
        }

        // The payload of the drag currently in flight, and the id it travels under.
        //
        // WHY THE SOURCE VIEW MUST NOT OWN IT. C# puts the MANAGED DataPackage straight into the OS
        // property bag — `e.Data.Properties[_doNotUsePropertyString] = args.Data;` (:249) — so the BAG is
        // the owner: the package survives for the whole drag session no matter what happens to the view
        // that started it, and the drop side's cast back (:209, :1098) is always valid. A WinRT property
        // bag stores IInspectable only, so the port cannot hand it a C++ object; publishing the ADDRESS of
        // a view-owned shared_ptr looks equivalent but is not — an OS drag session outlives the view (drag
        // a row, then navigate away mid-drag), the view's state is destroyed, and the next DragOver/Drop
        // in this app resolves that address into freed memory. The port therefore keeps ownership at the
        // scope the drag session actually has — the process — and publishes an ID. An id from a finished
        // session, from a foreign app, or from a real-MAUI drag (which travels under the same key :37 but
        // carries a managed object, unboxing to 0 here) all resolve to nullptr, i.e. to the same
        // "originated outside" path the port already had.
        //
        // ponytail: ONE slot, because Windows serialises drag sessions per app — a DragStarting means the
        // previous session is over, so its payload is released right there and nothing accumulates.
        // Concurrent (multi-touch) drags would lose the older payload: it degrades to an empty package,
        // never to a dangling one. Make this a map keyed by id if that ever matters. Not synchronised:
        // XAML routed events are delivered on the UI thread only.
        struct active_drag
        {
            std::uint64_t id = 0;      // 0 = nothing in flight
            std::uint64_t next_id = 1; // monotonic; ids are never reused, so a stale one cannot alias
            std::shared_ptr<data_package> payload;
        };

        [[nodiscard]] active_drag& active_drag_slot()
        {
            static active_drag slot;
            return slot;
        }

        // Take ownership of the payload for the new session and return the id it travels under.
        [[nodiscard]] std::uint64_t publish_drag_payload(std::shared_ptr<data_package> payload)
        {
            active_drag& slot = active_drag_slot();
            slot.payload = std::move(payload);
            slot.id = slot.next_id++;
            return slot.id;
        }

        // Release the session `id` started, if it is still the one in flight.
        void release_drag_payload(std::uint64_t id)
        {
            if (active_drag& slot = active_drag_slot(); id != 0 && slot.id == id)
            {
                slot.id = 0;
                slot.payload.reset();
            }
        }

        // The port data_package a hovering/dropping WinRT payload carries (C#'s ToDragEventArgs,
        // :1095-1101). The returned strong ref is what keeps it alive across the fan-out — the caller
        // holds it for the whole callback. Null when the drag did not originate in this process.
        [[nodiscard]] std::shared_ptr<data_package> payload_of(const wdt::DataPackageView& view)
        {
            if (view == nullptr)
            {
                return nullptr;
            }
            const winrt::hstring key{k_data_package_key};
            const auto properties = view.Properties();
            if (properties == nullptr || !properties.HasKey(key))
            {
                return nullptr;
            }
            const auto id = winrt::unbox_value_or<std::uint64_t>(properties.Lookup(key), 0);
            const active_drag& slot = active_drag_slot();
            return (id != 0 && id == slot.id) ? slot.payload : nullptr;
        }

        // HandleDragStarting (:237-285).
        void handle_drag_starting(gesture_platform_manager& manager, const winui::DragStartingEventArgs& args)
        {
            gesture_native_state* const state = access::state(manager);
            element* const sender = access::sender(manager);
            if (state == nullptr || sender == nullptr)
            {
                return;
            }
            for_each_gesture<drag_gesture_recognizer>(manager, [&](drag_gesture_recognizer& recognizer) {
                if (!recognizer.can_drag()) // :241-245
                {
                    args.Cancel(true);
                    return;
                }
                // send_drag_starting returns its args BY VALUE, and the package must outlive this callback
                // AND this view (the OS keeps dragging), so the process-scoped slot takes ownership and
                // the session id is what travels in the property bag.
                drag_starting_event_args port_args = recognizer.send_drag_starting(*sender);
                auto payload = std::make_shared<data_package>(std::move(port_args.data()));
                state->drag_payload_id = publish_drag_payload(payload);
                args.Data().Properties().Insert(winrt::hstring{k_data_package_key},
                                                winrt::box_value(state->drag_payload_id)); // :249

                // DOCUMENTED DEVIATION (file header note 6). The oracle guards the payload fill with
                //     if ((!args.Handled || (!args.PlatformArgs?.Handled ?? true)) && sender is IViewHandler handler)
                // (:252). `sender` is the UIElement the event fired on — never an IViewHandler — so the
                // literal condition is unsatisfiable and the whole block, INCLUDING
                // `e.AllowedOperations = Copy`, is dead: shipped MAUI would hand Windows a format-less
                // DataPackage with no allowed operation, which cannot start a drag at all. (It is a
                // Xamarin.Forms-era renderer check that survived the MAUI port — in XF `sender` WAS the
                // renderer, which did implement the interface.) The port runs the block, gated on the one
                // guard that has a port equivalent: DragStartingEventArgs.Handled.
                // TODO: verify against
                // src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.Windows.cs:252-281
                if (!port_args.handled())
                {
                    // TODO: verify against
                    // src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.Windows.cs:255-259
                    // — the oracle ALSO seeds the OS payload with the dragged Image's bitmap
                    // (e.Data.SetBitmap(RandomAccessStreamReference.CreateFromUri(bi.UriSource))) when the
                    // platform view is a XAML Image. That needs the image handler's live BitmapImage
                    // UriSource, which this seam cannot reach; TEXT marshalling below is ported in full,
                    // image marshalling is not.
                    if (const auto& text = payload->text(); text.has_value() && !text->empty())
                    {
                        const winrt::hstring value = maui::platform::windows::to_hstring(*text);
                        try
                        {
                            // C#'s Uri.TryCreate(text, UriKind.Absolute, out uri) (:263): WinRT's Uri ctor
                            // throws on a relative/invalid string, which is the same test spelled as an
                            // exception.
                            const winrt::Windows::Foundation::Uri uri{value};
                            if (starts_with_http(*text))
                            {
                                args.Data().SetWebLink(uri);
                            }
                            else
                            {
                                args.Data().SetApplicationLink(uri);
                            }
                        }
                        catch (const winrt::hresult_error&)
                        {
                            args.Data().SetText(value);
                        }
                    }
                    args.AllowedOperations(wdt::DataPackageOperation::Copy); // :280
                }

                args.Cancel(port_args.cancel()); // :283
            });
        }

        // HandleDropCompleted (:201-205).
        void handle_drop_completed(gesture_platform_manager& manager)
        {
            for_each_gesture<drag_gesture_recognizer>(manager, [](drag_gesture_recognizer& recognizer) {
                recognizer.send_drop_completed(drop_completed_event_args{});
            });
            // The drag is over, so the payload can go — but only the session THIS view started (C# lets
            // the GC reclaim its package once the OS releases the property bag).
            if (gesture_native_state* const state = access::state(manager); state != nullptr)
            {
                release_drag_payload(state->drag_payload_id);
                state->drag_payload_id = 0;
            }
        }

        // HandleDragOver (:180-199).
        void handle_drag_over(gesture_platform_manager& manager, const winui::DragEventArgs& args)
        {
            gesture_native_state* const state = access::state(manager);
            if (state == nullptr)
            {
                return;
            }
            // Held for the whole fan-out, so no handler can free the package under a later recognizer.
            const std::shared_ptr<data_package> payload = payload_of(args.DataView());
            drag_event_args port_args{payload != nullptr ? *payload : state->foreign_payload};
            for_each_gesture<drop_gesture_recognizer>(manager, [&](drop_gesture_recognizer& recognizer) {
                if (!recognizer.allow_drop())
                {
                    args.AcceptedOperation(wdt::DataPackageOperation::None); // :186-190
                    return;
                }
                recognizer.send_drag_over(port_args);
                args.AcceptedOperation(to_winrt_operation(port_args.accepted_operation()));
            });
        }

        // HandleDragLeave (:151-178).
        void handle_drag_leave(gesture_platform_manager& manager, const winui::DragEventArgs& args)
        {
            gesture_native_state* const state = access::state(manager);
            if (state == nullptr)
            {
                return;
            }
            const std::shared_ptr<data_package> payload = payload_of(args.DataView());
            drag_event_args port_args{payload != nullptr ? *payload : state->foreign_payload};
            for_each_gesture<drop_gesture_recognizer>(manager, [&](drop_gesture_recognizer& recognizer) {
                if (!recognizer.allow_drop())
                {
                    return;
                }
                const data_package_operation operation_prior_to_send = port_args.accepted_operation();
                recognizer.send_drag_leave(port_args);
                // DOCUMENTED DEVIATION (file header note 6). The oracle writes
                //     if (!dragEventArgs.PlatformArgs?.Handled ?? true && operationPriorToSend != ...)
                // (:172), which parses as `(!Handled) ?? (true && changed)` — always true, so it always
                // pushes the operation back. Its own comment two lines above (:166-171) states the
                // opposite intent, and names the symptom of getting it wrong: re-assigning the SAME
                // AcceptedOperation leaves the copy animation stuck on screen after the drag leaves. The
                // port implements the documented intent.
                // TODO: verify against
                // src/Controls/src/Core/Platform/GestureManager/GesturePlatformManager.Windows.cs:166-176
                if (operation_prior_to_send != port_args.accepted_operation())
                {
                    args.AcceptedOperation(to_winrt_operation(port_args.accepted_operation()));
                }
            });
        }

        // HandleDrop (:207-235). C#'s SendDrop is awaited; the port's is synchronous, so the try/catch
        // logging wrapper (:226-234) has no port equivalent.
        void handle_drop(gesture_platform_manager& manager, const winui::DragEventArgs& args)
        {
            gesture_native_state* const state = access::state(manager);
            element* const sender = access::sender(manager);
            if (state == nullptr)
            {
                return;
            }
            const std::shared_ptr<data_package> payload = payload_of(args.DataView());
            drop_event_args port_args{(payload != nullptr ? *payload : state->foreign_payload).view()};
            for_each_gesture<drop_gesture_recognizer>(manager, [&](drop_gesture_recognizer& recognizer) {
                if (!recognizer.allow_drop()) // :221-224
                {
                    return;
                }
                recognizer.send_drop(port_args, sender);
            });
        }

        // ---- tap (OnTap, :748-841) -----------------------------------------------------------------

        // ValidateGesture (:816-840).
        [[nodiscard]] bool validate_tap(const tap_gesture_recognizer& recognizer, tap_kind kind)
        {
            constexpr int k_single_tap = 1;
            constexpr int k_double_tap = 2;
            if (kind == tap_kind::right_tapped)
            {
                // Currently we only support single right clicks (:820).
                return contains(recognizer.buttons(), buttons_mask::secondary) &&
                       recognizer.number_of_taps_required() == k_single_tap;
            }
            if (!contains(recognizer.buttons(), buttons_mask::primary))
            {
                return false;
            }
            if (kind == tap_kind::double_tapped)
            {
                return recognizer.number_of_taps_required() == k_single_tap ||
                       recognizer.number_of_taps_required() == k_double_tap;
            }
            return recognizer.number_of_taps_required() == k_single_tap;
        }

        // Returns whether any recognizer fired (C#'s `handled`, used to set e.Handled).
        [[nodiscard]] bool handle_tap(gesture_platform_manager& manager, tap_kind kind,
                                      const winrt::Windows::Foundation::Point& position)
        {
            const maui::core::i_view* const view = virtual_view_of(manager);
            element* const sender = access::sender(manager);
            if (view == nullptr || sender == nullptr)
            {
                return false;
            }
            if (!view->is_enabled()) // :755-758
            {
                return false;
            }
            constexpr int k_double_tap = 2;
            // :787-800 — on a DoubleTapped, if ANY validated recognizer wants exactly two taps, ONLY the
            // two-tap recognizers fire.
            const bool only_double_taps =
                kind == tap_kind::double_tapped &&
                has_any_gesture<tap_gesture_recognizer>(manager, [kind](const tap_gesture_recognizer& recognizer) {
                    return validate_tap(recognizer, kind) && recognizer.number_of_taps_required() == k_double_tap;
                });

            bool handled = false;
            for_each_gesture<tap_gesture_recognizer>(manager, [&](tap_gesture_recognizer& recognizer) {
                if (!validate_tap(recognizer, kind))
                {
                    return;
                }
                if (only_double_taps && recognizer.number_of_taps_required() != k_double_tap)
                {
                    return;
                }
                recognizer.send_tapped(*sender, to_point(position));
                handled = true;
            });
            return handled;
        }

        // ---- pointer -------------------------------------------------------------------------------

        // GetPressedButton (:689-724).
        [[nodiscard]] buttons_mask pressed_button(const gesture_native_state& state,
                                                  const winui::Input::PointerRoutedEventArgs& args)
        {
            // Touch/Pen have no right-button semantics; treat as Primary (:692-693).
            const auto pointer = args.Pointer();
            if (pointer == nullptr || pointer.PointerDeviceType() != winput::PointerDeviceType::Mouse)
            {
                return buttons_mask::primary;
            }
            if (state.target == nullptr)
            {
                return buttons_mask::primary;
            }
            const auto point = args.GetCurrentPoint(state.target);
            if (point == nullptr)
            {
                return buttons_mask::primary;
            }
            const auto properties = point.Properties();
            if (properties == nullptr)
            {
                return buttons_mask::primary;
            }
            switch (properties.PointerUpdateKind())
            {
                case winput::PointerUpdateKind::RightButtonPressed:
                case winput::PointerUpdateKind::RightButtonReleased:
                    return buttons_mask::secondary;
                case winput::PointerUpdateKind::LeftButtonPressed:
                case winput::PointerUpdateKind::LeftButtonReleased:
                    return buttons_mask::primary;
                // Middle/other map to Primary by convention (:712-717).
                default:
                    break;
            }
            return properties.IsRightButtonPressed() ? buttons_mask::secondary : buttons_mask::primary;
        }

        [[nodiscard]] maui::graphics::point pointer_position(const gesture_native_state& state,
                                                             const winui::Input::PointerRoutedEventArgs& args)
        {
            return to_point(args.GetCurrentPoint(state.target).Position());
        }

        // ---- manipulation completion (SwipeComplete / PanComplete / PinchComplete, :844-906) --------

        void swipe_complete(gesture_platform_manager& manager, bool success)
        {
            gesture_native_state* const state = access::state(manager);
            element* const sender = access::sender(manager);
            if (state == nullptr || sender == nullptr || !state->is_swiping)
            {
                return;
            }
            if (success)
            {
                for_each_gesture<swipe_gesture_recognizer>(manager, [sender](swipe_gesture_recognizer& recognizer) {
                    // Accumulate-then-detect: the threshold check lives inside detect_swipe. NOT
                    // send_swiped — that is the iOS-only path, where UIKit detects the direction natively.
                    (void)recognizer.detect_swipe(*sender, recognizer.direction());
                });
            }
            state->is_swiping = false;
        }

        void pan_complete(gesture_platform_manager& manager, bool success)
        {
            gesture_native_state* const state = access::state(manager);
            element* const sender = access::sender(manager);
            if (state == nullptr || sender == nullptr || !state->is_panning)
            {
                return;
            }
            const auto touch_points = static_cast<int>(state->fingers.size());
            auto& current_id = pan_gesture_recognizer::current_id();
            for_each_gesture<pan_gesture_recognizer>(manager, [&](pan_gesture_recognizer& recognizer) {
                if (recognizer.touch_points() != touch_points)
                {
                    return;
                }
                if (success)
                {
                    recognizer.send_pan_completed(*sender, current_id.value());
                }
                else
                {
                    recognizer.send_pan_canceled(*sender, current_id.value());
                }
            });
            // :881 — unconditional, outside the loop and on BOTH outcomes.
            current_id.increment();
            state->is_panning = false;
        }

        void pinch_complete(gesture_platform_manager& manager, bool success)
        {
            gesture_native_state* const state = access::state(manager);
            element* const sender = access::sender(manager);
            if (state == nullptr || sender == nullptr || !state->is_pinching)
            {
                return;
            }
            for_each_gesture<pinch_gesture_recognizer>(manager, [&](pinch_gesture_recognizer& recognizer) {
                // NOTE: gated on the manager's _isPinching only — the oracle does NOT consult the
                // recognizer's own IsPinching here (that guard is the iOS bridge's, mirrored by
                // synthetic_pinch).
                if (success)
                {
                    recognizer.send_pinch_ended(*sender);
                }
                else
                {
                    recognizer.send_pinch_canceled(*sender);
                }
            });
            state->is_pinching = false;
        }

        // OnPointerExited (:556-564) — the finger bookkeeping half, distinct from the pointer-gesture
        // fan-out in OnPgrPointerExited.
        void tracking_pointer_exited(gesture_platform_manager& manager, std::uint32_t pointer_id)
        {
            swipe_complete(manager, true);
            gesture_native_state* const state = access::state(manager);
            if (state == nullptr)
            {
                return;
            }
            if (!state->is_panning && !state->is_pinching && !state->is_swiping)
            {
                std::erase(state->fingers, pointer_id);
            }
        }

        // ---- manipulation deltas (HandleSwipe / HandlePinch / HandlePan, :444-507) ------------------

        void handle_swipe(gesture_platform_manager& manager, const winui::Input::ManipulationDeltaRoutedEventArgs& args)
        {
            gesture_native_state* const state = access::state(manager);
            element* const sender = access::sender(manager);
            if (state == nullptr || sender == nullptr || state->fingers.size() > 1) // :446
            {
                return;
            }
            state->is_swiping = true;
            const auto translation = args.Cumulative().Translation;
            for_each_gesture<swipe_gesture_recognizer>(manager, [&](swipe_gesture_recognizer& recognizer) {
                recognizer.send_swipe(*sender, translation.X, translation.Y);
                args.Handled(true);
            });
        }

        void handle_pinch(gesture_platform_manager& manager, const winui::Input::ManipulationDeltaRoutedEventArgs& args)
        {
            gesture_native_state* const state = access::state(manager);
            element* const sender = access::sender(manager);
            const maui::core::i_view* const view = virtual_view_of(manager);
            constexpr std::size_t k_pinch_fingers = 2;
            if (state == nullptr || sender == nullptr || view == nullptr || state->fingers.size() < k_pinch_fingers)
            {
                return; // :483
            }
            const auto source = args.OriginalSource().try_as<winui::UIElement>();
            if (source == nullptr || state->target == nullptr)
            {
                return; // :488
            }
            // :490-491 — the focus point in the container's coordinates, normalized to view-relative UNIT
            // coordinates. No divide-by-zero guard, matching the oracle (a zero-sized view yields inf).
            const auto translation_point = source.TransformToVisual(state->target).TransformPoint(args.Position());
            const maui::graphics::point origin{static_cast<double>(translation_point.X) / view->width(),
                                               static_cast<double>(translation_point.Y) / view->height()};
            // RAW Delta.Scale — the per-update relative delta WinUI already reports. NOT pinch_scale_delta
            // (that converts UIPinch/NSMagnification's CUMULATIVE reading) and no starting-scale multiply
            // (that is Android's PinchGestureHandler).
            const double scale = args.Delta().Scale;
            for_each_gesture<pinch_gesture_recognizer>(manager, [&](pinch_gesture_recognizer& recognizer) {
                if (!state->was_pinch_started_sent)
                {
                    recognizer.send_pinch_started(*sender, origin);
                }
                recognizer.send_pinch(*sender, scale, origin);
                args.Handled(true);
            });
            state->was_pinch_started_sent = true; // :505 — after the loop, like the oracle
        }

        void handle_pan(gesture_platform_manager& manager, const winui::Input::ManipulationDeltaRoutedEventArgs& args)
        {
            gesture_native_state* const state = access::state(manager);
            element* const sender = access::sender(manager);
            if (state == nullptr || sender == nullptr)
            {
                return;
            }
            state->is_panning = true;
            const auto translation = args.Cumulative().Translation; // already DIPs on this platform
            const auto touch_points = static_cast<int>(state->fingers.size());
            auto& current_id = pan_gesture_recognizer::current_id();
            for_each_gesture<pan_gesture_recognizer>(manager, [&](pan_gesture_recognizer& recognizer) {
                if (recognizer.touch_points() != touch_points)
                {
                    return;
                }
                if (!state->was_pan_started_sent)
                {
                    recognizer.send_pan_started(*sender, current_id.value());
                }
                recognizer.send_pan(*sender, translation.X, translation.Y, current_id.value());
                args.Handled(true);
            });
            state->was_pan_started_sent = true; // :478 — after the loop, like the oracle
        }

        // ---- subscription sync (UpdatingGestureRecognizers, :948-1063) -------------------------------

        // UpdateDragAndDropGestureRecognizers (:908-946).
        void update_drag_and_drop(gesture_platform_manager& manager)
        {
            gesture_native_state* const state = access::state(manager);
            if (state == nullptr || state->target == nullptr)
            {
                return;
            }
            const winui::FrameworkElement target = state->target;
            auto* const manager_ptr = &manager;

            if (auto* const drag = first_gesture<drag_gesture_recognizer>(manager); drag != nullptr)
            {
                // :920-921 — unsubscribe-then-subscribe, so a repeat run never doubles up.
                if (state->drag_watched != drag || state->drag_watch_token == 0)
                {
                    if (state->drag_watched != nullptr && state->drag_watch_token != 0)
                    {
                        state->drag_watched->property_changed.disconnect(state->drag_watch_token);
                    }
                    state->drag_watched = drag;
                    state->drag_watch_token = drag->property_changed.connect([manager_ptr](std::string_view name) {
                        if (name == drag_gesture_recognizer::can_drag_property().name())
                        {
                            update_drag_and_drop(*manager_ptr); // :1086-1093
                        }
                    });
                }
                if (drag->can_drag() && !state->drag_subscribed) // :923
                {
                    state->drag_subscribed = true;
                    target.CanDrag(true);
                    state->drag_starting = target.DragStarting(
                        [manager_ptr](const winui::UIElement&, const winui::DragStartingEventArgs& args) {
                            handle_drag_starting(*manager_ptr, args);
                        });
                    state->drop_completed = target.DropCompleted(
                        [manager_ptr](const winui::UIElement&, const winui::DropCompletedEventArgs&) {
                            handle_drop_completed(*manager_ptr);
                        });
                }
            }

            if (auto* const drop = first_gesture<drop_gesture_recognizer>(manager); drop != nullptr)
            {
                if (state->drop_watched != drop || state->drop_watch_token == 0)
                {
                    if (state->drop_watched != nullptr && state->drop_watch_token != 0)
                    {
                        state->drop_watched->property_changed.disconnect(state->drop_watch_token);
                    }
                    state->drop_watched = drop;
                    state->drop_watch_token = drop->property_changed.connect([manager_ptr](std::string_view name) {
                        if (name == drop_gesture_recognizer::allow_drop_property().name())
                        {
                            update_drag_and_drop(*manager_ptr);
                        }
                    });
                }
                if (drop->allow_drop() && !state->drop_subscribed) // :937
                {
                    state->drop_subscribed = true;
                    target.AllowDrop(true);
                    state->drag_over =
                        target.DragOver([manager_ptr](const IInspectable&, const winui::DragEventArgs& args) {
                            handle_drag_over(*manager_ptr, args);
                        });
                    state->drop = target.Drop([manager_ptr](const IInspectable&, const winui::DragEventArgs& args) {
                        handle_drop(*manager_ptr, args);
                    });
                    state->drag_leave =
                        target.DragLeave([manager_ptr](const IInspectable&, const winui::DragEventArgs& args) {
                            handle_drag_leave(*manager_ptr, args);
                        });
                }
            }
        }

        // SubscribePointerEvents (:1065-1074). Shared by the pointer-gesture path and the
        // pan/pinch/swipe path, which is why the oracle guards the second call with `!hasPointerGesture`.
        void subscribe_pointer_events(gesture_platform_manager& manager, gesture_native_state& state)
        {
            const winui::FrameworkElement target = state.target;
            auto* const manager_ptr = &manager;
            state.pointer_subscribed = true;

            const auto fan_out = [manager_ptr](const winui::Input::PointerRoutedEventArgs& args, auto&& send) {
                element* const sender = access::sender(*manager_ptr);
                gesture_native_state* const live = access::state(*manager_ptr);
                if (sender == nullptr || live == nullptr || virtual_view_of(*manager_ptr) == nullptr)
                {
                    return; // C#'s `Element is not View` guard (:641-644)
                }
                const maui::graphics::point position = pointer_position(*live, args);
                // :646 reads ElementGestureRecognizers (the COMPOSITE collection), not
                // view.GestureRecognizers — the port has no composite, so the two coincide (header note 7).
                for_each_gesture<pointer_gesture_recognizer>(
                    *manager_ptr, [&](pointer_gesture_recognizer& recognizer) { send(recognizer, *sender, position); });
            };

            state.pointer_entered =
                target.PointerEntered([fan_out](const IInspectable&, const winui::Input::PointerRoutedEventArgs& args) {
                    fan_out(args, [](pointer_gesture_recognizer& recognizer, element& sender,
                                     const maui::graphics::point& position) {
                        recognizer.send_pointer_entered(sender, position);
                    });
                });
            state.pointer_moved =
                target.PointerMoved([fan_out](const IInspectable&, const winui::Input::PointerRoutedEventArgs& args) {
                    fan_out(args, [](pointer_gesture_recognizer& recognizer, element& sender,
                                     const maui::graphics::point& position) {
                        recognizer.send_pointer_moved(sender, position);
                    });
                });
            state.pointer_exited = target.PointerExited(
                [manager_ptr, fan_out](const IInspectable&, const winui::Input::PointerRoutedEventArgs& args) {
                    fan_out(args, [](pointer_gesture_recognizer& recognizer, element& sender,
                                     const maui::graphics::point& position) {
                        recognizer.send_pointer_exited(sender, position);
                    });
                    // :596-599 — the pointer path ALSO drives the manipulation bookkeeping when the
                    // manipulation events are subscribed (the two paths share these subscriptions).
                    gesture_native_state* const live = access::state(*manager_ptr);
                    if (live != nullptr && live->manipulation_subscribed && args.Pointer() != nullptr)
                    {
                        tracking_pointer_exited(*manager_ptr, args.Pointer().PointerId());
                    }
                });

            // HandlePgrPointerButtonAction (:613-637): the mask-filtered press/release fan-out, then the
            // finger bookkeeping when the manipulation events are subscribed.
            const auto button_action = [manager_ptr](const winui::Input::PointerRoutedEventArgs& args,
                                                     bool is_pressed) {
                element* const sender = access::sender(*manager_ptr);
                gesture_native_state* const live = access::state(*manager_ptr);
                if (live == nullptr)
                {
                    return;
                }
                if (sender != nullptr && virtual_view_of(*manager_ptr) != nullptr)
                {
                    const buttons_mask button = pressed_button(*live, args);
                    const maui::graphics::point position = pointer_position(*live, args);
                    // :617 reads the COMPOSITE collection — same set here (header note 7).
                    for_each_gesture<pointer_gesture_recognizer>(
                        *manager_ptr, [&](pointer_gesture_recognizer& recognizer) {
                            // CheckButtonMask (:726-734) — `(Buttons & current) == current`, which is
                            // exactly buttons_mask::contains. (Android's extra zero-mask branch is
                            // Android-only; this oracle has none.)
                            if (!contains(recognizer.buttons(), button))
                            {
                                return;
                            }
                            if (is_pressed)
                            {
                                recognizer.send_pointer_pressed(*sender, position, button);
                            }
                            else
                            {
                                recognizer.send_pointer_released(*sender, position, button);
                            }
                        });
                }
                if (!live->manipulation_subscribed || args.Pointer() == nullptr)
                {
                    return;
                }
                const std::uint32_t pointer_id = args.Pointer().PointerId();
                if (is_pressed)
                {
                    // OnPointerPressed (:566-573).
                    if (std::ranges::find(live->fingers, pointer_id) == live->fingers.end())
                    {
                        live->fingers.push_back(pointer_id);
                    }
                }
                else
                {
                    // OnPointerReleased (:575-581).
                    swipe_complete(*manager_ptr, true);
                    pan_complete(*manager_ptr, true);
                    std::erase(live->fingers, pointer_id);
                }
            };

            state.pointer_pressed = target.PointerPressed(
                [button_action](const IInspectable&, const winui::Input::PointerRoutedEventArgs& args) {
                    button_action(args, true);
                });
            state.pointer_released = target.PointerReleased(
                [button_action](const IInspectable&, const winui::Input::PointerRoutedEventArgs& args) {
                    button_action(args, false);
                });
        }

        void sync_native_subscriptions(gesture_platform_manager& manager)
        {
            gesture_native_state& state = access::ensure_state(manager);

            // The Container setter's clear-then-update (:67-83) folded into the sync: resolve the platform
            // view fresh, unsubscribe from whatever was subscribed, then adopt it.
            winui::FrameworkElement target{nullptr};
            if (maui::core::i_view_handler* const handler = access::handler(manager);
                handler != nullptr && handler->native_view() != nullptr)
            {
                target = maui::platform::windows::ref<winui::UIElement>(handler->native_view())
                             .try_as<winui::FrameworkElement>();
            }
            clear_subscriptions(state); // ClearContainerEventHandlers (:958)
            state.target = target;

            if (target == nullptr || access::recognizers(manager) == nullptr || access::sender(manager) == nullptr)
            {
                return; // :953 — `_container is null || gestures is null`
            }

            update_drag_and_drop(manager); // :959
            auto* const manager_ptr = &manager;

            constexpr int k_single_tap = 1;
            constexpr int k_double_tap = 2;
            const bool has_single_tap =
                has_any_gesture<tap_gesture_recognizer>(manager, [](const tap_gesture_recognizer& recognizer) {
                    return recognizer.number_of_taps_required() == k_single_tap;
                });
            if (has_single_tap) // :963-979
            {
                state.tap_subscribed = true;
                const auto on_tapped = [manager_ptr](const IInspectable&,
                                                     const winui::Input::TappedRoutedEventArgs& args) {
                    gesture_native_state* const live = access::state(*manager_ptr);
                    const auto reference = live != nullptr ? live->target : winui::FrameworkElement{nullptr};
                    if (handle_tap(*manager_ptr, tap_kind::tapped, args.GetPosition(reference)))
                    {
                        args.Handled(true);
                    }
                };
                if (target.try_as<winui::Controls::TextBox>() != nullptr)
                {
                    // :968-972 — a TextBox marks Tapped handled in its template, so only a
                    // handledEventsToo AddHandler is ever invoked.
                    state.tapped_handler = winui::Input::TappedEventHandler(on_tapped);
                    target.AddHandler(winui::UIElement::TappedEvent(), winrt::box_value(state.tapped_handler), true);
                }
                else
                {
                    state.tapped = target.Tapped(on_tapped);
                }
                state.right_tapped = target.RightTapped(
                    [manager_ptr](const IInspectable&, const winui::Input::RightTappedRoutedEventArgs& args) {
                        gesture_native_state* const live = access::state(*manager_ptr);
                        const auto reference = live != nullptr ? live->target : winui::FrameworkElement{nullptr};
                        if (handle_tap(*manager_ptr, tap_kind::right_tapped, args.GetPosition(reference)))
                        {
                            args.Handled(true);
                        }
                    });
            }
            // The `else` branch (:982-987) subscribes HandleTapped on the CONTROL under
            // PreventGestureBubbling — no port equivalent (file header note 1).

            const bool has_double_tap =
                has_any_gesture<tap_gesture_recognizer>(manager, [](const tap_gesture_recognizer& recognizer) {
                    return recognizer.number_of_taps_required() == k_single_tap ||
                           recognizer.number_of_taps_required() == k_double_tap;
                });
            if (has_double_tap) // :989-1003
            {
                state.double_tap_subscribed = true;
                const auto on_double_tapped = [manager_ptr](const IInspectable&,
                                                            const winui::Input::DoubleTappedRoutedEventArgs& args) {
                    gesture_native_state* const live = access::state(*manager_ptr);
                    const auto reference = live != nullptr ? live->target : winui::FrameworkElement{nullptr};
                    if (handle_tap(*manager_ptr, tap_kind::double_tapped, args.GetPosition(reference)))
                    {
                        args.Handled(true);
                    }
                };
                if (target.try_as<winui::Controls::TextBox>() != nullptr)
                {
                    state.double_tapped_handler = winui::Input::DoubleTappedEventHandler(on_double_tapped);
                    target.AddHandler(winui::UIElement::DoubleTappedEvent(),
                                      winrt::box_value(state.double_tapped_handler), true);
                }
                else
                {
                    state.double_tapped = target.DoubleTapped(on_double_tapped);
                }
            }

            // :1013 is the third and last site that reads the COMPOSITE collection rather than
            // view.GestureRecognizers. The port has no composite collection, so this query sees the same
            // set — but it is also why a PointerOver visual state alone does not subscribe pointer events
            // here the way it does in MAUI. Header note 7 has the citation and what closing it needs.
            const bool has_pointer_gesture = has_any_gesture<pointer_gesture_recognizer>(manager);
            if (has_pointer_gesture) // :1013-1018
            {
                subscribe_pointer_events(manager, state);
            }

            const bool has_swipe = has_any_gesture<swipe_gesture_recognizer>(manager);
            const bool has_pinch = has_any_gesture<pinch_gesture_recognizer>(manager);
            const bool has_pan = has_any_gesture<pan_gesture_recognizer>(manager);
            if (!has_swipe && !has_pinch && !has_pan) // :1024-1027
            {
                return;
            }
            // :1029-1049 — ManipulationMode.Scale and ManipulationModes.System cannot coexist, so MAUI
            // refuses pan/pinch/swipe on a ScrollView (and logs a warning per gesture kind; this port has
            // no logger seam here, so the refusal is silent).
            if (dynamic_cast<const scroll_view*>(access::sender(manager)) != nullptr)
            {
                return;
            }
            if (!has_pointer_gesture) // :1051-1055
            {
                subscribe_pointer_events(manager, state);
            }

            state.manipulation_subscribed = true;
            target.ManipulationMode(winui::Input::ManipulationModes::Scale |
                                    winui::Input::ManipulationModes::TranslateX |
                                    winui::Input::ManipulationModes::TranslateY); // :1058
            state.manipulation_delta = target.ManipulationDelta(
                [manager_ptr](const IInspectable&, const winui::Input::ManipulationDeltaRoutedEventArgs& args) {
                    // OnManipulationDelta (:523-533) — order matters: swipe, pinch, pan.
                    if (virtual_view_of(*manager_ptr) == nullptr)
                    {
                        return;
                    }
                    handle_swipe(*manager_ptr, args);
                    handle_pinch(*manager_ptr, args);
                    handle_pan(*manager_ptr, args);
                });
            state.manipulation_started = target.ManipulationStarted(
                [manager_ptr](const IInspectable&, const winui::Input::ManipulationStartedRoutedEventArgs&) {
                    // OnManipulationStarted (:535-545).
                    gesture_native_state* const live = access::state(*manager_ptr);
                    if (live == nullptr || virtual_view_of(*manager_ptr) == nullptr)
                    {
                        return;
                    }
                    live->is_pinching = true;
                    live->was_pinch_started_sent = false;
                    live->was_pan_started_sent = false;
                });
            state.manipulation_completed = target.ManipulationCompleted(
                [manager_ptr](const IInspectable&, const winui::Input::ManipulationCompletedRoutedEventArgs&) {
                    // OnManipulationCompleted (:514-521).
                    swipe_complete(*manager_ptr, true);
                    pinch_complete(*manager_ptr, true);
                    pan_complete(*manager_ptr, true);
                    if (gesture_native_state* const live = access::state(*manager_ptr); live != nullptr)
                    {
                        live->fingers.clear();
                    }
                });
            state.pointer_canceled =
                target.PointerCanceled([manager_ptr](const IInspectable&, const winui::Input::PointerRoutedEventArgs&) {
                    // OnPointerCanceled (:547-554).
                    swipe_complete(*manager_ptr, false);
                    pinch_complete(*manager_ptr, false);
                    pan_complete(*manager_ptr, false);
                    if (gesture_native_state* const live = access::state(*manager_ptr); live != nullptr)
                    {
                        live->fingers.clear();
                    }
                });
        }
    } // namespace

    // ---- the backend seam ---------------------------------------------------------------------------

    gesture_platform_manager::gesture_platform_manager() = default;

    gesture_platform_manager::~gesture_platform_manager()
    {
        native_detach_all(); // removes every routed-event subscription before the captured `this` dies
    }

    // Windows has NO per-recognizer native object (file header): both hooks mean "the collection changed
    // -> re-run the whole-collection subscription sync", which is idempotent and cheap.
    void gesture_platform_manager::native_attach(const std::shared_ptr<gesture_recognizer>& recognizer)
    {
        (void)recognizer;
        sync_native_subscriptions(*this);
    }

    void gesture_platform_manager::native_detach(const gesture_recognizer& recognizer)
    {
        (void)recognizer;
        sync_native_subscriptions(*this);
    }

    void gesture_platform_manager::native_detach_all()
    {
        if (!native_state_)
        {
            return;
        }
        // MUST NOT read recognizers_ here — see the file header (it dangles by the time ~view reaches
        // ~gesture_platform_manager). clear_subscriptions works purely off the stored state.
        clear_subscriptions(*native_state_);
        native_state_->target = nullptr;
        // Deliberately NOT release_drag_payload(): a drag session survives its source view, and so must
        // its package — C#'s lives in the OS property bag, which the view's teardown cannot reach either.
        // The slot is released by DropCompleted, or reclaimed by the next DragStarting.
        native_state_->drag_payload_id = 0;
        native_state_->fingers.clear();
        native_state_->is_panning = false;
        native_state_->is_swiping = false;
        native_state_->is_pinching = false;
        native_state_->was_pan_started_sent = false;
        native_state_->was_pinch_started_sent = false;
    }

    // --- drag&drop (W2-22): reported off the REAL registration state (CanDrag / AllowDrop set + the
    // tokens live), not off mere attachment. Only the FIRST drag/drop recognizer drives the install
    // (FirstGestureOrDefault, :915-916), so only that one is registered. ---
    bool gesture_platform_manager::native_registered_drag_source(const gesture_recognizer& recognizer) const
    {
        const gesture_native_state* const state = native_state_.get();
        if (state == nullptr || !state->drag_subscribed)
        {
            return false;
        }
        return first_gesture<drag_gesture_recognizer>(*this) == &recognizer;
    }

    bool gesture_platform_manager::native_registered_drop_target(const gesture_recognizer& recognizer) const
    {
        const gesture_native_state* const state = native_state_.get();
        if (state == nullptr || !state->drop_subscribed)
        {
            return false;
        }
        return first_gesture<drop_gesture_recognizer>(*this) == &recognizer;
    }
    // --- end drag&drop (W2-22) ---
} // namespace maui::controls
