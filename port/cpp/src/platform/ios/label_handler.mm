// label_handler — iOS (UIKit) platform recipe: a real UILabel (a MauiIosLabel subclass for the custom
// vertical alignment). Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from LabelHandler.iOS.cs + Platform/iOS/LabelExtensions.cs + MauiLabel.cs (the same
// oracles the AppKit twin in src/platform/apple/label_handler.mm was adapted from — UIKit needs no
// adaptation): CreatePlatformView = new MauiLabel(); MapText = UpdateTextPlainText + MapFormatting
// (LineHeight → TextDecorations → CharacterSpacing → HorizontalTextAlignment);
// UpdateTextColor / UpdateFont(LabelFontSize) / UpdateHorizontalTextAlignment (TextAlignment) /
// UpdateVerticalTextAlignment (MauiLabel.VerticalAlignment — the DrawText offset) / UpdateTextDecorations
// + UpdateCharacterSpacing (the attributed WithDecorations / WithCharacterSpacing passes) /
// UpdateLineHeight + UpdatePadding (MauiLabel.TextInsets — DrawText insets, sizeThatFits folds them).
// MapLineBreakMode + MapMaxLines (Label.iOS.cs) both call UILabel.SetLineBreakMode
// (TextExtensions.SetLineBreakMode): the LineBreakMode→NSLineBreakMode wrap mode plus the numberOfLines
// resolution (MaxLines<0 ? (TailTruncation ? 1 : 0) : MaxLines; NoWrap/Head/MiddleTruncation force 1).
// Not ported here (deferred with the cross-platform contract/mapper): MapBackground/NeedsContainer (no
// container infrastructure), and GetDesiredSize's single-line UILabel default measure via sizeThatFits.

#import <UIKit/UIKit.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_text_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// MauiIosLabel  <=  Microsoft.Maui.Platform.MauiLabel — a UILabel honoring a vertical content alignment
// by offsetting the draw rect (UILabel itself always centers vertically): Top pins the text height to
// the top, Bottom to the bottom, Center/Fill leave UIKit's own centering. It also insets the text by the
// label Padding (MauiLabel.TextInsets), flipping left/right under RTL, and reflects the insets in
// sizeThatFits (subtract before measuring, add back) exactly as MauiLabel does.
@interface MauiIosLabel : UILabel
@property(nonatomic) UIControlContentVerticalAlignment verticalAlignment;
@property(nonatomic) UIEdgeInsets textInsets;
@end

@implementation MauiIosLabel
- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        _verticalAlignment = UIControlContentVerticalAlignmentCenter; // MauiLabel's field default
        _textInsets = UIEdgeInsetsZero;                               // MauiLabel.TextInsets default
    }
    return self;
}

- (void)setVerticalAlignment:(UIControlContentVerticalAlignment)value
{
    _verticalAlignment = value;
    [self setNeedsDisplay]; // MauiLabel.VerticalAlignment setter
}

- (void)setTextInsets:(UIEdgeInsets)value
{
    _textInsets = value;
    [self invalidateIntrinsicContentSize];
    [self setNeedsDisplay];
}

// MauiLabel.AlignVertical: the required text height is the single-line font height (Lines == 1) or the
// measured wrap height; when it fits, Top/Bottom move the draw rect (Center/Fill never reach here).
- (CGRect)maui_alignVertical:(CGRect)rect
{
    const CGFloat required = self.numberOfLines == 1 ? self.font.lineHeight : [super sizeThatFits:rect.size].height;
    if (required < rect.size.height)
    {
        if (self.verticalAlignment == UIControlContentVerticalAlignmentTop)
        {
            rect.size.height = required;
        }
        else if (self.verticalAlignment == UIControlContentVerticalAlignmentBottom)
        {
            rect = CGRectMake(rect.origin.x, CGRectGetMaxY(rect) - required, rect.size.width, required);
        }
    }
    return rect;
}

// MauiLabel.DrawText: inset by the (RTL-flipped) TextInsets, then the vertical-alignment offset.
- (void)drawTextInRect:(CGRect)rect
{
    UIEdgeInsets insets = self.textInsets;
    // Respect RTL (flip left/right insets) — MauiLabel.DrawText's EffectiveUserInterfaceLayoutDirection.
    if (self.effectiveUserInterfaceLayoutDirection == UIUserInterfaceLayoutDirectionRightToLeft)
    {
        insets = UIEdgeInsetsMake(insets.top, insets.right, insets.bottom, insets.left);
    }
    rect = UIEdgeInsetsInsetRect(rect, insets);

    if (self.verticalAlignment != UIControlContentVerticalAlignmentCenter &&
        self.verticalAlignment != UIControlContentVerticalAlignmentFill)
    {
        rect = [self maui_alignVertical:rect];
    }
    [super drawTextInRect:rect];
}

// MauiLabel.SizeThatFits: reduce the padding before measuring, then add it back, clamped to the container.
- (CGSize)sizeThatFits:(CGSize)size
{
    const UIEdgeInsets insets = self.textInsets;
    const CGSize adjusted =
        CGSizeMake(size.width - insets.left - insets.right, size.height - insets.top - insets.bottom);
    const CGSize requested = [super sizeThatFits:adjusted];
    const CGFloat width = MIN(requested.width, size.width) + insets.left + insets.right;
    const CGFloat height = MIN(requested.height, size.height) + insets.top + insets.bottom;
    return CGSizeMake(width, height);
}

// Keep a gradient/image background sublayer (installed by apply_background) sized to the label's CURRENT
// bounds — apply_background runs at map time, before layout, when layer.bounds is still zero. The same
// resize_background_layers pattern the value-control MauiIos* subclasses use; a solid background paints
// layer.backgroundColor directly and needs no resize.
- (void)layoutSubviews
{
    [super layoutSubviews];
    maui::platform::ios::resize_background_layers((__bridge void*)self);
    // Re-frame the clip mask to the new bounds (WrapperView.LayoutSubviews re-runs SetClip): a
    // UIKit-driven resize / rotation that bypasses the handler would otherwise leave the mask sized
    // to the old (or 0×0 map-time) bounds. No-op when no clip is set.
    maui::platform::ios::reapply_clip((__bridge void*)self);
}
@end

namespace
{
    UILabel* as_label(void* native)
    {
        return (__bridge UILabel*)native;
    }

    using maui::platform::ios::to_ns_line_break_mode;
    using maui::platform::ios::to_ns_text_alignment;
    using maui::platform::ios::to_ui_color;
    using maui::platform::ios::to_ui_control_content_vertical_alignment;
    using maui::platform::ios::to_ui_font;
    using maui::platform::ios::with_character_spacing;
    using maui::platform::ios::with_decorations;
    using maui::platform::ios::with_line_height;

    // TextExtensions.SetLineBreakMode — the shared body MapLineBreakMode + MapMaxLines both invoke. Sets
    // UILabel.lineBreakMode from the LineBreakMode and UILabel.numberOfLines from the MaxLines×mode pair:
    //   maxLines = MaxLines<0 ? (TailTruncation ? 1 : 0) : MaxLines     (the pre-switch line count)
    //   NoWrap / HeadTruncation / MiddleTruncation then clamp maxLines = 1 (a truncating single line)
    // (numberOfLines == 0 means "as many lines as needed"). Mirrors the C# switch verbatim.
    void refresh_line_break_mode(UILabel* label, maui::core::line_break_mode mode, int max_lines)
    {
        using maui::core::line_break_mode;
        NSInteger lines = max_lines;
        if (max_lines < 0)
        {
            lines = mode == line_break_mode::tail_truncation ? 1 : 0;
        }
        label.lineBreakMode = to_ns_line_break_mode(mode);
        if (mode == line_break_mode::no_wrap || mode == line_break_mode::head_truncation ||
            mode == line_break_mode::middle_truncation)
        {
            lines = 1;
        }
        label.numberOfLines = lines;
    }

    // LabelHandler.MapFormatting: "Update all of the attributed text formatting properties" — LineHeight
    // (LabelExtensions.UpdateLineHeight), then TextDecorations (UpdateTextDecorations), then
    // CharacterSpacing (UpdateCharacterSpacing), then re-assert the horizontal alignment ("Setting any of
    // those may have removed text alignment settings"), matching the C# MapFormatting order exactly.
    void refresh_label_formatting(UILabel* label, const maui::core::i_label& view)
    {
        NSAttributedString* const with_lh = with_line_height(label.attributedText, view.line_height());
        if (with_lh != nil)
        {
            label.attributedText = with_lh;
        }
        NSAttributedString* const decorated = with_decorations(label.attributedText, view.text_decorations());
        if (decorated != nil)
        {
            label.attributedText = decorated;
        }
        NSAttributedString* const kerned = with_character_spacing(label.attributedText, view.character_spacing());
        if (kerned != nil)
        {
            label.attributedText = kerned;
        }
        label.textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
    }
} // namespace

namespace maui::core
{
    label_platform::~label_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void label_platform::update_visibility(maui::core::visibility value)
    {
        as_label(native).hidden = value != maui::core::visibility::visible;
    }

    void label_platform::update_opacity(double value)
    {
        as_label(native).alpha = value;
    }

    void label_platform::update_is_enabled(bool value)
    {
        // ViewExtensions.UpdateIsEnabled's non-UIControl branch: a UILabel only gets the interaction
        // toggle (UIControl's visual-feedback `enabled` is the button path).
        as_label(native).userInteractionEnabled = static_cast<BOOL>(value);
    }

    void label_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_label(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    // ViewHandler.MapClip → WrapperView.SetClip: mask the native view's layer to the clip
    // geometry, sized to the view's CURRENT bounds (0×0 before the first layout — the layout hook
    // re-frames it). apply_and_store_clip both applies and stashes the borrow for that re-frame.
    void label_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // MapBackground: paint the solid/gradient/image onto the UILabel's backing layer (PaintExtensions).
    // A solid sets layer.backgroundColor directly; a gradient/image installs a sublayer kept sized by the
    // MauiIosLabel layoutSubviews above. Null clears the override.
    void label_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::ios::apply_background(native, value);
    }

    std::unique_ptr<label_platform> label_handler::create_platform_view()
    {
        auto platform = std::make_unique<label_platform>();
        MauiIosLabel* const label = [[MauiIosLabel alloc] initWithFrame:CGRectZero]; // new MauiLabel()
        // Wrap by default (MAUI Label's default LineBreakMode is WordWrap with no MaxLines cap), so the
        // PreferredMaxLayoutWidth branch can measure multiple lines for an explicit Width.
        label.numberOfLines = 0;
        platform->native = (__bridge_retained void*)label; // the void* slot owns one reference
        return platform;
    }

    void label_handler::map_text(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string text(view.text());
        // stringWithUTF8String: is _Nullable (nil on invalid UTF-8); a nil label text simply clears it.
        NSString* const value = [NSString stringWithUTF8String:text.c_str()];
        UILabel* const label = as_label(platform->native);
        label.text = value; // LabelExtensions.UpdateTextPlainText
        // "Any text update requires that we update any attributed string formatting" (MapText →
        // MapFormatting).
        refresh_label_formatting(label, view);
    }

    void label_handler::map_text_color(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // LabelExtensions.UpdateTextColor — ToPlatform(LabelColor). C# ITextElement.TextColor is a Color?
        // defaulting to null → the dynamic system label color (UIColor.labelColor), which adapts to
        // light/dark; an explicit color wins. The port models TextColor as a NON-nullable value type whose
        // default-constructed value (color{}) is opaque BLACK, so a value compare (`!= color{}`) cannot
        // distinguish an explicit TextColor=Black from "unset" — it misreads the explicit black as unset and
        // falls to labelColor (which is WHITE in dark mode: the chat-bubble cell bug). Discriminate on
        // whether the property was explicitly SET (BindableObject.IsSet) instead — the faithful stand-in for
        // C#'s `TextColor != null`. The realized templated cell label stages TextColor at manual specificity,
        // so is_property_set() reports it as set and the bubble stays black in both appearances.
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
        as_label(platform->native).textColor = color_is_set ? to_ui_color(view.text_color()) : UIColor.labelColor;
    }

    void label_handler::map_font(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // LabelExtensions.UpdateFont with UIFont.LabelFontSize as the control default.
            as_label(platform->native).font = to_ui_font(view.font(), static_cast<double>(UIFont.labelFontSize));
        }
    }

    void label_handler::map_horizontal_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // LabelExtensions.UpdateHorizontalTextAlignment (the RTL flip rides with FlowDirection).
            as_label(platform->native).textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void label_handler::map_vertical_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UILabel* const label = as_label(platform->native);
        if ([label isKindOfClass:[MauiIosLabel class]])
        {
            // LabelExtensions.UpdateVerticalTextAlignment(MauiLabel) — ToPlatformVertical onto the
            // custom label's draw-rect alignment.
            ((MauiIosLabel*)label).verticalAlignment =
                to_ui_control_content_vertical_alignment(view.vertical_text_alignment());
        }
    }

    void label_handler::map_character_spacing(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // LabelExtensions.UpdateCharacterSpacing (within the MapFormatting pipeline, so decorations
            // and alignment stay consistent with the C# refresh ordering).
            refresh_label_formatting(as_label(platform->native), view);
        }
    }

    void label_handler::map_text_decorations(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // LabelExtensions.UpdateTextDecorations (via the shared formatting refresh).
            refresh_label_formatting(as_label(platform->native), view);
        }
    }

    void label_handler::map_line_height(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // LabelExtensions.UpdateLineHeight — folded into the shared formatting refresh (applied first,
            // matching the C# MapFormatting order), so kerning + decorations + alignment stay consistent.
            refresh_label_formatting(as_label(platform->native), view);
        }
    }

    void label_handler::map_padding(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // LabelExtensions.UpdatePadding → MauiLabel.TextInsets (Top, Left, Bottom, Right). The DrawText
        // override flips left/right under RTL; sizeThatFits reflects the insets into the measured size.
        UILabel* const label = as_label(platform->native);
        if ([label isKindOfClass:[MauiIosLabel class]])
        {
            const maui::core::thickness pad = view.padding();
            ((MauiIosLabel*)label).textInsets =
                UIEdgeInsetsMake(static_cast<CGFloat>(pad.top), static_cast<CGFloat>(pad.left),
                                 static_cast<CGFloat>(pad.bottom), static_cast<CGFloat>(pad.right));
        }
    }

    void label_handler::map_formatted_text(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        UILabel* const label = as_label(platform->native);
        const auto& runs = view.formatted_text_runs();
        if (runs.empty())
        {
            // LabelExtensions.UpdateText FormattedText==null branch: AttributedText = null, then revert to the
            // plain text path — platformLabel.Text = text. Re-assign label.text from view.text() directly
            // (UILabel clears its text when attributedText is set to nil), then re-apply the formatting.
            label.attributedText = nil;
            const std::string text(view.text());
            label.text = [NSString stringWithUTF8String:text.c_str()];
            refresh_label_formatting(label, view);
            label.textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
        else
        {
            // FormattedText!=null branch: platformLabel.AttributedText = label.ToNSAttributedString().
            label.attributedText =
                maui::platform::ios::attributed_from_runs(runs, static_cast<double>(UIFont.labelFontSize));
            label.textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    // LabelHandler.MapLineBreakMode / MapMaxLines (Label.iOS.cs): both call UILabel.SetLineBreakMode, so
    // both port map fns delegate to the shared refresh_line_break_mode (the wrap mode + numberOfLines pair
    // depends on BOTH LineBreakMode and MaxLines, hence the single shared body).
    void label_handler::map_line_break_mode(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            refresh_line_break_mode(as_label(platform->native), view.line_break_mode(), view.max_lines());
        }
    }

    void label_handler::map_max_lines(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            refresh_line_break_mode(as_label(platform->native), view.line_break_mode(), view.max_lines());
        }
    }

    maui::graphics::size label_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        UILabel* const label = as_label(platform->native);
        // LabelHandler.iOS.GetDesiredSize: when the virtual view carries an explicit Width, clamp the width
        // constraint to it and set PreferredMaxLayoutWidth so the wrapped text measures over multiple lines;
        // otherwise PreferredMaxLayoutWidth = 0 (single-line measure). Then the native view measures itself
        // (UIView.SizeThatFits; MauiLabel.SizeThatFits already folds the TextInsets in).
        const i_label* const v = virtual_view();
        const double virtual_width = v != nullptr ? v->width() : maui::core::dimension::unset;
        if (maui::core::dimension::is_explicit_set(virtual_width))
        {
            if (std::isfinite(width_constraint))
            {
                width_constraint = std::min(width_constraint, virtual_width);
            }
            else
            {
                width_constraint = virtual_width;
            }
            label.preferredMaxLayoutWidth = static_cast<CGFloat>(virtual_width);
        }
        else
        {
            label.preferredMaxLayoutWidth = 0;
        }
        // ViewHandlerExtensions.GetDesiredSizeFromHandler: infinite constraints become the platform maximum.
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [label sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void label_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_label(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
