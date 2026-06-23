// collection_view_handler — iOS (UIKit) platform partial: the REAL native virtualization stack
// (W3-29), the Items2 COMPOSITIONAL path. A genuine UICollectionView + UICollectionViewController is
// driven through a compositional UICollectionViewCompositionalLayout (built by layout_factory from the
// cross-platform items_layout), with a unified controller/cell/source architecture:
//
//   - MauiItemsCollectionViewController  <=  ItemsViewController2<TItemsView> + the Structured /
//     Selectable / Groupable / Reorderable controllers COLLAPSED into one class (the port already
//     collapsed the matching mapper chain in collection_view_handler; the carousel-only LoopSource /
//     orthogonal-scroll behavior is out of scope). It is the UICollectionViewDataSource +
//     UICollectionViewDelegate, reads the handler's i_items_view_source, dequeues recycled cells, and
//     fans user selection / reorder back to the control.
//   - MauiTemplatedCell  <=  TemplatedCell2 / DefaultCell2 (unified): hosts the data_template's native
//     content when an ItemTemplate is set, else a UILabel mirroring item.text() (the C# DefaultCell2
//     label). UICollectionView recycles it via PrepareForReuse — the recycler reuses cell INSTANCES,
//     which the on-simulator suite asserts under programmatic scroll.
//   - MauiSupplementaryView  <=  the *SupplementaryView2 / *DefaultSupplementalView2 family: the group
//     header/footer (grouped) and section header/footer (structured) boundary supplementary items.
//   - layout_factory  <=  LayoutFactory2: builds the NSCollectionLayoutSection (item → group → section,
//     boundary supplementary items for header/footer) and wraps it in a
//     UICollectionViewCompositionalLayout. Linear (1 column) and grid (Span columns) over both
//     orientations; snap points + KeepLastItemInView are documented simplifications (see notes).
//
// DOCUMENTED DEVIATION: the classic-Items iOS UICollectionViewFlowLayout path
// (ItemsViewController / ItemsViewLayout) is NOT ported — iOS uses the Items2 compositional path
// exclusively, exactly as the modern MAUI default. estimated-size self-sizing cells (the C# auto-
// measure via PreferredLayoutAttributesFittingAttributes) are reduced to a fixed per-cell estimate so
// the layout is deterministic under test; the data/selection/grouping/reorder/scroll semantics are
// faithful. The cross-platform simulator (collection_view_handler.cpp) still runs as the in-memory
// state mirror on this backend; these natives are what the on-simulator suite asserts against.
//
// Obj-C++ with ARC. The platform struct's native slots are __bridge_retained voids released in the
// backend-defined destructor.

#import <UIKit/UIKit.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

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
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    // The fixed per-cell estimate (see the file header's deviation note). A generous default so the
    // compositional layout realizes a sensible window during the test layout pass.
    constexpr CGFloat k_estimated_item_extent = 44;

    // The reuse identifiers — the recycler keys cells by class+id; the DEFAULT id is shared by every
    // default cell (so they all recycle into one pool), templated cells key off the template id_string
    // (the C# `{cellType}.{orientation}.{dataTemplate.Id}` reuse id, reduced to the template id).
    NSString* const k_default_cell_reuse_id = @"maui.collection_view.default_cell";
    NSString* const k_header_reuse_id = @"maui.collection_view.header";
    NSString* const k_footer_reuse_id = @"maui.collection_view.footer";
} // namespace

// ---- the unified cell (TemplatedCell2 + DefaultCell2) ----
@interface MauiCollectionViewCell : UICollectionViewCell
@property(nonatomic, strong) UILabel* label;           // the DefaultCell2 label (always present)
@property(nonatomic, strong) UIView* templatedContent; // the realized data_template native view, if any
// The collection's scroll direction (TemplatedCell2.ScrollDirection) — set on each dequeue from the
// handler's orientation so preferredLayoutAttributesFittingAttributes self-sizes the cell on the SCROLL
// axis (height for a vertical list, width for a horizontal one), exactly the C# self-measure.
@property(nonatomic, assign) UICollectionViewScrollDirection scrollDirection;
- (void)showText:(NSString*)text;
- (void)showTemplatedContent:(UIView*)content;
// Host the realized template content's native view AND retain the C++ content (which owns its handler +
// native view) for as long as this cell displays it — the C# TemplatedCell2 holding its PlatformHandler.
- (void)showTemplatedContent:(UIView*)content retainingRealized:(std::shared_ptr<maui::core::bindable_object>)realized;
// The text actually on screen: the realized template content's first UILabel when templated, otherwise
// the default-cell label. The test seam (native_cell_text) reads this so a template-bound label reports
// its bound value, not the cell's hidden default label.
- (NSString*)displayedText;
@end

@implementation MauiCollectionViewCell
{
    // The realized data_template content (owns its attached handler + native view). Held so the hosted
    // native view outlives this method call; released on reuse/replacement (the C# cell dropping its
    // PlatformHandler).
    std::shared_ptr<maui::core::bindable_object> _realizedContent;
}
- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        _label = [[UILabel alloc] initWithFrame:self.contentView.bounds];
        _label.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        [self.contentView addSubview:_label];

        // Selected-cell visual state (the C# CollectionView default selection highlight). UIKit shows
        // selectedBackgroundView automatically while the cell isSelected; the handler only ever selects a
        // cell when SelectionMode != None (selectItemAtIndexPath runs from the selection-sync /
        // programmatic-select paths), so a None-mode cell never highlights. MAUI's default selection fill
        // is ItemsViewCell's ColorExtensions.Gray == UIColor.systemGray (a medium adaptive gray), NOT the
        // lighter systemGray4 — the port used Gray4, rendering a too-light highlight vs the ref.
        UIView* const selectedBg = [[UIView alloc] initWithFrame:self.contentView.bounds];
        selectedBg.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        selectedBg.backgroundColor = [UIColor systemGrayColor];
        self.selectedBackgroundView = selectedBg;
    }
    return self;
}

- (void)showText:(NSString*)text
{
    [_templatedContent removeFromSuperview];
    _templatedContent = nil;
    _realizedContent.reset();
    _label.hidden = NO;
    _label.text = text;
}

- (void)showTemplatedContent:(UIView*)content
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
        [self.contentView addSubview:content];
        [self layoutTemplatedContent];
    }
}

- (void)showTemplatedContent:(UIView*)content retainingRealized:(std::shared_ptr<maui::core::bindable_object>)realized
{
    // Retain the realized content FIRST (it owns the handler that owns `content`) so replacing the
    // previous content never frees the incoming native view mid-swap.
    _realizedContent = std::move(realized);
    [self showTemplatedContent:content];
}

// Frame the hosted template content within the cell's contentView, honoring the realized root's
// HorizontalLayoutAlignment — the C# TemplatedCell2 measuring + arranging its content view rather than
// stretching it. A Fill root (the default — what almost every template uses) spans the full cell width,
// exactly as before. A Start / Center / End root is measured and sized to its own desired width, then
// left / center / right-aligned within the cell (the chat-bubble / pill behavior) by routing through the
// cross-platform measure + arrange: compute_frame resolves the aligned X and the margin-inset extent and
// pushes the frame to the native view via platform_arrange (recursing into any child views). Re-run from
// layoutSubviews so a recycled cell or a bounds change re-applies the frame — autoresizing alone cannot
// express the non-Fill alignments.
- (void)layoutTemplatedContent
{
    if (_templatedContent == nil)
    {
        return;
    }
    const CGRect bounds = self.contentView.bounds;
    auto* const view = dynamic_cast<maui::core::i_view*>(_realizedContent.get());
    if (view == nullptr || view->horizontal_layout_alignment() == maui::core::layout_alignment::fill)
    {
        // Fill (or a non-view root): the content spans the whole cell. Flexible autoresizing keeps it
        // stretched across UIKit's own layout passes (the prior behavior — most templates rely on this).
        _templatedContent.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        _templatedContent.frame = bounds;
        return;
    }
    // Content-width: measure the root against the cell, then arrange it within the cell bounds. arrange
    // resolves the aligned frame (compute_frame) and sets the native view's frame via platform_arrange,
    // so a Start/Center/End bubble lands content-width at the correct edge. autoresizing is cleared —
    // layoutSubviews re-runs this on every bounds change.
    _templatedContent.autoresizingMask = UIViewAutoresizingNone;
    view->measure(bounds.size.width, bounds.size.height);
    view->arrange(maui::graphics::rect{0.0, 0.0, bounds.size.width, bounds.size.height});
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    [self layoutTemplatedContent];
}

// Self-size the cell to the realized content's measured size on the SCROLL axis — the C# TemplatedCell2
// PreferredLayoutAttributesFittingAttributes + GetMeasureConstraints. The compositional layout vends the
// estimated cell frame (full cross extent, estimated scroll extent); this measures the realized MAUI view
// against the fixed cross dimension with the scroll axis unbounded and reports its desired extent — so a
// template with HeightRequest=100 (VariedSize MilkTemplate) occupies 100pt, a HeightRequest=50 one 50pt,
// and an auto cell its natural content height, instead of every cell collapsing to the 44pt estimate. A
// default (label-only, no template) cell keeps the proposed frame — UILabel autoresizing fits the estimate.
- (UICollectionViewLayoutAttributes*)preferredLayoutAttributesFittingAttributes:
    (UICollectionViewLayoutAttributes*)layoutAttributes
{
    auto* const view = dynamic_cast<maui::core::i_view*>(_realizedContent.get());
    if (view == nullptr)
    {
        return [super preferredLayoutAttributesFittingAttributes:layoutAttributes];
    }
    CGRect frame = layoutAttributes.frame;
    // GetMeasureConstraints: a vertical list fixes the width (cross axis) and frees the height (scroll
    // axis); a horizontal list fixes the height and frees the width. Measure under those constraints, then
    // overwrite ONLY the freed (scroll) axis with the desired extent — the C# `IsPositiveInfinity(width)?
    // measured : preferred` split. ceil so a fractional measure never clips the content.
    const bool vertical = self.scrollDirection == UICollectionViewScrollDirectionVertical;
    const double width_constraint = vertical ? frame.size.width : std::numeric_limits<double>::infinity();
    const double height_constraint = vertical ? std::numeric_limits<double>::infinity() : frame.size.height;
    const maui::graphics::size measured = view->measure(width_constraint, height_constraint);
    if (vertical)
    {
        frame.size.height = static_cast<CGFloat>(std::ceil(measured.height));
    }
    else
    {
        frame.size.width = static_cast<CGFloat>(std::ceil(measured.width));
    }
    layoutAttributes.frame = frame;
    return layoutAttributes;
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

// Depth-first search for the first UILabel in a view subtree (the realized label's native field).
+ (UILabel*)firstLabelIn:(UIView*)root
{
    if ([root isKindOfClass:[UILabel class]])
    {
        return (UILabel*)root;
    }
    for (UIView* const sub in root.subviews)
    {
        if (UILabel* const found = [MauiCollectionViewCell firstLabelIn:sub])
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
        if (UILabel* const found = [MauiCollectionViewCell firstLabelIn:_templatedContent])
        {
            return found.text != nil ? found.text : @"";
        }
    }
    return _label.text != nil ? _label.text : @"";
}
@end

// ---- the unified supplementary view (group/section header & footer) ----
// <= *SupplementaryView2 / *DefaultSupplementalView2: the TemplatedCell2 twin for supplementaries —
// hosts a group/CV header/footer template's realized native content when a template is set (the C#
// TemplatedSupplementaryView Bind path), else mirrors the boxed item's text (the DefaultSupplemental
// label / C# DefaultCell2.Label.Text = obj?.ToString()).
@interface MauiCollectionReusableView : UICollectionReusableView
@property(nonatomic, strong) UILabel* label;
@property(nonatomic, strong) UIView* templatedContent;
- (void)showText:(NSString*)text;
- (void)showTemplatedContent:(UIView*)content;
// Host the realized template content's native view AND retain the C++ content (which owns its handler +
// native view) for as long as this supplementary displays it — the C# TemplatedCell2 (used as a
// supplementary) holding its PlatformHandler. Mirrors the item cell's retaining overload.
- (void)showTemplatedContent:(UIView*)content retainingRealized:(std::shared_ptr<maui::core::bindable_object>)realized;
// The text actually on screen: the realized template content's first UILabel when templated, otherwise
// the default-supplemental label. The test seam (native_supplementary_text) reads this so a
// template-bound group header reports its bound value, not the hidden default label.
- (NSString*)displayedText;
@end

@implementation MauiCollectionReusableView
{
    // The realized header/footer template content (owns its attached handler + native view). Held so the
    // hosted native view outlives this method call; released on reuse/replacement (the C# supplementary
    // cell dropping its PlatformHandler).
    std::shared_ptr<maui::core::bindable_object> _realizedContent;
}
- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        _label = [[UILabel alloc] initWithFrame:self.bounds];
        _label.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        // The default (non-templated) string Header/Footer renders in the iOS headline text style — the C#
        // VerticalDefaultSupplementalView2 sets Label.Font = PreferredHeadline (#138). A templated /
        // boxed-view supplementary hides this label, so this only styles the bare string case.
        _label.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
        [self addSubview:_label];
    }
    return self;
}

- (void)showText:(NSString*)text
{
    [_templatedContent removeFromSuperview];
    _templatedContent = nil;
    _realizedContent.reset();
    _label.hidden = NO;
    _label.text = text;
}

- (void)showTemplatedContent:(UIView*)content
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
        // Clear autoresizing: the hosted content is a MAUI view (a templated Label or a boxed Grid) whose
        // children are positioned by the cross-platform arrange, NOT by UIKit autoresizing. layoutSubviews
        // measures + arranges it against the supplementary's bounds so the inner tree (image/labels/buttons)
        // lands. preferredLayoutAttributesFittingAttributes then grows the boundary to the desired height.
        content.autoresizingMask = UIViewAutoresizingNone;
        content.frame = self.bounds;
        [self addSubview:content];
        [self layoutSupplementaryContent];
    }
}

- (void)showTemplatedContent:(UIView*)content retainingRealized:(std::shared_ptr<maui::core::bindable_object>)realized
{
    // Retain the realized content FIRST (it owns the handler that owns `content`) so replacing the
    // previous content never frees the incoming native view mid-swap.
    _realizedContent = std::move(realized);
    [self showTemplatedContent:content];
}

// Measure + arrange the hosted MAUI view across the supplementary's bounds so its children get framed —
// a header/footer Grid (HeaderFooterView) or a templated group-header Label is a cross-platform view whose
// child frames come from arrange, not UIKit autoresizing. Re-run from layoutSubviews so a recycled view or
// a bounds change (the self-sizing grow) re-applies the frame.
- (void)layoutSupplementaryContent
{
    if (_templatedContent == nil)
    {
        return;
    }
    auto* const view = dynamic_cast<maui::core::i_view*>(_realizedContent.get());
    if (view == nullptr)
    {
        // A non-view realized content (defensive): keep it stretched to the bounds.
        _templatedContent.frame = self.bounds;
        return;
    }
    const CGRect bounds = self.bounds;
    view->measure(bounds.size.width, bounds.size.height);
    view->arrange(maui::graphics::rect{0.0, 0.0, bounds.size.width, bounds.size.height});
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    [self layoutSupplementaryContent];
}

// Self-size the boundary supplementary to the hosted content's desired height (the C#
// PreferredLayoutAttributesFittingAttributes auto-measure). The compositional layout vends the estimated
// boundary frame; this measures the hosted MAUI view at that width with an unbounded main axis and reports
// its desired extent, so a 100pt header Grid (or a one-line group-header Label) occupies its true height
// instead of the 44pt estimate. No hosted view (the bare-string default label case) keeps the attributes
// as-is — UILabel autoresizing already fits the estimated frame.
- (UICollectionViewLayoutAttributes*)preferredLayoutAttributesFittingAttributes:
    (UICollectionViewLayoutAttributes*)layoutAttributes
{
    auto* const view = dynamic_cast<maui::core::i_view*>(_realizedContent.get());
    if (view == nullptr)
    {
        return [super preferredLayoutAttributesFittingAttributes:layoutAttributes];
    }
    CGRect frame = layoutAttributes.frame;
    const maui::graphics::size desired = view->measure(frame.size.width, std::numeric_limits<double>::infinity());
    frame.size.height = static_cast<CGFloat>(std::ceil(desired.height));
    layoutAttributes.frame = frame;
    return layoutAttributes;
}

- (void)prepareForReuse
{
    [super prepareForReuse];
    // The recycler hands this instance back; clear the templated content so a fresh bind re-hosts, and
    // drop the realized content (its handler + native view) — the C# supplementary dropping its handler.
    [_templatedContent removeFromSuperview];
    _templatedContent = nil;
    _realizedContent.reset();
    _label.hidden = NO;
}

// Depth-first search for the first UILabel in a view subtree (the realized label's native field).
+ (UILabel*)firstLabelIn:(UIView*)root
{
    if ([root isKindOfClass:[UILabel class]])
    {
        return (UILabel*)root;
    }
    for (UIView* const sub in root.subviews)
    {
        if (UILabel* const found = [MauiCollectionReusableView firstLabelIn:sub])
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
        if (UILabel* const found = [MauiCollectionReusableView firstLabelIn:_templatedContent])
        {
            return found.text != nil ? found.text : @"";
        }
    }
    return _label.text != nil ? _label.text : @"";
}
@end

// ---- the unified controller (ItemsViewController2 + the Structured/Selectable/Groupable/Reorderable
//      subclasses collapsed) ----
@interface MauiItemsCollectionViewController : UICollectionViewController <UICollectionViewDelegate>
// A raw back-pointer to the C++ handler that owns this controller (set right after construction; the
// handler outlives the controller — the platform struct, which holds the controller, is owned by the
// handler). Stored as a void* to keep this Obj-C interface free of C++ types.
@property(nonatomic, assign) void* cppHandler;
// Every distinct cell instance the recycler has ever vended (by pointer): the on-simulator suite
// checks this stays bounded under scroll while the visible-item paths sweep the whole source.
@property(nonatomic, strong) NSMutableSet<NSValue*>* seenCellPointers;
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

    // The C++ side of the controller: builds the layout, drives reloads, exposes inspection. A
    // free-function helper set keeps the Obj-C interface thin (the controller forwards data-source
    // queries here through the handler).
    namespace
    {
        // The orientation the layout scrolls in.
        bool handler_is_horizontal(const collection_view_handler& handler)
        {
            const auto* platform = handler.typed_platform_view();
            return platform != nullptr && platform->orientation == items_layout_orientation::horizontal;
        }

        // Build the compositional layout for the control's current items_layout + grouping/header state —
        // the LayoutFactory2 port (item → group → section + boundary supplementary items). Returns a
        // retained UICollectionViewCompositionalLayout (caller owns it).
        UICollectionViewLayout* build_compositional_layout(collection_view_handler& handler)
        {
            const auto* platform = handler.typed_platform_view();
            auto* view = handler.virtual_view();
            const bool horizontal =
                platform != nullptr && platform->orientation == items_layout_orientation::horizontal;
            // LayoutFactory2.CreateCarouselLayout vs CreateListLayout: the carousel sizes each item to the
            // FULL viewport on the scroll axis (one item per page, swipeable one-at-a-time) — itemWidth =
            // FractionalWidth(1)/itemHeight = FractionalHeight(1) with a full-viewport group — whereas a
            // plain (horizontal) CollectionView flows items at their intrinsic estimated extent. Detect the
            // carousel via the virtual view type (same predicate the scroll-end writeback uses) so the
            // section block can pick the carousel item/group sizes below. Do NOT change the regular CV path.
            const bool is_carousel = dynamic_cast<maui::controls::carousel_view*>(view) != nullptr;
            const int span = platform != nullptr ? std::max(1, platform->span) : 1;
            const double item_spacing = platform != nullptr ? platform->item_spacing : 0;
            // CarouselViewHandler2.MapPeekAreaInsets → UpdateLayout: the peek is applied as the section's
            // leading/trailing content insets so the adjacent items "peek" in and the first cell's frame
            // shifts inward (the visible effect of the C# inset). Captured by value for the section block.
            const maui::core::thickness peek =
                platform != nullptr ? platform->peek_area_insets : maui::core::thickness{};

            const UICollectionViewScrollDirection scroll_direction =
                horizontal ? UICollectionViewScrollDirectionHorizontal : UICollectionViewScrollDirectionVertical;

            // LayoutFactory2.CreateSupplementaryItems: the CV-level (whole-collection) Header/Footer are
            // GLOBAL boundary items on the compositional layout's configuration, while the per-GROUP
            // header/footer are PER-SECTION boundary items on each section. Both can coexist (a grouped
            // CollectionView can carry a CV header/footer AND per-group headers/footers), exactly as the
            // C# CreateListLayout sets layoutConfiguration.BoundarySupplementaryItems (global, from the
            // LayoutHeaderFooterInfo) and section.BoundarySupplementaryItems (per-section, from the
            // LayoutGroupingInfo). Earlier the grouped path dropped the CV header/footer and put nothing
            // on the config, so "This is a header"/"Hey, a footer." never appeared on a grouped CV.
            const bool grouped = platform != nullptr && platform->grouped;
            // Per-section group header/footer: only on the grouped path, only when their template is set.
            bool group_header = false;
            bool group_footer = false;
            if (auto* groupable = dynamic_cast<groupable_items_view*>(view); groupable != nullptr && grouped)
            {
                group_header = groupable->group_header_template() != nullptr;
                group_footer = groupable->group_footer_template() != nullptr;
            }
            // CV-level (global) header/footer: the StructuredItemsView Header/Footer (value or template),
            // independent of grouping (LayoutHeaderFooterInfo feeds the global config in both shapes).
            bool cv_header = false;
            bool cv_footer = false;
            if (auto* structured = dynamic_cast<structured_items_view*>(view); structured != nullptr)
            {
                cv_header = structured->header().has_value() || structured->header_template() != nullptr;
                cv_footer = structured->footer().has_value() || structured->footer_template() != nullptr;
            }

            UICollectionViewCompositionalLayoutConfiguration* const config =
                [[UICollectionViewCompositionalLayoutConfiguration alloc] init];
            config.scrollDirection = scroll_direction;

            // The supplementary boundary item size: full cross extent, estimated scroll extent (mirrors the
            // C# group width/height passed to CreateSupplementaryItems — the same dimensions the section's
            // group uses below). Recomputed here so the global config items (added before the section block
            // runs) are sized identically to the per-section ones.
            NSCollectionLayoutDimension* const boundary_cross =
                horizontal ? [NSCollectionLayoutDimension estimatedDimension:k_estimated_item_extent]
                           : [NSCollectionLayoutDimension fractionalWidthDimension:1.0];
            NSCollectionLayoutDimension* const boundary_main =
                horizontal ? [NSCollectionLayoutDimension fractionalHeightDimension:1.0]
                           : [NSCollectionLayoutDimension estimatedDimension:k_estimated_item_extent];
            NSCollectionLayoutSize* const boundary_size = [NSCollectionLayoutSize sizeWithWidthDimension:boundary_cross
                                                                                         heightDimension:boundary_main];

            // Global (CV-level) boundary supplementary items on the layout configuration (C#
            // layoutConfiguration.BoundarySupplementaryItems). UIKit hands these a length-1 index path,
            // which viewForSupplementaryElementOfKind uses to bind them to the CV Header/Footer.
            NSMutableArray<NSCollectionLayoutBoundarySupplementaryItem*>* const global_boundaries =
                [NSMutableArray array];
            if (cv_header)
            {
                [global_boundaries
                    addObject:[NSCollectionLayoutBoundarySupplementaryItem
                                  boundarySupplementaryItemWithLayoutSize:boundary_size
                                                              elementKind:UICollectionElementKindSectionHeader
                                                                alignment:horizontal ? NSRectAlignmentLeading
                                                                                     : NSRectAlignmentTop]];
            }
            if (cv_footer)
            {
                [global_boundaries
                    addObject:[NSCollectionLayoutBoundarySupplementaryItem
                                  boundarySupplementaryItemWithLayoutSize:boundary_size
                                                              elementKind:UICollectionElementKindSectionFooter
                                                                alignment:horizontal ? NSRectAlignmentTrailing
                                                                                     : NSRectAlignmentBottom]];
            }
            config.boundarySupplementaryItems = global_boundaries;

            UICollectionViewCompositionalLayout* const layout = [[UICollectionViewCompositionalLayout alloc]
                initWithSectionProvider:^NSCollectionLayoutSection*(NSInteger /*sectionIndex*/,
                                                                    id<NSCollectionLayoutEnvironment> environment) {
                  // Item: along the cross axis it gets 1/span of the group; along the scroll axis it is
                  // estimated (the C# CreateEstimated(30f)). A CAROUSEL instead fills the page on BOTH axes
                  // — LayoutFactory2.CreateCarouselLayout uses itemWidth = FractionalWidth(1) and
                  // itemHeight = FractionalHeight(1) so each item is one full viewport (minus peek), the
                  // one-item-per-page snap shape — independent of orientation.
                  NSCollectionLayoutDimension* const item_width =
                      is_carousel  ? [NSCollectionLayoutDimension fractionalWidthDimension:1.0]
                      : horizontal ? [NSCollectionLayoutDimension estimatedDimension:k_estimated_item_extent]
                                   : [NSCollectionLayoutDimension fractionalWidthDimension:1.0 / span];
                  NSCollectionLayoutDimension* const item_height =
                      is_carousel  ? [NSCollectionLayoutDimension fractionalHeightDimension:1.0]
                      : horizontal ? [NSCollectionLayoutDimension fractionalHeightDimension:1.0 / span]
                                   : [NSCollectionLayoutDimension estimatedDimension:k_estimated_item_extent];
                  NSCollectionLayoutSize* const item_size = [NSCollectionLayoutSize sizeWithWidthDimension:item_width
                                                                                           heightDimension:item_height];
                  NSCollectionLayoutItem* const item = [NSCollectionLayoutItem itemWithLayoutSize:item_size];

                  // Group: full cross extent, estimated scroll extent; span items per row (grid). A CAROUSEL
                  // group fills the viewport on the SCROLL axis too so the single item it carries is one full
                  // page. LayoutFactory2.CreateCarouselLayout sizes the scroll axis to
                  // ABSOLUTE(environment.Container.ContentSize - peekAreaInsets.{Horizontal,Vertical}Thickness)
                  // so the page narrows by the full peek (the adjacent items peek in by peek/2 on each side,
                  // realized here by the section's leading/trailing content insets below). The cross axis
                  // stays FractionalWidth/Height(1). Fall back to fractional-1 when no environment (defensive).
                  const CGSize container_size = environment != nil ? environment.container.contentSize : CGSizeZero;
                  NSCollectionLayoutDimension* const group_width =
                      is_carousel   ? (horizontal && container_size.width > 0
                                           ? [NSCollectionLayoutDimension
                                                 absoluteDimension:container_size.width -
                                                                   static_cast<CGFloat>(peek.left + peek.right)]
                                           : [NSCollectionLayoutDimension fractionalWidthDimension:1.0])
                      : !horizontal ? [NSCollectionLayoutDimension fractionalWidthDimension:1.0]
                                    : [NSCollectionLayoutDimension estimatedDimension:k_estimated_item_extent];
                  NSCollectionLayoutDimension* const group_height =
                      is_carousel  ? (!horizontal && container_size.height > 0
                                          ? [NSCollectionLayoutDimension
                                                absoluteDimension:container_size.height -
                                                                  static_cast<CGFloat>(peek.top + peek.bottom)]
                                          : [NSCollectionLayoutDimension fractionalHeightDimension:1.0])
                      : horizontal ? [NSCollectionLayoutDimension fractionalHeightDimension:1.0]
                                   : [NSCollectionLayoutDimension estimatedDimension:k_estimated_item_extent];
                  NSCollectionLayoutSize* const group_size =
                      [NSCollectionLayoutSize sizeWithWidthDimension:group_width heightDimension:group_height];
                  NSCollectionLayoutGroup* const group =
                      horizontal ? [NSCollectionLayoutGroup verticalGroupWithLayoutSize:group_size
                                                                       repeatingSubitem:item
                                                                                  count:span]
                                 : [NSCollectionLayoutGroup horizontalGroupWithLayoutSize:group_size
                                                                         repeatingSubitem:item
                                                                                    count:span];

                  NSCollectionLayoutSection* const section = [NSCollectionLayoutSection sectionWithGroup:group];
                  section.interGroupSpacing = item_spacing;
                  // The peek shifts the section's content edges inward (leading/trailing on the scroll
                  // axis), so the first item starts inset and adjacent items peek in (CarouselView).
                  if (horizontal)
                  {
                      section.contentInsets = NSDirectionalEdgeInsetsMake(0, static_cast<CGFloat>(peek.left), 0,
                                                                          static_cast<CGFloat>(peek.right));
                  }
                  else
                  {
                      section.contentInsets = NSDirectionalEdgeInsetsMake(static_cast<CGFloat>(peek.top), 0,
                                                                          static_cast<CGFloat>(peek.bottom), 0);
                  }

                  // Per-section (group) boundary supplementary items (C# section.BoundarySupplementaryItems
                  // from the LayoutGroupingInfo). UIKit hands these a length-2 index path {section,0}, which
                  // viewForSupplementaryElementOfKind uses to bind them to the group key's template.
                  NSMutableArray<NSCollectionLayoutBoundarySupplementaryItem*>* const boundaries =
                      [NSMutableArray array];
                  NSCollectionLayoutSize* const supplementary_size =
                      [NSCollectionLayoutSize sizeWithWidthDimension:group_width heightDimension:group_height];
                  if (group_header)
                  {
                      [boundaries
                          addObject:[NSCollectionLayoutBoundarySupplementaryItem
                                        boundarySupplementaryItemWithLayoutSize:supplementary_size
                                                                    elementKind:UICollectionElementKindSectionHeader
                                                                      alignment:horizontal ? NSRectAlignmentLeading
                                                                                           : NSRectAlignmentTop]];
                  }
                  if (group_footer)
                  {
                      [boundaries
                          addObject:[NSCollectionLayoutBoundarySupplementaryItem
                                        boundarySupplementaryItemWithLayoutSize:supplementary_size
                                                                    elementKind:UICollectionElementKindSectionFooter
                                                                      alignment:horizontal ? NSRectAlignmentTrailing
                                                                                           : NSRectAlignmentBottom]];
                  }
                  section.boundarySupplementaryItems = boundaries;
                  return section;
                }
                          configuration:config];

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

        // Realize a type-activated template's content into a native UIView (the C# TemplatedCell2.Bind:
        // `CreateContent(...) as View` → set BindingContext → `view.ToPlatform(mauiContext)`). Returns the
        // realized content (which OWNS its attached handler + native view — the caller keeps it alive for
        // the cell's lifetime) and, out-param, its native UIView. Yields {nullptr, nil} when the template
        // is loader-only (no static control type) or no handler is registered for that type — the cell
        // then falls back to the item-text mirror, exactly as before.
        std::shared_ptr<maui::core::bindable_object> realize_template_content(
            collection_view_handler& handler, const std::shared_ptr<data_template>& tmpl, const boxed_item& value,
            UIView** out_native)
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

            const std::shared_ptr<maui::core::i_element_handler> child_handler =
                context->handlers().create_handler(*tmpl->content_type());
            auto* element = dynamic_cast<maui::core::i_element*>(content.get());
            if (!child_handler || element == nullptr)
            {
                return nullptr; // no registered handler (or non-element content) — fall back to text
            }
            child_handler->set_maui_context(context);
            element->set_handler(child_handler); // creates the platform view + runs the mapper

            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(child_handler.get()))
            {
                *out_native = (__bridge UIView*)view_handler->native_view();
            }
            return content;
        }

        // Realize a boxed VIEW (a Header/Footer set to a live View/Grid via boxed_item::of(view), NOT a
        // string and NOT a DataTemplate) into a native UIView — the C# `Header is View` / `Footer is View`
        // arm that hosts the View directly outside the scroll extent, and the headless oracle's
        // realize_supplemental `value.as_bindable()` branch (reuse_id "view"). The boxed view is already a
        // fully-built element tree; in the gallery path the page's attach_handlers has ALREADY attached its
        // handler + built its native view (gallery_attach.hpp), so this reuses that native_view() (the C#
        // ToPlatform on a view whose PlatformHandler is already set). A boxed view carries no static
        // content_type, so when it has no attached handler yet there is nothing to look the handler up by:
        // yield {nullptr, nil} and let the caller fall back to the text mirror. Returns the bindable (held
        // by the supplementary so the hosted native view outlives this call) + its native UIView out-param.
        std::shared_ptr<maui::core::bindable_object> realize_boxed_view(
            const std::shared_ptr<maui::core::bindable_object>& bindable, UIView** out_native)
        {
            *out_native = nil;
            if (!bindable)
            {
                return nullptr;
            }
            auto* element = dynamic_cast<maui::core::i_element*>(bindable.get());
            if (element == nullptr)
            {
                return nullptr;
            }
            // Common (gallery) case: the view already has a handler with a built native view — reuse it
            // (the C# View whose PlatformHandler is already set; ToPlatform returns the existing native view).
            if (const std::shared_ptr<maui::core::i_element_handler>& existing = element->handler())
            {
                if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(existing.get()))
                {
                    if (auto* const native = (__bridge UIView*)view_handler->native_view(); native != nil)
                    {
                        *out_native = native;
                        return bindable;
                    }
                }
            }
            return nullptr; // no attached handler / native view — caller falls back to the text mirror
        }
    } // namespace

    namespace
    {
        // Bind a header/footer supplementary view, mirroring the C# *ItemsViewController2
        // Update{Templated,Default}SupplementaryView split: with a template set, realize the template's
        // content bound to `context` (the group key for a group header/footer, the CV Header/Footer object
        // for a global one) and host it (the TemplatedCell2.Bind path); otherwise mirror the context's text
        // (the DefaultCell2.Label.Text = obj?.ToString() path). Kept as a free helper so both the grouped
        // (per-section) and structured (global) branches share one realization path.
        void bind_supplementary_view(collection_view_handler& handler, MauiCollectionReusableView* view,
                                     const std::shared_ptr<data_template>& tmpl, const boxed_item& context)
        {
            if (tmpl != nullptr)
            {
                const std::shared_ptr<data_template> resolved = resolve_item_template(
                    tmpl, context, dynamic_cast<maui::core::bindable_object*>(handler.virtual_view()));
                UIView* templated = nil;
                std::shared_ptr<maui::core::bindable_object> realized =
                    realize_template_content(handler, resolved, context, &templated);
                if (realized != nullptr && templated != nil)
                {
                    [view showTemplatedContent:templated retainingRealized:std::move(realized)];
                    return;
                }
            }
            // Boxed VIEW (no template): the Header/Footer is a live View/Grid (HeaderFooterView.xaml's
            // `<CollectionView.Header><Grid>…`). Host its already-realized native view directly — the C#
            // `Header is View` arm / the headless oracle's `value.as_bindable()` branch. Tried before the
            // text fallback so a boxed view never degrades to its (empty) ToString.
            if (const std::shared_ptr<maui::core::bindable_object>& bindable = context.as_bindable())
            {
                UIView* boxed = nil;
                std::shared_ptr<maui::core::bindable_object> realized = realize_boxed_view(bindable, &boxed);
                if (realized != nullptr && boxed != nil)
                {
                    [view showTemplatedContent:boxed retainingRealized:std::move(realized)];
                    return;
                }
            }
            // No template, no boxed view (or an unrealized one): mirror the context's text.
            [view showText:to_nsstring(context.text())];
        }
    } // namespace

    // ---- the native bridge (called from the cross-platform .cpp under #ifdef MAUI_PLATFORM_IOS) ----

    void collection_view_handler::native_reload()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        controller.cppHandler = this; // wire the back-pointer lazily (create_platform_view is static)
        [controller.collectionView reloadData];
        // Re-apply the selection mirror AFTER reloadData: a PRESELECTED selection (set on the control before
        // its ItemsSource was mapped to the native view) selects index paths that don't exist yet, so the
        // initial select is a no-op and the cells render unselected. reloadData drops all selection, so any
        // reload must restore platform->selected_paths (mirrors MAUI re-selecting after ReloadData) — this is
        // what makes preselected_items / multiple-bound preselection show the selected-cell highlight.
        native_update_platform_selection();
    }

    void collection_view_handler::native_rebuild_layout()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        controller.cppHandler = this;
        UICollectionViewLayout* const layout = build_compositional_layout(*this);

        // Swap the retained layout slot (release the old, retain the new).
        if (platform->layout != nullptr)
        {
            CFRelease(platform->layout);
        }
        platform->layout = (__bridge_retained void*)layout;
        [controller.collectionView setCollectionViewLayout:layout animated:NO];
    }

    void collection_view_handler::native_update_selection_mode()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        controller.collectionView.allowsSelection = platform->allows_selection;
        controller.collectionView.allowsMultipleSelection = platform->allows_multiple_selection;
    }

    void collection_view_handler::native_update_platform_selection()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        UICollectionView* const collection_view = controller.collectionView;

        // Deselect everything the native view currently has, then re-select the handler's mirror
        // (platform->selected_paths is kept in sync by the cross-platform update_platform_selection).
        NSArray<NSIndexPath*>* const current = [collection_view indexPathsForSelectedItems];
        for (NSUInteger i = 0; i < current.count; ++i)
        {
            [collection_view deselectItemAtIndexPath:current[i] animated:NO];
        }
        for (const index_path& path : platform->selected_paths)
        {
            if (path.section < 0 || path.item < 0)
            {
                continue;
            }
            NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:path.item inSection:path.section];
            [collection_view selectItemAtIndexPath:index_path_ns
                                          animated:NO
                                    scrollPosition:UICollectionViewScrollPositionNone];
        }
    }

    void collection_view_handler::native_update_empty_view()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        UICollectionView* const collection_view = controller.collectionView;

        // Tear down any previously-shown empty view.
        if (platform->empty_view_native != nullptr)
        {
            UIView* const old = (__bridge_transfer UIView*)platform->empty_view_native;
            [old removeFromSuperview];
            platform->empty_view_native = nullptr;
        }

        const bool empty = !source_ || source_->item_count() == 0;
        if (!empty || virtual_view() == nullptr)
        {
            return;
        }

        // Realize the empty view as a tagged host with a text-mirror label (the C# EmptyTag host). The
        // boxed-view / EmptyViewTemplate realization lives in the cross-platform supplemental record;
        // on-device the text mirror is the asserted surface — documented simplification.
        auto* view = virtual_view();
        const boxed_item& empty_value = view->empty_view();

        UIView* const host = [[UIView alloc] initWithFrame:collection_view.bounds];
        host.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        host.tag = 333; // C# ItemsViewController2.EmptyTag

        UILabel* const label = [[UILabel alloc] initWithFrame:host.bounds];
        label.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        label.textAlignment = NSTextAlignmentCenter;
        label.text = to_nsstring(empty_value.text());
        [host addSubview:label];

        [collection_view addSubview:host];
        platform->empty_view_native = (__bridge_retained void*)host;
    }

    void collection_view_handler::native_update_can_reorder()
    {
        // C# ReorderableItemsViewController2.UpdateCanReorderItems (un)installs the long-press drag
        // gesture. The port reorders by mutating the bound (typed observable) collection — exactly what
        // C#'s MoveItem does to the underlying IList — and reports through send_reorder_completed; the
        // resulting source_update fans into native_reload so the cells re-render in the new order. The
        // gate lives in send_reorder_completed (CanReorderItems). The erased i_item_collection is read-
        // only, so there is no native interactive-movement mutator to install here — documented.
    }

    void collection_view_handler::native_scroll_to(const index_path& path, controls::scroll_to_position position,
                                                   bool animate)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr || path.section < 0 || path.item < 0)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        controller.cppHandler = this;
        UICollectionView* const collection_view = controller.collectionView;

        const bool horizontal = handler_is_horizontal(*this);
        UICollectionViewScrollPosition scroll_position = UICollectionViewScrollPositionNone;
        switch (position)
        {
            case controls::scroll_to_position::start:
                scroll_position = horizontal ? UICollectionViewScrollPositionLeft : UICollectionViewScrollPositionTop;
                break;
            case controls::scroll_to_position::center:
                scroll_position = horizontal ? UICollectionViewScrollPositionCenteredHorizontally
                                             : UICollectionViewScrollPositionCenteredVertically;
                break;
            case controls::scroll_to_position::end:
                scroll_position =
                    horizontal ? UICollectionViewScrollPositionRight : UICollectionViewScrollPositionBottom;
                break;
            case controls::scroll_to_position::make_visible:
                scroll_position = UICollectionViewScrollPositionNone;
                break;
        }

        NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:path.item inSection:path.section];
        if (path.section < [collection_view numberOfSections] && path.item < [collection_view
                                                                                 numberOfItemsInSection:path.section])
        {
            [collection_view scrollToItemAtIndexPath:index_path_ns atScrollPosition:scroll_position animated:animate];
        }
    }

    void collection_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // `native` is the controller's collectionView (create_platform_view). A UICollectionViewController
        // vends it at FULL SCREEN with UIViewAutoresizingFlexibleWidth|FlexibleHeight, so once it is added
        // as a stack-sibling subview it re-stretches to the panel on every UIKit layout pass and paints
        // over its siblings. Clear the autoresizing mask so MAUI's arrange owns the frame (the C# cross-
        // platform layout frames the platform view; UIKit does not re-impose its own), then set the frame
        // to the arranged (bounded) rect — exactly like every other handler's platform_arrange.
        UICollectionView* const collection_view = (__bridge UICollectionView*)platform->native;
        collection_view.autoresizingMask = UIViewAutoresizingNone;
        collection_view.frame = CGRectMake(frame.x, frame.y, frame.width, frame.height);
    }

    int collection_view_handler::native_force_layout(double width, double height)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return 0;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        controller.cppHandler = this;
        UICollectionView* const collection_view = controller.collectionView;
        collection_view.frame = CGRectMake(0, 0, width, height);
        [collection_view layoutIfNeeded];
        // C# CarouselViewController2.UpdateInitialPosition flips InitialPositionSet true once the view is
        // loaded and laid out; this first layout pass is the port's analog. After it, scroll-end callbacks
        // are allowed to write Position back (set_position_from_scroll's initial_position_set_ guard).
        mark_initial_position_set();
        return static_cast<int>(collection_view.visibleCells.count);
    }

    int collection_view_handler::native_visible_cell_count() const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return 0;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        return static_cast<int>(controller.collectionView.visibleCells.count);
    }

    int collection_view_handler::native_distinct_cell_instances() const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return 0;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        return static_cast<int>(controller.seenCellPointers.count);
    }

    int collection_view_handler::native_visible_supplementary_count(bool header) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return 0;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        NSString* const kind = header ? UICollectionElementKindSectionHeader : UICollectionElementKindSectionFooter;
        return static_cast<int>([controller.collectionView visibleSupplementaryViewsOfKind:kind].count);
    }

    std::string collection_view_handler::native_supplementary_text(int section, bool header) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return {};
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        UICollectionView* const collection_view = controller.collectionView;
        NSString* const kind = header ? UICollectionElementKindSectionHeader : UICollectionElementKindSectionFooter;

        MauiCollectionReusableView* found = nil;
        if (section < 0)
        {
            // CV-level (global) supplementary: it carries a length-1 index path. Scan the visible
            // supplementaries of this kind for the first such one.
            for (UICollectionReusableView* const supplementary in
                 [collection_view visibleSupplementaryViewsOfKind:kind])
            {
                NSIndexPath* const path = [collection_view indexPathForSupplementaryView:supplementary];
                if ((path == nil || path.length < 2) &&
                    [supplementary isKindOfClass:[MauiCollectionReusableView class]])
                {
                    found = (MauiCollectionReusableView*)supplementary;
                    break;
                }
            }
        }
        else
        {
            // Per-group supplementary at {section, 0}.
            NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:0 inSection:section];
            UICollectionReusableView* const supplementary =
                [collection_view supplementaryViewForElementKind:kind atIndexPath:index_path_ns];
            if ([supplementary isKindOfClass:[MauiCollectionReusableView class]])
            {
                found = (MauiCollectionReusableView*)supplementary;
            }
        }
        if (found == nil)
        {
            return {};
        }
        const char* const utf8 = [found displayedText].UTF8String;
        return utf8 != nullptr ? std::string(utf8) : std::string();
    }

    int collection_view_handler::native_selected_count() const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return 0;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        return static_cast<int>([controller.collectionView indexPathsForSelectedItems].count);
    }

    void collection_view_handler::native_select(const index_path& path)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr || path.section < 0 || path.item < 0)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        controller.cppHandler = this;
        UICollectionView* const collection_view = controller.collectionView;
        if (path.section >= [collection_view numberOfSections] ||
            path.item >= [collection_view numberOfItemsInSection:path.section])
        {
            return;
        }
        NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:path.item inSection:path.section];
        // UICollectionView's selectItemAtIndexPath does NOT fire the delegate's didSelect; drive both
        // the native selection AND the delegate fan-out (the C# user-tap path) for fidelity.
        [collection_view selectItemAtIndexPath:index_path_ns
                                      animated:NO
                                scrollPosition:UICollectionViewScrollPositionNone];
        [controller collectionView:collection_view didSelectItemAtIndexPath:index_path_ns];
    }

    void collection_view_handler::native_deselect(const index_path& path)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr || path.section < 0 || path.item < 0)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        controller.cppHandler = this;
        UICollectionView* const collection_view = controller.collectionView;
        if (path.section >= [collection_view numberOfSections] ||
            path.item >= [collection_view numberOfItemsInSection:path.section])
        {
            return;
        }
        NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:path.item inSection:path.section];
        [collection_view deselectItemAtIndexPath:index_path_ns animated:NO];
        [controller collectionView:collection_view didDeselectItemAtIndexPath:index_path_ns];
    }

    std::string collection_view_handler::native_cell_text(const index_path& path) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr || path.section < 0 || path.item < 0)
        {
            return {};
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        UICollectionView* const collection_view = controller.collectionView;
        NSIndexPath* const index_path_ns = [NSIndexPath indexPathForItem:path.item inSection:path.section];
        UICollectionViewCell* const cell = [collection_view cellForItemAtIndexPath:index_path_ns];
        if (auto* const maui_cell = (MauiCollectionViewCell*)cell;
            [maui_cell isKindOfClass:[MauiCollectionViewCell class]])
        {
            // -[NSString UTF8String] is nullable-annotated; guard before constructing std::string.
            const char* const utf8 = [maui_cell displayedText].UTF8String;
            return utf8 != nullptr ? std::string(utf8) : std::string();
        }
        return {};
    }

    // ---- the carousel knobs on the native UICollectionView (CarouselViewHandler2.Map*) ----

    void collection_view_handler::native_update_swipe_enabled()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        // CarouselViewHandler2.MapIsSwipeEnabled → CollectionView.ScrollEnabled (the MauiCollectionView
        // SetSwipeEnabled wrapper is not ported; the port writes ScrollEnabled directly — the C# else).
        controller.collectionView.scrollEnabled = platform->swipe_enabled ? YES : NO;
    }

    void collection_view_handler::native_update_bounce_enabled()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->controller == nullptr)
        {
            return;
        }
        auto* const controller = (__bridge MauiItemsCollectionViewController*)platform->controller;
        // CarouselViewHandler2.MapIsBounceEnabled → CollectionView.Bounces.
        controller.collectionView.bounces = platform->bounce_enabled ? YES : NO;
    }

    void collection_view_handler::native_update_peek_area_insets()
    {
        // CarouselViewHandler2.MapPeekAreaInsets → handler.UpdateLayout(): rebuild the compositional
        // layout so the new section content insets (applied in build_compositional_layout from the peek
        // mirror) take effect — the first cell's frame shifts inward and the adjacent items peek in.
        native_rebuild_layout();
    }

    // ---- creation + teardown ----

    collection_view_platform::~collection_view_platform()
    {
        // Release the retained native slots in reverse order of acquisition. The controller owns the
        // collectionView; releasing the controller tears the whole UICollectionView tree down.
        if (empty_view_native != nullptr)
        {
            CFRelease(empty_view_native);
            empty_view_native = nullptr;
        }
        if (layout != nullptr)
        {
            CFRelease(layout);
            layout = nullptr;
        }
        if (controller != nullptr)
        {
            CFRelease(controller);
            controller = nullptr;
        }
        // `native` aliases the controller's collectionView (NOT separately retained) — nothing to free.
        native = nullptr;
    }

    std::unique_ptr<collection_view_platform> collection_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<collection_view_platform>();

        // The initial layout is a plain vertical list (the mapper rebuilds it from the real items_layout
        // on first run — C# OnCreatePlatformView builds an initial layout, then the mappers refine it).
        UICollectionViewCompositionalLayoutConfiguration* const config =
            [[UICollectionViewCompositionalLayoutConfiguration alloc] init];
        config.scrollDirection = UICollectionViewScrollDirectionVertical;
        UICollectionViewCompositionalLayout* const layout = [[UICollectionViewCompositionalLayout alloc]
            initWithSectionProvider:^NSCollectionLayoutSection*(NSInteger /*section*/,
                                                                id<NSCollectionLayoutEnvironment> /*environment*/) {
              NSCollectionLayoutSize* const item_size = [NSCollectionLayoutSize
                  sizeWithWidthDimension:[NSCollectionLayoutDimension fractionalWidthDimension:1.0]
                         heightDimension:[NSCollectionLayoutDimension estimatedDimension:k_estimated_item_extent]];
              NSCollectionLayoutItem* const item = [NSCollectionLayoutItem itemWithLayoutSize:item_size];
              NSCollectionLayoutSize* const group_size = [NSCollectionLayoutSize
                  sizeWithWidthDimension:[NSCollectionLayoutDimension fractionalWidthDimension:1.0]
                         heightDimension:[NSCollectionLayoutDimension estimatedDimension:k_estimated_item_extent]];
              NSCollectionLayoutGroup* const group = [NSCollectionLayoutGroup horizontalGroupWithLayoutSize:group_size
                                                                                           repeatingSubitem:item
                                                                                                      count:1];
              return [NSCollectionLayoutSection sectionWithGroup:group];
            }
                      configuration:config];

        // create_platform_view is static (the view_handler CRTP calls it as derived().create_platform_view()),
        // so the handler back-pointer can't be set here — every non-const bridge member wires it
        // (`controller.cppHandler = this`) right after fetching the controller. The mapper runs a bridge
        // call (map_items_source → native_reload) before any data-source query, so cppHandler is always
        // set before the controller reads data.
        auto* const controller = [[MauiItemsCollectionViewController alloc] initWithCollectionViewLayout:layout];
        controller.collectionView.backgroundColor = UIColor.clearColor;

        // Register the cell + supplementary classes (the C# RegisterViewTypes; the unified classes mean
        // a single registration covers default + templated, header + footer).
        [controller.collectionView registerClass:[MauiCollectionViewCell class]
                      forCellWithReuseIdentifier:k_default_cell_reuse_id];
        [controller.collectionView registerClass:[MauiCollectionReusableView class]
                      forSupplementaryViewOfKind:UICollectionElementKindSectionHeader
                             withReuseIdentifier:k_header_reuse_id];
        [controller.collectionView registerClass:[MauiCollectionReusableView class]
                      forSupplementaryViewOfKind:UICollectionElementKindSectionFooter
                             withReuseIdentifier:k_footer_reuse_id];

        platform->controller = (__bridge_retained void*)controller;
        platform->layout = (__bridge_retained void*)layout;
        // `native` is the real UICollectionView the handler composes into the view tree (the C#
        // CreatePlatformView returns Controller.View). It is NOT separately retained — the controller
        // owns it; the destructor frees only the controller.
        platform->native = (__bridge void*)controller.collectionView;
        return platform;
    }
} // namespace maui::controls

// ---- the controller's data source + delegate (reads through the C++ handler) ----
@implementation MauiItemsCollectionViewController

- (instancetype)initWithCollectionViewLayout:(UICollectionViewLayout*)layout
{
    self = [super initWithCollectionViewLayout:layout];
    if (self != nil)
    {
        _seenCellPointers = [NSMutableSet set];
    }
    return self;
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

- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView*)collectionView
{
    auto source = [self source];
    return source != nullptr ? source->group_count() : 0;
}

- (NSInteger)collectionView:(UICollectionView*)collectionView numberOfItemsInSection:(NSInteger)section
{
    auto source = [self source];
    if (source == nullptr || section < 0 || section >= source->group_count())
    {
        return 0;
    }
    return source->item_count_in_group(static_cast<int>(section));
}

- (UICollectionViewCell*)collectionView:(UICollectionView*)collectionView cellForItemAtIndexPath:(NSIndexPath*)indexPath
{
    MauiCollectionViewCell* cell = [collectionView dequeueReusableCellWithReuseIdentifier:k_default_cell_reuse_id
                                                                             forIndexPath:indexPath];
    [_seenCellPointers addObject:[NSValue valueWithNonretainedObject:cell]];

    auto* handler = [self handler];
    auto source = [self source];
    if (handler == nullptr || source == nullptr)
    {
        return cell;
    }

    // Wire the scroll direction so the cell self-sizes on the scroll axis (TemplatedCell2.ScrollDirection).
    const auto* platform = handler->typed_platform_view();
    const bool horizontal =
        platform != nullptr && platform->orientation == maui::controls::items_layout_orientation::horizontal;
    cell.scrollDirection =
        horizontal ? UICollectionViewScrollDirectionHorizontal : UICollectionViewScrollDirectionVertical;

    const maui::controls::index_path path{.section = static_cast<int>(indexPath.section),
                                          .item = static_cast<int>(indexPath.item)};
    const maui::controls::boxed_item value = source->item(path);

    // TemplatedCell2 vs DefaultCell2 (the C# split): with an ItemTemplate set, realize the template's
    // content as a real native view bound to the item (so a struct item renders its template-bound
    // fields, not just item.text() — which is empty for non-string items); with no template, the default
    // cell's UILabel mirrors item.text(). The realized content is retained on the cell for as long as it
    // hosts it (the C# cell holding its PlatformHandler), released on reuse.
    auto* itemsView = handler->virtual_view();
    const std::shared_ptr<maui::controls::data_template> tmpl =
        itemsView != nullptr
            ? maui::controls::resolve_item_template(itemsView->item_template(), value,
                                                    dynamic_cast<maui::core::bindable_object*>(itemsView))
            : nullptr;
    UIView* templated = nil;
    std::shared_ptr<maui::core::bindable_object> realized =
        maui::controls::realize_template_content(*handler, tmpl, value, &templated);
    if (realized != nullptr && templated != nil)
    {
        [cell showTemplatedContent:templated retainingRealized:std::move(realized)];
    }
    else
    {
        [cell showText:maui::controls::to_nsstring(value.text())];
    }
    return cell;
}

- (UICollectionReusableView*)collectionView:(UICollectionView*)collectionView
          viewForSupplementaryElementOfKind:(NSString*)kind
                                atIndexPath:(NSIndexPath*)indexPath
{
    const bool isHeader = [kind isEqualToString:UICollectionElementKindSectionHeader];
    MauiCollectionReusableView* view = [collectionView
        dequeueReusableSupplementaryViewOfKind:kind
                           withReuseIdentifier:(isHeader ? k_header_reuse_id : k_footer_reuse_id)forIndexPath
                                              :indexPath];

    auto* handler = [self handler];
    auto source = [self source];
    if (handler == nullptr || source == nullptr)
    {
        return view;
    }
    auto* itemsView = handler->virtual_view();

    // C# GroupableItemsViewController2.GetViewForSupplementaryElement: a GLOBAL (whole-collection)
    // Header/Footer supplementary gets a length-1 index path, a PER-GROUP one gets {section, 0}. When the
    // path is per-group AND a group header/footer template is set, bind that template to the group key;
    // otherwise it's the CV-level Header/Footer (the structured branch, the C# `base` fall-through).
    auto* groupable = dynamic_cast<maui::controls::groupable_items_view*>(itemsView);
    const bool is_grouped = groupable != nullptr && groupable->is_grouped();
    const std::shared_ptr<maui::controls::data_template> group_tmpl =
        groupable != nullptr ? (isHeader ? groupable->group_header_template() : groupable->group_footer_template())
                             : nullptr;
    const bool is_group_supplementary = is_grouped && indexPath.length >= 2 && group_tmpl != nullptr;

    if (is_group_supplementary)
    {
        // Per-group header/footer: bind the group template against the group KEY object (the C#
        // UpdateTemplatedSupplementaryView: cell.Bind(template, ItemsSource.Group(indexPath), ItemsView)).
        const maui::controls::index_path path{.section = static_cast<int>(indexPath.section), .item = -1};
        const maui::controls::boxed_item group = source->group(path);
        maui::controls::bind_supplementary_view(*handler, view, group_tmpl, group);
    }
    else if (auto* structured = dynamic_cast<maui::controls::structured_items_view*>(itemsView); structured != nullptr)
    {
        // CV-level (global) header/footer: bind the structured Header/Footer object against its template
        // (the C# StructuredItemsViewController2 Update*SupplementaryView path). A grouped CollectionView
        // reaches here too, for its whole-collection Header/Footer (the C# `base` fall-through).
        const maui::controls::boxed_item& value = isHeader ? structured->header() : structured->footer();
        const std::shared_ptr<maui::controls::data_template>& tmpl =
            isHeader ? structured->header_template() : structured->footer_template();
        maui::controls::bind_supplementary_view(*handler, view, tmpl, value);
    }
    return view;
}

// ---- selection (the user-tap path; programmatic selection goes through native_update_platform_selection) ----

- (void)collectionView:(UICollectionView*)collectionView didSelectItemAtIndexPath:(NSIndexPath*)indexPath
{
    auto* handler = [self handler];
    if (handler == nullptr)
    {
        return;
    }
    handler->simulate_select(maui::controls::index_path{.section = static_cast<int>(indexPath.section),
                                                        .item = static_cast<int>(indexPath.item)});
}

- (void)collectionView:(UICollectionView*)collectionView didDeselectItemAtIndexPath:(NSIndexPath*)indexPath
{
    auto* handler = [self handler];
    if (handler == nullptr)
    {
        return;
    }
    handler->simulate_deselect(maui::controls::index_path{.section = static_cast<int>(indexPath.section),
                                                          .item = static_cast<int>(indexPath.item)});
}

// ---- carousel scroll-end writeback (the UIScrollViewDelegate seam) ----
//
// CarouselViewController2 writes Position/CurrentItem back to the carousel after a scroll settles. The
// port wires that here on the controller (which IS the UICollectionView's delegate): when a swipe ends
// (deceleration finishes, or a drag ends with no deceleration), resolve the CENTERED item index from the
// content offset (the carousel's snap alignment is Center) and write it back through the handler. The
// suppress gate (set during a batch source update) drops spurious mid-update callbacks. The native
// compositional path carries no phantom loop wrap, so the centered index maps straight to the item
// ordinal (no LoopManager correction needed — documented W3-29 carry-over).

// The item ordinal whose layout frame is centered in the current visible viewport, or -1 if none. The
// carousel's snap alignment is Center, so the settled position is the visible item closest to the
// viewport center along the scroll axis (the orientation comes from the handler's layout mirror).
- (NSInteger)centeredItemIndex
{
    auto* handler = [self handler];
    if (handler == nullptr || handler->typed_platform_view() == nullptr)
    {
        return -1;
    }
    UICollectionView* const collectionView = self.collectionView;
    const BOOL isHorizontal =
        handler->typed_platform_view()->orientation == maui::controls::items_layout_orientation::horizontal;
    // The visible center along the scroll axis (contentInset shifts the offset frame, which UIKit folds
    // into contentOffset, so the raw offset center is already inset-aware).
    const CGFloat reference = isHorizontal ? collectionView.contentOffset.x + (collectionView.bounds.size.width / 2)
                                           : collectionView.contentOffset.y + (collectionView.bounds.size.height / 2);

    NSInteger best = -1;
    CGFloat bestDistance = CGFLOAT_MAX;
    for (NSIndexPath* const path in collectionView.indexPathsForVisibleItems)
    {
        if (path.section != 0)
        {
            continue; // the carousel is single-section
        }
        UICollectionViewLayoutAttributes* const attrs =
            [collectionView.collectionViewLayout layoutAttributesForItemAtIndexPath:path];
        if (attrs == nil)
        {
            continue;
        }
        const CGFloat cellCenter = isHorizontal ? CGRectGetMidX(attrs.frame) : CGRectGetMidY(attrs.frame);
        const CGFloat distance = std::abs(cellCenter - reference);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = path.item;
        }
    }
    return best;
}

// Resolve the centered index and write Position + CurrentItem back (gated on the view being a carousel).
- (void)writeBackCenteredPosition
{
    auto* handler = [self handler];
    if (handler == nullptr || dynamic_cast<maui::controls::carousel_view*>(handler->virtual_view()) == nullptr)
    {
        return; // only the carousel writes scroll position back; a plain collection does not
    }
    const NSInteger centered = [self centeredItemIndex];
    if (centered < 0)
    {
        return;
    }
    handler->set_position_from_scroll(static_cast<int>(centered));
}

- (void)scrollViewDidEndDecelerating:(UIScrollView*)scrollView
{
    (void)scrollView;
    [self writeBackCenteredPosition];
}

- (void)scrollViewDidEndDragging:(UIScrollView*)scrollView willDecelerate:(BOOL)decelerate
{
    (void)scrollView;
    // If the swipe will keep decelerating, defer to scrollViewDidEndDecelerating (the final resting
    // position); only a drag that stops without deceleration settles here.
    if (!decelerate)
    {
        [self writeBackCenteredPosition];
    }
}

@end
