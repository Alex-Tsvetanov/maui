// tabbed_page_handler — Apple (AppKit / macOS) platform recipe: a real NSTabViewController whose tab
// view items host the child pages. The real-native twin of the headless partial, the AppKit analog of
// the iOS UITabBarController twin (the W1-10 task's NSTabViewController choice):
//   - set_pages rebuilds tabViewItems — each page's native NSView becomes a wrapper NSViewController's
//     view inside one NSTabViewItem, labelled with the page's Title;
//   - set_current syncs selectedTabViewItemIndex virtual→native; the native→virtual direction is a
//     MauiTabViewController subclass forwarding tabView:didSelectTabViewItem: to
//     i_tabbed_view::on_tab_selected (suppressed during programmatic updates so the round trip never
//     re-enters);
//   - DEVIATION (documented): AppKit's tab chrome (the segmented control / window tab bar) has NO
//     bar-color API, so the four TabbedPage colors stay mirror-only here (update_bar fills the
//     platform mirrors; the iOS twin paints the real UITabBar).
// Compiled as Objective-C++ with ARC only for the `apple` backend.

#import <AppKit/AppKit.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/brushes/brush.hpp"
#include "maui/core/i_tabbed_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/tabbed_page_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// The NSTabViewController subclass forwarding a USER tab selection back to the virtual view. The
// suppress flag gates the callback while the handler itself mutates the items / selection (AppKit
// invokes the same delegate hook for programmatic changes, unlike UIKit's delegate).
@interface MauiTabViewController : NSTabViewController
@property(nonatomic) maui::core::tabbed_page_handler* mauiHandler;
@property(nonatomic) BOOL suppressSelection;
@end

@implementation MauiTabViewController
- (void)tabView:(NSTabView*)tabView didSelectTabViewItem:(NSTabViewItem*)tabViewItem
{
    [super tabView:tabView didSelectTabViewItem:tabViewItem];
    if (self.suppressSelection || self.mauiHandler == nullptr || tabViewItem == nil)
    {
        return;
    }
    const NSUInteger index = [self.tabViewItems indexOfObject:tabViewItem];
    if (index == static_cast<NSUInteger>(NSNotFound))
    {
        return;
    }
    if (auto* tabbed = dynamic_cast<maui::core::i_tabbed_view*>(self.mauiHandler->virtual_view()))
    {
        tabbed->on_tab_selected(static_cast<std::size_t>(index));
    }
}
@end

namespace
{
    MauiTabViewController* as_controller(void* handle)
    {
        return (__bridge MauiTabViewController*)handle;
    }

    // The page's native NSView, via its view-handler's native_view() (nil if the page is unattached or
    // its handler has no native view). Mirrors content_page_handler.mm's native_child helper.
    NSView* native_child(maui::core::i_view& page)
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
    tabbed_page_platform::~tabbed_page_platform()
    {
        // Release the retained AppKit handles (each balances a __bridge_retained in
        // create_platform_view). `delegate` is unused on AppKit (the subclass forwards selection).
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

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    // The host is the controller's plain NSView; is_enabled keeps the base mirror.
    void tabbed_page_platform::update_visibility(maui::core::visibility value)
    {
        as_controller(controller).view.hidden = value != maui::core::visibility::visible;
    }

    void tabbed_page_platform::update_opacity(double value)
    {
        as_controller(controller).view.alphaValue = value;
    }

    void tabbed_page_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_controller(controller).view.accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<tabbed_page_platform> tabbed_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<tabbed_page_platform>();
        MauiTabViewController* const tabs = [[MauiTabViewController alloc] init];
        tabs.tabStyle = NSTabViewControllerTabStyleSegmentedControlOnTop;
        platform->controller = (__bridge_retained void*)tabs;  // the slot owns one reference
        platform->native = (__bridge_retained void*)tabs.view; // forces the view to load; owns one ref
        return platform;
    }

    void tabbed_page_handler::on_connect_handler(tabbed_page_platform& platform)
    {
        // Wire the selection forwarding here (create_platform_view is static, so `this` is only
        // available now).
        as_controller(platform.controller).mauiHandler = this;
    }

    void tabbed_page_handler::set_pages(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view);
        if (tabbed == nullptr)
        {
            return;
        }
        platform->hosted_pages = tabbed->tabbed_pages();
        platform->tab_titles = tabbed->tabbed_titles();

        // Rebuild the tab view items: one wrapper NSViewController per page, its view the page's native
        // NSView (or an empty NSView for an unattached page), the item labelled with the page's Title.
        NSMutableArray<NSTabViewItem*>* const items = [NSMutableArray array];
        for (std::size_t i = 0; i < platform->hosted_pages.size(); ++i)
        {
            NSViewController* const wrapper = [[NSViewController alloc] init];
            NSView* const page_view = native_child(*platform->hosted_pages[i]);
            wrapper.view = page_view != nil ? page_view : [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
            NSTabViewItem* const item = [NSTabViewItem tabViewItemWithViewController:wrapper];
            NSString* const label = [NSString stringWithUTF8String:platform->tab_titles[i].c_str()];
            item.label = label != nil ? label : @"";
            [items addObject:item];
        }

        MauiTabViewController* const tabs = as_controller(platform->controller);
        tabs.suppressSelection = YES;
        tabs.tabViewItems = items;
        tabs.suppressSelection = NO;
    }

    void tabbed_page_handler::set_current(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view);
        if (tabbed == nullptr)
        {
            return;
        }
        platform->hosted_current = tabbed->tabbed_current_page();
        platform->selected_index = -1;
        for (std::size_t i = 0; i < platform->hosted_pages.size(); ++i)
        {
            if (platform->hosted_pages[i] == platform->hosted_current)
            {
                platform->selected_index = static_cast<int>(i);
                break;
            }
        }
        if (platform->selected_index < 0)
        {
            return;
        }
        MauiTabViewController* const tabs = as_controller(platform->controller);
        if (std::cmp_less(platform->selected_index, tabs.tabViewItems.count))
        {
            tabs.suppressSelection = YES;
            tabs.selectedTabViewItemIndex = platform->selected_index;
            tabs.suppressSelection = NO;
        }
    }

    void tabbed_page_handler::update_bar(i_view& view)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (const auto* tabbed = dynamic_cast<i_tabbed_view*>(&view))
        {
            // AppKit's tab chrome has no bar-color API — mirror-only here (header DEVIATION note). The
            // BarBackground brush is mirrored too (a non-owning aliasing borrow) but not painted: AppKit
            // has no tab-bar-fill API, so the iOS twin paints the real UITabBar while this captures only.
            platform->bar_background_color = tabbed->tab_bar_background_color();
            platform->bar_text_color = tabbed->tab_bar_text_color();
            platform->selected_tab_color = tabbed->tab_selected_color();
            platform->unselected_tab_color = tabbed->tab_unselected_color();

            const std::optional<maui::controls::brush*> brush = tabbed->tab_bar_background_brush();
            platform->bar_background_brush =
                (brush.has_value() && *brush != nullptr)
                    ? std::optional{std::shared_ptr<maui::controls::brush>{std::shared_ptr<void>{}, *brush}}
                    : std::nullopt;
        }
    }

    maui::graphics::size tabbed_page_handler::get_desired_size(double /*width_constraint*/,
                                                               double /*height_constraint*/) const
    {
        // The tabbed page sizes from its current page, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void tabbed_page_handler::platform_arrange(const maui::graphics::rect& frame)
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
