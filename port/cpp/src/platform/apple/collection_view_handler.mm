// collection_view_handler — Apple (AppKit / macOS) platform partial: the REAL native virtualization
// stack (W3-30), the AppKit twin of the iOS Items2 stack (W3-29). A genuine NSCollectionView inside an
// NSScrollView, driven by an NSCollectionViewFlowLayout + NSCollectionViewDataSource/Delegate, with a
// unified controller/cell/source architecture mirroring the iOS handler adapted to AppKit:
//
//   - MauiCollectionViewItem  <=  TemplatedCell2 / DefaultCell2 (unified): an NSCollectionViewItem whose
//     view hosts a default NSTextField mirroring item.text() (the DefaultCell label) and, when an
//     ItemTemplate is set, the realized data_template native content. NSCollectionView recycles it via
//     makeItemWithIdentifier: + prepareForReuse — the recycler reuses item INSTANCES, which the apple
//     seam suite asserts under programmatic scroll.
//   - MauiCollectionSupplementaryView  <=  the *SupplementaryView2 family: the section header/footer
//     (structured) and group header/footer (grouped) boundary supplementary views.
//   - MauiCollectionDataSource  <=  ItemsViewController2 + the Structured / Selectable / Groupable /
//     Reorderable controllers COLLAPSED into one class (the port already collapsed the matching mapper
//     chain). It is the NSCollectionViewDataSource + NSCollectionViewDelegate(FlowLayout), reads the
//     handler's i_items_view_source, dequeues recycled items, and fans user selection back to the
//     control through the handler.
//
// DOCUMENTED DEVIATION: the iOS COMPOSITIONAL-LAYOUT path (UICollectionViewCompositionalLayout +
// LayoutFactory2) and SNAP POINTS are iOS-ONLY. NSCollectionView has no compositional layout, so the
// port uses NSCollectionViewFlowLayout: linear list = 1 column; grid = `span` columns; both
// orientations via the layout's scrollDirection. Snap points (SnapPointsType / SnapPointsAlignment) are
// a UICollectionView feature with no NSCollectionViewFlowLayout analog and are not honored on AppKit
// (the cross-platform simulator still mirrors them as state). Header/footer use the flow layout's
// section header/footer reference sizes. estimated-size self-sizing cells are reduced to a fixed
// per-item size so the layout is deterministic under test; the data/selection/grouping/reorder/scroll
// semantics are faithful. The cross-platform simulator (collection_view_handler.cpp) still runs as the
// in-memory state mirror on this backend; these natives are what the apple seam suite asserts against.
//
// DOCUMENTED DEVIATION — the CarouselView (there is no MAUI AppKit CarouselView to copy; MAUI's macOS
// IS Mac Catalyst/UIKit, so the CONTRACT comes from the cross-platform CarouselView plus the iOS
// handler's behaviour, and this is its AppKit realisation):
//   * MIRRORED from iOS (src/platform/ios/collection_view_handler.mm build_compositional_layout, the
//     LayoutFactory2.CreateCarouselLayout port): a carousel PAGES — one item per page filling the
//     viewport on BOTH axes, detected by `dynamic_cast<carousel_view*>` (the same predicate the iOS
//     layout factory and the scroll writeback use), so the generic flow path is untouched. iOS spells
//     that FractionalWidth(1)/FractionalHeight(1); the flow layout has no fractional dimension, so the
//     port sets itemSize from the mapped VIEWPORT extent and zeroes the item/line spacing, which puts
//     every page boundary on an exact multiple of the page.
//   * MIRRORED from iOS: the settled page is written back through the SHARED seam
//     collection_view_handler::set_position_from_scroll (which owns the carousel-only guard, the
//     empty-source guard, the initial-position gate and the suppress gate) — never reimplemented here.
//   * WHAT APPKIT CANNOT DO (1) — a deceleration-end callback. UIKit's scrollViewDidEndDecelerating has
//     no AppKit analog: NSScrollView posts NSScrollViewDidEndLiveScrollNotification at the END OF LIVE
//     TRACKING (fingers up), and momentum may carry past it (NSScrollView.h:156-158). The port writes
//     back on that notification only. A drag/wheel/scroller settle therefore lands exactly; a
//     momentum-carried page lands on the next settle. Writing back on every didLiveScroll instead would
//     be WRONG, not merely chatty: carousel_view::raise_position_changed re-issues ScrollTo(center), so
//     a mid-drag writeback would programmatically fight the user's live scroll.
//   * WHAT APPKIT CANNOT DO (2) — a guaranteed snap. AppKit's ONLY declared snap seam is
//     -[NSCollectionViewLayout targetContentOffsetForProposedContentOffset:withScrollingVelocity:]
//     ("for layouts that want snap-to-point scrolling behavior", NSCollectionViewLayout.h:145).
//     MauiCarouselFlowLayout below implements it — the platform's own mechanism, not an invented one —
//     but nothing in AppKit documents that NSScrollView consults it for user-driven momentum (macOS
//     deceleration lives in NSScrollView, which has no UIScrollView-style deceleration-target
//     protocol), and it is UNVERIFIED on device: exercising it needs a real momentum gesture, which the
//     capture lane cannot synthesise. No timer/debounce "snap" is faked in its place.
//
// Obj-C++ with ARC. The platform struct's native slots are __bridge_retained voids released in the
// backend-defined destructor.

#import <AppKit/AppKit.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/items/collection_view_handler.hpp"
#include "maui/controls/items/grid_items_layout.hpp"
#include "maui/controls/items/groupable_items_view.hpp"
#include "maui/controls/items/items_layout.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/items/reorderable_items_view.hpp"
#include "maui/controls/items/selectable_items_view.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/items/structured_items_view.hpp"
#include "maui/controls/scroll_to_position.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/rect.hpp"

namespace
{
    // The fixed per-item estimate (see the file header's deviation note). A generous default so the
    // flow layout realizes a sensible window during the test layout pass.
    constexpr CGFloat k_estimated_item_extent = 44;
    // The section header/footer reference extent when a header/footer is present.
    constexpr CGFloat k_supplementary_extent = 28;

    // The reuse identifiers — NSCollectionView keys items by their registered identifier; one shared
    // id for every item (so they all recycle into one pool), and one per supplementary element kind.
    NSString* const k_item_reuse_id = @"maui.collection_view.item";
    NSString* const k_header_reuse_id = @"maui.collection_view.header";
    NSString* const k_footer_reuse_id = @"maui.collection_view.footer";
} // namespace

// ---- the unified item (TemplatedCell2 + DefaultCell2) ----
@interface MauiCollectionViewItem : NSCollectionViewItem
@property(nonatomic, strong) NSTextField* label;       // the DefaultCell label (always present)
@property(nonatomic, strong) NSView* templatedContent; // the realized data_template native view, if any
- (void)showText:(NSString*)text;
- (void)showTemplatedContent:(NSView*)content;
// Host the realized template content's native view AND retain the C++ content (which owns its handler +
// native view) for as long as this cell displays it — the C# TemplatedCell2 holding its PlatformHandler.
- (void)showTemplatedContent:(NSView*)content retainingRealized:(std::shared_ptr<maui::core::bindable_object>)realized;
// The text actually on screen: the realized template content's first NSTextField when templated,
// otherwise the default-cell label. The test seam (native_cell_text) reads this so a template-bound
// label reports its bound value, not the cell's hidden default label.
- (NSString*)displayedText;
// Route the container view's -layout (the AppKit layoutSubviews analog) back to re-frame the templated
// content on every bounds change (recycled item reused at a new size, split-view resize, etc).
- (void)layoutTemplatedContent;
@end

// The item's container view. NSCollectionViewItem builds its own top-level view lazily via -loadView; a
// plain NSView never gets a layout callback wired to anything, so a dedicated subclass overrides -layout
// (the NSView equivalent of UIView's layoutSubviews) and forwards it to the owning item.
@interface MauiCollectionViewItemContainerView : NSView
@property(nonatomic, weak) MauiCollectionViewItem* owningItem;
@end

@implementation MauiCollectionViewItemContainerView
- (void)layout
{
    [super layout];
    [self.owningItem layoutTemplatedContent];
}
@end

@implementation MauiCollectionViewItem
{
    // The realized data_template content (owns its attached handler + native view). Held so the hosted
    // native view outlives this method call; released on reuse/replacement (the C# cell drops its
    // PlatformHandler + VirtualView when it re-binds).
    std::shared_ptr<maui::core::bindable_object> _realizedContent;
}

// NSCollectionViewItem builds its view lazily; supply a flat container so we can host either surface.
- (void)loadView
{
    MauiCollectionViewItemContainerView* const container =
        [[MauiCollectionViewItemContainerView alloc] initWithFrame:NSMakeRect(0, 0, 100, k_estimated_item_extent)];
    container.owningItem = self;
    container.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    _label = [NSTextField labelWithString:@""];
    _label.frame = container.bounds;
    _label.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [container addSubview:_label];
    self.view = container;
}

- (void)showText:(NSString*)text
{
    [_templatedContent removeFromSuperview];
    _templatedContent = nil;
    _realizedContent.reset();
    _label.hidden = NO;
    _label.stringValue = text;
}

- (void)showTemplatedContent:(NSView*)content
{
    _label.hidden = YES;
    if (_templatedContent == content)
    {
        return;
    }
    [_templatedContent removeFromSuperview];
    _templatedContent = content;
    if (content != nil)
    {
        [self.view addSubview:content];
        [self layoutTemplatedContent];
    }
}

// Frame the hosted template content within the item's view, honoring the realized root's Margin /
// HorizontalLayoutAlignment via the cross-platform measure + arrange pipeline (compute_frame) — mirrors
// the iOS MauiCollectionViewCell layoutTemplatedContent. A bare `content.frame = self.view.bounds` assignment
// (the prior behavior) never runs the layout pipeline, so a templated view's Margin never gets translated
// into an inset frame. Re-run from MauiCollectionViewItemContainerView's -layout so a recycled item reused
// at a new size, or a bounds change, re-applies the frame — autoresizing alone cannot express Margin or a
// non-Fill alignment, and clearing it below lets measure/arrange own the frame instead.
- (void)layoutTemplatedContent
{
    if (_templatedContent == nil)
    {
        return;
    }
    const NSRect bounds = self.view.bounds;
    auto* const view = dynamic_cast<maui::core::i_view*>(_realizedContent.get());
    if (view == nullptr)
    {
        // A non-view root can't be arranged — stretch it across the whole item via flexible autoresizing.
        _templatedContent.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        _templatedContent.frame = bounds;
        return;
    }
    _templatedContent.autoresizingMask = NSViewNotSizable;
    view->measure(bounds.size.width, bounds.size.height);
    view->arrange(maui::graphics::rect{0.0, 0.0, bounds.size.width, bounds.size.height});
}

- (void)showTemplatedContent:(NSView*)content retainingRealized:(std::shared_ptr<maui::core::bindable_object>)realized
{
    // Retain the realized content FIRST (it owns the handler that owns `content`) so replacing the
    // previous content never frees the incoming native view mid-swap.
    _realizedContent = std::move(realized);
    [self showTemplatedContent:content];
}

- (void)prepareForReuse
{
    [super prepareForReuse];
    // The recycler hands this instance back; clear the templated content so a fresh bind re-hosts, and
    // drop the realized content (its handler + native view) — the C# cell dropping its PlatformHandler.
    [_templatedContent removeFromSuperview];
    _templatedContent = nil;
    _realizedContent.reset();
    _label.hidden = NO;
}

// Depth-first search for the first NSTextField in a view subtree (the realized label's native field).
+ (NSTextField*)firstTextFieldIn:(NSView*)root
{
    if ([root isKindOfClass:[NSTextField class]])
    {
        return (NSTextField*)root;
    }
    for (NSView* const sub in root.subviews)
    {
        if (NSTextField* const found = [MauiCollectionViewItem firstTextFieldIn:sub])
        {
            return found;
        }
    }
    return nil;
}

- (NSString*)displayedText
{
    if (_templatedContent != nil)
    {
        if (NSTextField* const field = [MauiCollectionViewItem firstTextFieldIn:_templatedContent])
        {
            return field.stringValue;
        }
    }
    return _label.stringValue;
}
@end

// ---- the unified supplementary view (section/group header & footer) ----
@interface MauiCollectionSupplementaryView : NSView <NSCollectionViewElement>
@property(nonatomic, strong) NSTextField* label;
- (void)showText:(NSString*)text;
@end

@implementation MauiCollectionSupplementaryView
- (instancetype)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        _label = [NSTextField labelWithString:@""];
        _label.frame = self.bounds;
        _label.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        [self addSubview:_label];
    }
    return self;
}

- (void)showText:(NSString*)text
{
    _label.stringValue = text;
}
@end

// ---- the carousel paging layout (used ONLY on the carousel branch of build_flow_layout) ----
// The flow layout the carousel pages with. Identical to NSCollectionViewFlowLayout except that it
// implements AppKit's declared snap seam, -targetContentOffsetForProposedContentOffset:
// withScrollingVelocity: ("return a point at which to rest after scrolling - for layouts that want
// snap-to-point scrolling behavior", NSCollectionViewLayout.h:145) — the closest AppKit gets to the
// Android PagerSnapHelper / UICollectionView paging. The page pitch is itemSize on the scroll axis,
// which is exact because the carousel branch zeroes the line/interitem spacing and the section inset.
// See the file header: whether AppKit actually consults this for user-driven momentum is UNVERIFIED.
@interface MauiCarouselFlowLayout : NSCollectionViewFlowLayout
@end

@implementation MauiCarouselFlowLayout
- (NSPoint)targetContentOffsetForProposedContentOffset:(NSPoint)proposedContentOffset
                                 withScrollingVelocity:(NSPoint)velocity
{
    (void)velocity; // the carousel snaps to the NEAREST page, not a velocity-projected one
    const BOOL horizontal = self.scrollDirection == NSCollectionViewScrollDirectionHorizontal;
    const CGFloat page = horizontal ? self.itemSize.width : self.itemSize.height;
    if (page < 1)
    {
        return proposedContentOffset; // degenerate page extent (unsized viewport) — leave the offset be
    }
    NSPoint snapped = proposedContentOffset;
    if (horizontal)
    {
        snapped.x = std::round(proposedContentOffset.x / page) * page;
    }
    else
    {
        snapped.y = std::round(proposedContentOffset.y / page) * page;
    }
    return snapped;
}
@end

// ---- the empty-view host marker (NSView.tag is read-only, unlike UIView.tag, so the empty host is
//      a distinct subclass the test/teardown locates by class — the C# EmptyTag analog) ----
@interface MauiCollectionEmptyHostView : NSView
@end
@implementation MauiCollectionEmptyHostView
@end

namespace maui::controls
{
    namespace
    {
        NSString* to_nsstring(const std::string& value)
        {
            return [NSString stringWithUTF8String:value.c_str()];
        }
    } // namespace
} // namespace maui::controls

// ---- the unified data source + delegate (reads through the C++ handler) ----
@interface MauiCollectionDataSource : NSObject <NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout>
// A raw back-pointer to the C++ handler that owns this data source (set in on_connect_handler; the
// handler outlives the data source — the platform struct, which holds it, is owned by the handler).
// Stored as a void* to keep this Obj-C interface free of C++ types.
@property(nonatomic, assign) void* cppHandler;
// Every distinct item instance the recycler has ever vended (by pointer): the apple seam suite checks
// this stays bounded under scroll while the visible-item paths sweep the whole source.
@property(nonatomic, strong) NSMutableSet<NSValue*>* seenItemPointers;
// The end-of-live-scroll settle — the AppKit stand-in for the iOS controller's
// scrollViewDidEndDecelerating (see the file header's carousel deviation). Resolves the settled page and
// hands it to the SHARED collection_view_handler::set_position_from_scroll, which owns every guard.
- (void)scrollViewDidEndLiveScroll:(NSNotification*)notification;
@end

namespace maui::controls
{
    namespace
    {
        bool handler_is_horizontal(const collection_view_handler& handler)
        {
            const auto* platform = handler.typed_platform_view();
            return platform != nullptr && platform->orientation == items_layout_orientation::horizontal;
        }

        MauiCollectionDataSource* data_source_of(const collection_view_handler& handler)
        {
            const auto* platform = handler.typed_platform_view();
            if (platform == nullptr || platform->data_source == nullptr)
            {
                return nil;
            }
            return (__bridge MauiCollectionDataSource*)platform->data_source;
        }

        NSCollectionView* collection_view_of(const collection_view_handler& handler)
        {
            const auto* platform = handler.typed_platform_view();
            if (platform == nullptr || platform->scroll == nullptr)
            {
                return nil;
            }
            NSScrollView* const scroll = (__bridge NSScrollView*)platform->scroll;
            return (NSCollectionView*)scroll.documentView;
        }

        // Whether the control wants a section header / footer (structured global, or grouped per-group).
        void header_footer_wanted(collection_view_handler& handler, bool& want_header, bool& want_footer)
        {
            want_header = false;
            want_footer = false;
            const auto* platform = handler.typed_platform_view();
            auto* view = handler.virtual_view();
            const bool grouped = platform != nullptr && platform->grouped;
            if (auto* groupable = dynamic_cast<groupable_items_view*>(view); groupable != nullptr && grouped)
            {
                want_header = groupable->group_header_template() != nullptr;
                want_footer = groupable->group_footer_template() != nullptr;
            }
            else if (auto* structured = dynamic_cast<structured_items_view*>(view); structured != nullptr)
            {
                want_header = structured->header().has_value() || structured->header_template() != nullptr;
                want_footer = structured->footer().has_value() || structured->footer_template() != nullptr;
            }
        }

        // Build the NSCollectionViewFlowLayout for the control's current items_layout + header state
        // (the LayoutFactory analog — see the file-header deviation note: FlowLayout, not compositional).
        // Returns a retained NSCollectionViewFlowLayout (caller owns it).
        NSCollectionViewFlowLayout* build_flow_layout(collection_view_handler& handler)
        {
            const auto* platform = handler.typed_platform_view();
            const bool horizontal =
                platform != nullptr && platform->orientation == items_layout_orientation::horizontal;
            const int span = platform != nullptr ? std::max(1, platform->span) : 1;
            // A CAROUSEL pages: one item per page filling the viewport (the iOS
            // FractionalWidth(1)/FractionalHeight(1) shape — see the file header). Detected by the same
            // dynamic_cast predicate iOS uses, so nothing here changes for a plain CollectionView.
            const bool is_carousel = dynamic_cast<carousel_view*>(handler.virtual_view()) != nullptr;
            // Zero spacing on a carousel so a page boundary lands on an exact multiple of the page
            // (MauiCarouselFlowLayout's snap arithmetic depends on that, and MAUI's carousel has no
            // inter-page gutter — the iOS layout gets its pitch from the full-viewport group).
            const CGFloat item_spacing =
                (platform != nullptr && !is_carousel) ? static_cast<CGFloat>(platform->item_spacing) : 0;

            NSCollectionViewFlowLayout* const layout =
                is_carousel ? [[MauiCarouselFlowLayout alloc] init] : [[NSCollectionViewFlowLayout alloc] init];
            layout.scrollDirection =
                horizontal ? NSCollectionViewScrollDirectionHorizontal : NSCollectionViewScrollDirectionVertical;
            layout.minimumLineSpacing = item_spacing;
            layout.minimumInteritemSpacing = item_spacing; // cross-axis spacing between grid columns
            layout.sectionInset = NSEdgeInsetsZero;

            // The cross-axis extent. NSCollectionViewFlowLayout validates the item cross-extent against
            // the REALIZED collection-view content width (which can be a few points under the requested
            // viewport — clip-view bounds, scroller inset), so prefer the LIVE collection-view bounds
            // when available, falling back to the mapped viewport mirror; then reserve a comfortable
            // margin so the item cross-extent is STRICTLY less than the available width (avoids the
            // AppKit "item width must be less than the width" diagnostic).
            NSCollectionView* const live = collection_view_of(handler);
            const CGFloat mirror_cross =
                platform != nullptr ? static_cast<CGFloat>(platform->viewport_cross_extent) : 200;
            const CGFloat live_cross =
                live != nil ? (horizontal ? live.bounds.size.height : live.bounds.size.width) : 0;
            CGFloat cross = mirror_cross;
            if (live_cross > 1 && live_cross < cross)
            {
                cross = live_cross; // never inflate past the realized width
            }
            constexpr CGFloat k_cross_margin = 4; // comfortable strict-less-than slack
            const CGFloat available = std::max<CGFloat>(1, cross - k_cross_margin - (item_spacing * (span - 1)));
            const CGFloat per = std::max<CGFloat>(1, available / span);
            // The SCROLL-axis extent. A plain CollectionView flows items at the fixed estimate; a
            // carousel's item IS the page, so it takes the whole mapped viewport on the scroll axis.
            // The MIRROR is the only honest source here: the live NSCollectionView is the scroll view's
            // DOCUMENT view, whose scroll-axis bounds is the CONTENT extent (all pages), not the
            // viewport — reading it would grow the page by a factor of the item count on every rebuild.
            // (The cross axis keeps the k_cross_margin slack the generic path reserves: AppKit requires
            // the item cross-extent to be STRICTLY less than the collection's, so a page is 4pt short of
            // the viewport across — the flow layout's price for the deviation.)
            const CGFloat main =
                is_carousel
                    ? std::max<CGFloat>(1, static_cast<CGFloat>(platform != nullptr ? platform->viewport_main_extent
                                                                                    : k_estimated_item_extent))
                    : k_estimated_item_extent;
            layout.itemSize = horizontal ? NSMakeSize(main, per) : NSMakeSize(per, main);

            // Section header/footer span the cross axis, reserving the same margin as items so the flow
            // layout never reports the boundary supplementary exceeding the collection width.
            const CGFloat supplementary_cross = std::max<CGFloat>(1, cross - k_cross_margin);
            bool want_header = false;
            bool want_footer = false;
            header_footer_wanted(handler, want_header, want_footer);
            if (horizontal)
            {
                layout.headerReferenceSize =
                    want_header ? NSMakeSize(k_supplementary_extent, supplementary_cross) : NSZeroSize;
                layout.footerReferenceSize =
                    want_footer ? NSMakeSize(k_supplementary_extent, supplementary_cross) : NSZeroSize;
            }
            else
            {
                layout.headerReferenceSize =
                    want_header ? NSMakeSize(supplementary_cross, k_supplementary_extent) : NSZeroSize;
                layout.footerReferenceSize =
                    want_footer ? NSMakeSize(supplementary_cross, k_supplementary_extent) : NSZeroSize;
            }
            return layout;
        }

        // Resolve a possibly-selector item template against one item (DataTemplateSelector.SelectTemplate;
        // the container is the items view itself, like C# passes the ItemsView). Mirrors the cross-platform
        // resolve_template in collection_view_handler.cpp (kept local — that one is .cpp-internal).
        std::shared_ptr<data_template> resolve_item_template(const std::shared_ptr<data_template>& candidate,
                                                             const boxed_item& item,
                                                             maui::core::bindable_object* container)
        {
            if (auto selector = std::dynamic_pointer_cast<data_template_selector>(candidate))
            {
                return selector->select_template(item.context_box(), container);
            }
            return candidate;
        }

        // On-demand mount of a realized template's DESCENDANTS. A composite cell (the carousel's Border
        // owning a Label, cv_visual_states' line_item_cell, HeaderFooterTemplate's Grid+Image+Label)
        // gets only its TOP-LEVEL handler attached below; its children are not logical children of the
        // page tree, so the page-level mount (app_host::mount_tree) never builds their native views and
        // the composite renders as an EMPTY container — the AppKit carousel's blank card. Straight port
        // of the iOS ensure_mounted (src/platform/ios/collection_view_handler.mm:873, itself the Android
        // twin), and it mirrors app_host::mount_tree exactly: depth-first POST-ORDER (each child's
        // native view exists before its parent hosts it), attach by the element's runtime
        // handler_type_tag with SetMauiContext before SetVirtualView (the C# order), then re-fire the
        // container host command so the now-attached children are hosted. Idempotent — an element that
        // already carries a handler is skipped, since re-attaching would rebuild and orphan its native
        // view. A leaf template (a bare Label) has no children, so this is a cheap no-op there.
        void ensure_mounted(maui::core::i_maui_context* context, maui::controls::element& root)
        {
            if (context == nullptr)
            {
                return;
            }
            root.visit_logical_children([context](maui::controls::element& child) { ensure_mounted(context, child); });

            auto* element_face = dynamic_cast<maui::core::i_element*>(&root);
            if (element_face == nullptr)
            {
                return;
            }
            if (!element_face->handler()) // skip an already-mounted element (idempotent re-mount guard)
            {
                if (const std::optional<maui::core::type_tag> tag = root.handler_type_tag(); tag.has_value())
                {
                    if (std::shared_ptr<maui::core::i_element_handler> handler =
                            context->handlers().create_handler(*tag))
                    {
                        handler->set_maui_context(context);            // SetMauiContext precedes SetVirtualView (C#)
                        element_face->set_handler(std::move(handler)); // the view owns its handler (PROFILE §11)
                    }
                }
            }
            root.mount_into_handler(); // re-host the (now-attached) children's native views
        }

        // Realize a type-activated template's content into a native NSView (the C# TemplatedCell2.Bind:
        // `CreateContent(...) as View` → set BindingContext → `view.ToPlatform(mauiContext)`). Returns the
        // realized content (which OWNS its attached handler + native view — the caller keeps it alive for
        // the cell's lifetime) and, out-param, its native NSView. Yields {nullptr, nil} when the template
        // is loader-only (no static control type) or no handler is registered for that type — the cell
        // then falls back to the item-text mirror, exactly as before.
        std::shared_ptr<maui::core::bindable_object> realize_template_content(
            collection_view_handler& handler, const std::shared_ptr<data_template>& tmpl, const boxed_item& value,
            NSView** out_native)
        {
            *out_native = nil;
            maui::core::i_maui_context* const context = handler.maui_context();
            if (tmpl == nullptr || context == nullptr || !tmpl->content_type().has_value())
            {
                return nullptr;
            }
            std::shared_ptr<maui::core::bindable_object> content = tmpl->create_content();
            if (!content)
            {
                return nullptr;
            }
            // BindingContext = the item (so the template's staged bindings resolve against it). Set BEFORE
            // attaching the handler so the first mapper pass already sees the bound property values.
            content->set_binding_context_box(value.context_box());

            std::shared_ptr<maui::core::i_element_handler> child_handler =
                context->handlers().create_handler(*tmpl->content_type());
            auto* element = dynamic_cast<maui::core::i_element*>(content.get());
            if (!child_handler || element == nullptr)
            {
                return nullptr; // no registered handler (or non-element content) — fall back to text
            }
            child_handler->set_maui_context(context);
            element->set_handler(child_handler); // creates the platform view + runs the mapper

            // Build the composite template's own children on demand (see ensure_mounted above) — without
            // this a Border/Grid cell hosts nothing and paints an empty box.
            if (auto* content_element = dynamic_cast<maui::controls::element*>(content.get()))
            {
                ensure_mounted(context, *content_element);
            }

            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(child_handler.get()))
            {
                *out_native = (__bridge NSView*)view_handler->native_view();
            }
            return content;
        }
    } // namespace

    // ---- the native bridge (called from the cross-platform .cpp under #ifdef MAUI_PLATFORM_APPLE) ----

    void collection_view_handler::on_connect_handler(collection_view_platform& platform)
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (collection_view == nil || platform.data_source == nullptr)
        {
            return;
        }
        auto* const source = (__bridge MauiCollectionDataSource*)platform.data_source;
        source.cppHandler = this; // wire the back-pointer (create_platform_view is static)
        collection_view.dataSource = source;
        collection_view.delegate = source;
        [collection_view reloadData];
    }

    void collection_view_handler::native_reload()
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (collection_view == nil)
        {
            return;
        }
        MauiCollectionDataSource* const source = data_source_of(*this);
        if (source != nil)
        {
            source.cppHandler = this; // keep the back-pointer fresh (on_disconnect calls reload after null source)
        }
        [collection_view reloadData];
    }

    void collection_view_handler::native_rebuild_layout()
    {
        auto* platform = typed_platform_view();
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (platform == nullptr || collection_view == nil)
        {
            return;
        }
        NSCollectionViewFlowLayout* const layout = build_flow_layout(*this);
        if (platform->flow_layout != nullptr)
        {
            CFRelease(platform->flow_layout);
        }
        platform->flow_layout = (__bridge_retained void*)layout;
        collection_view.collectionViewLayout = layout;
    }

    void collection_view_handler::native_update_selection_mode()
    {
        auto* platform = typed_platform_view();
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (platform == nullptr || collection_view == nil)
        {
            return;
        }
        collection_view.selectable = platform->allows_selection;
        collection_view.allowsMultipleSelection = platform->allows_multiple_selection;
        collection_view.allowsEmptySelection = YES;
    }

    void collection_view_handler::native_update_platform_selection()
    {
        auto* platform = typed_platform_view();
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (platform == nullptr || collection_view == nil)
        {
            return;
        }
        // Deselect everything the native view currently has, then re-select the handler's mirror
        // (platform->selected_paths is kept in sync by the cross-platform update_platform_selection).
        NSMutableSet<NSIndexPath*>* const desired = [NSMutableSet set];
        for (const index_path& path : platform->selected_paths)
        {
            if (path.section < 0 || path.item < 0)
            {
                continue;
            }
            [desired addObject:[NSIndexPath indexPathForItem:path.item inSection:path.section]];
        }
        NSSet<NSIndexPath*>* const current = collection_view.selectionIndexPaths;
        NSMutableSet<NSIndexPath*>* const to_deselect = [current mutableCopy];
        [to_deselect minusSet:desired];
        if (to_deselect.count > 0)
        {
            [collection_view deselectItemsAtIndexPaths:to_deselect];
        }
        if (desired.count > 0)
        {
            [collection_view selectItemsAtIndexPaths:desired scrollPosition:NSCollectionViewScrollPositionNone];
        }
    }

    void collection_view_handler::native_update_empty_view()
    {
        auto* platform = typed_platform_view();
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (platform == nullptr || collection_view == nil)
        {
            return;
        }
        // Tear down any previously-shown empty view.
        if (platform->empty_view_native != nullptr)
        {
            NSView* const old = (__bridge_transfer NSView*)platform->empty_view_native;
            [old removeFromSuperview];
            platform->empty_view_native = nullptr;
        }

        const bool empty = !source_ || source_->item_count() == 0;
        if (!empty || virtual_view() == nullptr)
        {
            return;
        }

        // Realize the empty view as a tagged host with a text-mirror label. The boxed-view /
        // EmptyViewTemplate realization lives in the cross-platform supplemental record; on-device the
        // text mirror is the asserted surface — documented simplification.
        auto* view = virtual_view();
        const boxed_item& empty_value = view->empty_view();

        MauiCollectionEmptyHostView* const host =
            [[MauiCollectionEmptyHostView alloc] initWithFrame:collection_view.bounds];
        host.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

        NSTextField* const label = [NSTextField labelWithString:to_nsstring(empty_value.text())];
        label.frame = host.bounds;
        label.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        label.alignment = NSTextAlignmentCenter;
        [host addSubview:label];

        [collection_view addSubview:host];
        platform->empty_view_native = (__bridge_retained void*)host;
    }

    void collection_view_handler::native_update_can_reorder()
    {
        // C# ReorderableItemsViewController.UpdateCanReorderItems (un)installs the drag handling. The
        // port reorders by mutating the bound (typed observable) collection — exactly what C#'s MoveItem
        // does to the underlying IList — and reports through send_reorder_completed; the resulting
        // source_update fans into native_reload so the items re-render in the new order. The gate lives
        // in send_reorder_completed (CanReorderItems). The erased i_item_collection is read-only, so
        // there is no native interactive-drag mutator to install here — documented (mirrors iOS).
    }

    void collection_view_handler::native_scroll_to(const index_path& path, controls::scroll_to_position position,
                                                   bool /*animate*/)
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (collection_view == nil || path.section < 0 || path.item < 0)
        {
            return;
        }
        if (path.section >= collection_view.numberOfSections ||
            path.item >= [collection_view numberOfItemsInSection:path.section])
        {
            return;
        }
        const bool horizontal = handler_is_horizontal(*this);
        NSCollectionViewScrollPosition scroll_position = NSCollectionViewScrollPositionNone;
        switch (position)
        {
            case controls::scroll_to_position::start:
                scroll_position = horizontal ? NSCollectionViewScrollPositionLeft : NSCollectionViewScrollPositionTop;
                break;
            case controls::scroll_to_position::center:
                scroll_position = horizontal ? NSCollectionViewScrollPositionCenteredHorizontally
                                             : NSCollectionViewScrollPositionCenteredVertically;
                break;
            case controls::scroll_to_position::end:
                scroll_position =
                    horizontal ? NSCollectionViewScrollPositionRight : NSCollectionViewScrollPositionBottom;
                break;
            case controls::scroll_to_position::make_visible:
                scroll_position = NSCollectionViewScrollPositionNearestHorizontalEdge |
                                  NSCollectionViewScrollPositionNearestVerticalEdge;
                break;
        }
        NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:path.item inSection:path.section];
        [collection_view scrollToItemsAtIndexPaths:[NSSet setWithObject:index_path_ns] scrollPosition:scroll_position];
        [collection_view layoutSubtreeIfNeeded];
    }

    void collection_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (platform == nullptr || collection_view == nil || platform->scroll == nullptr)
        {
            return;
        }
        NSScrollView* const scroll = (__bridge NSScrollView*)platform->scroll;
        // Frame the composed NSScrollView to the arranged (bounded) rect — the backend half of
        // platform_arrange that table_view_handler/border_handler do inline. The shared platform_arrange
        // re-sets the viewport mirror + re-realizes after this returns; mirror the new viewport here FIRST
        // and rebuild the flow layout for it BEFORE resizing, the same order native_force_layout uses, so
        // the resize never runs a flow-layout pass with a stale (larger) item size against an already-
        // shrunk collection-view width (the flow-layout "item width must be less than the collection
        // width" diagnostic).
        if (platform->orientation == items_layout_orientation::vertical)
        {
            platform->viewport_main_extent = frame.height;
            platform->viewport_cross_extent = frame.width;
        }
        else
        {
            platform->viewport_main_extent = frame.width;
            platform->viewport_cross_extent = frame.height;
        }
        native_rebuild_layout();
        scroll.frame = NSMakeRect(frame.x, frame.y, frame.width, frame.height);
        [scroll layoutSubtreeIfNeeded];
        [collection_view layoutSubtreeIfNeeded];
        // The first rebuild sized the item from the pre-resize live width; now that the collection view has
        // its new bounds, rebuild once more so the item cross-extent matches exactly (native_force_layout's
        // two-pass settle).
        native_rebuild_layout();
    }

    int collection_view_handler::native_force_layout(double width, double height)
    {
        auto* platform = typed_platform_view();
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (platform == nullptr || collection_view == nil)
        {
            return 0;
        }
        NSScrollView* const scroll = (__bridge NSScrollView*)platform->scroll;
        const NSRect frame = NSMakeRect(0, 0, width, height);
        // Refresh the cross-axis mirror and rebuild the flow layout for the NEW viewport BEFORE resizing
        // the scroll view — resizing first would run a layout pass with the stale (larger) item size
        // against the already-shrunk collection-view width, tripping the flow-layout diagnostic.
        if (platform->orientation == items_layout_orientation::vertical)
        {
            platform->viewport_main_extent = height;
            platform->viewport_cross_extent = width;
        }
        else
        {
            platform->viewport_main_extent = width;
            platform->viewport_cross_extent = height;
        }
        native_rebuild_layout();
        scroll.frame = frame;
        [scroll layoutSubtreeIfNeeded];
        [collection_view layoutSubtreeIfNeeded];
        // The first rebuild sized the item from the OLD (pre-resize) live width; now that the collection
        // view has the new bounds, rebuild once more so the item cross-extent matches exactly.
        native_rebuild_layout();
        [collection_view layoutSubtreeIfNeeded];
        // C# CarouselViewController2.UpdateInitialPosition flips InitialPositionSet true once the view is
        // loaded and laid out; this layout pass is the port's analog (the same site the iOS partial uses,
        // native_force_layout). After it, scroll-end settles are allowed to write Position back
        // (set_position_from_scroll's initial_position_set_ guard).
        mark_initial_position_set();
        return static_cast<int>(collection_view.visibleItems.count);
    }

    int collection_view_handler::native_visible_cell_count() const
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        return collection_view != nil ? static_cast<int>(collection_view.visibleItems.count) : 0;
    }

    int collection_view_handler::native_distinct_cell_instances() const
    {
        MauiCollectionDataSource* const source = data_source_of(*this);
        return source != nil ? static_cast<int>(source.seenItemPointers.count) : 0;
    }

    int collection_view_handler::native_visible_supplementary_count(bool header) const
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (collection_view == nil)
        {
            return 0;
        }
        NSString* const kind = header ? NSCollectionElementKindSectionHeader : NSCollectionElementKindSectionFooter;
        return static_cast<int>([collection_view visibleSupplementaryViewsOfKind:kind].count);
    }

    int collection_view_handler::native_selected_count() const
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        return collection_view != nil ? static_cast<int>(collection_view.selectionIndexPaths.count) : 0;
    }

    bool collection_view_handler::native_empty_view_shown() const
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (collection_view == nil)
        {
            return false;
        }
        for (NSView* const sub in collection_view.subviews)
        {
            if ([sub isKindOfClass:[MauiCollectionEmptyHostView class]])
            {
                return true;
            }
        }
        return false;
    }

    void collection_view_handler::native_select(const index_path& path)
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        MauiCollectionDataSource* const source = data_source_of(*this);
        if (collection_view == nil || source == nil || path.section < 0 || path.item < 0)
        {
            return;
        }
        if (path.section >= collection_view.numberOfSections ||
            path.item >= [collection_view numberOfItemsInSection:path.section])
        {
            return;
        }
        NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:path.item inSection:path.section];
        NSSet<NSIndexPath*>* const paths = [NSSet setWithObject:index_path_ns];
        // selectItemsAtIndexPaths does NOT fire didSelect; drive both the native selection AND the
        // delegate fan-out (the C# user-click path) for fidelity (mirrors iOS native_select).
        [collection_view selectItemsAtIndexPaths:paths scrollPosition:NSCollectionViewScrollPositionNone];
        [source collectionView:collection_view didSelectItemsAtIndexPaths:paths];
    }

    void collection_view_handler::native_deselect(const index_path& path)
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        MauiCollectionDataSource* const source = data_source_of(*this);
        if (collection_view == nil || source == nil || path.section < 0 || path.item < 0)
        {
            return;
        }
        if (path.section >= collection_view.numberOfSections ||
            path.item >= [collection_view numberOfItemsInSection:path.section])
        {
            return;
        }
        NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:path.item inSection:path.section];
        NSSet<NSIndexPath*>* const paths = [NSSet setWithObject:index_path_ns];
        [collection_view deselectItemsAtIndexPaths:paths];
        [source collectionView:collection_view didDeselectItemsAtIndexPaths:paths];
    }

    std::string collection_view_handler::native_cell_text(const index_path& path) const
    {
        NSCollectionView* const collection_view = collection_view_of(*this);
        if (collection_view == nil || path.section < 0 || path.item < 0)
        {
            return {};
        }
        NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:path.item inSection:path.section];
        NSCollectionViewItem* const item = [collection_view itemAtIndexPath:index_path_ns];
        if (auto* const maui_item = (MauiCollectionViewItem*)item;
            [maui_item isKindOfClass:[MauiCollectionViewItem class]])
        {
            const char* const utf8 = [maui_item displayedText].UTF8String;
            return utf8 != nullptr ? std::string(utf8) : std::string();
        }
        return {};
    }

    // ---- creation + teardown ----

    collection_view_platform::~collection_view_platform()
    {
        // Release the retained native slots in reverse order of acquisition. The scroll view owns the
        // document NSCollectionView; releasing the scroll view tears the whole tree down.
        if (empty_view_native != nullptr)
        {
            CFRelease(empty_view_native);
            empty_view_native = nullptr;
        }
        if (flow_layout != nullptr)
        {
            CFRelease(flow_layout);
            flow_layout = nullptr;
        }
        if (data_source != nullptr)
        {
            CFRelease(data_source);
            data_source = nullptr;
        }
        if (scroll != nullptr)
        {
            CFRelease(scroll);
            scroll = nullptr;
        }
        // `native` aliases the scroll view's document collection view (NOT separately retained) —
        // nothing to free.
        native = nullptr;
    }

    std::unique_ptr<collection_view_platform> collection_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<collection_view_platform>();

        // The initial layout is a plain vertical list (the mapper rebuilds it from the real items_layout
        // on first run — C# OnCreatePlatformView builds an initial layout, then the mappers refine it).
        NSCollectionViewFlowLayout* const layout = [[NSCollectionViewFlowLayout alloc] init];
        layout.scrollDirection = NSCollectionViewScrollDirectionVertical;
        // A modest placeholder item size — the mapper rebuilds the layout from the real items_layout
        // (and the real viewport) on first run, so this only matters transiently. Kept small so an
        // initial reload into a small/zero-width view never trips the flow-layout "item width must be
        // less than the collection width" diagnostic.
        layout.itemSize = NSMakeSize(k_estimated_item_extent, k_estimated_item_extent);

        // A sensible default frame so an initial reload (before native_force_layout / platform_arrange
        // sizes the view) realizes against a non-zero viewport — a zero-width collection view would
        // otherwise trip the flow-layout "item width must be less than the collection width" diagnostic.
        const NSRect default_frame = NSMakeRect(0, 0, 320, 480);
        NSCollectionView* const collection_view = [[NSCollectionView alloc] initWithFrame:default_frame];
        collection_view.collectionViewLayout = layout;
        collection_view.backgroundColors = @[ NSColor.clearColor ];
        collection_view.selectable = NO;
        collection_view.allowsEmptySelection = YES;

        // Register the item + supplementary classes (the C# RegisterViewTypes; the unified classes mean
        // one registration covers default + templated, and one per header/footer kind).
        [collection_view registerClass:[MauiCollectionViewItem class] forItemWithIdentifier:k_item_reuse_id];
        [collection_view registerClass:[MauiCollectionSupplementaryView class]
            forSupplementaryViewOfKind:NSCollectionElementKindSectionHeader
                        withIdentifier:k_header_reuse_id];
        [collection_view registerClass:[MauiCollectionSupplementaryView class]
            forSupplementaryViewOfKind:NSCollectionElementKindSectionFooter
                        withIdentifier:k_footer_reuse_id];

        NSScrollView* const scroll = [[NSScrollView alloc] initWithFrame:default_frame];
        scroll.documentView = collection_view;
        scroll.hasVerticalScroller = YES;
        scroll.hasHorizontalScroller = YES;
        // Overlay scrollers float over the content rather than reserving layout width — so the clip
        // view's content width equals the scroll view width, and the flow-layout item cross-extent
        // (viewport width minus a margin) never exceeds the realized collection-view width (a legacy
        // scroller would shrink the content by its track width and trip the flow-layout diagnostic).
        scroll.scrollerStyle = NSScrollerStyleOverlay;

        MauiCollectionDataSource* const source = [[MauiCollectionDataSource alloc] init];
        source.seenItemPointers = [NSMutableSet set];
        // The back-pointer is wired in on_connect_handler (create_platform_view is static).
        // The carousel's settled-page writeback (see the file header): registered HERE, exactly once per
        // platform view, because this is the only site that owns both objects and runs once — the
        // observer reads the handler back-pointer at fire time, so it does not need it yet. The gate is
        // in the callback (carousel only), never in the registration: an items_layout can change under
        // the mappers, and a plain CollectionView's callback is a single dynamic_cast that returns.
        [[NSNotificationCenter defaultCenter] addObserver:source
                                                 selector:@selector(scrollViewDidEndLiveScroll:)
                                                     name:NSScrollViewDidEndLiveScrollNotification
                                                   object:scroll];

        platform->scroll = (__bridge_retained void*)scroll;
        platform->data_source = (__bridge_retained void*)source;
        platform->flow_layout = (__bridge_retained void*)layout;
        // `native` is the real NSScrollView the handler composes into the view tree (the composed
        // native). It is NOT separately retained — `scroll` already retains it; the destructor frees
        // only the scroll slot.
        platform->native = (__bridge void*)scroll;
        return platform;
    }
} // namespace maui::controls

// ---- the data source + delegate implementation (reads through the C++ handler) ----
@implementation MauiCollectionDataSource

- (void)dealloc
{
    // Drop the live-scroll observer registered in create_platform_view (deterministic teardown — the
    // §8 doctrine; NSNotificationCenter's zeroing-weak observers would tolerate it, house style does not).
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (maui::controls::collection_view_handler*)handler
{
    return static_cast<maui::controls::collection_view_handler*>(_cppHandler);
}

- (std::shared_ptr<maui::controls::i_items_view_source>)source
{
    auto* handler = [self handler];
    return handler != nullptr ? handler->items_view_source() : nullptr;
}

- (NSInteger)numberOfSectionsInCollectionView:(NSCollectionView*)collectionView
{
    (void)collectionView;
    auto source = [self source];
    return source != nullptr ? source->group_count() : 0;
}

- (NSInteger)collectionView:(NSCollectionView*)collectionView numberOfItemsInSection:(NSInteger)section
{
    (void)collectionView;
    auto source = [self source];
    if (source == nullptr || section < 0 || section >= source->group_count())
    {
        return 0;
    }
    return source->item_count_in_group(static_cast<int>(section));
}

- (NSCollectionViewItem*)collectionView:(NSCollectionView*)collectionView
    itemForRepresentedObjectAtIndexPath:(NSIndexPath*)indexPath
{
    MauiCollectionViewItem* const item = (MauiCollectionViewItem*)[collectionView makeItemWithIdentifier:k_item_reuse_id
                                                                                            forIndexPath:indexPath];
    [_seenItemPointers addObject:[NSValue valueWithNonretainedObject:item]];

    auto* handler = [self handler];
    auto source = [self source];
    if (handler == nullptr || source == nullptr)
    {
        return item;
    }

    const maui::controls::index_path path{.section = static_cast<int>(indexPath.section),
                                          .item = static_cast<int>(indexPath.item)};
    const maui::controls::boxed_item value = source->item(path);

    // TemplatedCell2 vs DefaultCell2 (the C# split): with an ItemTemplate set, realize the template's
    // content as a real native view bound to the item (so a struct item renders its template-bound
    // fields, not just item.text() — which is empty for non-string items); with no template, the default
    // cell's NSTextField mirrors item.text(). The realized content is retained on the cell for as long as
    // it hosts it (the C# cell holding its PlatformHandler), released on reuse.
    auto* itemsView = handler->virtual_view();
    const std::shared_ptr<maui::controls::data_template> tmpl =
        itemsView != nullptr
            ? maui::controls::resolve_item_template(itemsView->item_template(), value,
                                                    dynamic_cast<maui::core::bindable_object*>(itemsView))
            : nullptr;
    NSView* templated = nil;
    std::shared_ptr<maui::core::bindable_object> realized =
        maui::controls::realize_template_content(*handler, tmpl, value, &templated);
    if (realized != nullptr && templated != nil)
    {
        [item showTemplatedContent:templated retainingRealized:std::move(realized)];
    }
    else
    {
        [item showText:maui::controls::to_nsstring(value.text())];
    }
    return item;
}

- (NSView*)collectionView:(NSCollectionView*)collectionView
    viewForSupplementaryElementOfKind:(NSCollectionViewSupplementaryElementKind)kind
                          atIndexPath:(NSIndexPath*)indexPath
{
    const bool isHeader = [kind isEqualToString:NSCollectionElementKindSectionHeader];
    MauiCollectionSupplementaryView* const view = (MauiCollectionSupplementaryView*)[collectionView
        makeSupplementaryViewOfKind:kind
                     withIdentifier:(isHeader ? k_header_reuse_id : k_footer_reuse_id)forIndexPath:indexPath];

    auto* handler = [self handler];
    auto source = [self source];
    if (handler == nullptr || source == nullptr)
    {
        return view;
    }
    auto* itemsView = handler->virtual_view();
    const maui::controls::index_path path{.section = static_cast<int>(indexPath.section), .item = -1};

    if (auto* groupable = dynamic_cast<maui::controls::groupable_items_view*>(itemsView);
        groupable != nullptr && groupable->is_grouped())
    {
        // Grouped: the supplementary's context is the group KEY object.
        const maui::controls::boxed_item group = source->group(path);
        [view showText:maui::controls::to_nsstring(group.text())];
    }
    else if (auto* structured = dynamic_cast<maui::controls::structured_items_view*>(itemsView); structured != nullptr)
    {
        // Structured: the global header/footer object.
        const maui::controls::boxed_item& value = isHeader ? structured->header() : structured->footer();
        [view showText:maui::controls::to_nsstring(value.text())];
    }
    return view;
}

// ---- selection (the user-click path; programmatic selection goes through native_update_platform_selection) ----

- (void)collectionView:(NSCollectionView*)collectionView didSelectItemsAtIndexPaths:(NSSet<NSIndexPath*>*)indexPaths
{
    (void)collectionView;
    auto* handler = [self handler];
    if (handler == nullptr)
    {
        return;
    }
    for (NSIndexPath* const indexPath in indexPaths)
    {
        handler->simulate_select(maui::controls::index_path{.section = static_cast<int>(indexPath.section),
                                                            .item = static_cast<int>(indexPath.item)});
    }
}

- (void)collectionView:(NSCollectionView*)collectionView didDeselectItemsAtIndexPaths:(NSSet<NSIndexPath*>*)indexPaths
{
    (void)collectionView;
    auto* handler = [self handler];
    if (handler == nullptr)
    {
        return;
    }
    for (NSIndexPath* const indexPath in indexPaths)
    {
        handler->simulate_deselect(maui::controls::index_path{.section = static_cast<int>(indexPath.section),
                                                              .item = static_cast<int>(indexPath.item)});
    }
}

// ---- carousel settled-page writeback (the AppKit scroll seam) ----
//
// The iOS twin is the controller's scrollViewDidEndDecelerating → writeBackCenteredPosition
// (src/platform/ios/collection_view_handler.mm). AppKit has no deceleration-end callback, so the port
// settles on NSScrollViewDidEndLiveScrollNotification — see the file header's carousel deviation for
// what that costs (momentum) and why writing back mid-scroll would be wrong rather than merely chatty.
//
// The settled PAGE is the offset in page units: the carousel branch of build_flow_layout makes the page
// pitch exactly itemSize on the scroll axis (zero spacing, zero section inset), so this needs no
// per-item layout-attribute sweep — unlike iOS, whose compositional pages carry the peek insets.
// EVERY guard (carousel-only, empty source, initial position, range, the batch-update suppress) lives
// in the shared set_position_from_scroll; nothing is re-implemented here.
- (void)scrollViewDidEndLiveScroll:(NSNotification*)notification
{
    auto* handler = [self handler];
    if (handler == nullptr || handler->typed_platform_view() == nullptr ||
        dynamic_cast<maui::controls::carousel_view*>(handler->virtual_view()) == nullptr)
    {
        return; // only the carousel writes scroll position back; a plain collection does not
    }
    NSScrollView* const scroll = (NSScrollView*)notification.object;
    if (![scroll isKindOfClass:[NSScrollView class]])
    {
        return;
    }
    NSCollectionViewFlowLayout* const layout =
        (NSCollectionViewFlowLayout*)((NSCollectionView*)scroll.documentView).collectionViewLayout;
    if (![layout isKindOfClass:[NSCollectionViewFlowLayout class]])
    {
        return;
    }
    const BOOL horizontal =
        handler->typed_platform_view()->orientation == maui::controls::items_layout_orientation::horizontal;
    const CGFloat page = horizontal ? layout.itemSize.width : layout.itemSize.height;
    if (page < 1)
    {
        return; // degenerate page extent (viewport never sized) — no page to resolve
    }
    // NSCollectionView is flipped, so the document-visible origin grows with the item ordinal on both
    // axes; the nearest page to the settled origin is the item now on screen.
    const NSRect visible = scroll.documentVisibleRect;
    const CGFloat offset = horizontal ? NSMinX(visible) : NSMinY(visible);
    handler->set_position_from_scroll(static_cast<int>(std::lround(offset / page)));
}

@end
