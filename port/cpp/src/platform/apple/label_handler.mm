// label_handler — Apple (AppKit / macOS) platform recipe: a non-editable, label-style NSTextField.
// Translated from LabelHandler.iOS.cs (UIKit's MauiLabel → AppKit's NSTextField label). Compiled as
// Objective-C++ with ARC for the `apple` backend.

#import <AppKit/AppKit.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "apple_text_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/label_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// A label-style NSTextFieldCell that honors a maui vertical text alignment. AppKit's NSTextField has no
// vertical alignment (MauiLabel.VerticalAlignment is custom on iOS), so this cell offsets the title rect
// within the cell bounds: Start = top, Center = middle, End = bottom — the AppKit analog of
// TextAlignmentExtensions.ToPlatformVertical (Start->Top / Center->Center / End->Bottom).
@interface MauiLabelCell : NSTextFieldCell
@property(nonatomic) maui::core::text_alignment verticalAlignment;
@end

@implementation MauiLabelCell
- (NSRect)titleRectForBounds:(NSRect)bounds
{
    NSRect rect = [super titleRectForBounds:bounds];
    const CGFloat full = bounds.size.height;
    const CGFloat text_height = rect.size.height;
    switch (self.verticalAlignment)
    {
        case maui::core::text_alignment::center:
        case maui::core::text_alignment::justify:
            rect.origin.y = bounds.origin.y + ((full - text_height) / 2);
            break;
        case maui::core::text_alignment::end:
            rect.origin.y = bounds.origin.y + (full - text_height);
            break;
        case maui::core::text_alignment::start:
            rect.origin.y = bounds.origin.y;
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

    // Rebuild the label's attributed string from its current plain text with the given kerning, mirroring
    // LabelExtensions.UpdateCharacterSpacing (AttributedText.WithCharacterSpacing). When spacing == 0 the
    // attributed value is reset to the plain string so the un-kerned text path is used. Re-applies the
    // alignment afterward (NSTextField resets paragraph alignment when the attributed value is replaced).
    void refresh_label_text_formatting(NSTextField* field, const maui::core::i_label& view)
    {
        const double spacing = view.character_spacing();
        NSAttributedString* const attributed = maui::platform::apple::kern_attributed(field.stringValue, spacing, nil);
        if (attributed != nil)
        {
            field.attributedStringValue = attributed;
        }
        else
        {
            field.attributedStringValue = [[NSAttributedString alloc] initWithString:field.stringValue];
        }
        field.alignment = to_ns_text_alignment(view.horizontal_text_alignment());
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
            as_label(platform->native).textColor = maui::platform::apple::to_ns_color(view.text_color());
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

    maui::graphics::size label_handler::get_desired_size(double /*width_constraint*/,
                                                         double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_label(platform->native) fittingSize];
        return {fitting.width, fitting.height};
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
} // namespace maui::core
