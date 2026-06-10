// label_handler — iOS (UIKit) platform recipe: a real UILabel (a MauiIosLabel subclass for the custom
// vertical alignment). Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from LabelHandler.iOS.cs + Platform/iOS/LabelExtensions.cs + MauiLabel.cs (the same
// oracles the AppKit twin in src/platform/apple/label_handler.mm was adapted from — UIKit needs no
// adaptation): CreatePlatformView = new MauiLabel(); MapText = UpdateTextPlainText + MapFormatting
// (TextDecorations → CharacterSpacing → HorizontalTextAlignment, with LineHeight deferred);
// UpdateTextColor / UpdateFont(LabelFontSize) / UpdateHorizontalTextAlignment (TextAlignment) /
// UpdateVerticalTextAlignment (MauiLabel.VerticalAlignment — the DrawText offset) / UpdateTextDecorations
// + UpdateCharacterSpacing (the attributed WithDecorations / WithCharacterSpacing passes). Not ported
// here (deferred with the cross-platform contract/mapper): LineHeight + Padding (TextInsets) + the
// LineBreakMode/MaxLines pair (not in i_label), MapBackground/NeedsContainer (no container
// infrastructure), and GetDesiredSize's PreferredMaxLayoutWidth branch (needs the virtual Width;
// single-line UILabel default measure via sizeThatFits, like the button recipe).

#import <UIKit/UIKit.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_text_ops.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// MauiIosLabel  <=  Microsoft.Maui.Platform.MauiLabel — a UILabel honoring a vertical content alignment
// by offsetting the draw rect (UILabel itself always centers vertically): Top pins the text height to
// the top, Bottom to the bottom, Center/Fill leave UIKit's own centering. TextInsets (Padding) is
// deferred with the cross-platform padding mapping, so DrawText's inset/RTL-flip half is not ported.
@interface MauiIosLabel : UILabel
@property(nonatomic) UIControlContentVerticalAlignment verticalAlignment;
@end

@implementation MauiIosLabel
- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        _verticalAlignment = UIControlContentVerticalAlignmentCenter; // MauiLabel's field default
    }
    return self;
}

- (void)setVerticalAlignment:(UIControlContentVerticalAlignment)value
{
    _verticalAlignment = value;
    [self setNeedsDisplay]; // MauiLabel.VerticalAlignment setter
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

// MauiLabel.DrawText (minus the TextInsets half — padding is deferred).
- (void)drawTextInRect:(CGRect)rect
{
    if (self.verticalAlignment != UIControlContentVerticalAlignmentCenter &&
        self.verticalAlignment != UIControlContentVerticalAlignmentFill)
    {
        rect = [self maui_alignVertical:rect];
    }
    [super drawTextInRect:rect];
}
@end

namespace
{
    UILabel* as_label(void* native)
    {
        return (__bridge UILabel*)native;
    }

    using maui::platform::ios::to_ui_color;
    using maui::platform::ios::to_ui_control_content_vertical_alignment;
    using maui::platform::ios::to_ui_font;
    using maui::platform::ios::to_ns_text_alignment;
    using maui::platform::ios::with_character_spacing;
    using maui::platform::ios::with_decorations;

    // LabelHandler.MapFormatting: "Update all of the attributed text formatting properties" —
    // TextDecorations (LabelExtensions.UpdateTextDecorations), then CharacterSpacing
    // (UpdateCharacterSpacing), then re-assert the horizontal alignment ("Setting any of those may have
    // removed text alignment settings"). LineHeight is deferred with the cross-platform mapper.
    void refresh_label_formatting(UILabel* label, const maui::core::i_label& view)
    {
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

    std::unique_ptr<label_platform> label_handler::create_platform_view()
    {
        auto platform = std::make_unique<label_platform>();
        MauiIosLabel* const label = [[MauiIosLabel alloc] initWithFrame:CGRectZero]; // new MauiLabel()
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
        if (platform != nullptr)
        {
            // LabelExtensions.UpdateTextColor — ToPlatform(LabelColor); the TextColor == null fallback to
            // the theme label color has no analog (the port's color is a non-nullable value type, the
            // same collapse as the button twin).
            as_label(platform->native).textColor = to_ui_color(view.text_color());
        }
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

    maui::graphics::size label_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // ViewHandlerExtensions.GetDesiredSizeFromHandler: infinite constraints become the platform
        // maximum, then the native view measures itself (UIView.SizeThatFits). LabelHandler.iOS's
        // PreferredMaxLayoutWidth branch (explicit virtual Width) is deferred with multi-line wrapping.
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_label(platform->native) sizeThatFits:CGSizeMake(width, height)];
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
