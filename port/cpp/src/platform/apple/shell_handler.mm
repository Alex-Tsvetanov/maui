// shell_handler — Apple (AppKit / macOS) platform recipe: an NSSplitViewController whose SIDEBAR item is
// the flyout list and whose content item hosts an NSTabView of the shell_items' current pages. Realizes
// the W2-21 shell model adapted to AppKit:
//
//   NSSplitViewController                 (the container)
//     ├─ sidebar NSSplitViewItem          (the FLYOUT list — one NSTableView row per flyout item)
//     └─ content NSSplitViewItem
//          └─ NSTabView                    (one NSTabViewItem per visible shell_section of the current
//                                           item — the tab host; the selected tab = the current section)
//                                          each tab's view = the section's CURRENT (top-most) page view
//
// VISUAL DEVIATIONS (documented — AppKit has no UIKit equivalents):
//   - NO pan-presented DRAWER. AppKit has no slide-over drawer, so the flyout is a persistent SIDEBAR
//     (an NSSplitViewItem). FlyoutIsPresented drives the sidebar item's `collapsed` (open = visible)
//     instead of a slide animation — the flyout_page_handler.mm precedent.
//   - NO per-section NAVIGATION CONTROLLER. AppKit has no NSNavigationController, so each section tab
//     hosts only its CURRENT (top-most) page's view (the section's vc_stack.back()); the pushed-page
//     STACK is recorded in the shell_render_tree mirror (assertable) but the visible chrome shows the
//     top page only — a back swap, not an animated push. This matches the navigation_page_handler.mm
//     AppKit choice (a custom container, no UINavigationController).
//   - the native→virtual flyout-presented sync (a user collapse of the sidebar) is not observed back
//     (one-way virtual→native), same documented deviation as flyout_page_handler.mm.
//
// Compiled as Objective-C++ with ARC only for the `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/search_handler.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// AppKit trampoline for the shell search box: an NSSearchField's editing notifications + search action
// route to the C++ search_handler model. AppKit DEVIATION (documented in shell_handler.hpp): macOS has no
// UISearchController, so the search field is added as a subview above the tab content rather than installed
// into a navigation item.
@interface MauiShellSearchTarget : NSObject <NSSearchFieldDelegate>
@property(nonatomic) maui::controls::search_handler* handler;
- (void)onSearch:(id)sender;
@end

@implementation MauiShellSearchTarget
- (void)controlTextDidChange:(NSNotification*)notification
{
    if (self.handler == nullptr)
    {
        return;
    }
    NSSearchField* const field = (NSSearchField*)notification.object;
    NSString* const text = field.stringValue != nil ? field.stringValue : @"";
    // A native edit funnels through Query (SearchHandler's two-way Query path → OnQueryChanged).
    self.handler->send_query_changed(std::string(text.UTF8String));
}

- (void)onSearch:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        self.handler->query_confirmed();
    }
}
@end

namespace
{
    NSSplitViewController* as_controller(void* handle)
    {
        return (__bridge NSSplitViewController*)handle;
    }

    // A page's native NSView, via its view-handler's native_view() (nil if the page is unattached or its
    // handler has no native view). Mirrors content_page_handler.mm's native_child helper.
    NSView* native_page_view(maui::controls::content_page* page)
    {
        if (page == nullptr)
        {
            return nil;
        }
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page->handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge NSView*)handler->native_view();
    }
} // namespace

namespace maui::core
{
    shell_platform::~shell_platform()
    {
        // Release the retained AppKit handles (each balances a __bridge_retained below).
        if (search_delegate != nullptr)
        {
            CFRelease(search_delegate);
            search_delegate = nullptr;
        }
        if (search_controller != nullptr)
        {
            CFRelease(search_controller);
            search_controller = nullptr;
        }
        if (section_hosts != nullptr)
        {
            CFRelease(section_hosts);
            section_hosts = nullptr;
        }
        if (tab_host != nullptr)
        {
            CFRelease(tab_host);
            tab_host = nullptr;
        }
        if (flyout_host != nullptr)
        {
            CFRelease(flyout_host);
            flyout_host = nullptr;
        }
        if (controller != nullptr)
        {
            CFRelease(controller);
            controller = nullptr;
        }
        if (native != nullptr)
        {
            CFRelease(native);
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base). The
    // host is the split controller's plain NSView; is_enabled keeps the base mirror.
    void shell_platform::update_visibility(maui::core::visibility value)
    {
        as_controller(controller).view.hidden = value != maui::core::visibility::visible;
    }

    void shell_platform::update_opacity(double value)
    {
        as_controller(controller).view.alphaValue = value;
    }

    void shell_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_controller(controller).view.accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<shell_platform> shell_handler::create_platform_view()
    {
        auto platform = std::make_unique<shell_platform>();
        NSSplitViewController* const split = [[NSSplitViewController alloc] init];

        // The sidebar item hosts the flyout list (a plain NSView wrapping an NSTableView); the content
        // item hosts the NSTabView of shell_items (the tab host). rebuild()/realize_tree() fill both.
        NSViewController* const sidebar_vc = [[NSViewController alloc] init];
        sidebar_vc.view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 200, 0)];
        NSTableView* const flyout_table = [[NSTableView alloc] initWithFrame:NSZeroRect];
        [sidebar_vc.view addSubview:flyout_table];

        NSViewController* const content_vc = [[NSViewController alloc] init];
        NSTabView* const tabs = [[NSTabView alloc] initWithFrame:NSZeroRect];
        content_vc.view = tabs;

        NSSplitViewItem* const sidebar = [NSSplitViewItem sidebarWithViewController:sidebar_vc];
        NSSplitViewItem* const content = [NSSplitViewItem splitViewItemWithViewController:content_vc];
        split.splitViewItems = @[ sidebar, content ];

        platform->controller = (__bridge_retained void*)split;         // owns one reference
        platform->tab_host = (__bridge_retained void*)tabs;            // the content NSTabView
        platform->flyout_host = (__bridge_retained void*)flyout_table; // the sidebar list
        platform->native = (__bridge_retained void*)split.view;        // forces the view to load; owns one ref
        return platform;
    }

    void shell_handler::realize_tree()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->tab_host == nullptr)
        {
            return;
        }
        NSTabView* const tabs = (__bridge NSTabView*)platform->tab_host;
        const maui::controls::shell_item_renderer& item_renderer = platform->tree.current_item_renderer;

        // Detach the previous tab items' page views (a NSView can only have one superview).
        NSArray<NSTabViewItem*>* const previous = [tabs.tabViewItems copy];
        for (NSTabViewItem* const item in previous)
        {
            [item.view removeFromSuperview];
            [tabs removeTabViewItem:item];
        }

        // Rebuild one NSTabViewItem per visible section. AppKit has no NSNavigationController, so the tab
        // shows the section's CURRENT (top-most) page only (vc_stack.back()); the full stack lives in the
        // mirror (documented deviation).
        for (const maui::controls::shell_section_renderer& section : item_renderer.sections)
        {
            NSTabViewItem* const tab_item = [[NSTabViewItem alloc] initWithIdentifier:nil];
            maui::controls::content_page* const top =
                section.vc_stack.empty() ? section.root_page : section.vc_stack.back();
            NSView* const page_view = native_page_view(top);
            tab_item.view = page_view != nil ? page_view : [[NSView alloc] initWithFrame:NSZeroRect];
            const std::string title = top != nullptr ? std::string{top->title()} : std::string{};
            NSString* const ns_title = [NSString stringWithUTF8String:title.c_str()];
            tab_item.label = ns_title != nil ? ns_title : @"";
            [tabs addTabViewItem:tab_item];
        }

        // Select the current section's tab.
        if (item_renderer.selected_index >= 0 &&
            item_renderer.selected_index < static_cast<int>(tabs.numberOfTabViewItems))
        {
            [tabs selectTabViewItemAtIndex:item_renderer.selected_index];
        }

        // The flyout list is data-driven from the mirror; AppKit's NSTableView reloads from its source —
        // here the rows are recorded in tree.flyout_rows (assertable); a full NSTableViewDataSource is not
        // wired (the list is a visual surface, the model drives navigation). Reload to clear stale rows.
        if (platform->flyout_host != nullptr)
        {
            NSTableView* const flyout = (__bridge NSTableView*)platform->flyout_host;
            [flyout reloadData];
        }
    }

    void shell_handler::update_flyout_presented(maui::controls::shell& host)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        platform->tree.flyout_presented = host.flyout_is_presented();
        NSSplitViewController* const split = as_controller(platform->controller);
        if (split.splitViewItems.count == 0)
        {
            return;
        }
        // DEVIATION: no drawer — IsPresented collapses/expands the persistent sidebar instead.
        NSSplitViewItem* const sidebar = split.splitViewItems.firstObject;
        sidebar.collapsed = platform->tree.flyout_presented ? NO : YES;
    }

    void shell_handler::realize_search_box()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        const maui::controls::shell_search_box& box = platform->tree.search_box;

        // Lazily create the NSSearchField + its trampoline on first install. AppKit DEVIATION: no
        // UISearchController — the field is added as a subview at the top of the split controller's view.
        if (platform->search_controller == nullptr && box.present)
        {
            NSSearchField* const field = [[NSSearchField alloc] initWithFrame:NSMakeRect(0, 0, 240, 24)];
            MauiShellSearchTarget* const target = [[MauiShellSearchTarget alloc] init];
            field.delegate = target; // weak — the field references the target weakly
            field.target = target;   // weak — fired on Return (the search action)
            field.action = @selector(onSearch:);
            NSView* const host = as_controller(platform->controller).view;
            [host addSubview:field];
            platform->search_controller = (__bridge_retained void*)field; // owns one ref
            platform->search_delegate = (__bridge_retained void*)target;  // owns one ref (control holds it weakly)
        }

        if (platform->search_controller == nullptr)
        {
            return; // never installed and not present this pass
        }

        NSSearchField* const field = (__bridge NSSearchField*)platform->search_controller;
        MauiShellSearchTarget* const target = (__bridge MauiShellSearchTarget*)platform->search_delegate;
        target.handler = box.handler;

        if (!box.present)
        {
            // Visibility Hidden (or no handler): hide the field (the C# RemoveSearchController analog). The
            // field is kept retained for re-show; hiding is the AppKit equivalent of detaching it.
            field.hidden = YES;
            return;
        }
        field.hidden = NO;
        field.enabled = box.enabled ? YES : NO;
        NSString* const ph = [NSString stringWithUTF8String:box.placeholder.c_str()];
        field.placeholderString = ph != nil ? ph : @"";
        // Drive the field's text from the model WITHOUT bouncing it back (guard the delegate during the
        // programmatic set so controlTextDidChange doesn't re-enter send_query_changed).
        NSString* const query = [NSString stringWithUTF8String:box.query.c_str()];
        NSString* const current = field.stringValue != nil ? field.stringValue : @"";
        if (![current isEqualToString:(query != nil ? query : @"")])
        {
            id<NSSearchFieldDelegate> const saved = field.delegate;
            field.delegate = nil;
            field.stringValue = query != nil ? query : @"";
            field.delegate = saved;
        }
    }

    void shell_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const host = as_controller(platform->controller).view;
        [host setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
