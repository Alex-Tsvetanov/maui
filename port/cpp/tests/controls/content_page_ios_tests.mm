// iOS (UIKit) backend tests for the content_page seam — the content's real native view becomes a
// subview of the host UIView after set_content (and leaves when the content is replaced/cleared), and
// the host frames correctly on arrange. The host is a plain UIView container; the content here is a
// button (its handler owns a real UIButton — label/entry are still headless on ios). ALSO the
// on-simulator proof of the shared ios_semantics_ops.hpp fidelity details this unit ships (the
// UIAccessibilityTraitHeader keying, the UIControl element carve-out, the empty-string -> nil mapping,
// null-safety). Run only for MAUI_BACKEND=ios (executed ON the iOS simulator via tools/ios-sim-run.sh).
// Compiled as Objective-C++ with ARC.
#import <UIKit/UIKit.h>

#include <memory>
#include <string>

#include "ios_semantics_ops.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/platform_configuration/ios_specific/page.hpp" // U20: use_safe_area legacy fallback
#include "maui/controls/templates/content_presenter.hpp"
#include "maui/core/button_handler.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/entry_handler.hpp"
#include "maui/core/i_safe_area_view.hpp" // U20: GetSafeAreaRegionsForEdge contract
#include "maui/core/i_view_handler.hpp"
#include "maui/core/safe_area_edges.hpp"   // U20
#include "maui/core/safe_area_regions.hpp" // U20
#include "maui/core/semantics.hpp"
#include "maui/graphics/rect.hpp"
#include "resign_first_responder_touch_gesture_recognizer.hpp" // U04 test seam (fire-for-testing)
#include <gtest/gtest.h>

namespace
{
    using maui::controls::button;
    using maui::controls::content_page;
    using maui::core::button_handler;
    using maui::core::content_page_handler;
    using maui::core::semantic_heading_level;
    using maui::core::semantics;
    using maui::platform::ios::apply_input_transparent;
    using maui::platform::ios::apply_semantics;

    // -[NSString UTF8String] is nullable-annotated; convert through this guard so the std::string
    // construction never receives a null pointer (the values under test are always non-null).
    std::string to_std_string(NSString* value)
    {
        const char* const utf8 = value.UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    UIView* native_host(const std::shared_ptr<content_page_handler>& handler)
    {
        return (__bridge UIView*)handler->typed_platform_view()->native;
    }

    // A button with its handler attached, so it owns a real native UIButton the host can re-parent.
    // Returns the content's native UIView for superview assertions.
    UIView* attach_button(button& control)
    {
        auto handler = std::make_shared<button_handler>();
        control.set_handler(handler);
        return (__bridge UIView*)handler->native_view();
    }

    TEST(ios_content_page_seam, host_is_a_uiview)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        ASSERT_NE(handler->platform_view(), nullptr);
        EXPECT_TRUE([native_host(handler) isKindOfClass:[UIView class]]);
        EXPECT_EQ(native_host(handler).subviews.count, 0U);
    }

    TEST(ios_content_page_seam, content_becomes_a_subview)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        // A content child with its own native view (a button-backed UIButton). native_view() returns the
        // real UIButton the handler's pimpl owns (platform_view() would return the pimpl pointer).
        button child;
        UIView* const child_native = attach_button(child);
        ASSERT_NE(child_native, nil);

        page.set_content(child); // -> handler->invoke("set_content") -> map_set_content -> addSubview:

        EXPECT_EQ(native_host(handler).subviews.count, 1U);
        EXPECT_EQ(child_native.superview, native_host(handler));
        EXPECT_EQ(handler->typed_platform_view()->hosted_content, &child);
    }

    TEST(ios_content_page_seam, replacing_content_swaps_the_subview)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        button first;
        UIView* const first_native = attach_button(first);

        button second;
        UIView* const second_native = attach_button(second);

        page.set_content(first);
        EXPECT_EQ(native_host(handler).subviews.count, 1U);
        EXPECT_EQ(first_native.superview, native_host(handler));

        page.set_content(second); // the old content leaves, the new one is hosted
        EXPECT_EQ(native_host(handler).subviews.count, 1U);
        EXPECT_EQ(second_native.superview, native_host(handler));
        EXPECT_EQ(first_native.superview, nil);

        page.set_content(nullptr); // clearing empties the host
        EXPECT_EQ(native_host(handler).subviews.count, 0U);
    }

    TEST(ios_content_page_seam, arrange_sizes_the_host)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);

        page.arrange(maui::graphics::rect(5, 10, 200, 120)); // -> handler->platform_arrange sizes the host

        const CGRect frame = native_host(handler).frame;
        EXPECT_EQ(frame.origin.x, 5.0);
        EXPECT_EQ(frame.origin.y, 10.0);
        EXPECT_EQ(frame.size.width, 200.0);
        EXPECT_EQ(frame.size.height, 120.0);
    }

    // A content_presenter resolves the SAME content_page_handler and hosts its packed content as a native
    // subview (the ControlTemplate-presenter path the templated_view gallery page exercises). Critically,
    // when the presenter is at a NON-ZERO arrange origin (a deeply-nested template scope), the content is
    // framed HOST-RELATIVE (origin ~0 inside the presenter's host), NOT at the absolute page position —
    // the double-offset that pushed templated card bodies off-screen before the content_presenter::arrange
    // fix. Mirrors the content_page seam tests; the content here is a button (it owns a real UIButton).
    TEST(ios_content_presenter_seam, hosts_and_frames_packed_content_host_relative)
    {
        maui::controls::content_presenter presenter;
        auto handler = std::make_shared<content_page_handler>();
        presenter.set_handler(handler);

        auto child = std::make_shared<button>();
        auto child_handler = std::make_shared<button_handler>();
        child->set_handler(child_handler);
        UIView* const child_native = (__bridge UIView*)child_handler->native_view();
        ASSERT_NE(child_native, nil);

        presenter.set_content(child); // -> invoke("set_content") -> the child becomes a host subview
        EXPECT_EQ(native_host(handler).subviews.count, 1U);
        EXPECT_EQ(child_native.superview, native_host(handler));

        presenter.arrange(maui::graphics::rect(40, 80, 200, 50)); // a nested, non-zero-origin frame
        // The host takes the absolute frame...
        EXPECT_EQ(native_host(handler).frame.origin.x, 40.0);
        EXPECT_EQ(native_host(handler).frame.origin.y, 80.0);
        // ...but the content is arranged host-relative (no padding -> ~origin), never re-offset to (40,80).
        EXPECT_LT(child_native.frame.origin.x, 1.0);
        EXPECT_LT(child_native.frame.origin.y, 1.0);
        EXPECT_EQ(child_native.frame.size.width, 200.0);
        EXPECT_EQ(child_native.frame.size.height, 50.0);
    }

    // U20: setting SafeAreaEdges changes ISafeAreaView2.GetSafeAreaRegionsForEdge results without error,
    // PER-EDGE (0=Left,1=Top,2=Right,3=Bottom), driven through the real on-simulator content_page. When the
    // property is unset, GetSafeAreaRegionsForEdge falls back to the legacy UseSafeArea boolean.
    TEST(ios_content_page_safe_area, safe_area_edges_drives_get_regions_for_edge)
    {
        using maui::core::i_safe_area_view2;
        using maui::core::safe_area_edges;
        using maui::core::safe_area_regions;
        namespace ios_page = maui::controls::platform_configuration::ios_specific::page;

        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);
        i_safe_area_view2& face = page;

        // Unset → legacy fallback: UseSafeArea false → every edge None (edge-to-edge).
        for (int edge = 0; edge < 4; ++edge)
        {
            EXPECT_EQ(face.get_safe_area_regions_for_edge(edge), safe_area_regions::none);
        }

        // Unset + UseSafeArea true → every edge Container (legacy obey).
        ios_page::set_use_safe_area(page, true);
        for (int edge = 0; edge < 4; ++edge)
        {
            EXPECT_EQ(face.get_safe_area_regions_for_edge(edge), safe_area_regions::container);
        }

        // Explicit per-edge property wins over the legacy boolean: "None,All,None,All".
        page.set_safe_area_edges(safe_area_edges{safe_area_regions::none, safe_area_regions::all,
                                                 safe_area_regions::none, safe_area_regions::all});
        EXPECT_EQ(face.get_safe_area_regions_for_edge(0), safe_area_regions::none); // left
        EXPECT_EQ(face.get_safe_area_regions_for_edge(1), safe_area_regions::all);  // top
        EXPECT_EQ(face.get_safe_area_regions_for_edge(2), safe_area_regions::none); // right
        EXPECT_EQ(face.get_safe_area_regions_for_edge(3), safe_area_regions::all);  // bottom
    }

    // Semantics + InputTransparent reach the page's host UIView through the content_page_platform
    // update_semantics / update_input_transparent overrides (SemanticExtensions.UpdateSemantics /
    // ViewExtensions.UpdateInputTransparent).
    TEST(ios_content_page_seam, semantics_and_input_transparent_reach_the_host)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);
        UIView* const host = native_host(handler);

        auto sem = std::make_shared<semantics>();
        sem->set_description("Settings page");
        sem->set_hint("Adjusts preferences");
        page.set_semantics(sem);
        EXPECT_EQ(to_std_string(host.accessibilityLabel), "Settings page");
        EXPECT_EQ(to_std_string(host.accessibilityHint), "Adjusts preferences");
        EXPECT_TRUE(host.isAccessibilityElement); // a plain UIView with a label/hint becomes an element

        page.set_input_transparent(true); // UserInteractionEnabled = !InputTransparent
        EXPECT_FALSE(host.userInteractionEnabled);
        page.set_input_transparent(false);
        EXPECT_TRUE(host.userInteractionEnabled);
    }

    // IsHeading adds UIAccessibilityTraitHeader; dropping the heading removes the trait ONLY because it
    // was present — exactly C#'s hasHeader keying — driven end-to-end through the control.
    TEST(ios_content_page_seam, heading_trait_set_and_cleared_through_the_control)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);
        UIView* const host = native_host(handler);

        auto heading = std::make_shared<semantics>();
        heading->set_description("Section");
        heading->set_heading_level(semantic_heading_level::level1);
        page.set_semantics(heading);
        EXPECT_EQ(host.accessibilityTraits & UIAccessibilityTraitHeader, UIAccessibilityTraitHeader);

        auto plain = std::make_shared<semantics>(); // no heading level
        plain->set_description("Section");
        page.set_semantics(plain);
        EXPECT_EQ(host.accessibilityTraits & UIAccessibilityTraitHeader, 0U);
    }

    // A null Semantics is a no-op: it leaves the previously-pushed accessibility properties untouched
    // (C# UpdateSemantics returns early on a null Semantics — it does not clear the label/hint).
    TEST(ios_content_page_seam, semantics_null_leaves_accessibility_untouched)
    {
        content_page page;
        auto handler = std::make_shared<content_page_handler>();
        page.set_handler(handler);
        UIView* const host = native_host(handler);

        auto sem = std::make_shared<semantics>();
        sem->set_description("Keep me");
        page.set_semantics(sem);
        ASSERT_EQ(to_std_string(host.accessibilityLabel), "Keep me");

        page.set_semantics(nullptr); // maps update_semantics(nullptr) -> no-op
        EXPECT_EQ(to_std_string(host.accessibilityLabel), "Keep me");
    }

    // ---- the shared ios_semantics_ops fidelity details, driven directly (the same helper every
    // control's retrofit override will call) ----

    // The port's empty Description/Hint is C#'s null: it maps to a nil accessibilityLabel/Hint (keeping
    // UIKit's own fallbacks), and a whitespace/empty pair does NOT mark the view an element
    // (string.IsNullOrWhiteSpace keying).
    TEST(ios_semantics_ops, empty_strings_map_to_nil_and_do_not_mark_an_element)
    {
        UIView* const view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 10, 10)];
        const semantics empty; // description "" + hint "" = C# null/null
        apply_semantics(view, &empty);
        EXPECT_EQ(view.accessibilityLabel, nil);
        EXPECT_EQ(view.accessibilityHint, nil);
        EXPECT_FALSE(view.isAccessibilityElement);
    }

    // A UIControl-derived view is NOT re-marked an element (iOS already marks controls; C# only re-marks
    // the UIStepper/UIPageControl composites) — but the label still lands on it.
    TEST(ios_semantics_ops, uicontrol_keeps_its_own_element_marking)
    {
        UIButton* const control = [UIButton buttonWithType:UIButtonTypeSystem];
        semantics sem;
        sem.set_description("Submit");
        apply_semantics(control, &sem);
        EXPECT_EQ(to_std_string(control.accessibilityLabel), "Submit");
        // The composite carve-out applies to UIStepper/UIPageControl; a plain UIButton is untouched
        // here, keeping whatever element marking UIKit gave it natively.
    }

    // Defensive: the helpers are null-safe (a handler-less control / detached path never crashes).
    TEST(ios_semantics_ops, helpers_are_null_safe)
    {
        const semantics sem;
        apply_semantics(nil, &sem);         // null view -> no-op
        apply_input_transparent(nil, true); // null view -> no-op
        UIView* const view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 10, 10)];
        apply_semantics(view, nullptr); // null semantics -> no-op (properties left at UIKit defaults)
        EXPECT_EQ(view.accessibilityLabel, nil);
        SUCCEED();
    }

    // ---- U04: ContentPage.HideSoftInputOnTapped — the resign-first-responder tap recognizer ----
    // (HideSoftInputOnTappedChangedManager.iOS.cs + ResignFirstResponderTouchGestureRecognizer.iOS.cs).
    // These exercise the iOS manager + the gesture recognizer on a real UIWindow / UITextField. The
    // tap itself is driven through the no-touch-synthesis seam resign_first_responder_fire_for_testing
    // (UIGestureRecognizer cannot accept a synthesized UITouch from a unit test — the same constraint
    // gesture_ios_tests works around with its fire_registered_target seam).

    using maui::controls::entry;
    using maui::core::entry_handler;
    using maui::core::i_view_handler;
    using maui::platform::ios::hide_soft_input_on_tapped_manager;
    using maui::platform::ios::resign_first_responder_fire_for_testing;

    // The number of resign-first-responder recognizers currently armed on `window`.
    NSUInteger maui_resign_recognizer_count(UIWindow* window)
    {
        NSUInteger count = 0;
        for (UIGestureRecognizer* const recognizer in window.gestureRecognizers)
        {
            // The subclass is file-private to the recognizer .mm; identify it by class name on-device.
            if ([NSStringFromClass([recognizer class]) hasPrefix:@"MauiResignFirstResponder"])
            {
                ++count;
            }
        }
        return count;
    }

    // The real UITextField behind an entry whose handler is attached.
    UITextField* entry_field(const std::shared_ptr<entry_handler>& handler)
    {
        return (__bridge UITextField*)handler->native_view();
    }

    // A key window holding a root content view (the keyboard_auto_manager_ios_tests precedent). The window
    // must be key+visible for becomeFirstResponder to take effect; [[UIWindow alloc] init] adopts a
    // placeholder scene in the spawned test process (avoiding the iOS-26 initWithFrame: deprecation).
    // Torn down at scope exit so each test starts clean.
    struct key_window_scope
    {
        UIWindow* window;
        UIView* root;

        key_window_scope()
        {
            window = [[UIWindow alloc] init];
            window.frame = CGRectMake(0, 0, 320, 480);
            root = [[UIView alloc] initWithFrame:window.bounds];
            UIViewController* const controller = [[UIViewController alloc] init];
            controller.view = root;
            window.rootViewController = controller;
            [window makeKeyAndVisible];
        }

        key_window_scope(const key_window_scope&) = delete;
        key_window_scope& operator=(const key_window_scope&) = delete;
        key_window_scope(key_window_scope&&) = delete;
        key_window_scope& operator=(key_window_scope&&) = delete;

        ~key_window_scope()
        {
            [root endEditing:YES];
            window.hidden = YES;
            window.rootViewController = nil;
        }
    };

    // Focusing a text input (already first responder, in a window) arms a tap recognizer on the window;
    // firing the tap resigns the input (hiding the keyboard) and removes the recognizer.
    TEST(content_page_ios_hide_soft_input, arms_recognizer_and_resigns_focused_input)
    {
        key_window_scope scope;

        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        page.set_hide_soft_input_on_tapped(true);
        page.send_appearing(); // has_navigated_to / FeatureEnabled gate

        entry input;
        auto input_handler = std::make_shared<entry_handler>();
        input.set_handler(input_handler);
        UITextField* const field = entry_field(input_handler);
        ASSERT_NE(field, nil);
        [scope.root addSubview:field];
        ASSERT_TRUE([field becomeFirstResponder]);
        input.set_is_focused(true); // mirror the xplat focus state the manager reads

        hide_soft_input_on_tapped_manager& manager = page_handler->soft_input_manager();
        manager.update_page(page);            // track the page (FeatureEnabled is now true)
        manager.update_focus_for_view(input); // wire the gesture onto the window

        EXPECT_EQ(maui_resign_recognizer_count(scope.window), 1U);
        EXPECT_TRUE(field.isFirstResponder);

        const int fired = resign_first_responder_fire_for_testing(scope.window); // simulate the page tap
        EXPECT_EQ(fired, 1);
        EXPECT_FALSE(field.isFirstResponder);                      // keyboard dismissed
        EXPECT_EQ(maui_resign_recognizer_count(scope.window), 0U); // recognizer cleaned itself up
    }

    // U04 PRODUCTION routing: with the InputView parented under the page, a plain IsFocused change
    // (set_is_focused — the funnel the native editing-begin callback drives) must auto-arm the tap gesture
    // WITHOUT any manual update_focus_for_view. This proves the is_focused mapper routes InputView focus
    // changes to the page's HideSoftInputOnTapped manager (C# InputView.MapIsFocused), the gap U04 closed.
    TEST(content_page_ios_hide_soft_input, focus_change_auto_arms_gesture_in_production)
    {
        key_window_scope scope;

        // The page owns the input as its content (so the input's logical-parent chain reaches the page).
        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        entry input;
        auto input_handler = std::make_shared<entry_handler>();
        input.set_handler(input_handler);
        page.set_content(input);
        page.set_handler(page_handler);

        // Enable the feature, then appear: the page's appearing hook re-runs UpdatePage with has_appeared()
        // true, so the page is tracked (FeatureEnabled) — the production path, no manual update_page.
        page.set_hide_soft_input_on_tapped(true);
        page.send_appearing();

        UITextField* const field = entry_field(input_handler);
        ASSERT_NE(field, nil);
        [scope.root addSubview:field];
        ASSERT_TRUE([field becomeFirstResponder]);

        ASSERT_EQ(maui_resign_recognizer_count(scope.window), 0U); // nothing armed before focus
        input.set_is_focused(true);                                // the ONLY trigger — no manual update_focus_for_view

        EXPECT_EQ(maui_resign_recognizer_count(scope.window), 1U)
            << "an InputView focus change must auto-arm the tap gesture via the is_focused mapper";

        // And a focus LOSS auto-disarms it (the same routing, view.is_focused() now false).
        input.set_is_focused(false);
        EXPECT_EQ(maui_resign_recognizer_count(scope.window), 0U)
            << "an InputView focus loss must auto-disarm the tap gesture";
    }

    // The feature is gated: when the page does NOT have HideSoftInputOnTapped set, focusing the input
    // does NOT arm a recognizer (FeatureEnabled is false → DisconnectFromPlatform / no setup).
    TEST(content_page_ios_hide_soft_input, disabled_page_arms_nothing)
    {
        key_window_scope scope;

        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        // flag left false (default)
        page.send_appearing();

        entry input;
        auto input_handler = std::make_shared<entry_handler>();
        input.set_handler(input_handler);
        UITextField* const field = entry_field(input_handler);
        [scope.root addSubview:field];
        ASSERT_TRUE([field becomeFirstResponder]);
        input.set_is_focused(true);

        hide_soft_input_on_tapped_manager& manager = page_handler->soft_input_manager();
        manager.update_page(page);
        manager.update_focus_for_view(input);

        EXPECT_EQ(maui_resign_recognizer_count(scope.window), 0U);
        EXPECT_TRUE(field.isFirstResponder); // untouched
    }

    // Toggling the flag OFF after the recognizer is armed tears it down (UpdatePage removes the page →
    // FeatureEnabled false → DisconnectFromPlatform clears the watching-for-taps token + recognizer).
    TEST(content_page_ios_hide_soft_input, toggle_off_cleans_up_gesture)
    {
        key_window_scope scope;

        content_page page;
        auto page_handler = std::make_shared<content_page_handler>();
        page.set_handler(page_handler);
        page.set_hide_soft_input_on_tapped(true);
        page.send_appearing();

        entry input;
        auto input_handler = std::make_shared<entry_handler>();
        input.set_handler(input_handler);
        UITextField* const field = entry_field(input_handler);
        [scope.root addSubview:field];
        ASSERT_TRUE([field becomeFirstResponder]);
        input.set_is_focused(true);

        hide_soft_input_on_tapped_manager& manager = page_handler->soft_input_manager();
        manager.update_page(page);
        manager.update_focus_for_view(input);
        ASSERT_EQ(maui_resign_recognizer_count(scope.window), 1U);

        page.set_hide_soft_input_on_tapped(false); // routes through the mapper → manager.update_page
        EXPECT_EQ(maui_resign_recognizer_count(scope.window), 0U);
    }
} // namespace
