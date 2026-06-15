#pragma once
// Shared iOS keyboard next-responder utilities — the port of the UIView-tree helpers behind
// Microsoft.Maui.Platform.KeyboardAutoManager.GoToNextResponderOrResign
// (src/Core/src/Platform/iOS/KeyboardAutoManager.cs) and the supporting
// Microsoft.Maui.Platform.ViewExtensions helpers (src/Core/src/Platform/iOS/ViewExtensions.cs:864-1043:
// FindResponder<T>, FindNextView (both overloads), FindNextInTableView / FindTableViewCellIndex,
// ChangeFocusedView, GetContainerView, FindDescendantView). Objective-C++ only — include exclusively
// from .mm files compiled as Objective-C++.
//
// These are PURE UIKit view-tree walks with no virtual-view coupling: GoToNextResponderOrResign starts
// at the editing UIView, checks if the view is eligible for a "Next" jump (a UITextField whose
// ReturnKeyType == Next, or any UITextView), and — if so — walks the sibling/descendant tree from the
// container to find the next editable field, focusing it (BecomeFirstResponder); otherwise it resigns
// first responder. This is the return-key "next-responder walk" the entry handler invokes from
// textFieldShouldReturn.

#import <UIKit/UIKit.h>

namespace maui::platform::ios
{
    // ViewExtensions.FindResponder<T> (864-882): walk the nextResponder chain for the first responder of
    // type T, bailing if a UIView in the chain has no Window (the disposed-superview guard).
    template <typename T> T* find_responder(UIView* view)
    {
        UIResponder* next_responder = view;
        while (next_responder != nil)
        {
            // "We check for Window to avoid scenarios where an invalidate might propagate up the tree to a
            // SuperView that's been disposed which will cause a crash when trying to access it."
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

    // ViewExtensions.FindTableViewCellIndex (990-1001): the VisibleCells index of the cell containing
    // `view`, or -1 if none.
    inline NSInteger find_table_view_cell_index(UIView* view, UITableView* table)
    {
        NSArray<UITableViewCell*>* const cells = table.visibleCells;
        UITableViewCell* const view_cell = find_responder<UITableViewCell>(view);
        for (NSUInteger i = 0; i < cells.count; ++i)
        {
            if (cells[i] == view_cell)
            {
                return static_cast<NSInteger>(i);
            }
        }
        return -1;
    }

    // ViewExtensions.FindNextView(view, index, isValidType) (952-976): search `view`'s siblings (or, for a
    // UITableView, its VisibleCells) from `index`, descending into the first non-hidden child branch
    // before considering the sibling itself. Declared ahead of the table helper that calls it.
    template <typename Predicate>
    UIView* find_next_view_from_index(UIView* view, NSInteger index, Predicate is_valid_type);

    // ViewExtensions.FindNextInTableView (978-988): when `view` itself is valid, continue the search from
    // the cell AFTER the one holding it (table/collection cells may not be in subview order).
    template <typename Predicate>
    UIView* find_next_in_table_view(UIView* view, UITableView* table, Predicate is_valid_type)
    {
        if (is_valid_type(view))
        {
            const NSInteger index = find_table_view_cell_index(view, table);
            return index == -1 ? nil : find_next_view_from_index(table, index + 1, is_valid_type);
        }
        return nil;
    }

    template <typename Predicate>
    UIView* find_next_view_from_index(UIView* view, NSInteger index, Predicate is_valid_type)
    {
        // search through the view's siblings and traverse down their branches; a UITableView searches its
        // VisibleCells instead of its raw subviews (cells may not be in subview order).
        NSArray<UIView*>* siblings = nil;
        if ([view isKindOfClass:[UITableView class]])
        {
            siblings = ((UITableView*)view).visibleCells;
        }
        else
        {
            siblings = view.superview.subviews;
        }

        if (siblings == nil)
        {
            return nil;
        }

        for (NSUInteger i = (index < 0 ? 0 : static_cast<NSUInteger>(index)); i < siblings.count; ++i)
        {
            UIView* const sibling = siblings[i];
            if (!sibling.hidden && sibling.subviews.count > 0)
            {
                UIView* const child_val = find_next_view_from_index(sibling.subviews[0], 0, is_valid_type);
                if (child_val != nil)
                {
                    return child_val;
                }
            }
            if (is_valid_type(sibling))
            {
                return sibling;
            }
        }
        return nil;
    }

    // ViewExtensions.FindNextView(view, containerView, isValidType) (920-950): the outer walk — climb from
    // `view` to `containerView`, at each level searching the siblings after `view` (or, inside a table,
    // the cells after this cell's), then climbing to the superview. If nothing is found before reaching
    // the container, restart the search from the container's first subview (wrap to the top).
    template <typename Predicate> UIView* find_next_view(UIView* view, UIView* container_view, Predicate is_valid_type)
    {
        UIView* next_view = nil;

        while (view != nil && view != container_view && next_view == nil)
        {
            NSArray<UIView*>* const siblings = view.superview.subviews;
            if (siblings == nil)
            {
                break;
            }

            // TableView and ListView cells may not be in order so handle separately.
            if (UITableView* const table_view = find_responder<UITableView>(view))
            {
                next_view = find_next_in_table_view(view, table_view, is_valid_type);
                if (next_view == nil)
                {
                    view = table_view;
                }
            }
            else
            {
                const NSUInteger current_index = [siblings indexOfObject:view];
                // NSNotFound + 1 wraps; C# IndexOf returns -1 -> +1 == 0. Mirror that floor at 0.
                const NSInteger start = (current_index == NSNotFound) ? 0 : static_cast<NSInteger>(current_index) + 1;
                next_view = find_next_view_from_index(view, start, is_valid_type);
            }

            view = view.superview;
        }

        // if we did not find the next view, try to find the first one (wrap to the top of the container).
        if (next_view == nil && container_view.subviews.count > 0)
        {
            next_view = find_next_view_from_index(container_view.subviews[0], 0, is_valid_type);
        }

        return next_view;
    }

    // ViewExtensions.ChangeFocusedView (1003-1010): focus the next field, or resign if there is none.
    inline void change_focused_view(UIView* view, UIView* new_view)
    {
        if (new_view == nil)
        {
            [view resignFirstResponder];
        }
        else
        {
            [new_view becomeFirstResponder];
        }
    }

    // ViewExtensions.GetContainerView (1012-1025): the C# original walks the responder chain for a
    // ContainerViewController and returns its View, falling back to the top UIViewController's content
    // ContentView. The port's iOS page host is a plain UIView under the window's rootViewController.view
    // (there is no ContainerViewController type), so the faithful equivalent is the highest non-window
    // ancestor of the editing view — the content root the input lives in. Used only by the scroll engine
    // and by GoToNextResponderOrResign when no explicit container is supplied; the next-responder tests
    // always pass the container explicitly, so this fallback is exercised by the scroll-engine path.
    inline UIView* get_container_view(UIView* starting_point)
    {
        if (starting_point == nil)
        {
            return nil;
        }
        // Prefer the window's root content view when the view is in a window.
        if (UIWindow* const window = starting_point.window)
        {
            if (UIViewController* const root = window.rootViewController; root != nil && root.viewIfLoaded != nil)
            {
                return root.viewIfLoaded;
            }
        }
        // Otherwise climb to the topmost superview (below the window).
        UIView* container = starting_point;
        while (container.superview != nil && ![container.superview isKindOfClass:[UIWindow class]])
        {
            container = container.superview;
        }
        return container;
    }

    // KeyboardAutoManager.CheckIfEligible (41-49): a UITextField is eligible only when its ReturnKeyType
    // is Next; a UITextView is always eligible (its return key cannot otherwise advance focus).
    inline bool check_if_eligible(UIView* view)
    {
        if ([view isKindOfClass:[UITextField class]])
        {
            return ((UITextField*)view).returnKeyType == UIReturnKeyNext;
        }
        if ([view isKindOfClass:[UITextView class]])
        {
            return true;
        }
        return false;
    }

    // KeyboardAutoManager.GoToNextResponderOrResign (15-39): if the editing view is not eligible for a
    // "Next" jump, resign first responder. Otherwise resolve the container (explicit override or
    // GetContainerView), then walk for the next editable field — a UITextView that is Editable +
    // UserInteractionEnabled, or a UITextField that is Enabled — skipping hidden / zero-alpha views, and
    // focus it (or resign if none is found).
    inline void go_to_next_responder_or_resign(UIView* view, UIView* custom_super_view = nil)
    {
        if (!check_if_eligible(view))
        {
            [view resignFirstResponder];
            return;
        }

        UIView* const superview = custom_super_view != nil ? custom_super_view : get_container_view(view);
        if (superview == nil)
        {
            [view resignFirstResponder];
            return;
        }

        UIView* const next_field = find_next_view(view, superview, [](UIView* candidate) {
            const bool is_valid_text_view = [candidate isKindOfClass:[UITextView class]] &&
                                            ((UITextView*)candidate).editable && candidate.userInteractionEnabled;
            const bool is_valid_text_field =
                [candidate isKindOfClass:[UITextField class]] && ((UITextField*)candidate).enabled;
            return (is_valid_text_view || is_valid_text_field) && !candidate.hidden && candidate.alpha != 0.0F;
        });

        change_focused_view(view, next_field);
    }
} // namespace maui::platform::ios
