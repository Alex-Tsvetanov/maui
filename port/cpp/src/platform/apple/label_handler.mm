// label_handler — Apple (AppKit / macOS) platform recipe: a non-editable, label-style NSTextField.
// Translated from LabelHandler.iOS.cs (UIKit's MauiLabel → AppKit's NSTextField label). Compiled as
// Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_text_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// A label-style NSTextFieldCell that honors a maui vertical text alignment + the label Padding. AppKit's
// NSTextField has no vertical alignment (MauiLabel.VerticalAlignment is custom on iOS), so this cell
// offsets the title rect within the cell bounds: Start = top, Center = middle, End = bottom — the AppKit
// analog of TextAlignmentExtensions.ToPlatformVertical (Start->Top / Center->Center / End->Bottom). It
// also insets the drawing rect by the label Padding (the cell analog of MauiLabel.TextInsets), flipping
// the left/right insets under RTL exactly as MauiLabel.DrawText does.
@interface MauiLabelCell : NSTextFieldCell
@property(nonatomic) maui::core::text_alignment verticalAlignment;
@property(nonatomic) maui::core::thickness textInsets;
// Mirror of UILabel.numberOfLines (0 = "as many as needed"). AppKit's NSTextFieldCell has no direct
// numberOfLines analog — SetLineBreakMode's UILabel.Lines half — so the cell carries the resolved line
// count itself and drives NSCell.usesSingleLineMode at set time for the single-line modes.
@property(nonatomic) NSInteger numberOfLines;
@end

@implementation MauiLabelCell
// SetLineBreakMode's UILabel.Lines half on AppKit: 1 line => usesSingleLineMode (the cell collapses to a
// single truncating/clipping line); 0 ("unset") or >1 leave the cell multi-line. NSCell.usesSingleLineMode
// is the available analog (the cell has no maximumNumberOfLines on this SDK); the resolved count is kept on
// the custom property so the handler/tests observe the faithful UILabel.Lines value.
- (void)setNumberOfLines:(NSInteger)value
{
    _numberOfLines = value;
    self.usesSingleLineMode = value == 1 ? YES : NO;
}

// Inset the bounds by the (RTL-flipped) Padding — the AppKit equivalent of MauiLabel.DrawText's
// `insets.InsetRect(rect)`. The layout direction is read live from the control view at draw/measure time
// (MauiLabel.DrawText reads EffectiveUserInterfaceLayoutDirection live), so a later flow-direction change
// is honored without re-mapping Padding.
- (NSRect)maui_insetBounds:(NSRect)bounds
{
    const maui::core::thickness insets = self.textInsets;
    const BOOL rtl =
        self.controlView.userInterfaceLayoutDirection == NSUserInterfaceLayoutDirectionRightToLeft ? YES : NO;
    const CGFloat left = rtl ? insets.right : insets.left;
    const CGFloat right = rtl ? insets.left : insets.right;
    bounds.origin.x += left;
    bounds.size.width -= (left + right);
    bounds.origin.y += insets.top;
    bounds.size.height -= (insets.top + insets.bottom);
    if (bounds.size.width < 0)
    {
        bounds.size.width = 0;
    }
    if (bounds.size.height < 0)
    {
        bounds.size.height = 0;
    }
    return bounds;
}

- (NSRect)titleRectForBounds:(NSRect)bounds
{
    const NSRect inset = [self maui_insetBounds:bounds];
    NSRect rect = [super titleRectForBounds:inset];
    const CGFloat full = inset.size.height;
    const CGFloat text_height = rect.size.height;
    switch (self.verticalAlignment)
    {
        case maui::core::text_alignment::center:
        case maui::core::text_alignment::justify:
            rect.origin.y = inset.origin.y + ((full - text_height) / 2);
            break;
        case maui::core::text_alignment::end:
            rect.origin.y = inset.origin.y + (full - text_height);
            break;
        case maui::core::text_alignment::start:
            rect.origin.y = inset.origin.y;
            break;
    }
    return rect;
}

- (void)drawInteriorWithFrame:(NSRect)frame inView:(NSView*)controlView
{
    [super drawInteriorWithFrame:[self titleRectForBounds:frame] inView:controlView];
}
@end

namespace
{
    NSTextField* as_label(void* native)
    {
        return (__bridge NSTextField*)native;
    }

    NSTextAlignment to_ns_text_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return NSTextAlignmentCenter;
            case maui::core::text_alignment::end:
                return NSTextAlignmentRight;
            case maui::core::text_alignment::justify:
                return NSTextAlignmentJustified;
            case maui::core::text_alignment::start:
                return NSTextAlignmentLeft;
        }
        return NSTextAlignmentLeft;
    }

    // Rebuild the label's attributed string from its current plain text with the given line-height +
    // decorations + kerning, mirroring the LabelHandler.MapFormatting pipeline in C# order (UpdateLineHeight
    // → UpdateTextDecorations → UpdateCharacterSpacing → re-assert HorizontalTextAlignment). Each pass is
    // cumulative over the running attributed value; a nil return (the "nothing to do" branch of the
    // corresponding With* helper) leaves the prior value untouched. Re-applies the alignment afterward
    // (NSTextField resets paragraph alignment when the attributed value is replaced).
    void refresh_label_text_formatting(NSTextField* field, const maui::core::i_label& view)
    {
        const double spacing = view.character_spacing();
        NSAttributedString* attributed = maui::platform::apple::kern_attributed(field.stringValue, spacing, nil);
        if (attributed == nil)
        {
            attributed = [[NSAttributedString alloc] initWithString:field.stringValue];
        }
        // UpdateLineHeight: a paragraph-style lineHeightMultiple (no-op for the -1 default with no style).
        NSAttributedString* const with_lh = maui::platform::apple::with_line_height(attributed, view.line_height());
        if (with_lh != nil)
        {
            attributed = with_lh;
        }
        NSAttributedString* const decorated =
            maui::platform::apple::with_decorations(attributed, view.text_decorations());
        field.attributedStringValue = decorated != nil ? decorated : attributed;
        field.alignment = to_ns_text_alignment(view.horizontal_text_alignment());
    }

    // The AppKit twin of TextExtensions.SetLineBreakMode (the shared body MapLineBreakMode + MapMaxLines
    // both invoke): set NSTextFieldCell.lineBreakMode from the LineBreakMode, and the resolved numberOfLines
    // (computed exactly as the UIKit twin) onto the custom cell's numberOfLines:
    //   maxLines = MaxLines<0 ? (TailTruncation ? 1 : 0) : MaxLines;  NoWrap/Head/MiddleTruncation force 1.
    void refresh_line_break_mode(NSTextField* field, maui::core::line_break_mode mode, int max_lines)
    {
        using maui::core::line_break_mode;
        NSInteger lines = max_lines;
        if (max_lines < 0)
        {
            lines = mode == line_break_mode::tail_truncation ? 1 : 0;
        }
        if (mode == line_break_mode::no_wrap || mode == line_break_mode::head_truncation ||
            mode == line_break_mode::middle_truncation)
        {
            lines = 1;
        }
        field.lineBreakMode = maui::platform::apple::to_ns_line_break_mode(mode);
        if ([field.cell isKindOfClass:[MauiLabelCell class]])
        {
            ((MauiLabelCell*)field.cell).numberOfLines = lines;
        }
    }
} // namespace

namespace maui::core
{
    label_platform::~label_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native);
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
        as_label(native).alphaValue = value;
    }

    void label_platform::update_is_enabled(bool value)
    {
        [as_label(native) setEnabled:static_cast<BOOL>(value)];
    }

    void label_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_label(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<label_platform> label_handler::create_platform_view()
    {
        auto platform = std::make_unique<label_platform>();
        NSTextField* const field = [NSTextField labelWithString:@""]; // non-editable, borderless label style
        // Swap in the vertical-alignment-aware cell, copying the label cell's style so the borderless,
        // non-editable, transparent-background label appearance is preserved (map_vertical_text_alignment
        // then drives its alignment).
        MauiLabelCell* const cell = [[MauiLabelCell alloc] initTextCell:@""];
        cell.editable = NO;
        cell.selectable = NO;
        cell.bordered = NO;
        cell.bezeled = NO;
        cell.drawsBackground = NO;
        cell.verticalAlignment = maui::core::text_alignment::start;
        cell.textInsets = maui::core::thickness{}; // zero Padding default (MauiLabel.TextInsets default)
        field.cell = cell;
        platform->native = (__bridge_retained void*)field;
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
        NSString* const value = [NSString stringWithUTF8String:text.c_str()];
        NSTextField* const field = as_label(platform->native);
        field.stringValue = value != nil ? value : @"";
        // Any text update re-applies the attributed formatting (C# MapText -> MapFormatting).
        refresh_label_text_formatting(field, view);
    }

    void label_handler::map_text_color(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // Unset TextColor -> NSColor.textColor: dynamic (black in Aqua, white in DarkAqua), so the
            // label follows the window appearance instead of the port's opaque-black default sentinel.
            //
            // NOT NSColor.labelColor, the obvious UIColor.labelColor twin: AppKit's labelColor is black at
            // 85% ALPHA, which renders (39,39,39) on white. Measured — it moved every light frame off pure
            // black, and MAUI's own label_light draws (0,0,0) on both Catalyst and iOS. textColor is
            // opaque, so it fixes dark without touching light.
            NSColor* const explicit_color = maui::platform::apple::explicit_text_color_or_nil(view);
            as_label(platform->native).textColor = explicit_color != nil ? explicit_color : NSColor.textColor;
        }
    }

    void label_handler::map_font(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_label(platform->native).font = maui::platform::apple::to_ns_font(view.font());
        }
    }

    void label_handler::map_horizontal_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_label(platform->native).alignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void label_handler::map_vertical_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSTextField* const field = as_label(platform->native);
        if ([field.cell isKindOfClass:[MauiLabelCell class]])
        {
            ((MauiLabelCell*)field.cell).verticalAlignment = view.vertical_text_alignment();
            [field setNeedsDisplay:YES];
        }
    }

    void label_handler::map_character_spacing(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            refresh_label_text_formatting(as_label(platform->native), view);
        }
    }

    void label_handler::map_text_decorations(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // LabelExtensions.UpdateTextDecorations — the refresh applies WithDecorations (and keeps the
            // kerning + alignment in place, as the C# MapFormatting ordering guarantees).
            refresh_label_text_formatting(as_label(platform->native), view);
        }
    }

    void label_handler::map_line_height(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // LabelExtensions.UpdateLineHeight — folded into the shared formatting refresh (which applies it
            // first, then decorations + kerning, matching the C# MapFormatting order).
            refresh_label_text_formatting(as_label(platform->native), view);
        }
    }

    void label_handler::map_padding(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // LabelExtensions.UpdatePadding(MauiLabel.TextInsets) — the AppKit cell carries the inset and flips
        // it under RTL when drawing (the cell reads the control view's layout direction live, so a later
        // flow-direction change is honored without re-mapping Padding).
        NSTextField* const field = as_label(platform->native);
        if ([field.cell isKindOfClass:[MauiLabelCell class]])
        {
            ((MauiLabelCell*)field.cell).textInsets = view.padding();
            [field setNeedsDisplay:YES];
        }
    }

    void label_handler::map_formatted_text(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        NSTextField* const field = as_label(platform->native);
        const auto& runs = view.formatted_text_runs();
        if (runs.empty())
        {
            // LabelExtensions.UpdateText FormattedText==null branch: revert to the plain text path —
            // platformLabel.Text = text. Re-assign stringValue from view.text() directly (setting an empty
            // attributedStringValue would wipe the field), then re-apply the attributed formatting.
            const std::string text(view.text());
            NSString* const value = [NSString stringWithUTF8String:text.c_str()];
            field.stringValue = value != nil ? value : @"";
            refresh_label_text_formatting(field, view);
        }
        else
        {
            // LabelExtensions.UpdateText FormattedText!=null branch: platformLabel.AttributedText =
            // label.ToNSAttributedString() — build one attributed substring per resolved run.
            field.attributedStringValue = maui::platform::apple::attributed_from_runs(runs);
            // Re-apply the alignment: replacing the attributed value resets the field's paragraph alignment.
            field.alignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    // LabelHandler.MapLineBreakMode / MapMaxLines (Label.iOS.cs, AppKit twin): both call the cell's
    // SetLineBreakMode equivalent, so both port map fns delegate to the shared refresh_line_break_mode
    // (the wrap mode + numberOfLines pair depends on BOTH LineBreakMode and MaxLines).
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

    maui::graphics::size label_handler::get_desired_size(double width_constraint, double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        NSTextField* const field = as_label(platform->native);
        // LabelHandler.iOS.GetDesiredSize's PreferredMaxLayoutWidth branch: an explicit virtual Width caps
        // the wrap width (clamped by the incoming constraint) and turns on multi-line wrapping; otherwise
        // the label measures single-line (PreferredMaxLayoutWidth = 0). NSTextField wraps when its cell's
        // `wraps` is set and `preferredMaxLayoutWidth` is finite.
        const i_label* const v = virtual_view();
        const double virtual_width = v != nullptr ? v->width() : maui::core::dimension::unset;
        if (maui::core::dimension::is_explicit_set(virtual_width))
        {
            double wrap_width = virtual_width;
            if (std::isfinite(width_constraint) && width_constraint < wrap_width)
            {
                wrap_width = width_constraint;
            }
            field.cell.wraps = YES;
            field.preferredMaxLayoutWidth = static_cast<CGFloat>(wrap_width);
        }
        else
        {
            field.cell.wraps = NO;
            field.preferredMaxLayoutWidth = 0;
        }
        const NSSize fitting = [field fittingSize];
        // MauiLabel.SizeThatFits adds the TextInsets back onto the measured content — the cell's inset
        // shrinks the drawing rect, so the desired size must include the padding.
        const maui::core::thickness pad = v != nullptr ? v->padding() : maui::core::thickness{};
        return {fitting.width + pad.left + pad.right, fitting.height + pad.top + pad.bottom};
    }

    void label_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_label(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform + flow direction pushed to the native view via the shared apple_view_ops helpers
    // (M4c: the generic-IView ViewMapper widening). `native` is this struct's NSView handle.
    void label_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void label_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    // Background / shadow / clip pushed to the native view's layer via the shared apple_visual_ops helpers
    // (M4d ViewMapper visuals). `native` is this struct's NSView handle.
    void label_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void label_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void label_platform::update_clip(const maui::graphics::i_shape* value)
    {
        // The clip mask is sized to the view's current bounds (WrapperView.SetClip uses the view frame).
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    // Accessibility metadata + the input-transparent flag pushed to the native view via the shared
    // apple_semantics_ops helpers (M5d native a11y / hit-test). `native` is this struct's NSView handle.
    void label_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void label_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }
} // namespace maui::core
