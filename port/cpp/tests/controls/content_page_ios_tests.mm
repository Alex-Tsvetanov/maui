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
#include "maui/core/button_handler.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/semantics.hpp"
#include "maui/graphics/rect.hpp"
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
} // namespace
