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
#include <optional>
#include <string>
#include <string_view>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/search_handler.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_appearance.hpp"
#include "maui/controls/shell_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

#include "ios_conversions.hpp"

// UIKit trampoline for the shell search box: the UISearchBar's text edits + search-button taps + the
// bookmark (clear-placeholder) button route to the C++ search_handler model. Mirrors
// ShellPageRendererTracker's UISearchController wiring: the search bar's delegate funnels live edits to
// Query (OnQueryChanged), the search button to QueryConfirmed, and the bookmark button to
// ClearPlaceholderClicked.
@interface MauiShellSearchDelegate : NSObject <UISearchBarDelegate>
@property(nonatomic) maui::controls::search_handler* handler;
@end

@implementation MauiShellSearchDelegate
- (void)searchBar:(UISearchBar*)searchBar textDidChange:(NSString*)searchText
{
    (void)searchBar;
    if (self.handler != nullptr)
    {
        self.handler->send_query_changed(std::string(searchText != nil ? searchText.UTF8String : ""));
    }
}

- (void)searchBarSearchButtonClicked:(UISearchBar*)searchBar
{
    (void)searchBar;
    if (self.handler != nullptr)
    {
        self.handler->query_confirmed();
    }
}

- (void)searchBarBookmarkButtonClicked:(UISearchBar*)searchBar
{
    (void)searchBar;
    if (self.handler != nullptr)
    {
        self.handler->clear_placeholder_clicked();
    }
}
@end

// The flyout HEADER container (ShellFlyoutHeaderContainer): a UIView that hosts the header's native view and
// applies a safe-area-EXCEPT-bottom margin (the header sits ABOVE the content, so the window's bottom safe
// area must NOT be treated as a gap between header and content — C# ShellFlyoutHeaderContainer.Margin
// returns Thickness(left, top, right, 0)). It is added at subview index 0 of the flyout drawer's view.
@interface MauiShellFlyoutHeaderContainer : UIView
@property(nonatomic, strong) UIView* contentView;
@end

@implementation MauiShellFlyoutHeaderContainer
- (void)setContentView:(UIView*)contentView
{
    if (_contentView == contentView)
    {
        return;
    }
    [_contentView removeFromSuperview];
    _contentView = contentView;
    if (_contentView != nil)
    {
        [self addSubview:_contentView];
    }
    [self setNeedsLayout];
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    if (self.contentView == nil)
    {
        return;
    }
    // Safe area EXCEPT bottom: inset the content by the window safe area's top/left/right, bottom = 0.
    const UIEdgeInsets safe = self.safeAreaInsets;
    const CGRect bounds = self.bounds;
    self.contentView.frame =
        CGRectMake(safe.left, safe.top, bounds.size.width - safe.left - safe.right, bounds.size.height - safe.top);
}
@end

// The flyout FOOTER container: a clip-to-bounds UIView pinned to the BOTTOM of the flyout drawer's view. Its
// layoutSubviews re-positions itself to the bottom (ReMeasure then UpdatePosition order from
// ShellFlyoutContentRenderer.UpdateFooterPosition) and lays the footer content to fill it. A recursion guard
// prevents the bottom-reposition (a frame change) from re-entering layout endlessly.
@interface MauiShellFlyoutFooterContainer : UIView
@property(nonatomic, strong) UIView* contentView;
@property(nonatomic, assign) BOOL repositioning;
@end

@implementation MauiShellFlyoutFooterContainer
- (instancetype)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame]) != nil)
    {
        self.clipsToBounds = YES;
    }
    return self;
}

- (void)setContentView:(UIView*)contentView
{
    if (_contentView == contentView)
    {
        return;
    }
    [_contentView removeFromSuperview];
    _contentView = contentView;
    if (_contentView != nil)
    {
        [self addSubview:_contentView];
    }
    [self setNeedsLayout];
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    if (self.contentView != nil)
    {
        self.contentView.frame = self.bounds;
    }

    // UpdateFooterPosition: pin to the bottom of the superview (above its bottom safe area). Guarded so the
    // frame mutation does not re-enter layout (ReMeasure → UpdatePosition once, no infinite recursion).
    if (self.repositioning || self.superview == nil)
    {
        return;
    }
    self.repositioning = YES;
    const CGRect parent = self.superview.bounds;
    const CGFloat footerHeight = self.frame.size.height;
    const UIEdgeInsets parentSafe = self.superview.safeAreaInsets;
    self.frame = CGRectMake(0, parent.size.height - footerHeight - parentSafe.bottom, parent.size.width, footerHeight);
    self.repositioning = NO;
}
@end

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

    // A generic view's native UIView (the flyout header/footer are arbitrary Views, not pages): via its
    // view-handler's native_view() (nil when unattached / handler-less). Same seam as native_page_view.
    UIView* native_view_of(maui::core::i_view* view)
    {
        if (view == nullptr)
        {
            return nil;
        }
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(view->handler().get());
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
        if (flyout_footer_container != nullptr)
        {
            CFRelease(flyout_footer_container);
            flyout_footer_container = nullptr;
        }
        if (flyout_header_container != nullptr)
        {
            CFRelease(flyout_header_container);
            flyout_header_container = nullptr;
        }
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

        // Rebuilding the tab host replaced the nav controllers (fresh top VCs), so the search box's host
        // nav item is now stale — reinstall it onto the new current section's nav item. (realize_tree runs
        // both from the full rebuild AND from the flyout-only paths map_flyout_items / set_flyout_item_
        // template, which do not call rebuild_search_box, so reinstalling here keeps the box attached.)
        realize_search_box();

        // Same reasoning for the appearance: fresh nav bars + tab bar default to the system colors, so re-push
        // the resolved appearance from the mirror (the full-rebuild path also re-resolves it afterward via
        // rebuild_appearance — re-applying here keeps the flyout-only realize_tree paths tinted too).
        realize_appearance();
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

    void shell_handler::realize_flyout()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->flyout_host == nullptr)
        {
            return;
        }
        const maui::controls::shell_render_tree& tree = platform->tree;
        UITableViewController* const flyout = (__bridge UITableViewController*)platform->flyout_host;
        UIView* const flyout_view = flyout.viewIfLoaded != nil ? flyout.viewIfLoaded : flyout.view;

        // ---- the HEADER container (ShellFlyoutHeaderContainer, subview index 0) ----
        UIView* const header_content = native_view_of(tree.flyout_header);
        MauiShellFlyoutHeaderContainer* header_container =
            platform->flyout_header_container != nullptr
                ? (__bridge MauiShellFlyoutHeaderContainer*)platform->flyout_header_container
                : nil;
        if (header_content != nil)
        {
            if (header_container == nil)
            {
                header_container = [[MauiShellFlyoutHeaderContainer alloc] initWithFrame:CGRectZero];
                platform->flyout_header_container = (__bridge_retained void*)header_container; // owns one ref
            }
            header_container.contentView = header_content;
            if (header_container.superview != flyout_view)
            {
                [header_container removeFromSuperview];
                [flyout_view insertSubview:header_container atIndex:0]; // HeaderIndex
            }
            [header_container setNeedsLayout];
        }
        else if (header_container != nil)
        {
            // No header resolves: detach + release the container (UpdateFlyoutHeader's removal branch).
            [header_container removeFromSuperview];
            CFRelease(platform->flyout_header_container);
            platform->flyout_header_container = nullptr;
        }

        // ---- the FOOTER container (clip-to-bounds, pinned to the bottom) ----
        UIView* const footer_content = native_view_of(tree.flyout_footer);
        MauiShellFlyoutFooterContainer* footer_container =
            platform->flyout_footer_container != nullptr
                ? (__bridge MauiShellFlyoutFooterContainer*)platform->flyout_footer_container
                : nil;
        if (footer_content != nil)
        {
            if (footer_container == nil)
            {
                footer_container = [[MauiShellFlyoutFooterContainer alloc] initWithFrame:CGRectZero];
                platform->flyout_footer_container = (__bridge_retained void*)footer_container; // owns one ref
            }
            footer_container.contentView = footer_content;
            if (footer_container.superview != flyout_view)
            {
                [footer_container removeFromSuperview];
                [flyout_view addSubview:footer_container]; // bottom-most subview
            }
            [footer_container setNeedsLayout];
        }
        else if (footer_container != nil)
        {
            [footer_container removeFromSuperview];
            CFRelease(platform->flyout_footer_container);
            platform->flyout_footer_container = nullptr;
        }

        // ---- the flyout WIDTH (FlyoutWidth → the split VC's primary column) ----
        // A resolved positive width sizes the primary (flyout) column; nullopt / the C# -1 sentinel restores
        // the platform default (UISplitViewControllerAutomaticDimension). preferredPrimaryColumnWidth is the
        // point-width control on the double-column split (iOS 14+); the fraction variant is the relative one.
        if (platform->controller != nullptr)
        {
            UISplitViewController* const split = as_split(platform->controller);
            if (tree.flyout_width.has_value())
            {
                split.preferredPrimaryColumnWidth = static_cast<CGFloat>(*tree.flyout_width);
            }
            else
            {
                split.preferredPrimaryColumnWidth = UISplitViewControllerAutomaticDimension;
            }
        }
    }

    void shell_handler::realize_search_box()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->tab_host == nullptr)
        {
            return;
        }
        const maui::controls::shell_search_box& box = platform->tree.search_box;

        // Lazily create the UISearchController + its UISearchBar delegate on first install (mirrors
        // ShellPageRendererTracker.AttachSearchController). The results VC is nil even when box.shows_results
        // is true — DEFERRED (documented): the full ItemsSource-bound results renderer (C#
        // ShellSearchResultsRenderer — a UITableViewController bound to SearchHandler.ItemsSource via the
        // ListProxy, rendering DisplayMemberName cells and firing ItemSelected on a row tap) is not wired
        // here. The model-side results infra already exists (search_handler::results() / the search_box
        // mirror's result_count + search_handler::item_selected as the row-tap seam); only the native
        // table-binding renderer is the follow-up. The search bar itself (query/confirm/clear) works fully.
        if (platform->search_controller == nullptr && box.present)
        {
            UISearchController* const controller = [[UISearchController alloc] initWithSearchResultsController:nil];
            MauiShellSearchDelegate* const del = [[MauiShellSearchDelegate alloc] init];
            controller.searchBar.delegate = del; // weak — the bar references the delegate weakly
            platform->search_controller = (__bridge_retained void*)controller; // owns one ref
            platform->search_delegate = (__bridge_retained void*)del;          // owns one ref (bar holds it weakly)
        }
        if (platform->search_controller == nullptr)
        {
            return;
        }

        UISearchController* const controller = (__bridge UISearchController*)platform->search_controller;
        MauiShellSearchDelegate* const del = (__bridge MauiShellSearchDelegate*)platform->search_delegate;
        del.handler = box.handler;

        // Drive the bar's state from the model.
        controller.searchBar.userInteractionEnabled = box.enabled ? YES : NO;
        controller.searchBar.showsBookmarkButton = (box.handler != nullptr && box.handler->clear_placeholder_enabled());
        NSString* const ph = [NSString stringWithUTF8String:box.placeholder.c_str()];
        controller.searchBar.placeholder = ph != nil ? ph : @"";
        NSString* const query = [NSString stringWithUTF8String:box.query.c_str()];
        NSString* const current = controller.searchBar.text != nil ? controller.searchBar.text : @"";
        if (![current isEqualToString:(query != nil ? query : @"")])
        {
            controller.searchBar.text = query != nil ? query : @""; // setting text does not call the delegate
        }

        // Install/remove the controller on the current section's nav item (UpdateSearchVisibility):
        // present (Collapsible/Expanded) → set searchController + HidesSearchBarWhenScrolling; Hidden → nil.
        // Resolve the active nav controller by the mirror's selected_index (deterministic — matches what
        // realize_tree set); selectedViewController is unreliable for a tab controller not yet in a window.
        UITabBarController* const tabs = (__bridge UITabBarController*)platform->tab_host;
        const int selected_index = platform->tree.current_item_renderer.selected_index;
        UIViewController* selected = nil;
        if (selected_index >= 0 && static_cast<NSUInteger>(selected_index) < tabs.viewControllers.count)
        {
            selected = tabs.viewControllers[static_cast<NSUInteger>(selected_index)];
        }
        UINavigationController* const nav =
            [selected isKindOfClass:[UINavigationController class]] ? (UINavigationController*)selected : nil;
        UIViewController* const top = nav != nil ? nav.topViewController : nil;
        if (top == nil)
        {
            return; // no nav item to host the search controller yet
        }
        if (box.present)
        {
            top.navigationItem.searchController = controller;
            top.navigationItem.hidesSearchBarWhenScrolling = box.collapsible ? YES : NO;
        }
        else
        {
            top.navigationItem.searchController = nil; // RemoveSearchController
        }
    }

    void shell_handler::realize_appearance()
    {
        // Push the resolved appearance (the applied_appearance mirror) onto the native chrome: the per-section
        // UINavigationBar (each tab's nav controller) + the UITabBar of the tab host. Mirrors the compat
        // ShellNavBarAppearanceTracker.SetAppearance (nav bar) + ShellTabBarAppearanceTracker.SetAppearance
        // (tab bar). A missing color slot leaves that attribute on the system default (nil), exactly like the
        // C# null-color path.
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->tab_host == nullptr)
        {
            return;
        }
        UITabBarController* const tabs = (__bridge UITabBarController*)platform->tab_host;
        const std::optional<maui::controls::shell_appearance>& appearance = platform->tree.applied_appearance;

        using maui::platform::ios::to_ui_color;
        const auto ui = [](const std::optional<maui::graphics::color>& c) -> UIColor* {
            return c.has_value() ? to_ui_color(*c) : nil;
        };

        // ---- the nav bar of every section's UINavigationController (ShellNavBarAppearanceTracker) ----
        // BackgroundColor → barTintColor; ForegroundColor → tintColor (bar-button items); TitleColor →
        // titleTextAttributes. A null appearance resets every attribute to the system default.
        UIColor* const bar_background = appearance ? ui(appearance->background_color()) : nil;
        UIColor* const bar_foreground = appearance ? ui(appearance->foreground_color()) : nil;
        UIColor* const bar_title = appearance ? ui(appearance->title_color()) : nil;
        for (UIViewController* const child in tabs.viewControllers)
        {
            UINavigationController* const nav =
                [child isKindOfClass:[UINavigationController class]] ? (UINavigationController*)child : nil;
            if (nav == nil)
            {
                continue;
            }
            UINavigationBar* const navbar = nav.navigationBar;
            UINavigationBarAppearance* const bar_appearance = [[UINavigationBarAppearance alloc] init];
            if (bar_background != nil)
            {
                [bar_appearance configureWithOpaqueBackground];
                bar_appearance.backgroundColor = bar_background;
            }
            else
            {
                [bar_appearance configureWithDefaultBackground];
            }
            if (bar_title != nil)
            {
                NSDictionary<NSAttributedStringKey, id>* const title_attrs =
                    @{NSForegroundColorAttributeName : bar_title};
                bar_appearance.titleTextAttributes = title_attrs;
                bar_appearance.largeTitleTextAttributes = title_attrs;
            }
            navbar.standardAppearance = bar_appearance;
            navbar.scrollEdgeAppearance = bar_appearance;
            navbar.tintColor = bar_foreground; // nil restores the default
        }

        // ---- the tab bar (ShellTabBarAppearanceTracker) ----
        // EffectiveTabBarBackgroundColor → background; EffectiveTabBarTitleColor → item title text;
        // EffectiveTabBarForegroundColor → tintColor (selected); EffectiveTabBarUnselectedColor →
        // unselectedItemTintColor. "Effective" applies the TabBar* ?? base fallback (IShellAppearanceElement).
        UITabBar* const bar = tabs.tabBar;
        UITabBarAppearance* const tab_appearance = [[UITabBarAppearance alloc] init];
        UIColor* const tab_background = appearance ? ui(appearance->effective_tab_bar_background_color()) : nil;
        if (tab_background != nil)
        {
            [tab_appearance configureWithOpaqueBackground];
            tab_appearance.backgroundColor = tab_background;
        }
        else
        {
            [tab_appearance configureWithDefaultBackground];
        }
        if (UIColor* const tab_title = appearance ? ui(appearance->effective_tab_bar_title_color()) : nil)
        {
            NSDictionary<NSAttributedStringKey, id>* const attrs = @{NSForegroundColorAttributeName : tab_title};
            tab_appearance.stackedLayoutAppearance.normal.titleTextAttributes = attrs;
            tab_appearance.stackedLayoutAppearance.selected.titleTextAttributes = attrs;
        }
        bar.standardAppearance = tab_appearance;
        bar.scrollEdgeAppearance = tab_appearance;
        bar.tintColor = appearance ? ui(appearance->effective_tab_bar_foreground_color()) : nil;
        bar.unselectedItemTintColor = appearance ? ui(appearance->effective_tab_bar_unselected_color()) : nil;
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
