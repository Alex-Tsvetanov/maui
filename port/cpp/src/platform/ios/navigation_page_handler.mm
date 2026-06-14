// navigation_page_handler — iOS (UIKit) platform recipe: a UIView CONTAINER that stacks a CUSTOM
// navigation BAR (a UIView with a UILabel title + a back UIButton) above a CONTENT area, and hosts the
// navigation stack's current (top-most) page as the content area's single subview, swapping it on each
// push/pop. A SEPARATE modal OVERLAY (a UIView covering the whole container) presents the top modal.
// The real-native twin of the headless partial, ported from the AppKit twin
// (src/platform/apple/navigation_page_handler.mm) — the same custom-bar recipe, translated to UIKit's
// TOP-left origin (the bar pins to y = 0, the content fills below it; AppKit pinned the bar at
// y = height - bar_height).
//
// DELIBERATE DEVIATION from NavigationViewHandler.iOS.cs: C#'s iOS host is a UINavigationController
// (StackNavigationManager + UIViewControllers supplying the bar and push/pop transitions). This slice
// does NOT integrate UINavigationController — the handler keeps the port's own custom bar + subview
// swap (the cross-platform handler seam is unchanged), because the port hosts plain UIViews (no
// UIViewController tree exists yet, and a spawned bundle-less test process cannot run a real
// UIApplication/UIWindow for controller containment — see button_ios_tests.mm). So:
//   - the bar is built here (host_current's bar refresh reads the view's chrome state — title +
//     back-button visibility + bar styling — and populates the UILabel / UIButton). The back UIButton's
//     TouchUpInside target-action routes to i_stack_navigation::send_back_button_pressed() (→ pop()).
//   - the content swap is a plain re-parent (remove the old content subview, add the new below the
//     bar), CROSS-FADED via a CoreAnimation CATransition on the page's layer when the request is
//     animated, instant otherwise. The transition is synchronous as far as the view tree is concerned —
//     the cross-platform handler reports IStackNavigation.NavigationFinished inline after host_current
//     returns (the CATransition animates the layer asynchronously without blocking).
//   - a modal is overlaid (not a presented view controller) — host_modal adds the modal's native view
//     inside a full-container overlay UIView (fading it in when animated), removing it when the modal
//     stack empties (popping restores the underlying content, which was never removed).
//
// Compiled as Objective-C++ with ARC only for the `ios` backend.

#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "ios_conversions.hpp"
#include "ios_semantics_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/i_menu_element.hpp" // --- chrome (W1-11) ---
#include "maui/core/i_stack_navigation.hpp"
#include "maui/core/i_toolbar_item.hpp" // --- chrome (W1-11) ---
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the back UIButton's TouchUpInside target-action to the navigation view
// (the handler's virtual view, reached as i_stack_navigation). Mirrors button_handler.mm's
// MauiButtonEventProxy.
@interface MauiNavBackProxy : NSObject
@property(nonatomic) maui::core::navigation_page_handler* handler;
- (void)onBack:(id)sender;
@end

@implementation MauiNavBackProxy
- (void)onBack:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        // The virtual view is the navigation_page; reach its back-button hook via i_stack_navigation so
        // this core trampoline does not depend on the controls layer.
        if (auto* navigation = dynamic_cast<maui::core::i_stack_navigation*>(self.handler->virtual_view()))
        {
            navigation->send_back_button_pressed();
        }
    }
}
@end

// --- chrome (W1-11): the toolbar-item tap trampoline — one per bar button, routing TouchUpInside back
// to the borrowed i_menu_element (ToolbarItem click → MenuItem.Activate → clicked). ---
@interface MauiToolbarItemProxy : NSObject
@property(nonatomic) maui::core::i_menu_element* element;
- (void)onTap:(id)sender;
@end

@implementation MauiToolbarItemProxy
- (void)onTap:(id)sender
{
    (void)sender;
    if (self.element != nullptr)
    {
        self.element->send_clicked();
    }
}
@end

namespace
{
    constexpr double k_bar_height = 44.0; // the custom navigation bar's height (points)

    // Typed views of the platform's retained void* slots (routing casts through helpers, like
    // content_page_handler.mm's as_host, keeps direct casts out of variable initializers).
    UIView* as_container(void* native)
    {
        return (__bridge UIView*)native;
    }

    UIView* as_view(void* handle)
    {
        return (__bridge UIView*)handle;
    }

    UILabel* as_label(void* handle)
    {
        return (__bridge UILabel*)handle;
    }

    UIButton* as_button(void* handle)
    {
        return (__bridge UIButton*)handle;
    }

    // chrome (W1-11): typed view of the retained toolbar-buttons array slot.
    NSArray<UIButton*>* as_button_array(void* handle)
    {
        return (__bridge NSArray<UIButton*>*)handle;
    }

    // The page's native UIView, via its view-handler's native_view() (nil if the page is unattached or
    // its handler has no native view). native_view() returns the real UIView the pimpl owns — not the
    // pimpl pointer that platform_view() returns. Mirrors layout_handler.mm's native_child helper.
    UIView* native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge UIView*)handler->native_view();
    }

    // Run a short CoreAnimation cross-fade on `view`'s layer (a fire-and-forget CATransition; the view
    // tree is mutated synchronously by the caller, only the layer animates).
    void cross_fade(UIView* view)
    {
        if (view == nil)
        {
            return;
        }
        CATransition* const transition = [CATransition animation];
        transition.type = kCATransitionFade;
        transition.duration = 0.25;
        [view.layer addAnimation:transition forKey:@"maui_nav_crossfade"];
    }

    // chrome (W1-11): hand the borrowed element to the tap trampoline. A plain function parameter (not
    // the Obj-C property dot-assignment) so the non-const use is visible where the element pointer is
    // consumed — the proxy's onTap: calls the element's non-const send_clicked().
    void set_proxy_element(MauiToolbarItemProxy* proxy, maui::core::i_menu_element* element)
    {
        proxy.element = element;
    }

    // chrome (W1-11): right-align the bar's toolbar buttons (the FIRST button sits rightmost, like
    // UINavigationBar's rightBarButtonItems). Called by the rebuild and by platform_arrange.
    constexpr double k_toolbar_button_width = 70.0;

    void layout_toolbar_buttons(maui::core::navigation_page_platform& platform, double bar_width)
    {
        if (platform.toolbar_buttons == nullptr)
        {
            return;
        }
        NSArray<UIButton*>* const buttons = as_button_array(platform.toolbar_buttons);
        for (NSUInteger i = 0; i < buttons.count; ++i)
        {
            const double x = bar_width - (k_toolbar_button_width * static_cast<double>(i + 1));
            buttons[i].frame = CGRectMake(x, 0, k_toolbar_button_width, k_bar_height);
        }
    }
} // namespace

namespace maui::core
{
    navigation_page_platform::~navigation_page_platform()
    {
        // Release every retained UIKit handle (each balances a __bridge_retained in create_platform_view /
        // on_connect_handler / host_modal / host_title_view). The bar's subviews are owned by the bar; the
        // back trampoline + the hosted title view are held via their own slots.
        if (title_view_host != nullptr)
        {
            CFRelease(title_view_host);
            title_view_host = nullptr;
        }
        if (bar_backdrop != nullptr) // W2-24: the translucent bar's blur backdrop
        {
            CFRelease(bar_backdrop);
            bar_backdrop = nullptr;
        }
        if (modal_overlay != nullptr)
        {
            CFRelease(modal_overlay);
            modal_overlay = nullptr;
        }
        if (back_target != nullptr)
        {
            CFRelease(back_target);
            back_target = nullptr;
        }
        // chrome (W1-11): the bar's toolbar-item buttons + their tap trampolines.
        if (toolbar_buttons != nullptr)
        {
            CFRelease(toolbar_buttons);
            toolbar_buttons = nullptr;
        }
        if (toolbar_targets != nullptr)
        {
            CFRelease(toolbar_targets);
            toolbar_targets = nullptr;
        }
        if (back_button != nullptr)
        {
            CFRelease(back_button);
            back_button = nullptr;
        }
        if (title_field != nullptr)
        {
            CFRelease(title_field);
            title_field = nullptr;
        }
        if (bar != nullptr)
        {
            CFRelease(bar);
            bar = nullptr;
        }
        if (native != nullptr)
        {
            CFRelease(native);
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base). The
    // container is a plain UIView; is_enabled has no UIView equivalent, so it is left to the base mirror.
    void navigation_page_platform::update_visibility(maui::core::visibility value)
    {
        as_container(native).hidden = value != maui::core::visibility::visible;
    }

    void navigation_page_platform::update_opacity(double value)
    {
        as_container(native).alpha = value;
    }

    void navigation_page_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_container(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<navigation_page_platform> navigation_page_handler::create_platform_view()
    {
        auto platform = std::make_unique<navigation_page_platform>();
        UIView* const container = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];

        // The custom navigation bar (a UIView pinned to the top) holding the title + back button.
        UIView* const bar = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, k_bar_height)];
        UILabel* const title = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
        title.textAlignment = NSTextAlignmentCenter;
        UIButton* const back = [UIButton buttonWithType:UIButtonTypeSystem];
        // The back affordance is the system back-chevron symbol image (the iOS back affordance), so
        // there is no user-facing string literal to localize.
        [back setImage:[UIImage systemImageNamed:@"chevron.backward"] forState:UIControlStateNormal];
        back.hidden = YES; // hidden at the root (depth 1); the bar refresh reveals it when depth > 1

        [bar addSubview:back];
        [bar addSubview:title];
        [container addSubview:bar];

        platform->native = (__bridge_retained void*)container; // each void* slot owns one reference
        platform->bar = (__bridge_retained void*)bar;
        platform->title_field = (__bridge_retained void*)title;
        platform->back_button = (__bridge_retained void*)back;
        return platform;
    }

    void navigation_page_handler::on_connect_handler(navigation_page_platform& platform)
    {
        // Wire the back button's action to the trampoline here (create_platform_view is static, so `this`
        // is only available now). UIControl holds its targets weakly → keep the trampoline alive in the
        // platform's back_target slot, like button_handler.mm's associated-object retention.
        UIButton* const back = as_button(platform.back_button);
        MauiNavBackProxy* const proxy = [[MauiNavBackProxy alloc] init];
        proxy.handler = this;
        [back addTarget:proxy action:@selector(onBack:) forControlEvents:UIControlEventTouchUpInside];
        platform.back_target = (__bridge_retained void*)proxy;
    }

    void navigation_page_handler::host_current(i_view* top, i_view& view, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIView* const container = as_container(platform->native);
        UIView* const bar = as_view(platform->bar);
        // A presented modal overlay must survive a content swap (navigating the underlying stack while a
        // modal covers it) — preserve it alongside the bar.
        UIView* const overlay = platform->modal_overlay != nullptr ? as_view(platform->modal_overlay) : nil;

        // Swap the current page's CONTENT subview (everything except the bar + the modal overlay). Snapshot
        // the container's subviews (removeFromSuperview mutates the live array) and remove the content
        // without an Obj-C fast-enumeration loop (which clang-tidy's init-variables check misreads as
        // uninitialized).
        NSArray<UIView*>* const snapshot = [container.subviews copy];
        for (NSUInteger i = 0; i < snapshot.count; ++i)
        {
            UIView* const existing = snapshot[i];
            if (existing != bar && existing != overlay)
            {
                [existing removeFromSuperview];
            }
        }

        platform->hosted_page = top;
        platform->last_animated = animated;
        if (top != nullptr)
        {
            if (UIView* const subview = native_child(*top))
            {
                [subview removeFromSuperview];
                // Add below the bar so the bar stays on top (UIKit's native below-sibling insert).
                [container insertSubview:subview belowSubview:bar];
                if (animated)
                {
                    cross_fade(subview);
                }
            }
        }

        // Refresh the bar from the view's navigation chrome state (title + back-button visibility + the bar
        // styling: background color, title color, and a TitleView shown instead of the label).
        if (auto* navigation = dynamic_cast<i_stack_navigation*>(&view))
        {
            const std::string title_text(navigation->navigation_bar_title());
            NSString* const raw = [NSString stringWithUTF8String:title_text.c_str()];
            UILabel* const title_field = as_label(platform->title_field);
            title_field.text = raw != nil ? raw : @"";

            const bool back_visible = navigation->navigation_back_button_visible();
            as_button(platform->back_button).hidden = !static_cast<BOOL>(back_visible);

            // ---- bar styling ----
            // Background color (C# BarBackgroundColor): paint the bar's layer when set; leave the system
            // default (clear the layer color) when unset.
            const std::optional<maui::graphics::color> background = navigation->navigation_bar_background_color();
            bar.layer.backgroundColor =
                background.has_value() ? maui::platform::ios::to_ui_color(*background).CGColor : nil;

            // Title (text) color (C# BarTextColor): set the label's textColor when set; UIKit's default
            // label color otherwise (UILabel's textColor is labelColor by default — nil is not assignable).
            const std::optional<maui::graphics::color> text_color = navigation->navigation_bar_text_color();
            title_field.textColor =
                text_color.has_value() ? maui::platform::ios::to_ui_color(*text_color) : UIColor.labelColor;

            // TitleView (C# NavigationPage.TitleView): host the view's native subview in the bar instead of
            // the title label. When a title view is set, hide the label + add the title view; when cleared,
            // remove the previously-hosted title view + show the label again.
            i_view* const title_view = navigation->navigation_bar_title_view();
            host_title_view(*platform, title_view);

            // Mirror the chrome onto the platform too (so ios tests can read it like the headless ones).
            platform->bar_title = title_text;
            platform->back_button_visible = back_visible;
            platform->bar_background_color = background;
            platform->bar_text_color = text_color;
            platform->hosted_title_view = title_view;

            // chrome (W1-11): materialize the page-surfaced toolbar items as REAL buttons on the bar's
            // right (C#'s UINavigationBar rightBarButtonItems path — the custom bar's analog). The
            // aggregate arrives priority-sorted; secondary items simply follow the primaries (the
            // overflow split is a desktop affordance — documented simplification). Rebuilt whole.
            const std::vector<i_toolbar_item*> items = navigation->navigation_toolbar_items();
            platform->toolbar_items = items;
            if (platform->toolbar_buttons != nullptr)
            {
                NSArray<UIButton*>* const old_buttons = as_button_array(platform->toolbar_buttons);
                for (NSUInteger i = 0; i < old_buttons.count; ++i)
                {
                    [old_buttons[i] removeFromSuperview];
                }
                CFRelease(platform->toolbar_buttons);
                platform->toolbar_buttons = nullptr;
            }
            if (platform->toolbar_targets != nullptr)
            {
                CFRelease(platform->toolbar_targets);
                platform->toolbar_targets = nullptr;
            }
            NSMutableArray<UIButton*>* const buttons = [NSMutableArray array];
            NSMutableArray* const targets = [NSMutableArray array];
            // Primaries first, then secondaries (each group keeps the tracker's priority order).
            std::vector<i_menu_element*> ordered;
            ordered.reserve(items.size());
            for (const bool secondary_pass : {false, true})
            {
                for (i_toolbar_item* const item : items)
                {
                    if (item != nullptr && item->is_secondary() == secondary_pass)
                    {
                        ordered.push_back(item);
                    }
                }
            }
            for (i_menu_element* const element : ordered)
            {
                UIButton* const button = [UIButton buttonWithType:UIButtonTypeSystem];
                const std::string text(element->text());
                NSString* const label = [NSString stringWithUTF8String:text.c_str()];
                [button setTitle:(label != nil ? label : @"") forState:UIControlStateNormal];
                button.enabled = static_cast<BOOL>(element->is_enabled());
                MauiToolbarItemProxy* const proxy = [[MauiToolbarItemProxy alloc] init];
                set_proxy_element(proxy, element);
                [button addTarget:proxy action:@selector(onTap:) forControlEvents:UIControlEventTouchUpInside];
                [targets addObject:proxy];
                [buttons addObject:button];
                [bar addSubview:button];
            }
            platform->toolbar_buttons = (__bridge_retained void*)buttons;
            platform->toolbar_targets = (__bridge_retained void*)targets;
            layout_toolbar_buttons(*platform, bar.bounds.size.width);
        }
    }

    void navigation_page_handler::host_title_view(navigation_page_platform& platform, i_view* title_view)
    {
        UIView* const bar = as_view(platform.bar);
        UILabel* const title_field = as_label(platform.title_field);

        // Remove any previously-hosted title view's native subview from the bar (a change or a clear).
        if (platform.title_view_host != nullptr)
        {
            [as_view(platform.title_view_host) removeFromSuperview];
            CFRelease(platform.title_view_host);
            platform.title_view_host = nullptr;
        }

        if (title_view == nullptr)
        {
            title_field.hidden = NO; // no title view -> the label shows again
            return;
        }

        // Host the title view's native UIView in the bar (above the label, which we hide).
        title_field.hidden = YES;
        if (UIView* const subview = native_child(*title_view))
        {
            [subview removeFromSuperview];
            [bar addSubview:subview];
            platform.title_view_host = (__bridge_retained void*)subview; // own a reference while hosted
        }
    }

    // --- platform configuration (W2-24): the iOSSpecific IsNavigationBarTranslucent push — the
    // UINavigationBar.Translucent analog over the port's custom bar (NavigationRenderer.UpdateTranslucent):
    // translucent gives the bar a system-material blur backdrop (what a translucent UINavigationBar draws)
    // and lets the current page's content extend UNDER the bar; opaque removes the backdrop and re-frames
    // the content below the bar. Idempotent; the mirror records the realized state for the seam tests.
    void navigation_page_handler::update_bar_translucent(bool value)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->bar_translucent = value;
        UIView* const bar = as_view(platform->bar);
        if (value && platform->bar_backdrop == nullptr)
        {
            UIVisualEffectView* const backdrop = [[UIVisualEffectView alloc]
                initWithEffect:[UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemChromeMaterial]];
            backdrop.frame = bar.bounds;
            backdrop.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            [bar insertSubview:backdrop atIndex:0];
            platform->bar_backdrop = (__bridge_retained void*)backdrop; // the slot owns one reference
        }
        else if (!value && platform->bar_backdrop != nullptr)
        {
            [as_view(platform->bar_backdrop) removeFromSuperview];
            CFRelease(platform->bar_backdrop);
            platform->bar_backdrop = nullptr;
        }
        // Re-frame the hosted page for the new content origin (under / below the bar), from the
        // container's current bounds (platform_arrange re-derives the same frames on the next pass).
        UIView* const container = as_container(platform->native);
        const CGRect bounds = container.bounds;
        if (platform->hosted_page != nullptr && bounds.size.height > 0)
        {
            if (UIView* const subview = native_child(*platform->hosted_page))
            {
                const double content_y = value ? 0.0 : k_bar_height;
                const double page_height =
                    value ? bounds.size.height
                          : (bounds.size.height > k_bar_height ? bounds.size.height - k_bar_height : 0.0);
                [subview setFrame:CGRectMake(0, content_y, bounds.size.width, page_height)];
            }
        }
    }

    void navigation_page_handler::host_modal(i_view* top_modal, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIView* const container = as_container(platform->native);
        platform->last_animated = animated;

        // Tear down any existing overlay first (clearing or replacing the presented modal). Detach the
        // overlay's subviews too (the dismissed modal page's native view) so the page is freed from the
        // live tree — C# PopModal removes the modal's content, not just the host.
        if (platform->modal_overlay != nullptr)
        {
            UIView* const old_overlay = as_view(platform->modal_overlay);
            NSArray<UIView*>* const snapshot = [old_overlay.subviews copy];
            [snapshot makeObjectsPerformSelector:@selector(removeFromSuperview)];
            [old_overlay removeFromSuperview];
            CFRelease(platform->modal_overlay);
            platform->modal_overlay = nullptr;
        }

        platform->hosted_modal = top_modal;
        if (top_modal == nullptr)
        {
            return; // the modal stack emptied — the underlying content (never removed) is revealed
        }

        // Build an overlay UIView filling the container and host the modal's native view inside it. The
        // overlay sits ABOVE the bar + content (added last = top of the z-order), so the modal covers
        // everything (the simpler-faithful stand-in for iOS's presented modal page — see the header).
        UIView* const overlay = [[UIView alloc] initWithFrame:container.bounds];
        if (UIView* const subview = native_child(*top_modal))
        {
            [subview removeFromSuperview];
            [subview setFrame:overlay.bounds];
            [overlay addSubview:subview];
        }
        [container addSubview:overlay];
        platform->modal_overlay = (__bridge_retained void*)overlay;
        if (animated)
        {
            cross_fade(overlay);
        }
    }

    maui::graphics::size navigation_page_handler::get_desired_size(double /*width_constraint*/,
                                                                   double /*height_constraint*/) const
    {
        // The navigation page sizes from its current page, not the handler, so it reports nothing here.
        return {0, 0};
    }

    void navigation_page_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        UIView* const container = as_container(platform->native);
        [container setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];

        // UIKit's coordinate space is top-left origin: the bar pins to the TOP (y = 0) and the content
        // fills the remaining area below it (the AppKit twin pinned the bar at y = height - bar_height).
        const double content_height = frame.height > k_bar_height ? frame.height - k_bar_height : 0.0;
        UIView* const bar = as_view(platform->bar);
        [bar setFrame:CGRectMake(0, 0, frame.width, k_bar_height)];
        // Lay the bar's title + back button: back on the left, title filling the rest.
        const double title_width = frame.width > 80 ? frame.width - 80 : 0;
        as_button(platform->back_button).frame = CGRectMake(0, 0, 80, k_bar_height);
        const CGRect title_frame = CGRectMake(80, 0, title_width, k_bar_height);
        as_label(platform->title_field).frame = title_frame;
        // A hosted TitleView (if any) occupies the same area as the title label.
        if (platform->title_view_host != nullptr)
        {
            [as_view(platform->title_view_host) setFrame:title_frame];
        }
        // chrome (W1-11): keep the toolbar buttons pinned to the bar's right edge.
        layout_toolbar_buttons(*platform, frame.width);

        // The current page fills the content area below the bar (y = bar_height in the container's
        // space) — or the WHOLE container when the bar is translucent (W2-24: UINavigationBar.Translucent
        // lets the content extend under the bar; the blur backdrop keeps it legible).
        if (platform->hosted_page != nullptr)
        {
            if (UIView* const subview = native_child(*platform->hosted_page))
            {
                const double content_y = platform->bar_translucent ? 0.0 : k_bar_height;
                const double page_height = platform->bar_translucent ? frame.height : content_height;
                [subview setFrame:CGRectMake(0, content_y, frame.width, page_height)];
            }
        }
        // The modal overlay (if presented) fills the WHOLE container, covering the bar + content.
        if (platform->modal_overlay != nullptr)
        {
            UIView* const overlay = as_view(platform->modal_overlay);
            [overlay setFrame:CGRectMake(0, 0, frame.width, frame.height)];
            if (platform->hosted_modal != nullptr)
            {
                if (UIView* const subview = native_child(*platform->hosted_modal))
                {
                    [subview setFrame:CGRectMake(0, 0, frame.width, frame.height)];
                }
            }
        }
    }

    // Background / shadow / clip pushed to the container's layer via the shared ios_visual_ops helpers
    // (the direct PaintExtensions / ShadowExtensions / WrapperView.SetClip ports). `native` is this
    // struct's UIView handle.
    void navigation_page_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::ios::apply_background(native, value);
    }

    void navigation_page_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::ios::apply_shadow(native, value);
    }

    void navigation_page_platform::update_clip(const maui::graphics::i_shape* value)
    {
        // The clip mask is sized to the view's current bounds (WrapperView.SetClip uses the view frame).
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag pushed to the container via the shared
    // ios_semantics_ops helpers (SemanticExtensions.UpdateSemantics / ViewExtensions.
    // UpdateInputTransparent). `native` is this struct's UIView handle.
    void navigation_page_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::ios::apply_semantics((__bridge UIView*)native, value);
    }

    void navigation_page_platform::update_input_transparent(bool value)
    {
        maui::platform::ios::apply_input_transparent((__bridge UIView*)native, value);
    }
} // namespace maui::core
