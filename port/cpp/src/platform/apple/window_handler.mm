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
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "apple_menu_ops.hpp" // --- chrome (W1-11): the shared NSMenu builder ---
#include "maui/core/dimension.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_menu_bar.hpp"      // --- chrome (W1-11) ---
#include "maui/core/i_menu_bar_item.hpp" // --- chrome (W1-11) ---
#include "maui/core/i_menu_element.hpp"  // --- chrome (W1-11) ---
#include "maui/core/i_title_bar.hpp"     // --- chrome (W1-11) ---
#include "maui/core/i_toolbar.hpp"       // --- chrome (W1-11) ---
#include "maui/core/i_toolbar_item.hpp"  // --- chrome (W1-11) ---
#include "maui/core/i_view.hpp"          // --- chrome (W1-11): the title bar content's native view ---
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

// --- chrome (W1-11): the NSToolbarDelegate trampoline. Built EAGERLY from the i_toolbar's items (the
// primary items as NSToolbarItems; the secondary/overflow items as one trailing NSMenuToolbarItem whose
// menu apple_menu_ops builds — the Toolbar.Windows.cs primary/SecondaryCommands split translated to
// AppKit), then served to the NSToolbar through the delegate methods. Item clicks route back through
// i_menu_element::send_clicked (the represented element is borrowed; the controls own it). The eager
// array also makes the chrome ASSERTABLE without showing the window (NSToolbar materializes its items
// lazily on display). ---
@interface MauiToolbarDelegate : NSObject <NSToolbarDelegate>
@property(nonatomic, strong) NSArray<NSToolbarItem*>* items;
@property(nonatomic, strong) NSArray<NSToolbarItemIdentifier>* identifiers;
- (void)rebuildFromToolbar:(maui::core::i_toolbar*)toolbar;
- (void)itemClicked:(NSToolbarItem*)sender;
@end

@implementation MauiToolbarDelegate
{
    // The element behind each identifier (borrowed; index-aligned with _items).
    std::vector<maui::core::i_menu_element*> _elements;
}

- (void)rebuildFromToolbar:(maui::core::i_toolbar*)toolbar
{
    NSMutableArray<NSToolbarItem*>* const items = [NSMutableArray array];
    NSMutableArray<NSToolbarItemIdentifier>* const identifiers = [NSMutableArray array];
    _elements.clear();

    std::vector<maui::core::i_menu_element*> secondary;
    for (std::size_t i = 0; toolbar != nullptr && i < toolbar->item_count(); ++i)
    {
        maui::core::i_toolbar_item* const item = toolbar->item_at(i);
        if (item == nullptr)
        {
            continue;
        }
        if (item->is_secondary())
        {
            secondary.push_back(item); // → the overflow menu (Toolbar.Windows SecondaryCommands)
            continue;
        }
        NSString* const identifier = [NSString stringWithFormat:@"maui-toolbar-item-%zu", i];
        NSToolbarItem* const native = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];
        const std::string text(item->text());
        NSString* const label = [NSString stringWithUTF8String:text.c_str()];
        native.label = label != nil ? label : @"";
        native.enabled = static_cast<BOOL>(item->is_enabled());
        native.target = self;
        native.action = @selector(itemClicked:);
        [items addObject:native];
        [identifiers addObject:identifier];
        _elements.push_back(item);
    }
    if (!secondary.empty())
    {
        // One trailing overflow item carrying the secondary items as a menu (NSMenuToolbarItem).
        NSMenuToolbarItem* const overflow = [[NSMenuToolbarItem alloc] initWithItemIdentifier:@"maui-toolbar-overflow"];
        overflow.label = NSLocalizedString(@"More", @"the toolbar overflow (secondary items) label");
        overflow.menu = maui::platform::apple::build_menu(@"", secondary);
        [items addObject:overflow];
        [identifiers addObject:@"maui-toolbar-overflow"];
        _elements.push_back(nullptr); // index-aligned placeholder (the menu routes its own clicks)
    }
    self.items = items;
    self.identifiers = identifiers;
}

- (void)itemClicked:(NSToolbarItem*)sender
{
    const NSUInteger index = [self.items indexOfObject:sender];
    if (index == NSNotFound || index >= _elements.size())
    {
        return;
    }
    if (maui::core::i_menu_element* const element = _elements[index])
    {
        element->send_clicked(); // ToolbarItem click → MenuItem.Activate → clicked
    }
}

- (NSArray<NSToolbarItemIdentifier>*)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar
{
    (void)toolbar;
    return self.identifiers != nil ? self.identifiers : @[];
}

- (NSArray<NSToolbarItemIdentifier>*)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar
{
    (void)toolbar;
    return self.identifiers != nil ? self.identifiers : @[];
}

- (NSToolbarItem*)toolbar:(NSToolbar*)toolbar
        itemForItemIdentifier:(NSToolbarItemIdentifier)itemIdentifier
    willBeInsertedIntoToolbar:(BOOL)flag
{
    (void)toolbar;
    (void)flag;
    // An index loop, not Obj-C fast enumeration (which clang-tidy's init-variables check misreads as
    // uninitialized — the same workaround host_current uses).
    for (NSUInteger i = 0; i < self.items.count; ++i)
    {
        NSToolbarItem* const item = self.items[i];
        if ([item.itemIdentifier isEqualToString:itemIdentifier])
        {
            return item;
        }
    }
    return nil;
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
            // What disconnect() does, for the handler that is destroyed without one.
            if (native != nullptr)
            {
                as_window(native).delegate = nil;
            }
            ((__bridge MauiWindowDelegate*)notification_trampoline).window =
                nullptr;                        // the back-pointer live_view re-reads after user code
            CFRelease(notification_trampoline); // balances the __bridge_retained in connect()
            notification_trampoline = nullptr;
        }
        // --- chrome (W1-11): release the retained chrome slots (each balances a __bridge_retained in
        // the apply_* below). ---
        if (chrome_toolbar != nullptr)
        {
            CFRelease(chrome_toolbar);
            chrome_toolbar = nullptr;
        }
        if (chrome_toolbar_delegate != nullptr)
        {
            CFRelease(chrome_toolbar_delegate);
            chrome_toolbar_delegate = nullptr;
        }
        if (chrome_main_menu != nullptr)
        {
            CFRelease(chrome_main_menu);
            chrome_main_menu = nullptr;
        }
        if (chrome_menu_target != nullptr)
        {
            CFRelease(chrome_menu_target);
            chrome_menu_target = nullptr;
        }
        if (chrome_title_bar != nullptr)
        {
            CFRelease(chrome_title_bar);
            chrome_title_bar = nullptr;
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

    // --- chrome (W1-11): the AppKit chrome recipes -----------------------------------------------------

    // MapToolbar → a REAL NSToolbar on the NSWindow: the delegate trampoline eagerly builds the
    // NSToolbarItems (primary) + the overflow NSMenuToolbarItem (secondary) from the i_toolbar and
    // serves them to the toolbar. A null/invisible toolbar detaches it (window.toolbar = nil).
    void window_handler::apply_toolbar(i_toolbar* toolbar) const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->hosted_toolbar = toolbar; // keep the portable mirror in sync (tests read it)
        NSWindow* const window = as_window(platform->native);
        if (toolbar == nullptr || !toolbar->is_visible())
        {
            window.toolbar = nil;
            // Release the now-detached chrome (the delegate keeps borrowed i_toolbar_item pointers —
            // dropping it here keeps nothing stale alive while the toolbar is hidden).
            if (platform->chrome_toolbar != nullptr)
            {
                CFRelease(platform->chrome_toolbar);
                platform->chrome_toolbar = nullptr;
            }
            if (platform->chrome_toolbar_delegate != nullptr)
            {
                CFRelease(platform->chrome_toolbar_delegate);
                platform->chrome_toolbar_delegate = nullptr;
            }
            return;
        }
        MauiToolbarDelegate* delegate = nil;
        if (platform->chrome_toolbar_delegate == nullptr)
        {
            delegate = [[MauiToolbarDelegate alloc] init];
            platform->chrome_toolbar_delegate = (__bridge_retained void*)delegate;
        }
        else
        {
            delegate = (__bridge MauiToolbarDelegate*)platform->chrome_toolbar_delegate;
        }
        [delegate rebuildFromToolbar:toolbar];
        // Rebuild the NSToolbar whole on every change (an attached NSToolbar does not re-query its
        // delegate for an item-set change; whole-rebuild is the same strategy the menu chrome uses).
        if (platform->chrome_toolbar != nullptr)
        {
            CFRelease(platform->chrome_toolbar);
            platform->chrome_toolbar = nullptr;
        }
        NSToolbar* const native = [[NSToolbar alloc] initWithIdentifier:@"maui-window-toolbar"];
        native.delegate = delegate; // NSToolbar holds its delegate weakly — the slot above retains it
        platform->chrome_toolbar = (__bridge_retained void*)native;
        window.toolbar = native;
    }

    // MapMenuBar → a REAL NSMenu main menu: each i_menu_bar_item becomes a top-level NSMenuItem whose
    // submenu apple_menu_ops builds from the drop-down elements (separators / sub-menus / accelerators /
    // click routing included). Assigned to NSApp.mainMenu when an app instance exists (the unit tests
    // assert the BUILT menu through the retained slot instead — NSApp.mainMenu needs a running app).
    void window_handler::apply_menu_bar(i_menu_bar* menu_bar) const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->hosted_menu_bar = menu_bar; // keep the portable mirror in sync
        if (platform->chrome_main_menu != nullptr)
        {
            CFRelease(platform->chrome_main_menu);
            platform->chrome_main_menu = nullptr;
        }
        if (menu_bar == nullptr)
        {
            if (NSApp != nil)
            {
                NSApp.mainMenu = nil;
            }
            return;
        }
        NSMenu* const main = [[NSMenu alloc] initWithTitle:@""];
        main.autoenablesItems = NO;
        for (std::size_t i = 0; i < menu_bar->item_count(); ++i)
        {
            const i_menu_bar_item* const bar_item = menu_bar->item_at(i);
            if (bar_item == nullptr)
            {
                continue;
            }
            const std::string text(bar_item->text());
            NSString* const title = [NSString stringWithUTF8String:text.c_str()];
            NSMenuItem* const top = [[NSMenuItem alloc] initWithTitle:title != nil ? title : @""
                                                               action:nil
                                                        keyEquivalent:@""];
            top.enabled = static_cast<BOOL>(bar_item->is_enabled());
            std::vector<i_menu_element*> children;
            children.reserve(bar_item->item_count());
            for (std::size_t child = 0; child < bar_item->item_count(); ++child)
            {
                children.push_back(bar_item->item_at(child));
            }
            top.submenu = maui::platform::apple::build_menu(top.title, children);
            [main addItem:top];
        }
        platform->chrome_main_menu = (__bridge_retained void*)main;
        if (NSApp != nil)
        {
            NSApp.mainMenu = main;
        }
    }

    // MapTitleBar → an NSTitlebarAccessoryViewController: a small accessory view carrying the title (+
    // subtitle) text — or the title bar's custom Content native view when it has one — pinned to the
    // titlebar area. Replacing removes the previous accessory; null clears it. (The C# TitleBar maps on
    // Windows + Mac Catalyst; this is the AppKit-basics translation — STATUS.md.)
    void window_handler::apply_title_bar(i_title_bar* title_bar) const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->hosted_title_bar = title_bar; // keep the portable mirror in sync
        NSWindow* const window = as_window(platform->native);
        // Remove + release the previous accessory (a replace or a clear).
        if (platform->chrome_title_bar != nullptr)
        {
            auto* const previous = (__bridge NSTitlebarAccessoryViewController*)platform->chrome_title_bar;
            [previous removeFromParentViewController];
            CFRelease(platform->chrome_title_bar);
            platform->chrome_title_bar = nullptr;
        }
        if (title_bar == nullptr)
        {
            return;
        }
        NSTitlebarAccessoryViewController* const accessory = [[NSTitlebarAccessoryViewController alloc] init];
        NSView* content = nil;
        if (title_bar->content() != nullptr)
        {
            // Host the custom Content's native view when its handler is attached.
            if (auto* content_handler = dynamic_cast<i_view_handler*>(title_bar->content()->handler().get()))
            {
                content = (__bridge NSView*)content_handler->native_view();
            }
        }
        if (content == nil)
        {
            // The basics: one line of "Title — Subtitle" text (TitleBar.Title/Subtitle).
            const std::string title_text(title_bar->title());
            const std::string subtitle_text(title_bar->subtitle());
            std::string combined = title_text;
            if (!subtitle_text.empty())
            {
                combined += combined.empty() ? subtitle_text : (" — " + subtitle_text);
            }
            NSString* const value = [NSString stringWithUTF8String:combined.c_str()];
            NSTextField* const field = [NSTextField labelWithString:value != nil ? value : @""];
            content = field;
        }
        accessory.view = content;
        accessory.layoutAttribute = NSLayoutAttributeBottom; // a strip under the standard title bar
        [window addTitlebarAccessoryViewController:accessory];
        platform->chrome_title_bar = (__bridge_retained void*)accessory;
    }
} // namespace maui::core
