// shell_handler — iOS (UIKit) platform recipe: a real container realizing the W2-21 shell model as a
// native VC hierarchy, ported from the compat ShellRenderer + ShellItemRenderer + ShellSectionRenderer:
//
//   container UIViewController            (ShellRenderer)
//     └─ UISplitViewController            (the pan-presented FLYOUT drawer + the content)
//          ├─ primary column: the flyout drawer (a UITableView-style list of the flyout rows)
//          └─ secondary column: the current item's tab host
//               └─ UITabBarController     (ShellItemRenderer) — one tab per visible shell_section
//                    └─ UINavigationController (ShellSectionRenderer) per tab
//                         └─ viewControllers[0] = root content page, then the pushed pages (stack[1..])
//
// realize_tree() reads the shell_render_tree mirror (built by the cross-platform rebuild()) and rebuilds
// the real hierarchy: the UITabBarController's viewControllers become one UINavigationController per
// section, each navigation controller's viewControllers become its vc_stack (root + pushed). The tab
// host's selectedIndex = the current section. This is what the on-simulator e2e asserts: after
// go_to("//route?id=3") the model reconfigures, the property mapper fires map_current_item/state →
// rebuild → realize_tree, and the real UINavigationController stack matches the model section stack.
//
// FLYOUT: the primary column hosts the drawer; FlyoutIsPresented drives the split's preferredDisplayMode
// (oneBesideSecondary = open; secondaryOnly = closed) — the pan-presented drawer. Each flyout row is a
// UITableViewCell titled with the row title (the data_template-built row content is materialized when a
// template was set — its native UIView is hosted in the cell; otherwise the cell's textLabel shows the
// title). DEVIATION (documented): the native→virtual presented sync (a user drag/tap dismiss) needs a
// live UIWindow + transition coordinator the bundle-less test process cannot host (the
// flyout_page_handler.mm precedent) — the model-driven direction is the gate; flyout-row taps route to
// shell::on_flyout_item_selected.
//
// Compiled as Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    UISplitViewController* as_split(void* handle)
    {
        return (__bridge UISplitViewController*)handle;
    }

    // A page's native UIView, via its view-handler's native_view() (nil if the page is unattached or its
    // handler has no native view). Mirrors content_page_handler.mm's native_child helper.
    UIView* native_page_view(maui::controls::content_page* page)
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
        return (__bridge UIView*)handler->native_view();
    }

    // Wrap a page in a child UIViewController whose view is the page's native UIView (or an empty UIView
    // for an unattached page — a navigation stack entry requires a view controller).
    UIViewController* wrap_page(maui::controls::content_page* page)
    {
        UIViewController* const wrapper = [[UIViewController alloc] init];
        UIView* const page_view = native_page_view(page);
        wrapper.view = page_view != nil ? page_view : [[UIView alloc] initWithFrame:CGRectZero];
        if (page != nullptr)
        {
            const std::string title{page->title()};
            NSString* const ns_title = [NSString stringWithUTF8String:title.c_str()];
            wrapper.title = ns_title != nil ? ns_title : @"";
        }
        return wrapper;
    }

    // Build one section's UINavigationController from its renderer's vc_stack (root first, then pushed).
    UINavigationController* build_section_controller(const maui::controls::shell_section_renderer& renderer)
    {
        NSMutableArray<UIViewController*>* const stack = [NSMutableArray array];
        for (maui::controls::content_page* const page : renderer.vc_stack)
        {
            [stack addObject:wrap_page(page)];
        }
        if (stack.count == 0)
        {
            [stack addObject:[[UIViewController alloc] init]]; // a nav controller needs a root
        }
        UINavigationController* const nav = [[UINavigationController alloc] init];
        nav.viewControllers = stack;
        return nav;
    }
} // namespace

namespace maui::core
{
    shell_platform::~shell_platform()
    {
        // Release the retained UIKit handles (each balances a __bridge_retained below).
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
    // host is the split controller's plain UIView; is_enabled keeps the base mirror.
    void shell_platform::update_visibility(maui::core::visibility value)
    {
        as_split(controller).view.hidden = value != maui::core::visibility::visible;
    }

    void shell_platform::update_opacity(double value)
    {
        as_split(controller).view.alpha = value;
    }

    void shell_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_split(controller).view.accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<shell_platform> shell_handler::create_platform_view()
    {
        auto platform = std::make_unique<shell_platform>();
        // The container is a UISplitViewController: primary column = the flyout drawer, secondary = the
        // tab host. A double-column split gives the pan-presented drawer behavior (oneBesideSecondary /
        // secondaryOnly displays). The flyout drawer starts as an empty list controller (rebuild fills it).
        UISplitViewController* const split =
            [[UISplitViewController alloc] initWithStyle:UISplitViewControllerStyleDoubleColumn];
        UITabBarController* const tabs = [[UITabBarController alloc] init];
        UITableViewController* const flyout = [[UITableViewController alloc] initWithStyle:UITableViewStylePlain];
        [split setViewController:flyout forColumn:UISplitViewControllerColumnPrimary];
        [split setViewController:tabs forColumn:UISplitViewControllerColumnSecondary];
        split.preferredDisplayMode = UISplitViewControllerDisplayModeSecondaryOnly; // drawer closed

        platform->controller = (__bridge_retained void*)split;   // the slot owns one reference
        platform->tab_host = (__bridge_retained void*)tabs;      // the secondary column's tab host
        platform->flyout_host = (__bridge_retained void*)flyout; // the primary column's drawer list
        platform->native = (__bridge_retained void*)split.view;  // forces the view to load; owns one ref
        return platform;
    }

    void shell_handler::realize_tree()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->tab_host == nullptr)
        {
            return;
        }
        UITabBarController* const tabs = (__bridge UITabBarController*)platform->tab_host;
        const maui::controls::shell_item_renderer& item_renderer = platform->tree.current_item_renderer;

        // Detach the PREVIOUS tab VCs first: a UIView can only belong to one view controller at a time, so
        // the discarded wrappers must release their page views before fresh wrappers adopt them.
        NSArray<UIViewController*>* const previous = tabs.viewControllers;
        tabs.viewControllers = @[];
        for (NSUInteger i = 0; i < previous.count; ++i)
        {
            if (auto* nav = (UINavigationController*)(
                    [previous[i] isKindOfClass:[UINavigationController class]] ? previous[i] : nil))
            {
                for (UIViewController* const child in nav.viewControllers)
                {
                    [child.viewIfLoaded removeFromSuperview];
                    child.view = nil;
                }
            }
        }

        // Rebuild one UINavigationController per visible section (the ShellSectionRenderer set); each
        // navigation controller's viewControllers ARE the section's vc_stack (root content + pushed pages).
        NSMutableArray<UIViewController*>* const children = [NSMutableArray array];
        for (const maui::controls::shell_section_renderer& section : item_renderer.sections)
        {
            UINavigationController* const nav = build_section_controller(section);
            const std::string title =
                section.root_page != nullptr ? std::string{section.root_page->title()} : std::string{};
            NSString* const ns_title = [NSString stringWithUTF8String:title.c_str()];
            nav.tabBarItem.title = ns_title != nil ? ns_title : @"";
            [children addObject:nav];
        }
        tabs.viewControllers = children;

        // Select the current section's tab (ShellItemRenderer.GoTo → selectedIndex).
        if (item_renderer.selected_index >= 0 &&
            static_cast<NSUInteger>(item_renderer.selected_index) < tabs.viewControllers.count)
        {
            tabs.selectedIndex = static_cast<NSUInteger>(item_renderer.selected_index);
        }

        // Re-point the retained section-hosts slot at the new nav controllers (so they outlive the call).
        if (platform->section_hosts != nullptr)
        {
            CFRelease(platform->section_hosts);
        }
        platform->section_hosts = (__bridge_retained void*)[children copy];

        // Refresh the flyout drawer list rows.
        if (platform->flyout_host != nullptr)
        {
            UITableViewController* const flyout = (__bridge UITableViewController*)platform->flyout_host;
            [flyout.tableView reloadData];
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
        UISplitViewController* const split = as_split(platform->controller);
        split.preferredDisplayMode = platform->tree.flyout_presented
                                         ? UISplitViewControllerDisplayModeOneBesideSecondary
                                         : UISplitViewControllerDisplayModeSecondaryOnly;
    }

    void shell_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIView* const host = as_split(platform->controller).view;
        [host setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
