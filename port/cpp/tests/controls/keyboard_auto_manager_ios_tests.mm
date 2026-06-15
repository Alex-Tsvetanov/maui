// iOS (UIKit) backend tests for the keyboard auto-manager's next-responder walk — the port of
// KeyboardAutoManager.GoToNextResponderOrResign + the supporting ViewExtensions view-tree helpers
// (ios_keyboard_manager_ops.hpp). Run only for MAUI_BACKEND=ios (executed ON the iOS simulator via
// tools/ios-sim-run.sh). Compiled as Objective-C++ with ARC.
//
// These mirror the behavioral oracle in src/Core/tests/DeviceTests/Handlers/Entry/EntryHandlerTests.iOS.cs
// (NextMovesToNextEntry / NextMovesPastNotEnabledEntry / NextMovesToEditor / NextMovesPastNotEnabledEditor
// / NextMovesBackToTop / NextMovesSkipsHiddenSibling / NextMovesSkipsHiddenParent), but exercised against
// raw UIKit hierarchies so the walk is tested in isolation from the handler seam. The walk's responder
// chain bails when a view has no Window (the disposed-superview guard), so every hierarchy is mounted in
// a key UIWindow — exactly as the C# DeviceTests AttachAndRun harness does.
//
// The scroll-avoidance engine (KeyboardAutoManagerScroll: keyboard WillShow/WillHide/DidHide observers +
// AdjustPosition geometry) needs a live keyboard + run-loop animation that a spawned simulator process
// without a UIApplication cannot drive, so connect/disconnect idempotence is smoke-tested here and the
// geometry is covered by the app-bundle test-host lane (deferred with the scaffold).
#import <UIKit/UIKit.h>

#include "ios_keyboard_manager_ops.hpp"
#include "maui/core/keyboard_auto_manager.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::keyboard_auto_manager;
    using maui::platform::ios::check_if_eligible;
    using maui::platform::ios::find_next_view;
    using maui::platform::ios::go_to_next_responder_or_resign;

    // A key window holding a root content view, torn down at scope exit so each test starts clean. The
    // window must be key+visible for becomeFirstResponder to take effect and for the responder-chain
    // Window guard to pass.
    struct key_window_scope
    {
        UIWindow* window;
        UIView* root;

        key_window_scope()
        {
            // [[UIWindow alloc] init] adopts a placeholder scene in the spawned test process (the
            // window_handler.mm / collection_view_ios_tests precedent), avoiding the iOS-26 initWithFrame:
            // deprecation; give it an explicit content frame afterward.
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

        ~key_window_scope()
        {
            // Resign any active first responder in this hierarchy so it does not bleed into the next test,
            // then dismiss the window.
            [root endEditing:YES];
            window.hidden = YES;
            window.rootViewController = nil;
        }
    };

    UITextField* make_field(UIReturnKeyType return_type = UIReturnKeyDefault)
    {
        UITextField* const field = [[UITextField alloc] initWithFrame:CGRectMake(0, 0, 200, 30)];
        field.returnKeyType = return_type;
        return field;
    }

    UITextView* make_text_view()
    {
        return [[UITextView alloc] initWithFrame:CGRectMake(0, 0, 200, 60)];
    }

    // KeyboardAutoManager.GoToNextResponderOrResign + FindNextView: from entry1 (focused) the walk lands
    // on the next enabled UITextField sibling.
    TEST(keyboard_auto_manager_ios, next_responder_walk_finds_next_field)
    {
        key_window_scope scope;
        UITextField* const field1 = make_field(UIReturnKeyNext);
        UITextField* const field2 = make_field(UIReturnKeyNext);
        [scope.root addSubview:field1];
        [scope.root addSubview:field2];
        ASSERT_TRUE([field1 becomeFirstResponder]);
        ASSERT_TRUE(field1.isFirstResponder);

        go_to_next_responder_or_resign(field1, scope.root);

        EXPECT_TRUE(field2.isFirstResponder);
        EXPECT_FALSE(field1.isFirstResponder);
    }

    // NextMovesBackToTop: from the LAST field the walk wraps to the container's first subview.
    TEST(keyboard_auto_manager_ios, next_responder_walk_wraps_to_top)
    {
        key_window_scope scope;
        UITextField* const field1 = make_field(UIReturnKeyNext);
        UITextField* const field2 = make_field(UIReturnKeyNext);
        [scope.root addSubview:field1];
        [scope.root addSubview:field2];
        ASSERT_TRUE([field2 becomeFirstResponder]);

        go_to_next_responder_or_resign(field2, scope.root);

        EXPECT_TRUE(field1.isFirstResponder);
    }

    // CheckIfEligible: a UITextField whose ReturnKeyType is NOT Next is ineligible — the walk resigns
    // first responder instead of advancing, even though a next field exists.
    TEST(keyboard_auto_manager_ios, no_next_responder_resigns_when_not_eligible)
    {
        key_window_scope scope;
        UITextField* const field1 = make_field(UIReturnKeyDefault); // not Next -> ineligible
        UITextField* const field2 = make_field(UIReturnKeyNext);
        [scope.root addSubview:field1];
        [scope.root addSubview:field2];
        ASSERT_TRUE([field1 becomeFirstResponder]);

        go_to_next_responder_or_resign(field1, scope.root);

        EXPECT_FALSE(field1.isFirstResponder); // resigned
        EXPECT_FALSE(field2.isFirstResponder); // no jump
    }

    // A single eligible field with no other editable sibling: the wrap finds only itself, so it stays
    // first responder (ChangeFocusedView re-focuses the same view rather than resigning).
    TEST(keyboard_auto_manager_ios, sole_field_wraps_to_itself)
    {
        key_window_scope scope;
        UITextField* const only = make_field(UIReturnKeyNext);
        [scope.root addSubview:only];
        ASSERT_TRUE([only becomeFirstResponder]);

        go_to_next_responder_or_resign(only, scope.root);

        EXPECT_TRUE(only.isFirstResponder);
    }

    // NextMovesPastNotEnabledEntry + NextMovesSkipsHiddenSibling: the filter skips a disabled field and a
    // hidden field, landing on the next fully eligible one.
    TEST(keyboard_auto_manager_ios, respects_enabled_and_hidden_filters)
    {
        key_window_scope scope;
        UITextField* const field1 = make_field(UIReturnKeyNext);
        UITextField* const disabled = make_field(UIReturnKeyNext);
        disabled.enabled = NO;
        UITextField* const hidden = make_field(UIReturnKeyNext);
        hidden.hidden = YES;
        UITextField* const target = make_field(UIReturnKeyNext);
        [scope.root addSubview:field1];
        [scope.root addSubview:disabled];
        [scope.root addSubview:hidden];
        [scope.root addSubview:target];
        ASSERT_TRUE([field1 becomeFirstResponder]);

        go_to_next_responder_or_resign(field1, scope.root);

        EXPECT_TRUE(target.isFirstResponder);
        EXPECT_FALSE(disabled.isFirstResponder);
        EXPECT_FALSE(hidden.isFirstResponder);
    }

    // NextMovesToEditor: the walk treats an editable + interactive UITextView as a valid target.
    TEST(keyboard_auto_manager_ios, next_responder_walk_moves_to_editor)
    {
        key_window_scope scope;
        UITextField* const field = make_field(UIReturnKeyNext);
        UITextView* const editor = make_text_view();
        editor.editable = YES;
        editor.userInteractionEnabled = YES;
        [scope.root addSubview:field];
        [scope.root addSubview:editor];
        ASSERT_TRUE([field becomeFirstResponder]);

        go_to_next_responder_or_resign(field, scope.root);

        EXPECT_TRUE(editor.isFirstResponder);
    }

    // NextMovesPastNotEnabledEditor: a non-editable UITextView is skipped (Editable is the gate).
    TEST(keyboard_auto_manager_ios, next_responder_walk_skips_readonly_editor)
    {
        key_window_scope scope;
        UITextField* const field = make_field(UIReturnKeyNext);
        UITextView* const readonly_editor = make_text_view();
        readonly_editor.editable = NO;
        UITextView* const editor = make_text_view();
        editor.editable = YES;
        [scope.root addSubview:field];
        [scope.root addSubview:readonly_editor];
        [scope.root addSubview:editor];
        ASSERT_TRUE([field becomeFirstResponder]);

        go_to_next_responder_or_resign(field, scope.root);

        EXPECT_TRUE(editor.isFirstResponder);
        EXPECT_FALSE(readonly_editor.isFirstResponder);
    }

    // NextMovesSkipsHiddenParent: a field nested under a hidden container subtree is skipped (the parent's
    // Hidden short-circuits the descend before the child is considered).
    TEST(keyboard_auto_manager_ios, next_responder_walk_skips_hidden_parent_subtree)
    {
        key_window_scope scope;
        UIView* const group1 = [[UIView alloc] initWithFrame:scope.root.bounds];
        UIView* const group2 = [[UIView alloc] initWithFrame:scope.root.bounds];
        UIView* const group3 = [[UIView alloc] initWithFrame:scope.root.bounds];
        group2.hidden = YES;
        UITextField* const field1 = make_field(UIReturnKeyNext);
        UITextField* const buried = make_field(UIReturnKeyNext);
        UITextField* const field3 = make_field(UIReturnKeyNext);
        [group1 addSubview:field1];
        [group2 addSubview:buried];
        [group3 addSubview:field3];
        [scope.root addSubview:group1];
        [scope.root addSubview:group2];
        [scope.root addSubview:group3];
        ASSERT_TRUE([field1 becomeFirstResponder]);

        go_to_next_responder_or_resign(field1, scope.root);

        EXPECT_TRUE(field3.isFirstResponder);
        EXPECT_FALSE(buried.isFirstResponder);
    }

    // find_next_view directly: the predicate-driven walk returns the next matching view (no focus side
    // effect), proving the helper is usable standalone (e.g. by the scroll engine's hierarchy walk).
    TEST(keyboard_auto_manager_ios, find_next_view_returns_next_match)
    {
        key_window_scope scope;
        UITextField* const field1 = make_field(UIReturnKeyNext);
        UITextField* const field2 = make_field(UIReturnKeyNext);
        [scope.root addSubview:field1];
        [scope.root addSubview:field2];

        UIView* const found = find_next_view(field1, scope.root, [](UIView* candidate) {
            return static_cast<bool>([candidate isKindOfClass:[UITextField class]]);
        });

        EXPECT_EQ(found, field2);
    }

    // CheckIfEligible matrix: UITextField is eligible only with ReturnKeyType Next; UITextView is always
    // eligible; anything else is not.
    TEST(keyboard_auto_manager_ios, eligibility_matches_return_type_and_view_kind)
    {
        UITextField* const next_field = make_field(UIReturnKeyNext);
        UITextField* const default_field = make_field(UIReturnKeyDefault);
        UITextView* const editor = make_text_view();
        UIView* const plain = [[UIView alloc] initWithFrame:CGRectZero];

        EXPECT_TRUE(check_if_eligible(next_field));
        EXPECT_FALSE(check_if_eligible(default_field));
        EXPECT_TRUE(check_if_eligible(editor));
        EXPECT_FALSE(check_if_eligible(plain));
    }

    // KeyboardAutoManagerScroll.Connect/Disconnect idempotence: connecting twice then disconnecting twice
    // must not crash (the geometry is exercised by the app-bundle test-host lane). The observers are
    // installed/removed on the default NSNotificationCenter; this smoke-tests the token lifecycle.
    TEST(keyboard_auto_manager_ios, connect_disconnect_is_idempotent)
    {
        keyboard_auto_manager::connect_scroll_handler();
        keyboard_auto_manager::connect_scroll_handler(); // second connect is a no-op
        keyboard_auto_manager::disconnect_scroll_handler();
        keyboard_auto_manager::disconnect_scroll_handler(); // second disconnect is a no-op
        SUCCEED();
    }
} // namespace
