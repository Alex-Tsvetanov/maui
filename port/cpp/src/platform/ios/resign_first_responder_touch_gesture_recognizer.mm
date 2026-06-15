// resign_first_responder_touch_gesture_recognizer — iOS (UIKit) impl, ported from
// Microsoft.Maui.Platform.ResignFirstResponderTouchGestureRecognizer
// (src/Controls/src/Core/ContentPage/HideSoftInputOnTappedChanged/
//  ResignFirstResponderTouchGestureRecognizer.iOS.cs). Compiled as Objective-C++ with ARC.
//
// The class is a UITapGestureRecognizer attached to the editing input's WINDOW. While a text input is the
// first responder, a tap anywhere on the window — except inside a text-input control or a UITableView /
// UITableViewCell (ShouldReceiveTouch) — resigns the input's first responder (hiding the soft keyboard),
// then removes itself. Editing-begin re-arms a fresh recognizer; editing-end / window removal tears it
// down. The target holds a WEAK ref so the recognizer never keeps the input alive (C#'s
// WeakReference<UIView>); the action-target token is removed on disconnect (C#'s Token cleanup).

#import <UIKit/UIKit.h>

#include "resign_first_responder_touch_gesture_recognizer.hpp"

#include <functional>

@interface MauiResignFirstResponderTouchGestureRecognizer : UITapGestureRecognizer <UIGestureRecognizerDelegate>
- (instancetype)initWithTargetView:(UIView*)targetView;
- (void)disconnect;
- (void)performResignAndDisconnect; // the OnTapped (state == Ended) body, exposed for the test seam
@end

namespace
{
    // C# ResignFirstResponderTouchGestureRecognizer.ViewAndSuperviewsOfView + OnShouldReceiveTouch: a tap
    // is rejected if its view (or any superview) is a UITableView / UITableViewCell or can itself become
    // first responder (i.e. is a text-input control we should not steal the tap from).
    BOOL should_receive_touch(UITouch* touch)
    {
        UIView* view = touch.view;
        while (view != nil)
        {
            if ([view isKindOfClass:[UITableView class]] || [view isKindOfClass:[UITableViewCell class]] ||
                [view canBecomeFirstResponder])
            {
                return NO;
            }
            view = view.superview;
        }
        return YES;
    }

    // C# Remove(UIWindow?): disconnect every ResignFirstResponderTouchGestureRecognizer currently on the
    // window (walked in reverse, as the C# loop does, since disconnect mutates the collection).
    void remove_from_window(UIWindow* window)
    {
        NSArray<UIGestureRecognizer*>* const recognizers = window.gestureRecognizers;
        if (recognizers == nil)
        {
            return;
        }
        for (NSInteger i = static_cast<NSInteger>(recognizers.count) - 1; i >= 0; --i)
        {
            UIGestureRecognizer* const recognizer = recognizers[static_cast<NSUInteger>(i)];
            if ([recognizer isKindOfClass:[MauiResignFirstResponderTouchGestureRecognizer class]])
            {
                [static_cast<MauiResignFirstResponderTouchGestureRecognizer*>(recognizer) disconnect];
            }
        }
    }

    // C# OnEditingDidBegin: remove any existing recognizer from the view's window, then arm a fresh one.
    void on_editing_did_begin(UIView* view)
    {
        if (view == nil || view.window == nil)
        {
            return;
        }
        remove_from_window(view.window);
        MauiResignFirstResponderTouchGestureRecognizer* const recognizer =
            [[MauiResignFirstResponderTouchGestureRecognizer alloc] initWithTargetView:view];
        [view.window addGestureRecognizer:recognizer];
    }

    // C# OnEditingDidEnd: tear down the recognizer on the (former) editing view's window.
    void on_editing_did_end(UIView* view)
    {
        if (view != nil)
        {
            remove_from_window(view.window);
        }
    }
} // namespace

// The Objective-C observer that bridges the UITextField/UIControl editing target-action and the
// UITextView begin/end notifications back to the static editing handlers. One per wired input; held alive
// by the cleanup closure (a strong capture) and released when that closure runs (C#'s event -= unhook).
@interface MauiResignFirstResponderEditingObserver : NSObject
@property(nonatomic, weak) UIView* trackedView;
- (instancetype)initWithView:(UIView*)view;
- (void)connect;
- (void)disconnect;
@end

@implementation MauiResignFirstResponderEditingObserver

- (instancetype)initWithView:(UIView*)view
{
    self = [super init];
    if (self != nil)
    {
        _trackedView = view;
    }
    return self;
}

// C# ConnectToPlatformEvents: UITextView uses the begin/end editing notifications, UIControl
// (UITextField) uses EditingDidBegin/EditingDidEnd target-action.
- (void)connect
{
    UIView* const view = self.trackedView;
    if ([view isKindOfClass:[UITextView class]])
    {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(textViewBegan:)
                                                     name:UITextViewTextDidBeginEditingNotification
                                                   object:view];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(textViewEnded:)
                                                     name:UITextViewTextDidEndEditingNotification
                                                   object:view];
    }
    else if ([view isKindOfClass:[UIControl class]])
    {
        UIControl* const control = static_cast<UIControl*>(view);
        [control addTarget:self action:@selector(editingDidBegin:) forControlEvents:UIControlEventEditingDidBegin];
        [control addTarget:self action:@selector(editingDidEnd:) forControlEvents:UIControlEventEditingDidEnd];
    }
}

// C# DisconnectFromPlatformEvents.
- (void)disconnect
{
    UIView* const view = self.trackedView;
    if ([view isKindOfClass:[UITextView class]])
    {
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:UITextViewTextDidBeginEditingNotification
                                                      object:view];
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:UITextViewTextDidEndEditingNotification
                                                      object:view];
    }
    else if ([view isKindOfClass:[UIControl class]])
    {
        UIControl* const control = static_cast<UIControl*>(view);
        [control removeTarget:self action:@selector(editingDidBegin:) forControlEvents:UIControlEventEditingDidBegin];
        [control removeTarget:self action:@selector(editingDidEnd:) forControlEvents:UIControlEventEditingDidEnd];
    }
}

- (void)editingDidBegin:(UIControl*)sender
{
    on_editing_did_begin(sender);
}

- (void)editingDidEnd:(UIControl*)sender
{
    on_editing_did_end(sender);
}

- (void)textViewBegan:(NSNotification*)note
{
    if ([note.object isKindOfClass:[UIView class]])
    {
        on_editing_did_begin(static_cast<UIView*>(note.object));
    }
}

- (void)textViewEnded:(NSNotification*)note
{
    if ([note.object isKindOfClass:[UIView class]])
    {
        on_editing_did_end(static_cast<UIView*>(note.object));
    }
}

@end

@implementation MauiResignFirstResponderTouchGestureRecognizer
{
    __weak UIView* _targetView; // C#'s WeakReference<UIView> _targetView
}

- (instancetype)initWithTargetView:(UIView*)targetView
{
    self = [super initWithTarget:self action:@selector(onTapped:)];
    if (self != nil)
    {
        // C# ctor: simultaneous recognition, receive every touch (filtered by ShouldReceiveTouch), and do
        // not swallow / delay touches from the views beneath.
        self.cancelsTouchesInView = NO;
        self.delaysTouchesEnded = NO;
        self.delaysTouchesBegan = NO;
        self.delegate = (id<UIGestureRecognizerDelegate>)self;
        _targetView = targetView;
    }
    return self;
}

// C# OnTapped: only on the Ended state; resign the (still-first-responder) target, then disconnect self.
- (void)onTapped:(MauiResignFirstResponderTouchGestureRecognizer*)recognizer
{
    if (recognizer.state != UIGestureRecognizerStateEnded)
    {
        return;
    }
    [self performResignAndDisconnect];
}

// C# OnTapped's body (the state == Ended branch): resign the (still-first-responder) target, then
// disconnect self. Split out so the test seam can drive it without synthesizing a UITouch.
- (void)performResignAndDisconnect
{
    UIView* const target = self->_targetView;
    if (target != nil && target.isFirstResponder)
    {
        [target resignFirstResponder];
    }
    [self disconnect];
}

// C# Disconnect: remove the action target (here: detach from its window) — ARC reclaims the recognizer
// once the window no longer retains it.
- (void)disconnect
{
    UIView* const target = self->_targetView;
    if (target != nil && [target.window isKindOfClass:[UIWindow class]])
    {
        [target.window removeGestureRecognizer:self];
    }
}

// C# ShouldRecognizeSimultaneously = (_, _) => true.
- (BOOL)gestureRecognizer:(UIGestureRecognizer*)gestureRecognizer
    shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer*)otherGestureRecognizer
{
    return YES;
}

// C# ShouldReceiveTouch = OnShouldReceiveTouch.
- (BOOL)gestureRecognizer:(UIGestureRecognizer*)gestureRecognizer shouldReceiveTouch:(UITouch*)touch
{
    return should_receive_touch(touch);
}

@end

namespace maui::platform::ios
{
    // C# static ResignFirstResponderTouchGestureRecognizer.Update(UIView).
    std::function<void()> resign_first_responder_update(UIView* ui_view)
    {
        if (ui_view == nil)
        {
            return {};
        }

        UIWindow* const window = ui_view.window;
        MauiResignFirstResponderEditingObserver* const observer =
            [[MauiResignFirstResponderEditingObserver alloc] initWithView:ui_view];

        if (![window isKindOfClass:[UIWindow class]])
        {
            // C# Update: no window -> DisconnectFromPlatformEvents (a fresh observer was never connected,
            // so disconnect is a harmless no-op) and return null.
            [observer disconnect];
            return {};
        }

        // C# ConnectToPlatformEvents -> false bail (only UITextField/UITextView/UIControl qualify).
        const BOOL is_input = [ui_view isKindOfClass:[UITextView class]] || [ui_view isKindOfClass:[UIControl class]];
        if (!is_input)
        {
            return {};
        }
        [observer connect];

        // C# Update: if already first responder, arm immediately.
        if (ui_view.isFirstResponder)
        {
            on_editing_did_begin(ui_view);
        }

        // C# Update's ActionDisposable: unhook the editing events + remove any armed recognizer. The
        // observer is captured strongly into the C++ closure so it lives exactly as long as the cleanup
        // token (the manager's _watchingForTaps) under ARC, then disconnects + is released. window is
        // captured by value (a strong ref) so the recognizer removal targets the same window even after the
        // input loses it.
        MauiResignFirstResponderEditingObserver* held_observer = observer;
        UIWindow* const held_window = window;
        return [held_observer, held_window]() mutable {
            [held_observer disconnect];
            remove_from_window(held_window);
            held_observer = nil;
        };
    }

    // Test seam: drive the tap-Ended branch on every armed recognizer of `window` (a snapshot is taken
    // first, since performResignAndDisconnect removes the recognizer from the window's live collection).
    int resign_first_responder_fire_for_testing(UIWindow* window)
    {
        if (![window isKindOfClass:[UIWindow class]] || window.gestureRecognizers == nil)
        {
            return 0;
        }
        int fired = 0;
        for (UIGestureRecognizer* const recognizer in [window.gestureRecognizers copy])
        {
            if ([recognizer isKindOfClass:[MauiResignFirstResponderTouchGestureRecognizer class]])
            {
                [static_cast<MauiResignFirstResponderTouchGestureRecognizer*>(recognizer) performResignAndDisconnect];
                ++fired;
            }
        }
        return fired;
    }
} // namespace maui::platform::ios
