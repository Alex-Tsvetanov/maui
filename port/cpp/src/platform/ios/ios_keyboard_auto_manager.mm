// ios_keyboard_auto_manager — iOS (UIKit) implementation of the keyboard scroll-avoidance engine, the
// DIRECT port of Microsoft.Maui.Platform.KeyboardAutoManagerScroll
// (src/Core/src/Platform/iOS/KeyboardAutoManagerScroll.cs, adapted from IQKeyboardManager — MIT).
// Compiled as Objective-C++ with ARC only for the `ios` backend; the apple + headless backends compile
// no-op stubs of the same contract (keyboard_auto_manager.hpp).
//
// The engine keeps a set of main-thread-only static globals (the C# static fields): the scroll views it
// has touched, the starting insets/offsets it must restore, the keyboard frame, the container view's
// pre-keyboard origin, and the five NSNotificationCenter observer tokens. Connect() registers the
// observers ONCE (idempotent); Disconnect() removes them and clears the tokens (so no observer dangles
// after teardown). DidBeginEditing captures the editing view + its container and schedules a debounced
// AdjustPosition (a 30ms GCD dispatch_after, NEVER a main-thread sleep — the C# `await Task.Delay(30)`).
// WillShow parses the keyboard end-frame from the notification userInfo; WillHide animates the touched
// scroll view back and restores the container origin; DidHide clears the handling flags.
//
// AdjustPosition (the geometry core, KeyboardAutoManagerScroll.cs:339-753) computes the editing cursor's
// rect in window coordinates, compares it against the keyboard's top edge, and walks the enclosing
// UIScrollView hierarchy adjusting ContentOffset / ContentInset (table- and collection-cell-aware) so the
// cursor stays visible; if no scroll view can absorb the move, it shifts the container view's frame.
//
// DEVIATIONS from the C# oracle (documented, not invented):
//   - The `MauiView.IsSoftInputHandledByParent(View)` early-out in AdjustPositionDebounce is OMITTED: it
//     inspects the MANAGED ISafeAreaView2 carrier, which the port's plain-UIView platform layer does not
//     attach to the native view. The SafeAreaEdges.SoftInput coordination it guards lives in the managed
//     safe-area subsystem (out of this unit's scope); the ShouldIgnoreSafeAreaAdjustment / ShouldScrollAgain
//     statics are kept for fidelity but have no managed reader in the port.
//   - PrefersLargeTitles handling (AdjustForLargeTitles) is ported as in the oracle.

#import <UIKit/UIKit.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

#include "maui/core/keyboard_auto_manager.hpp"

namespace
{
    // ---- Static globals (KeyboardAutoManagerScroll's static fields). Main-thread-only. ----
    bool g_is_keyboard_auto_scroll_handling = false;
    UIScrollView* g_last_scroll_view = nil;
    UIScrollView* g_scrolled_view = nil;
    CGPoint g_starting_content_offset = CGPointZero;
    UIEdgeInsets g_starting_scroll_indicator_insets = UIEdgeInsetsZero;
    UIEdgeInsets g_starting_content_insets = UIEdgeInsetsZero;
    CGRect g_keyboard_frame = CGRectZero;
    const CGPoint k_invalid_point = {CGFLOAT_MAX, CGFLOAT_MAX};
    CGPoint g_top_view_begin_origin = {CGFLOAT_MAX, CGFLOAT_MAX};
    CGSize g_top_view_begin_container_size = CGSizeZero;
    double g_animation_duration = 0.25;
    UIView* g_view = nil;
    UIView* g_container_view = nil;
    bool g_has_cursor_rect = false;
    CGRect g_cursor_rect = CGRectZero;
    bool g_is_keyboard_showing = false;
    constexpr CGFloat k_text_view_distance_from_bottom = 20;

    // The five observer tokens (the AddObserver return values). Cleared in Disconnect.
    id g_will_show_token = nil;
    id g_will_hide_token = nil;
    id g_did_hide_token = nil;
    id g_text_field_token = nil;
    id g_text_view_token = nil;

    bool g_should_ignore_safe_area_adjustment = false;
    bool g_should_scroll_again = false;

    bool points_equal(CGPoint lhs, CGPoint rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    bool insets_equal(UIEdgeInsets lhs, UIEdgeInsets rhs)
    {
        return UIEdgeInsetsEqualToEdgeInsets(lhs, rhs);
    }

    // ViewExtensions.FindResponder<T> inline twin (the engine needs UIScrollView / UINavigationController /
    // UITableView / UICollectionView lookups). Kept local to avoid pulling the ARC-only ops header into a
    // file that also carries the C-style globals; behavior is identical to ios_keyboard_manager_ops.hpp.
    template <typename T> T* find_responder(UIView* view)
    {
        UIResponder* next_responder = view;
        while (next_responder != nil)
        {
            if ([next_responder isKindOfClass:[UIView class]] && ((UIView*)next_responder).window == nil)
            {
                return nil;
            }
            next_responder = next_responder.nextResponder;
            if ([next_responder isKindOfClass:[T class]])
            {
                return (T*)next_responder;
            }
        }
        return nil;
    }

    // GetContainerView fallback — the port's iOS page host is a plain UIView under the window's
    // rootViewController.view (no ContainerViewController type), so the editing view's content root is the
    // window root or the topmost non-window ancestor (see ios_keyboard_manager_ops.hpp::get_container_view).
    UIView* container_view_of(UIView* starting_point)
    {
        if (starting_point == nil)
        {
            return nil;
        }
        if (UIWindow* const window = starting_point.window)
        {
            if (UIViewController* const root = window.rootViewController; root != nil && root.viewIfLoaded != nil)
            {
                return root.viewIfLoaded;
            }
        }
        UIView* container = starting_point;
        while (container.superview != nil && ![container.superview isKindOfClass:[UIWindow class]])
        {
            container = container.superview;
        }
        return container;
    }

    // ---- Cursor geometry (FindLocalCursorPosition / FindCursorPosition, 135-153). ----
    bool find_local_cursor_position(CGRect& out_rect)
    {
        id<UITextInput> const text_input =
            [g_view conformsToProtocol:@protocol(UITextInput)] ? (id<UITextInput>)g_view : nil;
        UITextRange* const selected = text_input.selectedTextRange;
        if (selected == nil)
        {
            return false;
        }
        out_rect = [text_input caretRectForPosition:selected.start];
        return true;
    }

    bool find_cursor_position(CGRect& out_rect)
    {
        CGRect local = CGRectZero;
        if (find_local_cursor_position(local) && g_container_view != nil)
        {
            const CGRect cursor_in_container = [g_container_view convertRect:local fromView:g_view];
            out_rect = [g_container_view convertRect:cursor_in_container toView:nil];
            return true;
        }
        return false;
    }

    // DescriptionToCGRect (268-303): parse "NSRect: {{x, y}, {w, h}}" by keeping only digits/commas, split
    // into 4 numbers. Used to read the keyboard end-frame from the notification (CGRectFromString is not a
    // public binding here either, so the same defensive parse is faithful).
    bool description_to_cg_rect(NSString* description, CGRect& out_rect)
    {
        if (description == nil)
        {
            return false;
        }
        const std::string raw(description.UTF8String != nullptr ? description.UTF8String : "");
        std::string filtered;
        filtered.reserve(raw.size());
        for (const char character : raw)
        {
            if ((character >= '0' && character <= '9') || character == ',' || character == '.' || character == '-')
            {
                filtered.push_back(character);
            }
        }
        NSString* const ns = [NSString stringWithUTF8String:filtered.c_str()];
        NSArray<NSString*>* const parts = [ns componentsSeparatedByString:@","];
        if (parts.count != 4)
        {
            return false;
        }
        out_rect = CGRectMake(parts[0].doubleValue, parts[1].doubleValue, parts[2].doubleValue, parts[3].doubleValue);
        return true;
    }

    void set_animation_duration(NSDictionary* user_info)
    {
        if (user_info == nil)
        {
            return;
        }
        NSNumber* const duration = user_info[UIKeyboardAnimationDurationUserInfoKey];
        if (duration != nil && duration.doubleValue != 0)
        {
            g_animation_duration = duration.doubleValue;
        }
    }

    void animate_root_view(CGRect rect)
    {
        if (g_container_view != nil)
        {
            g_container_view.frame = rect;
        }
    }

    void animate_inset(UIScrollView* scroll_view, UIEdgeInsets moved_insets, CGFloat bottom_scroll_indicator_inset)
    {
        if (scroll_view == nil)
        {
            return;
        }
        scroll_view.contentInset = moved_insets;
        UIEdgeInsets new_indicator = scroll_view.verticalScrollIndicatorInsets;
        new_indicator.bottom = bottom_scroll_indicator_inset;
        scroll_view.scrollIndicatorInsets = new_indicator;
    }

    void animate_starting_last_scroll_view()
    {
        if (g_last_scroll_view != nil)
        {
            g_last_scroll_view.contentInset = g_starting_content_insets;
            g_last_scroll_view.scrollIndicatorInsets = g_starting_scroll_indicator_insets;
        }
    }

    bool is_horizontal_collection_view(UIView* view)
    {
        if (![view isKindOfClass:[UICollectionView class]])
        {
            return false;
        }
        UICollectionViewLayout* const layout = ((UICollectionView*)view).collectionViewLayout;
        if ([layout isKindOfClass:[UICollectionViewFlowLayout class]])
        {
            return ((UICollectionViewFlowLayout*)layout).scrollDirection == UICollectionViewScrollDirectionHorizontal;
        }
        return false;
    }

    // FindParentScroll (856-869): the nearest enclosing scroll view that is scroll-enabled and not a
    // horizontal collection view.
    UIScrollView* find_parent_scroll(UIScrollView* view)
    {
        while (view != nil)
        {
            if (view.scrollEnabled && !is_horizontal_collection_view(view))
            {
                return view;
            }
            view = find_responder<UIScrollView>(view);
        }
        return nil;
    }

    // GetPreviousIndexPath (979-1009): the index path immediately before `indexPath`, crossing section
    // boundaries (using the collection/table's section counts), or nil at the very top.
    NSIndexPath* previous_index_path(UIScrollView* scroll_view, NSIndexPath* index_path)
    {
        NSInteger previous_row = index_path.row - 1;
        NSInteger previous_section = index_path.section;
        if (previous_row < 0)
        {
            previous_section -= 1;
            if (previous_section >= 0 && [scroll_view isKindOfClass:[UICollectionView class]])
            {
                previous_row = [(UICollectionView*)scroll_view numberOfItemsInSection:previous_section] - 1;
            }
            else if (previous_section >= 0 && [scroll_view isKindOfClass:[UITableView class]])
            {
                previous_row = [(UITableView*)scroll_view numberOfRowsInSection:previous_section] - 1;
            }
            else
            {
                return nil;
            }
        }
        if (previous_row >= 0 && previous_section >= 0)
        {
            return [NSIndexPath indexPathForRow:previous_row inSection:previous_section];
        }
        return nil;
    }

    // AdjustForLargeTitles (892-930): subtract the navbar's collapsible height difference from the move so
    // the large-title collapse does not eat the scroll.
    CGFloat adjust_for_large_titles(CGFloat move, UIScrollView* super_scroll_view, UINavigationController* nav)
    {
        if (UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPhone &&
            (UIDevice.currentDevice.orientation == UIDeviceOrientationLandscapeLeft ||
             UIDevice.currentDevice.orientation == UIDeviceOrientationLandscapeRight))
        {
            return move;
        }
        const CGFloat nav_collapsed_height =
            UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPhone ? 44 : 50;
        const CGFloat nav_expanded_height = [nav.navigationBar sizeThatFits:CGSizeMake(0, 0)].height;
        const CGFloat min_move = nav_expanded_height - nav_collapsed_height;
        const CGFloat amount_scrolled = super_scroll_view.contentOffset.y;
        const CGFloat amount_left = min_move - amount_scrolled;
        const CGFloat collapse_difference = nav.navigationBar.frame.size.height - nav_collapsed_height;
        if (move >= amount_left)
        {
            if (move - collapse_difference < amount_left)
            {
                return amount_left;
            }
            return move - collapse_difference;
        }
        return move;
    }

    // ApplyContentInset (796-854): set the scrolled view's bottom inset so the keyboard-covered region is
    // padded and the user can still scroll to the bottom; an inner editor that ran past the keyboard gets a
    // tighter inset keyed off the live cursor.
    void apply_content_inset(UIScrollView* scrolled_view, UIScrollView* last_scroll_view, bool did_move,
                             bool is_inner_editor)
    {
        if (scrolled_view == nil || last_scroll_view == nil || g_container_view == nil)
        {
            return;
        }
        const CGRect frame_in_container = [g_container_view convertRect:scrolled_view.frame
                                                               fromView:scrolled_view.superview];
        const CGRect frame_in_window = [g_container_view convertRect:frame_in_container toView:nil];
        const CGRect keyboard_intersect = CGRectIntersection(g_keyboard_frame, frame_in_window);
        CGFloat bottom_inset = CGRectIsNull(keyboard_intersect) ? 0 : keyboard_intersect.size.height;

        if (![scrolled_view isKindOfClass:[UITextView class]] && bottom_inset > 0)
        {
            bottom_inset += k_text_view_distance_from_bottom;
        }
        CGFloat bottom_scroll_indicator_inset = bottom_inset;

        const bool is_text_view_in_cv = [scrolled_view isKindOfClass:[UITextView class]] &&
                                        [g_last_scroll_view isKindOfClass:[UICollectionView class]];
        bottom_inset = is_text_view_in_cv ? bottom_inset : std::max(g_starting_content_insets.bottom, bottom_inset);
        bottom_scroll_indicator_inset =
            std::max(g_starting_scroll_indicator_insets.bottom, bottom_scroll_indicator_inset);

        bottom_inset -= scrolled_view.safeAreaInsets.bottom;
        bottom_scroll_indicator_inset -= scrolled_view.safeAreaInsets.bottom;

        UIEdgeInsets moved_insets = scrolled_view.contentInset;
        moved_insets.bottom = bottom_inset;

        if (did_move && is_inner_editor && [scrolled_view isKindOfClass:[UITextView class]])
        {
            CGRect cursor = CGRectZero;
            if (find_cursor_position(cursor))
            {
                const CGFloat editor_bottom_inset =
                    CGRectGetMaxY(frame_in_window) - CGRectGetMaxY(cursor) - k_text_view_distance_from_bottom;
                moved_insets.bottom = std::max<CGFloat>(0, editor_bottom_inset);
                bottom_scroll_indicator_inset = std::max<CGFloat>(0, editor_bottom_inset);
            }
        }

        if (!insets_equal(last_scroll_view.contentInset, moved_insets))
        {
            [UIView animateWithDuration:g_animation_duration
                                  delay:0
                                options:UIViewAnimationOptionCurveEaseOut
                             animations:^{
                               animate_inset(scrolled_view, moved_insets, bottom_scroll_indicator_inset);
                             }
                             completion:nil];
        }
    }

    // The forward declaration so the notification handlers can schedule it.
    void adjust_position();

    // AdjustPositionDebounce (307-336): a 30ms GCD delay (NEVER a main-thread sleep) before AdjustPosition,
    // then a second 30ms beat that re-runs it if the layout asked to scroll again. Only fires while the
    // keyboard is showing.
    void adjust_position_debounce()
    {
        if (!g_is_keyboard_showing)
        {
            return;
        }
        // "Universal 30ms delay ... to ensure proper timing coordination between keyboard auto-scroll and
        // safe area adjustments." (The C# `await Task.Delay(30)` becomes dispatch_after on the main queue.)
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(0.030 * NSEC_PER_SEC)),
                       dispatch_get_main_queue(), ^{
                         adjust_position();
                         dispatch_after(dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(0.030 * NSEC_PER_SEC)),
                                        dispatch_get_main_queue(), ^{
                                          if (g_should_scroll_again)
                                          {
                                              adjust_position();
                                          }
                                        });
                       });
    }

    // ---- Notification handlers (DidUITextBeginEditing / WillKeyboardShow / WillHideKeyboard /
    //      DidHideKeyboard, 113-212). ----
    void did_text_begin_editing(NSNotification* notification)
    {
        g_is_keyboard_auto_scroll_handling = true;
        if (notification.object == nil)
        {
            return;
        }
        g_view = [notification.object isKindOfClass:[UIView class]] ? (UIView*)notification.object : nil;
        if (g_view == nil || find_responder<UIAlertController>(g_view) != nil)
        {
            g_is_keyboard_auto_scroll_handling = false;
            return;
        }
        g_has_cursor_rect = false;
        g_container_view = container_view_of(g_view);
        adjust_position_debounce();
    }

    void restore_position();

    void will_keyboard_show(NSNotification* notification)
    {
        NSDictionary* const user_info = notification.userInfo;
        const CGRect old_keyboard_frame = g_keyboard_frame;
        if (user_info != nil)
        {
            NSValue* const frame_value = user_info[UIKeyboardFrameEndUserInfoKey];
            // C# parses the NSValue's description by hand because CGRectFromString is not bound there
            // (DescriptionToCGRect); in Obj-C++ the NSValue's CGRectValue is available and exact, so use
            // it directly, falling back to the description parse if the value is some other encoding.
            if (frame_value != nil)
            {
                if ([frame_value isKindOfClass:[NSValue class]] && strcmp(frame_value.objCType, @encode(CGRect)) == 0)
                {
                    g_keyboard_frame = frame_value.CGRectValue;
                }
                else
                {
                    CGRect parsed = CGRectZero;
                    if (description_to_cg_rect(frame_value.description, parsed))
                    {
                        g_keyboard_frame = parsed;
                    }
                }
            }
            set_animation_duration(user_info);
        }

        if (!g_is_keyboard_showing)
        {
            g_is_keyboard_showing = true;
            adjust_position_debounce();
        }
        else if (!CGRectEqualToRect(old_keyboard_frame, g_keyboard_frame))
        {
            // keyboard already showing but its frame changed (e.g. keyboard type switch).
            adjust_position_debounce();
        }
    }

    void animate_hiding_keyboard()
    {
        if (g_last_scroll_view != nil && !insets_equal(g_last_scroll_view.contentInset, g_starting_content_insets))
        {
            g_last_scroll_view.contentInset = g_starting_content_insets;
            g_last_scroll_view.scrollIndicatorInsets = g_starting_scroll_indicator_insets;
        }
        UIScrollView* super_scroll_view = g_last_scroll_view;
        while (super_scroll_view != nil)
        {
            const CGSize content_size =
                CGSizeMake(std::max(super_scroll_view.contentSize.width, super_scroll_view.frame.size.width),
                           std::max(super_scroll_view.contentSize.height, super_scroll_view.frame.size.height));
            const CGFloat min_y = content_size.height - super_scroll_view.frame.size.height;
            if (min_y < super_scroll_view.contentOffset.y)
            {
                const CGPoint new_offset = CGPointMake(super_scroll_view.contentOffset.x, min_y);
                if (!points_equal(super_scroll_view.contentOffset, new_offset))
                {
                    if ([g_view.superview isKindOfClass:[UIStackView class]])
                    {
                        [super_scroll_view setContentOffset:new_offset animated:UIView.areAnimationsEnabled];
                    }
                    else
                    {
                        super_scroll_view.contentOffset = new_offset;
                    }
                }
            }
            super_scroll_view = find_responder<UIScrollView>(super_scroll_view);
        }
    }

    void will_hide_keyboard(NSNotification* notification)
    {
        set_animation_duration(notification.userInfo);

        if (g_last_scroll_view.window != nil)
        {
            [UIView animateWithDuration:g_animation_duration
                                  delay:0
                                options:UIViewAnimationOptionCurveEaseOut
                             animations:^{
                               animate_hiding_keyboard();
                             }
                             completion:nil];
        }

        if (g_is_keyboard_showing)
        {
            restore_position();
        }

        g_is_keyboard_showing = false;
        g_view = nil;
        g_last_scroll_view = nil;
        g_keyboard_frame = CGRectZero;
        g_starting_content_insets = UIEdgeInsetsZero;
        g_starting_scroll_indicator_insets = UIEdgeInsetsZero;
    }

    void did_hide_keyboard(NSNotification* /*notification*/)
    {
        g_is_keyboard_auto_scroll_handling = false;
        g_should_ignore_safe_area_adjustment = false;
        g_should_scroll_again = false;
    }

    // RestorePosition (932-977): animate the container origin back to its pre-keyboard value (unless the
    // device rotated while the keyboard was up — the stored origin belongs to the old orientation), clear
    // any leftover content insets, and reset the tracking state.
    void restore_position()
    {
        if (g_container_view != nil &&
            (g_container_view.frame.origin.x != g_top_view_begin_origin.x ||
             g_container_view.frame.origin.y != g_top_view_begin_origin.y) &&
            !points_equal(g_top_view_begin_origin, k_invalid_point))
        {
            const CGSize current_size = g_container_view.frame.size;
            constexpr CGFloat k_size_change_tolerance = 1.0F;
            const bool size_changed =
                !CGSizeEqualToSize(g_top_view_begin_container_size, CGSizeZero) &&
                (std::abs(current_size.width - g_top_view_begin_container_size.width) > k_size_change_tolerance ||
                 std::abs(current_size.height - g_top_view_begin_container_size.height) > k_size_change_tolerance);

            if (!size_changed)
            {
                CGRect rect = g_container_view.frame;
                rect.origin.x = g_top_view_begin_origin.x;
                rect.origin.y = g_top_view_begin_origin.y;
                [UIView animateWithDuration:g_animation_duration
                                      delay:0
                                    options:UIViewAnimationOptionCurveEaseOut
                                 animations:^{
                                   animate_root_view(rect);
                                 }
                                 completion:nil];
            }
        }

        if (g_scrolled_view != nil && !insets_equal(g_scrolled_view.contentInset, UIEdgeInsetsZero))
        {
            UIScrollView* const scrolled = g_scrolled_view;
            [UIView animateWithDuration:g_animation_duration
                                  delay:0
                                options:UIViewAnimationOptionCurveEaseOut
                             animations:^{
                               animate_inset(scrolled, UIEdgeInsetsZero, 0);
                             }
                             completion:nil];
        }

        if ([g_view isKindOfClass:[UIScrollView class]] && [g_view isKindOfClass:[UITextView class]] &&
            !insets_equal(((UIScrollView*)g_view).contentInset, UIEdgeInsetsZero))
        {
            UIScrollView* const editor_scroll = (UIScrollView*)g_view;
            [UIView animateWithDuration:g_animation_duration
                                  delay:0
                                options:UIViewAnimationOptionCurveEaseOut
                             animations:^{
                               animate_inset(editor_scroll, UIEdgeInsetsZero, 0);
                             }
                             completion:nil];
        }

        g_scrolled_view = nil;
        g_view = nil;
        g_container_view = nil;
        g_top_view_begin_origin = k_invalid_point;
        g_top_view_begin_container_size = CGSizeZero;
        g_has_cursor_rect = false;
        g_should_ignore_safe_area_adjustment = false;
        g_should_scroll_again = false;
    }

    // AdjustPosition (339-753): the geometry + animation core. See the file header for the staged
    // breakdown; this is a direct line-by-line port of the C# method.
    void adjust_position() // NOLINT-equivalent length is inherent to the ported algorithm
    {
        if (g_container_view == nil ||
            (![g_view isKindOfClass:[UITextField class]] && ![g_view isKindOfClass:[UITextView class]]) ||
            ![g_view isDescendantOfView:g_container_view])
        {
            g_is_keyboard_auto_scroll_handling = false;
            return;
        }

        if (points_equal(g_top_view_begin_origin, k_invalid_point))
        {
            g_top_view_begin_origin = CGPointMake(g_container_view.frame.origin.x, g_container_view.frame.origin.y);
            g_top_view_begin_container_size = g_container_view.frame.size;
        }

        CGPoint root_view_origin =
            CGPointMake(CGRectGetMinX(g_container_view.frame), CGRectGetMinY(g_container_view.frame));
        UIWindow* const window = g_container_view.window;
        if (window == nil)
        {
            g_is_keyboard_auto_scroll_handling = false;
            return;
        }

        const CGRect intersect_rect = CGRectIntersection(g_keyboard_frame, window.frame);
        const CGSize kb_size =
            CGRectIsNull(intersect_rect) ? CGSizeMake(g_keyboard_frame.size.width, 0) : intersect_rect.size;

        CGFloat navigation_bar_area_height = 0;
        if (UINavigationController* const nav = find_responder<UINavigationController>(g_view))
        {
            if ([g_view isDescendantOfView:nav.navigationBar])
            {
                g_is_keyboard_auto_scroll_handling = false;
                return;
            }
            navigation_bar_area_height = CGRectGetMaxY(nav.navigationBar.frame);
        }
        else
        {
            // Mirror C#'s `window.WindowScene?.StatusBarManager?.StatusBarFrame.Height ?? 0`.
            // Deployment floor is iOS 26, so no pre-iOS-13 UIApplication.statusBarFrame fallback
            // (deprecated; would be dead code). Just nil-guard the windowScene chain.
            CGFloat status_bar_height = 0;
            if (window.windowScene != nil && window.windowScene.statusBarManager != nil)
            {
                status_bar_height = window.windowScene.statusBarManager.statusBarFrame.size.height;
            }
            navigation_bar_area_height = status_bar_height;
        }

        const CGFloat top_layout_guide = std::max(navigation_bar_area_height, g_container_view.layoutMargins.top);

        if (!find_cursor_position(g_cursor_rect))
        {
            g_has_cursor_rect = false;
            g_is_keyboard_auto_scroll_handling = false;
            return;
        }
        g_has_cursor_rect = true;
        CGRect cursor_rect = g_cursor_rect;

        const CGRect view_rect_in_container = [g_container_view convertRect:g_view.frame fromView:g_view.superview];
        const CGRect view_rect_in_window = [g_container_view convertRect:view_rect_in_container toView:nil];

        // Pickers have a zero-height cursor; use the picker's own height instead.
        if (cursor_rect.size.height == 0)
        {
            cursor_rect.size.height = g_view.bounds.size.height;
        }

        const CGFloat keyboard_y_position =
            window.frame.size.height - kb_size.height - k_text_view_distance_from_bottom;

        CGFloat cursor_not_in_view_scroll = 0;
        CGFloat move = 0;
        bool cursor_too_high = false;
        const bool cursor_too_low = false;

        // Find the next parent ScrollView that is scrollable, or use the current View if it is one.
        UIScrollView* super_view_seed = find_responder<UIScrollView>(g_view);
        if (super_view_seed == nil && [g_view isKindOfClass:[UIScrollView class]])
        {
            super_view_seed = (UIScrollView*)g_view;
        }
        UIScrollView* super_scroll_view = find_parent_scroll(super_view_seed);

        bool has_super_scroll_rect = false;
        CGRect super_scroll_view_rect = CGRectZero;
        CGFloat top_boundary = top_layout_guide;
        CGFloat bottom_boundary = keyboard_y_position;

        if (super_scroll_view != nil)
        {
            const CGRect in_container = [g_container_view convertRect:super_scroll_view.frame
                                                             fromView:super_scroll_view.superview];
            super_scroll_view_rect = [g_container_view convertRect:in_container toView:nil];
            has_super_scroll_rect = true;
            top_boundary = std::max(top_boundary, CGRectGetMinY(super_scroll_view_rect));
            CGFloat super_scroll_view_bottom = CGRectGetMaxY(super_scroll_view_rect) - k_text_view_distance_from_bottom;
            if ([super_scroll_view isKindOfClass:[UITextView class]] &&
                CGRectGetMaxY(super_scroll_view_rect) - k_text_view_distance_from_bottom < CGRectGetMaxY(cursor_rect))
            {
                super_scroll_view_bottom = CGRectGetMaxY(super_scroll_view_rect);
            }
            bottom_boundary = std::min(bottom_boundary, super_scroll_view_bottom);
        }

        bool force_set_content_insets = true;

        if ([g_view isKindOfClass:[UITextView class]] && g_is_keyboard_showing &&
            cursor_rect.origin.y >= CGRectGetMaxY(view_rect_in_window))
        {
            move = CGRectGetMaxY(view_rect_in_window) - bottom_boundary;
        }
        else if ([g_view isKindOfClass:[UITextView class]] && g_is_keyboard_showing &&
                 cursor_rect.origin.y < CGRectGetMinY(view_rect_in_window))
        {
            move = CGRectGetMinY(view_rect_in_window) - bottom_boundary;
            if (move < 0)
            {
                move = 0;
            }
        }
        else if (CGRectGetMaxY(cursor_rect) > bottom_boundary && cursor_rect.origin.y > top_boundary)
        {
            move = CGRectGetMaxY(cursor_rect) - bottom_boundary;
        }
        else if (cursor_rect.origin.y <= top_boundary && CGRectGetMaxY(cursor_rect) <= bottom_boundary)
        {
            move = cursor_rect.origin.y - top_boundary;
        }
        else if (cursor_rect.origin.y <= top_boundary && CGRectGetMaxY(cursor_rect) >= bottom_boundary)
        {
            cursor_not_in_view_scroll = CGRectGetMinY(view_rect_in_window) - cursor_rect.origin.y;
            move = CGRectGetMaxY(cursor_rect) - bottom_boundary - cursor_not_in_view_scroll;
            cursor_too_high = true;
        }

        // Keyboard already showing and we tapped another field — restore the previous scroll view if there
        // is no current one.
        if (g_last_scroll_view != nil)
        {
            if (super_scroll_view == nil)
            {
                if (!insets_equal(g_last_scroll_view.contentInset, g_starting_content_insets))
                {
                    [UIView animateWithDuration:g_animation_duration
                                          delay:0
                                        options:UIViewAnimationOptionCurveEaseOut
                                     animations:^{
                                       animate_starting_last_scroll_view();
                                     }
                                     completion:nil];
                }
                if (!points_equal(g_last_scroll_view.contentOffset, g_starting_content_offset))
                {
                    if (find_responder<UIStackView>(g_view) != nil)
                    {
                        [g_last_scroll_view setContentOffset:g_starting_content_offset
                                                    animated:UIView.areAnimationsEnabled];
                    }
                    else
                    {
                        g_last_scroll_view.contentOffset = g_starting_content_offset;
                    }
                }
                g_starting_content_insets = UIEdgeInsetsZero;
                g_starting_scroll_indicator_insets = UIEdgeInsetsZero;
                g_starting_content_offset = CGPointZero;
                g_last_scroll_view = nil;
            }
        }
        else if (super_scroll_view != nil)
        {
            g_last_scroll_view = super_scroll_view;
            g_starting_content_insets = super_scroll_view.contentInset;
            g_starting_content_offset = super_scroll_view.contentOffset;
            g_starting_scroll_indicator_insets = super_scroll_view.verticalScrollIndicatorInsets;
        }

        // Calculate the move for the ScrollViews (the hierarchy walk).
        if (g_last_scroll_view != nil)
        {
            super_scroll_view = g_last_scroll_view;
            CGFloat inner_scroll_value = 0;
            CGFloat temp_move = 0;

            while (super_scroll_view != nil)
            {
                bool should_continue = false;

                if (cursor_not_in_view_scroll != 0)
                {
                    temp_move = move;
                    move = cursor_not_in_view_scroll;
                    should_continue = true;
                }
                else if (move > 0 || temp_move > 0)
                {
                    if (move == 0)
                    {
                        move = temp_move;
                    }
                    should_continue = move > -super_scroll_view.contentOffset.y - super_scroll_view.contentInset.top;
                }
                else if (UITableView* const table_view = find_responder<UITableView>(super_scroll_view))
                {
                    should_continue = super_scroll_view.contentOffset.y > 0;
                    UITableViewCell* const table_cell = find_responder<UITableViewCell>(g_view);
                    if (should_continue && table_cell != nil)
                    {
                        NSIndexPath* const index_path = [table_view indexPathForCell:table_cell];
                        NSIndexPath* const previous =
                            index_path != nil ? previous_index_path(table_view, index_path) : nil;
                        if (previous != nil)
                        {
                            const CGRect previous_cell_rect = [table_view rectForRowAtIndexPath:previous];
                            if (!CGRectIsEmpty(previous_cell_rect))
                            {
                                const CGRect in_root = [table_view convertRect:previous_cell_rect
                                                                        toView:g_container_view.superview];
                                move = std::min<CGFloat>(0, CGRectGetMaxY(in_root) - top_boundary);
                            }
                        }
                    }
                }
                else if (UICollectionView* const collection_view = find_responder<UICollectionView>(super_scroll_view))
                {
                    should_continue = super_scroll_view.contentOffset.y > 0;
                    UICollectionViewCell* const collection_cell = find_responder<UICollectionViewCell>(g_view);
                    if (should_continue && collection_cell != nil)
                    {
                        NSIndexPath* const index_path = [collection_view indexPathForCell:collection_cell];
                        NSIndexPath* const previous =
                            index_path != nil ? previous_index_path(collection_view, index_path) : nil;
                        UICollectionViewLayoutAttributes* const attributes =
                            previous != nil ? [collection_view layoutAttributesForItemAtIndexPath:previous] : nil;
                        if (attributes != nil)
                        {
                            const CGRect previous_cell_rect = attributes.frame;
                            if (!CGRectIsEmpty(previous_cell_rect))
                            {
                                const CGRect in_root = [collection_view convertRect:previous_cell_rect
                                                                             toView:g_container_view.superview];
                                move = std::min<CGFloat>(0, CGRectGetMaxY(in_root) - top_boundary);
                            }
                        }
                    }
                }
                else
                {
                    should_continue =
                        !(inner_scroll_value == 0 && cursor_rect.origin.y + cursor_not_in_view_scroll >= top_boundary &&
                          CGRectGetMaxY(cursor_rect) + cursor_not_in_view_scroll <= bottom_boundary);
                    if (cursor_rect.origin.y - inner_scroll_value < top_boundary && !cursor_too_high)
                    {
                        move = cursor_rect.origin.y - inner_scroll_value - top_boundary;
                    }
                    else if (cursor_rect.origin.y - inner_scroll_value > bottom_boundary && !cursor_too_low)
                    {
                        move = cursor_rect.origin.y - inner_scroll_value - bottom_boundary;
                    }
                }

                if (should_continue)
                {
                    force_set_content_insets = false;

                    UIScrollView* const temp_scroll_view = find_responder<UIScrollView>(super_scroll_view);
                    UIScrollView* const next_scroll_view = find_parent_scroll(temp_scroll_view);

                    UINavigationController* const nav_controller = find_responder<UINavigationController>(g_view);
                    const bool prefers_large_titles =
                        nav_controller != nil && nav_controller.navigationBar.prefersLargeTitles;
                    if (prefers_large_titles)
                    {
                        move = adjust_for_large_titles(move, super_scroll_view, nav_controller);
                    }

                    const CGFloat orig_content_offset_y = super_scroll_view.contentOffset.y;
                    const CGFloat should_offset_y =
                        super_scroll_view.contentOffset.y - std::min(super_scroll_view.contentOffset.y, -move);
                    const CGFloat requested_move = move;
                    move -= (should_offset_y - super_scroll_view.contentOffset.y);

                    CGPoint new_content_offset = CGPointMake(super_scroll_view.contentOffset.x, should_offset_y);

                    if ((!points_equal(super_scroll_view.contentOffset, new_content_offset) ||
                         inner_scroll_value != 0) &&
                        has_super_scroll_rect)
                    {
                        if ((next_scroll_view == nil && super_scroll_view_rect.origin.y + cursor_rect.size.height +
                                                                k_text_view_distance_from_bottom <
                                                            bottom_boundary) ||
                            cursor_not_in_view_scroll != 0)
                        {
                            UIScrollView* const animating_scroll_view = super_scroll_view;
                            const CGFloat captured_inner = inner_scroll_value;
                            [UIView animateWithDuration:g_animation_duration
                                                  delay:0
                                                options:UIViewAnimationOptionCurveEaseOut
                                             animations:^{
                                               CGPoint offset = new_content_offset;
                                               offset.y += captured_inner;
                                               g_scrolled_view = animating_scroll_view;
                                               if (find_responder<UIStackView>(g_view) != nil)
                                               {
                                                   [animating_scroll_view setContentOffset:offset
                                                                                  animated:UIView.areAnimationsEnabled];
                                               }
                                               else
                                               {
                                                   animating_scroll_view.contentOffset = offset;
                                               }
                                             }
                                             completion:nil];
                            inner_scroll_value = 0;

                            const CGFloat actual_scrolled = super_scroll_view.contentOffset.y - orig_content_offset_y;
                            const CGFloat amount_not_scrolled = requested_move - actual_scrolled;
                            if (prefers_large_titles && amount_not_scrolled > 1)
                            {
                                g_should_scroll_again = true;
                            }
                        }
                        else
                        {
                            inner_scroll_value += new_content_offset.y - super_scroll_view.contentOffset.y;
                        }
                    }

                    if (cursor_not_in_view_scroll != 0)
                    {
                        cursor_not_in_view_scroll = 0;
                    }
                    else
                    {
                        super_scroll_view = next_scroll_view;
                    }
                }
                else
                {
                    move += inner_scroll_value;
                    break;
                }
            }

            move += inner_scroll_value;

            // Adjust the parent's ContentInset.Bottom so we can still scroll to the top with the keyboard up.
            if (force_set_content_insets && super_scroll_view != nil)
            {
                apply_content_inset(super_scroll_view, g_last_scroll_view, false, false);
                if (super_scroll_view != g_view && [g_view isKindOfClass:[UITextView class]])
                {
                    apply_content_inset((UIScrollView*)g_view, (UIScrollView*)g_view, false, true);
                }
            }
            else
            {
                apply_content_inset(g_scrolled_view, g_last_scroll_view, true, false);
                if (g_scrolled_view != g_view && [g_view isKindOfClass:[UITextView class]])
                {
                    apply_content_inset((UIScrollView*)g_view, (UIScrollView*)g_view, true, true);
                }
            }
        }

        if (move >= 0)
        {
            root_view_origin.y = std::max(root_view_origin.y - move,
                                          std::min<CGFloat>(0, -kb_size.height - k_text_view_distance_from_bottom));
            if (g_container_view.frame.origin.x != root_view_origin.x ||
                g_container_view.frame.origin.y != root_view_origin.y)
            {
                g_should_ignore_safe_area_adjustment = true;
                CGRect rect = g_container_view.frame;
                rect.origin.x = root_view_origin.x;
                rect.origin.y = root_view_origin.y;
                [UIView animateWithDuration:g_animation_duration
                                      delay:0
                                    options:UIViewAnimationOptionCurveEaseOut
                                 animations:^{
                                   animate_root_view(rect);
                                 }
                                 completion:nil];
                if (g_last_scroll_view != nil)
                {
                    apply_content_inset(g_last_scroll_view, g_last_scroll_view, false, false);
                }
            }
        }
        else
        {
            root_view_origin.y -= move;
            if (g_container_view.frame.origin.x != root_view_origin.x ||
                g_container_view.frame.origin.y != root_view_origin.y)
            {
                CGRect rect = g_container_view.frame;
                rect.origin.x = root_view_origin.x;
                rect.origin.y = root_view_origin.y;
                [UIView animateWithDuration:g_animation_duration
                                      delay:0
                                    options:UIViewAnimationOptionCurveEaseOut
                                 animations:^{
                                   animate_root_view(rect);
                                 }
                                 completion:nil];
            }
        }
    }
} // namespace

namespace maui::core
{
    void keyboard_auto_manager::connect_scroll_handler()
    {
        // KeyboardAutoManagerScroll.Connect: idempotent — if already connected, do nothing.
        if (g_text_field_token != nil)
        {
            return;
        }
        NSNotificationCenter* const center = NSNotificationCenter.defaultCenter;
        g_text_field_token = [center addObserverForName:UITextFieldTextDidBeginEditingNotification
                                                 object:nil
                                                  queue:nil
                                             usingBlock:^(NSNotification* note) {
                                               did_text_begin_editing(note);
                                             }];
        g_text_view_token = [center addObserverForName:UITextViewTextDidBeginEditingNotification
                                                object:nil
                                                 queue:nil
                                            usingBlock:^(NSNotification* note) {
                                              did_text_begin_editing(note);
                                            }];
        g_will_show_token = [center addObserverForName:UIKeyboardWillShowNotification
                                                object:nil
                                                 queue:nil
                                            usingBlock:^(NSNotification* note) {
                                              will_keyboard_show(note);
                                            }];
        g_will_hide_token = [center addObserverForName:UIKeyboardWillHideNotification
                                                object:nil
                                                 queue:nil
                                            usingBlock:^(NSNotification* note) {
                                              will_hide_keyboard(note);
                                            }];
        g_did_hide_token = [center addObserverForName:UIKeyboardDidHideNotification
                                               object:nil
                                                queue:nil
                                           usingBlock:^(NSNotification* note) {
                                             did_hide_keyboard(note);
                                           }];
    }

    void keyboard_auto_manager::disconnect_scroll_handler()
    {
        // KeyboardAutoManagerScroll.Disconnect: remove every observer and clear its token so none dangles.
        NSNotificationCenter* const center = NSNotificationCenter.defaultCenter;
        if (g_will_show_token != nil)
        {
            [center removeObserver:g_will_show_token];
            g_will_show_token = nil;
        }
        if (g_will_hide_token != nil)
        {
            [center removeObserver:g_will_hide_token];
            g_will_hide_token = nil;
        }
        if (g_did_hide_token != nil)
        {
            [center removeObserver:g_did_hide_token];
            g_did_hide_token = nil;
        }
        if (g_text_field_token != nil)
        {
            [center removeObserver:g_text_field_token];
            g_text_field_token = nil;
        }
        if (g_text_view_token != nil)
        {
            [center removeObserver:g_text_view_token];
            g_text_view_token = nil;
        }
        g_is_keyboard_auto_scroll_handling = false;
    }
} // namespace maui::core
