// window_handler — Apple (AppKit / macOS) platform recipe: a real NSWindow host. The real-native twin of
// the headless partial. The managed platform view is an NSWindow (held, retained, in
// window_platform::native); on connect the handler sets the NSWindow's contentView to the root page's
// native view, pushes the title, and installs an NSWindowDelegate trampoline that maps the native window
// notifications back to the window lifecycle:
//   NSWindowDidBecomeMainNotification → i_window::send_activated()
//   NSWindowWillCloseNotification     → i_window::send_destroying()
//
// Translated from WindowHandler.iOS.cs (UIKit's UIWindow): mainline MAUI's macOS is Mac Catalyst (UIKit),
// so there is no AppKit WindowHandler in the read-only C# source to port verbatim — the cross-platform
// contract (i_window, the mapper: MapTitle/MapContent/MapX/Y/Width/Height) is faithful, and the AppKit
// specifics are the standard NSWindow equivalents of the UIWindow recipe (contentView ← root view
// controller's view; window.title ← Title; setFrame: ← X/Y/Width/Height; the activated/will-close
// notifications stand in for UIKit's didBecomeVisible / scene lifecycle). The NSWindow is created but NOT
// shown here (AppKit objects construct without being ordered front) — the sample app orders it front.
// Compiled as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <cmath>
#include <memory>
#include <string>

#include "maui/core/dimension.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"

// Obj-C trampoline: an NSWindowDelegate that forwards the native window's main/close notifications to the
// C++ handler's virtual view (the i_window). Mirrors entry_handler.mm's MauiEntryDelegate pattern — the
// window holds its delegate weakly, so the cross-platform window_platform retains this object in its
// notification_trampoline slot for the window's lifetime.
@interface MauiWindowDelegate : NSObject <NSWindowDelegate>
@property(nonatomic) maui::core::i_window* window;
@end

@implementation MauiWindowDelegate
- (void)windowDidBecomeMain:(NSNotification*)notification
{
    (void)notification;
    if (self.window != nullptr)
    {
        self.window->send_activated(); // NSWindowDidBecomeMainNotification → IWindow.Activated
    }
}

// NOTE: windowDidResignMain is intentionally NOT mapped to send_deactivated. The port's send_deactivated
// drives the page Disappearing/Unloaded (the M5c windowed-appearing model), but C# Window.cs only Disappears
// the page on Destroying — not on a mere loss of main-window focus. Mapping resign-main here would Disappear
// the page every time the user switches apps. Deactivation therefore flows only through send_destroying
// (will-close), matching the WindowHandler.iOS scope (become-main + will-close).

- (void)windowWillClose:(NSNotification*)notification
{
    (void)notification;
    if (self.window != nullptr)
    {
        self.window->send_destroying(); // NSWindowWillCloseNotification → IWindow.Destroying
    }
}
@end

namespace
{
    NSWindow* as_window(void* native)
    {
        return (__bridge NSWindow*)native;
    }

    // The root page's native NSView, via its view-handler's native_view() (nil if the page is unattached or
    // its handler has no native view). Mirrors content_page_handler.mm's native_child helper.
    NSView* page_native_view(maui::core::i_element& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge NSView*)handler->native_view();
    }
} // namespace

namespace maui::core
{
    window_platform::~window_platform()
    {
        if (notification_trampoline != nullptr)
        {
            CFRelease(notification_trampoline); // balances the __bridge_retained in connect()
            notification_trampoline = nullptr;
        }
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    std::unique_ptr<window_platform> window_handler::create_platform_view()
    {
        auto platform = std::make_unique<window_platform>();
        // A titled / closable / resizable window. Created but NOT ordered front (AppKit objects construct
        // without being shown — the host app calls makeKeyAndOrderFront:).
        const NSWindowStyleMask style =
            NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
        NSWindow* const window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 0, 0)
                                                             styleMask:style
                                                               backing:NSBackingStoreBuffered
                                                                 defer:NO];
        platform->native = (__bridge_retained void*)window; // the void* slot owns one reference
        return platform;
    }

    // C# ConnectHandler: install the notification trampoline (the NSWindowDelegate) + host the page. The
    // mapper that follows (set_virtual_view) pushes the title/geometry; the content is hosted here AND by
    // the "content" property map (host_content is idempotent — it just re-sets the contentView).
    void window_handler::connect()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSWindow* const window = as_window(platform->native);
        MauiWindowDelegate* const delegate = [[MauiWindowDelegate alloc] init];
        delegate.window = window_view_;
        window.delegate = delegate;                                            // NSWindow holds the delegate weakly...
        platform->notification_trampoline = (__bridge_retained void*)delegate; // ...so retain it here.
    }

    void window_handler::disconnect() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        as_window(platform->native).delegate = nil;
        if (platform->notification_trampoline != nullptr)
        {
            CFRelease(platform->notification_trampoline);
            platform->notification_trampoline = nullptr;
        }
    }

    void window_handler::host_content()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSWindow* const window = as_window(platform->native);
        platform->content_hosted = false;
        if (window_view_ == nullptr)
        {
            return;
        }
        // C# MapContent: RootViewController = window.Content.ToUIViewController(...). The AppKit analog is
        // contentView = the page's native view. The page (a content_page) is an i_element whose view-handler
        // owns the real NSView.
        auto* page = window_view_->content();
        if (page == nullptr)
        {
            window.contentView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)]; // empty host
            return;
        }
        if (NSView* const view = page_native_view(*page))
        {
            window.contentView = view;
            platform->content_hosted = true;
        }
    }

    void window_handler::apply_title()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || window_view_ == nullptr)
        {
            return;
        }
        const std::string title(window_view_->title());
        NSString* const value = [NSString stringWithUTF8String:title.c_str()];
        as_window(platform->native).title = value != nil ? value : @"";
        platform->title = title; // keep the mirror in sync (tests read it)
    }

    void window_handler::apply_frame()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr || window_view_ == nullptr)
        {
            return;
        }
        // Only size/move when the geometry is explicitly set (a NaN coordinate is Dimension.Unset — leave
        // the window where AppKit placed it). The window expresses its frame in screen coordinates.
        const double x = window_view_->x();
        const double y = window_view_->y();
        const double w = window_view_->width();
        const double h = window_view_->height();
        if (std::isnan(x) || std::isnan(y) || std::isnan(w) || std::isnan(h))
        {
            return;
        }
        [as_window(platform->native) setFrame:NSMakeRect(x, y, w, h) display:YES];
    }
} // namespace maui::core
