// navigation_page_handler — Apple (AppKit / macOS) platform recipe: a FLIPPED NSView CONTAINER
// (create_flipped_host: isFlipped=YES, top-left origin) that stacks a CUSTOM navigation BAR (an NSView
// with an NSTextField title + a back NSButton) above a CONTENT area, and hosts the navigation stack's
// current (top-most) page as the content area's single subview, swapping it on each push/pop. A SEPARATE
// modal OVERLAY (an NSView covering the whole container) presents the top modal. Because the container is
// flipped (top-left origin, like UIKit), the bar pins to y = 0 (the TOP) and the content fills below it
// at y = k_bar_height — identical to the iOS twin (navigation_page_handler.iOS.cs). The real-native twin
// of the headless partial.
//
// AppKit has NO UINavigationController (iOS's host, which supplies the bar + push/pop transitions), so:
//   - the bar is built here (host_current's update_bar reads the view's chrome state — title +
//     back-button visibility — and populates the NSTextField / NSButton). The back NSButton's
//     target-action routes to i_stack_navigation::send_back_button_pressed() (→ pop()).
//   - the content swap is a plain re-parent (remove the old content subview, add the new), CROSS-FADED via
//     a CoreAnimation CATransition on the content area's layer when the request is animated, instant
//     otherwise (mirroring iOS's animated SetViewControllers). The transition is synchronous as far as the
//     view tree is concerned — the cross-platform handler reports IStackNavigation.NavigationFinished
//     inline after host_current returns (the CATransition animates the layer asynchronously without
//     blocking; the final view-tree state is correct synchronously).
//   - a modal is overlaid (not a child NSWindow) — the simpler-faithful choice so the modal participates
//     in the same view tree (and the headless mirror is observable). host_modal adds the modal's native
//     view as a full-container overlay (fading it in when animated), removing it when the modal stack
//     empties (popping restores the underlying content, which was never removed).
//
// Compiled as Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/runtime.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "apple_view_ops.hpp"
#include "flipped_container.hpp"
#include "maui/core/i_stack_navigation.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/navigation_page_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the back NSButton's target-action to the navigation view (the handler's
// virtual view, reached as i_stack_navigation). Mirrors button_handler.mm's MauiButtonTarget.
@interface MauiNavBackTarget : NSObject
@property(nonatomic) maui::core::navigation_page_handler* handler;
- (void)onBack:(id)sender;
@end

@implementation MauiNavBackTarget
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

namespace
{
    constexpr double k_bar_height = 44.0; // the custom navigation bar's height (points)

    // Typed views of the platform's retained void* slots (routing casts through helpers, like
    // content_page_handler.mm's as_host, keeps direct casts out of variable initializers).
    NSView* as_container(void* native)
    {
        return (__bridge NSView*)native;
    }

    NSView* as_view(void* handle)
    {
        return (__bridge NSView*)handle;
    }

    NSTextField* as_text_field(void* handle)
    {
        return (__bridge NSTextField*)handle;
    }

    NSButton* as_button(void* handle)
    {
        return (__bridge NSButton*)handle;
    }

    // The page's native NSView, via its view-handler's native_view() (nil if the page is unattached or
    // its handler has no native view). native_view() returns the real NSView the pimpl owns — not the
    // pimpl pointer that platform_view() returns. Mirrors layout_handler.mm's native_child helper.
    NSView* native_child(maui::core::i_view& page)
    {
        auto* handler = dynamic_cast<maui::core::i_view_handler*>(page.handler().get());
        if (handler == nullptr)
        {
            return nil;
        }
        return (__bridge NSView*)handler->native_view();
    }

    // Run a short CoreAnimation cross-fade on `view`'s layer (a fire-and-forget CATransition; the view
    // tree is mutated synchronously by the caller, only the layer animates).
    void cross_fade(NSView* view)
    {
        if (view == nil)
        {
            return;
        }
        view.wantsLayer = YES;
        CATransition* const transition = [CATransition animation];
        transition.type = kCATransitionFade;
        transition.duration = 0.25;
        [view.layer addAnimation:transition forKey:@"maui_nav_crossfade"];
    }
} // namespace

namespace maui::core
{
    navigation_page_platform::~navigation_page_platform()
    {
        // Release every retained AppKit handle (each balances a __bridge_retained in create_platform_view /
        // host_modal / host_title_view). The bar's subviews are owned by the bar; the back trampoline + the
        // hosted title view are held via their own slots.
        if (title_view_host != nullptr)
        {
            CFRelease(title_view_host);
            title_view_host = nullptr;
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
    // container is a plain NSView; is_enabled has no NSView equivalent, so it is left to the base mirror.
    void navigation_page_platform::update_visibility(maui::core::visibility value)
    {
        as_container(native).hidden = value != maui::core::visibility::visible;
    }

    void navigation_page_platform::update_opacity(double value)
    {
        as_container(native).alphaValue = value;
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
        // A flipped (top-left origin) container so the bar pins to the top (y=0) and the page content
        // fills below it (y=k_bar_height), matching the iOS twin's top-down layout. The factory returns a
        // RETAINED handle: store it straight into the void* slot (which owns the one reference) and borrow
        // a non-owning NSView* for the local wiring below.
        platform->native = maui::platform::apple::create_flipped_host(); // the void* slot owns one reference
        NSView* const container = (__bridge NSView*)platform->native;

        // The custom navigation bar (an NSView pinned to the top) holding the title + back button.
        NSView* const bar = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 0, k_bar_height)];
        NSTextField* const title = [NSTextField labelWithString:@""]; // non-editable, borderless label
        title.alignment = NSTextAlignmentCenter;
        NSButton* const back = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
        [back setButtonType:NSButtonTypeMomentaryPushIn];
        [back setBezelStyle:NSBezelStyleRounded];
        // The back affordance is the system back-chevron template image (the AppKit analog of iOS's back
        // button), so there is no user-facing string literal to localize.
        back.image = [NSImage imageNamed:NSImageNameGoBackTemplate];
        back.imagePosition = NSImageOnly;
        back.hidden = YES; // hidden at the root (depth 1); update_bar reveals it when depth > 1

        [bar addSubview:back];
        [bar addSubview:title];
        [container addSubview:bar];

        platform->bar = (__bridge_retained void*)bar; // each remaining void* slot owns one reference
        platform->title_field = (__bridge_retained void*)title;
        platform->back_button = (__bridge_retained void*)back;
        return platform;
    }

    void navigation_page_handler::on_connect_handler(navigation_page_platform& platform)
    {
        // Wire the back button's action to the trampoline here (create_platform_view is static, so `this`
        // is only available now). NSButton holds its target weakly → keep the trampoline alive in the
        // platform's back_target slot, like button_handler.mm.
        NSButton* const back = as_button(platform.back_button);
        MauiNavBackTarget* const target = [[MauiNavBackTarget alloc] init];
        target.handler = this;
        back.target = target;
        back.action = @selector(onBack:);
        platform.back_target = (__bridge_retained void*)target;
    }

    void navigation_page_handler::host_current(i_view* top, i_view& view, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const container = as_container(platform->native);
        NSView* const bar = as_view(platform->bar);
        // A presented modal overlay must survive a content swap (navigating the underlying stack while a
        // modal covers it) — preserve it alongside the bar.
        NSView* const overlay = platform->modal_overlay != nullptr ? as_view(platform->modal_overlay) : nil;

        // Swap the current page's CONTENT subview (everything except the bar + the modal overlay). Snapshot
        // the container's subviews (removeFromSuperview mutates the live array) and remove the content
        // without an Obj-C fast-enumeration loop (which clang-tidy's init-variables check misreads as
        // uninitialized).
        NSArray<NSView*>* const snapshot = [container.subviews copy];
        for (NSUInteger i = 0; i < snapshot.count; ++i)
        {
            NSView* const existing = snapshot[i];
            if (existing != bar && existing != overlay)
            {
                [existing removeFromSuperview];
            }
        }

        platform->hosted_page = top;
        platform->last_animated = animated;
        if (top != nullptr)
        {
            if (NSView* const subview = native_child(*top))
            {
                [subview removeFromSuperview];
                // Add below the bar so the bar stays on top (positionedBelow nil = bottom of the order).
                [container addSubview:subview positioned:NSWindowBelow relativeTo:bar];
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
            NSTextField* const title_field = as_text_field(platform->title_field);
            title_field.stringValue = raw != nil ? raw : @"";

            const bool back_visible = navigation->navigation_back_button_visible();
            as_button(platform->back_button).hidden = !static_cast<BOOL>(back_visible);

            // ---- bar styling ----
            // Background color (C# BarBackgroundColor): paint the bar's layer when set; leave the system
            // default (clear the layer color) when unset.
            const std::optional<maui::graphics::color> background = navigation->navigation_bar_background_color();
            bar.wantsLayer = YES;
            bar.layer.backgroundColor =
                background.has_value() ? maui::platform::apple::to_ns_color(*background).CGColor : nil;

            // Title (text) color (C# BarTextColor): set the label's textColor when set; AppKit's default
            // label color otherwise.
            const std::optional<maui::graphics::color> text_color = navigation->navigation_bar_text_color();
            title_field.textColor = text_color.has_value() ? maui::platform::apple::to_ns_color(*text_color) : nil;

            // TitleView (C# NavigationPage.TitleView): host the view's native subview in the bar instead of
            // the title label. When a title view is set, hide the label + add the title view; when cleared,
            // remove the previously-hosted title view + show the label again.
            i_view* const title_view = navigation->navigation_bar_title_view();
            host_title_view(*platform, title_view);

            // Mirror the chrome onto the platform too (so apple tests can read it like the headless ones).
            platform->bar_title = title_text;
            platform->back_button_visible = back_visible;
            platform->bar_background_color = background;
            platform->bar_text_color = text_color;
            platform->hosted_title_view = title_view;
            // chrome (W1-11): mirror the page-surfaced toolbar items. AppKit keeps the MIRROR only — the
            // items materialize on the WINDOW's NSToolbar (window_handler.mm), not in this custom bar;
            // the iOS twin builds real bar buttons from the same aggregate.
            platform->toolbar_items = navigation->navigation_toolbar_items();
        }
    }

    void navigation_page_handler::host_title_view(navigation_page_platform& platform, i_view* title_view)
    {
        NSView* const bar = as_view(platform.bar);
        NSTextField* const title_field = as_text_field(platform.title_field);

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

        // Host the title view's native NSView in the bar (above the label, which we hide).
        title_field.hidden = YES;
        if (NSView* const subview = native_child(*title_view))
        {
            [subview removeFromSuperview];
            [bar addSubview:subview];
            platform.title_view_host = (__bridge_retained void*)subview; // own a reference while hosted
        }
    }

    // --- platform configuration (W2-24): the iOSSpecific IsNavigationBarTranslucent push — an iOS-only
    // knob in C# (the AppKit twin keeps the cross-platform mirror; nothing native to drive on macOS).
    void navigation_page_handler::update_bar_translucent(bool value)
    {
        if (auto* platform = typed_platform_view())
        {
            platform->bar_translucent = value;
        }
    }

    void navigation_page_handler::host_modal(i_view* top_modal, bool animated)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSView* const container = as_container(platform->native);
        platform->last_animated = animated;

        // Tear down any existing overlay first (clearing or replacing the presented modal). Detach the
        // overlay's subviews too (the dismissed modal page's native view) so the page is freed from the
        // live tree — C# PopModal removes the modal's content, not just the host.
        if (platform->modal_overlay != nullptr)
        {
            NSView* const old_overlay = as_view(platform->modal_overlay);
            NSArray<NSView*>* const snapshot = [old_overlay.subviews copy];
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

        // Build an overlay NSView filling the container and host the modal's native view inside it. The
        // overlay sits ABOVE the bar + content (added last = top of the z-order), so the modal covers
        // everything (the AppKit analog of a presented modal page).
        NSView* const overlay = [[NSView alloc] initWithFrame:container.bounds];
        if (NSView* const subview = native_child(*top_modal))
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
        NSView* const container = as_container(platform->native);
        [container setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];

        // The container is flipped (top-left origin, like UIKit): the bar pins to the TOP (y = 0) and the
        // content fills the remaining area below it — identical to the iOS twin.
        const double content_height = frame.height > k_bar_height ? frame.height - k_bar_height : 0.0;
        NSView* const bar = as_view(platform->bar);
        [bar setFrame:NSMakeRect(0, 0, frame.width, k_bar_height)];
        // Lay the bar's title + back button: back on the left, title filling the rest.
        const double title_width = frame.width > 80 ? frame.width - 80 : 0;
        as_button(platform->back_button).frame = NSMakeRect(0, 0, 80, k_bar_height);
        const NSRect title_frame = NSMakeRect(80, 0, title_width, k_bar_height);
        as_text_field(platform->title_field).frame = title_frame;
        // A hosted TitleView (if any) occupies the same area as the title label.
        if (platform->title_view_host != nullptr)
        {
            [as_view(platform->title_view_host) setFrame:title_frame];
        }

        // The current page fills the content area below the bar (y = bar_height in the container's space)
        // — or the WHOLE container when the bar is translucent (the content extends under the bar), like
        // the iOS twin.
        if (platform->hosted_page != nullptr)
        {
            if (NSView* const subview = native_child(*platform->hosted_page))
            {
                const double content_y = platform->bar_translucent ? 0.0 : k_bar_height;
                const double page_height = platform->bar_translucent ? frame.height : content_height;
                [subview setFrame:NSMakeRect(0, content_y, frame.width, page_height)];
            }
        }
        // The modal overlay (if presented) fills the WHOLE container, covering the bar + content.
        if (platform->modal_overlay != nullptr)
        {
            NSView* const overlay = as_view(platform->modal_overlay);
            [overlay setFrame:NSMakeRect(0, 0, frame.width, frame.height)];
            if (platform->hosted_modal != nullptr)
            {
                if (NSView* const subview = native_child(*platform->hosted_modal))
                {
                    [subview setFrame:NSMakeRect(0, 0, frame.width, frame.height)];
                }
            }
        }
    }

    // Render transform + flow direction pushed to the native view via the shared apple_view_ops helpers
    // (M4c: the generic-IView ViewMapper widening). `native` is this struct's NSView handle.
    void navigation_page_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void navigation_page_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }
} // namespace maui::core
