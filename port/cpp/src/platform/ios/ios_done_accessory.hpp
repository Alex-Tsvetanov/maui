#pragma once
// Shared iOS input-accessory builder for the picker family — the port of
// Microsoft.Maui.Platform.MauiDoneAccessoryView (src/Core/src/Platform/iOS/MauiDoneAccessoryView.cs):
// a UIToolbar whose right-aligned Done bar button fires the given target-action (each picker's
// FinishSelectItem / SetVirtualViewDate / SetVirtualViewTime commit). Objective-C++ only — include
// exclusively from .mm files compiled as Objective-C++.
//
// The caller keeps the target alive (bar-button targets are weak) — the picker partials retain it
// via an associated object on the text field, the same pattern as the control trampolines.

#import <UIKit/UIKit.h>

namespace maui::platform::ios
{
    inline UIToolbar* make_done_accessory(id target, SEL action)
    {
        UIToolbar* const toolbar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, 320, 44)];
        UIBarButtonItem* const spacer =
            [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                          target:nil
                                                          action:nil];
        UIBarButtonItem* const done =
            [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:target action:action];
        toolbar.items = @[ spacer, done ];
        return toolbar;
    }
} // namespace maui::platform::ios
