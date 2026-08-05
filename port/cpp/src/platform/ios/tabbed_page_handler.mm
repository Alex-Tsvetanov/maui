// tabbed_page_handler — iOS (UIKit) platform recipe: a real UITabBarController whose child view
// controllers host the pages (the W1-10 task's child-VC composition): each page's native UIView is
// wrapped in a child UIViewController (vc.view = the page's view), its tabBarItem titled with the
// page's Title; CurrentPage syncs to selectedIndex virtual→native, and the native→virtual direction is
// a retained UITabBarControllerDelegate proxy forwarding didSelectViewController to
// i_tabbed_view::on_tab_selected (UIKit only calls the delegate for USER selections, so the
// programmatic sync never re-enters). The C# analog is the Compatibility TabbedRenderer (a
// UITabBarController subclass) — the modern TabbedViewHandler.iOS maps are empty stubs, so the
// renderer's tab/bar behavior is ported onto the handler seam here.
//
// Bar styling (the TabbedRenderer Update* methods): BarBackgroundColor → a UITabBarAppearance
// background (standard + scrollEdge); BarTextColor → the stacked item title attributes;
// SelectedTabColor → tabBar.tintColor; UnselectedTabColor → tabBar.unselectedItemTintColor. Unset
// colors restore the system defaults.
//
// Compiled as Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "ios_conversions.hpp"
#include "ios_visual_ops.hpp" // apply_bar_background + k_bar_background_layer_name (the brush CALayer fill)
#include "maui/controls/brushes/brush.hpp"
#include "maui/controls/brushes/brush_paint_bridge.hpp"
#include "maui/controls/brushes/gradient_brush.hpp"
#include "maui/core/i_tabbed_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/tabbed_page_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// The UITabBarControllerDelegate proxy forwarding a USER tab selection back to the virtual view
// (UIKit does not invoke the delegate for programmatic selectedIndex changes). Mirrors
// navigation_page_handler.mm's MauiNavBackProxy retention pattern.
@interface MauiTabBarDelegate : NSObject <UITabBarControllerDelegate>
@property(nonatomic) maui::core::tabbed_page_handler* mauiHandler;
@end

@implementation MauiTabBarDelegate
- (void)tabBarController:(UITabBarController*)tabBarController didSelectViewController:(UIViewController*)viewController
{
    if (self.mauiHandler == nullptr)
    {
        return;
    }
    const NSUInteger index = [tabBarController.viewControllers indexOfObject:viewController];
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
    UITabBarController* as_controller(void* handle)
    {
        return (__bridge UITabBarController*)handle;
    }

    // The page's native UIView, via its view-handler's native_view() (nil if the page is unattached or
    // its handler has no native view). Mirrors content_page_handler.mm's native_child helper.
    UIView* native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge UIView*)handler->native_view();
    }

    // Paint (or clear) the tab bar's brush background — C# TabBar.UpdateBackground(brush) via
    // BrushExtensions: remove the old layer, then (only for a non-empty brush) bridge the controls brush to
    // the graphics paint the layer helper renders and install the bar-background CALayer at index 0. A null
    // OR empty brush (Brush.IsNullOrEmpty — e.g. a SolidColorBrush with a null color, or a stop-less
    // gradient) just clears the layer, matching C#'s `if (Brush.IsNullOrEmpty(brush)) return;` after the
    // removal (a value-type solid_paint would otherwise paint opaque black for a null-color brush).
    void paint_bar_background_brush(UITabBar* bar, maui::controls::brush* brush)
    {
        if (bar == nil)
        {
            return;
        }
        const std::shared_ptr<maui::graphics::paint> paint =
            maui::controls::brush_is_null_or_empty_as_paint(brush) ? nullptr : maui::controls::to_paint(*brush);
        if (paint != nullptr)
        {
            // C# BrushExtensions.UpdateBackground:34 sets control.BackgroundColor = UIColor.Clear when a
            // background layer is inserted, so the UITabBar's native fill does not show through the brush
            // layer. Only on the non-empty-brush path (the empty path just removes the layer, no clear).
            bar.backgroundColor = nil;
        }
        maui::platform::ios::apply_bar_background(bar.layer, paint.get(), bar.layer.bounds);
    }
} // namespace

namespace maui::core
{
    tabbed_page_platform::~tabbed_page_platform()
    {
        // Release the retained UIKit handles (each balances a __bridge_retained below).
        if (delegate != nullptr)
        {
            ((__bridge MauiTabBarDelegate*)delegate).mauiHandler =
                nullptr; // the back-pointer live_view re-reads after user code
            CFRelease(delegate);
            delegate = nullptr;
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

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    // The host is the controller's plain UIView; is_enabled keeps the base mirror.
    void tabbed_page_platform::update_visibility(maui::core::visibility value)
    {
        as_controller(controller).view.hidden = value != maui::core::visibility::visible;
    }

    void tabbed_page_platform::update_opacity(double value)
    {
        as_controller(controller).view.alpha = value;
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
        UITabBarController* const tabs = [[UITabBarController alloc] init];
        platform->controller = (__bridge_retained void*)tabs;  // the slot owns one reference
        platform->native = (__bridge_retained void*)tabs.view; // forces the view to load; owns one ref
        return platform;
    }

    void tabbed_page_handler::on_connect_handler(tabbed_page_platform& platform)
    {
        // Wire the selection delegate here (create_platform_view is static, so `this` is only available
        // now). UITabBarController holds its delegate weakly → retain the proxy in the delegate slot.
        MauiTabBarDelegate* const proxy = [[MauiTabBarDelegate alloc] init];
        proxy.mauiHandler = this;
        as_controller(platform.controller).delegate = proxy;
        platform.delegate = (__bridge_retained void*)proxy;
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

        // Detach the PREVIOUS wrappers first: a UIView can only be associated with one view controller
        // at a time (UIViewControllerHierarchyInconsistency), so the old wrapper must release the
        // page's view before a fresh wrapper adopts it.
        UITabBarController* const tabs = as_controller(platform->controller);
        NSArray<UIViewController*>* const previous = tabs.viewControllers;
        tabs.viewControllers = @[];
        for (NSUInteger i = 0; i < previous.count; ++i)
        {
            [previous[i].viewIfLoaded removeFromSuperview]; // pull the page view out of the old chrome
            previous[i].view = nil;                         // the discarded wrapper releases the page-view association
        }

        // Rebuild the child view controllers: one wrapper UIViewController per page, its view the
        // page's native UIView (or an empty UIView for an unattached page), its tabBarItem titled with
        // the page's Title (the TabbedRenderer SetTabBarItem role).
        NSMutableArray<UIViewController*>* const children = [NSMutableArray array];
        for (std::size_t i = 0; i < platform->hosted_pages.size(); ++i)
        {
            UIViewController* const wrapper = [[UIViewController alloc] init];
            UIView* const page_view = native_child(*platform->hosted_pages[i]);
            wrapper.view = page_view != nil ? page_view : [[UIView alloc] initWithFrame:CGRectZero];
            NSString* const title = [NSString stringWithUTF8String:platform->tab_titles[i].c_str()];
            wrapper.tabBarItem.title = title != nil ? title : @"";
            [children addObject:wrapper];
        }
        tabs.viewControllers = children;
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
        UITabBarController* const tabs = as_controller(platform->controller);
        if (static_cast<NSUInteger>(platform->selected_index) < tabs.viewControllers.count)
        {
            tabs.selectedIndex = static_cast<NSUInteger>(platform->selected_index);
        }
    }

    void tabbed_page_handler::update_bar(i_view& view)
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
        platform->bar_background_color = tabbed->tab_bar_background_color();
        platform->bar_text_color = tabbed->tab_bar_text_color();
        platform->selected_tab_color = tabbed->tab_selected_color();
        platform->unselected_tab_color = tabbed->tab_unselected_color();

        UITabBar* const bar = as_controller(platform->controller).tabBar;

        // BarBackgroundColor + BarTextColor through UITabBarAppearance (the TabbedRenderer
        // UpdateBarBackgroundColor / UpdateBarTextColor pair on the appearance API).
        UITabBarAppearance* const appearance = [[UITabBarAppearance alloc] init];
        if (platform->bar_background_color.has_value())
        {
            [appearance configureWithOpaqueBackground];
            appearance.backgroundColor = maui::platform::ios::to_ui_color(*platform->bar_background_color);
        }
        else
        {
            [appearance configureWithDefaultBackground];
        }
        if (platform->bar_text_color.has_value())
        {
            UIColor* const text = maui::platform::ios::to_ui_color(*platform->bar_text_color);
            NSDictionary<NSAttributedStringKey, id>* const attributes = @{NSForegroundColorAttributeName : text};
            appearance.stackedLayoutAppearance.normal.titleTextAttributes = attributes;
            appearance.stackedLayoutAppearance.selected.titleTextAttributes = attributes;
        }
        bar.standardAppearance = appearance;
        bar.scrollEdgeAppearance = appearance;

        // SelectedTabColor → tintColor; UnselectedTabColor → unselectedItemTintColor (TabbedRenderer
        // UpdateTabBarAppearance); nil restores the system defaults.
        bar.tintColor = platform->selected_tab_color.has_value()
                            ? maui::platform::ios::to_ui_color(*platform->selected_tab_color)
                            : nil;
        bar.unselectedItemTintColor = platform->unselected_tab_color.has_value()
                                          ? maui::platform::ios::to_ui_color(*platform->unselected_tab_color)
                                          : nil;

        // BarBackground (Brush) — the TabbedRenderer UpdateBarBackground / TabBar.UpdateBackground pair.
        // Unsubscribe the OLD gradient brush's InvalidateGradientBrushRequested, fetch the new brush, store
        // a non-owning mirror, (re-)subscribe ONLY when the new brush is a gradient (subscribing on a
        // solid/null brush would be wrong — and null has no event), then paint the bar's CALayer fill.
        platform->bar_background_invalidate_token.reset(); // drop the prior subscription (idempotent)

        const std::optional<maui::controls::brush*> brush_opt = tabbed->tab_bar_background_brush();
        maui::controls::brush* const brush = (brush_opt.has_value()) ? *brush_opt : nullptr;
        platform->bar_background_brush =
            (brush != nullptr) ? std::optional{std::shared_ptr<maui::controls::brush>{std::shared_ptr<void>{}, brush}}
                               : std::nullopt;

        if (auto* const gradient = dynamic_cast<maui::controls::gradient_brush*>(brush))
        {
            // Repaint on every stop change (OnBarBackgroundChanged). The callback weakly captures `this`
            // and re-reads the LIVE mirror, so it never paints a stale brush; the scoped_connection is
            // dropped above before the next subscription and in the platform dtor (the control owns the
            // brush and outlives the handler).
            tabbed_page_handler* const self = this;
            platform->bar_background_invalidate_token =
                maui::core::connect_scoped(gradient->invalidate_gradient_brush_requested, [self]() {
                    auto* const live = self->typed_platform_view();
                    if (live == nullptr || live->controller == nullptr || !live->bar_background_brush.has_value())
                    {
                        return;
                    }
                    paint_bar_background_brush(as_controller(live->controller).tabBar,
                                               live->bar_background_brush->get());
                });
        }

        paint_bar_background_brush(bar, brush);
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
        UIView* const host = as_controller(platform->controller).view;
        [host setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
