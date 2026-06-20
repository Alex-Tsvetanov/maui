// check_box_handler — iOS (UIKit) platform recipe. UIKit has NO native check box, so the managed
// platform view is the DRAWN control ported from src/Core/src/Platform/iOS/MauiCheckBox.cs: a UIButton
// subclass that renders an 18pt CoreGraphics circle-with-checkmark (the C# file's "chosen to just
// match the android drawables" geometry — oval box path, 2pt line, the 3-point check polyline drawn in
// unit space and DestinationOut-punched through the filled box), template-rendered so the tint color
// applies, with the AlwaysOriginal disabled+tinted variants cached per instance. TouchUpInside toggles
// IsChecked and raises the checked-changed callback, which the handler routes to
// i_check_box::send_is_checked (CheckBoxHandler.OnCheckedChanged). Compiled as Objective-C++ with ARC
// only for the `ios` backend.
//
// Handler recipe from CheckBoxHandler.iOS.cs: CreatePlatformView (MinimumViewSize = MinimumSize 44),
// Connect/Disconnect (the CheckedChanged subscription), MapIsChecked / MapForeground
// (CheckBoxExtensions.UpdateIsChecked / UpdateForeground — solid paints only, null resets the default
// tint), and the GetDesiredSize minimum-size substitution. Not ported (documented): the
// SwitchAccessibilityTraits lazy-trait merge (the port pushes semantics through the shared semantics
// ops instead) and IUIViewLifeCycleEvents.MovedToWindow (no lifecycle-event consumers in the port yet).

#import <UIKit/UIKit.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/check_box_handler.hpp"
#include "maui/core/i_check_box.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// The drawn check box (MauiCheckBox.cs). All drawing constants match the C# original.
@interface MauiCheckBox : UIButton
@property(nonatomic) BOOL isChecked;
@property(nonatomic) CGFloat minimumViewSize;
// The CheckedChanged event as a block (the Obj-C analog of C#'s event; the handler installs/clears it).
@property(nonatomic, copy) void (^onCheckedChanged)(void);
// CheckBoxTintColor (nullable; nil = the default white-rendered template tinted by the view's tint).
@property(nonatomic, strong) UIColor* checkBoxTintColor;
@end

namespace
{
    // MauiCheckBox.cs drawing constants ("chosen to just match the android drawables").
    constexpr CGFloat k_default_size = 18.0;
    constexpr CGFloat k_line_width = 2.0;
} // namespace

@implementation MauiCheckBox
{
    // The process-wide template images (C#'s static Checked/Unchecked) live in checkBoxImage below;
    // these are the per-instance disabled+tinted variants (invalidated when the tint changes).
    UIImage* _checkedDisabledAndTinted;
    UIImage* _uncheckedDisabledAndTinted;
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        self.contentMode = UIViewContentModeCenter;
        self.imageView.contentMode = UIViewContentModeScaleAspectFit;
        self.contentHorizontalAlignment = UIControlContentHorizontalAlignmentCenter;
        self.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
        // Deprecated on iOS 15+ but still functional for non-configuration buttons; C# sets both under
        // the same suppression (CA1416/CA1422), so the port mirrors that — including the suppression.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        self.adjustsImageWhenDisabled = NO;
        self.adjustsImageWhenHighlighted = NO;
#pragma clang diagnostic pop
        [self addTarget:self action:@selector(onTouchUpInside:) forControlEvents:UIControlEventTouchUpInside];
    }
    return self;
}

- (void)onTouchUpInside:(id)sender
{
    (void)sender;
    self.isChecked = !self.isChecked;
    if (self.onCheckedChanged != nil)
    {
        self.onCheckedChanged();
    }
}

- (void)setIsChecked:(BOOL)value
{
    if (value == _isChecked)
    {
        return;
    }
    _isChecked = value;
    [self updateDisplay];
}

- (void)setCheckBoxTintColor:(UIColor*)value
{
    if (value == _checkBoxTintColor)
    {
        return;
    }
    _checkedDisabledAndTinted = nil;
    _uncheckedDisabledAndTinted = nil;
    _checkBoxTintColor = value;
    // C# CheckBoxTintUIColor set: the template tint flows through ImageView.TintColor + TintColor.
    self.imageView.tintColor = value;
    self.tintColor = value;
    if (self.enabled)
    {
        [self setNeedsDisplay];
    }
    else
    {
        [self updateDisplay];
    }
}

// The effective tint (C# CheckBoxTintUIColor getter: the stored color, else white).
- (UIColor*)effectiveTintColor
{
    return _checkBoxTintColor != nil ? _checkBoxTintColor : UIColor.whiteColor;
}

- (void)setEnabled:(BOOL)value
{
    const BOOL changed = super.enabled != value;
    super.enabled = value;
    if (changed)
    {
        [self updateDisplay];
    }
}

// CreateBoxPath: the box is an oval over the padded square.
+ (UIBezierPath*)boxPathForRect:(CGRect)backgroundRect
{
    return [UIBezierPath bezierPathWithOvalInRect:backgroundRect];
}

// CreateCheckPath + DrawCheckMark: the unit-space check polyline (stroked at 0.077, round caps/joins).
+ (UIBezierPath*)checkPath
{
    UIBezierPath* const path = [UIBezierPath bezierPath];
    path.lineWidth = (CGFloat)0.077;
    path.lineCapStyle = kCGLineCapRound;
    path.lineJoinStyle = kCGLineJoinRound;
    [path moveToPoint:CGPointMake(0.72F, 0.22F)];
    [path addLineToPoint:CGPointMake(0.33F, 0.6F)];
    [path addLineToPoint:CGPointMake(0.15F, 0.42F)];
    return path;
}

// CreateCheckMark/RenderCheckMark(CGContext): the white check stroked in a translated+scaled unit space.
+ (UIImage*)createCheckMark
{
    UIGraphicsImageRenderer* const renderer =
        [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(k_default_size, k_default_size)];
    return [renderer imageWithActions:^(UIGraphicsImageRendererContext* ctx) {
      CGContextRef context = ctx.CGContext; // CGContextRef is itself a pointer typedef (no const slot)
      CGContextSaveGState(context);
      const CGFloat padding = k_line_width / 2;
      const CGFloat diameter = k_default_size - k_line_width;
      CGContextTranslateCTM(context, padding + (CGFloat)(0.05 * diameter), padding + (CGFloat)(0.1 * diameter));
      CGContextScaleCTM(context, diameter, diameter);
      UIBezierPath* const path = [MauiCheckBox checkPath];
      [UIColor.whiteColor setStroke];
      [path stroke];
      CGContextRestoreGState(context);
    }];
}

// CreateCheckBox/RenderCheckMark(CGContext, UIImage?): the stroked oval box; when `check` is given the
// box is filled and the check mark is punched out of the fill (DestinationOut).
- (UIImage*)createCheckBoxImageWithCheck:(UIImage*)check
{
    UIColor* const tint = [self effectiveTintColor];
    UIGraphicsImageRenderer* const renderer =
        [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(k_default_size, k_default_size)];
    return [renderer imageWithActions:^(UIGraphicsImageRendererContext* ctx) {
      (void)ctx;
      [tint setFill];
      [tint setStroke];
      const CGFloat padding = k_line_width / 2;
      const CGFloat diameter = k_default_size - k_line_width;
      const CGRect backgroundRect = CGRectMake(padding, padding, diameter, diameter);
      UIBezierPath* const boxPath = [MauiCheckBox boxPathForRect:backgroundRect];
      boxPath.lineWidth = k_line_width;
      [boxPath stroke];
      if (check != nil)
      {
          [boxPath fill];
          [check drawAtPoint:CGPointZero blendMode:kCGBlendModeDestinationOut alpha:1];
      }
    }];
}

// GetCheckBoxImage: the disabled+tinted variants render AlwaysOriginal ("when disabled it always tints
// them grey" otherwise); the normal pair are process-wide AlwaysTemplate statics.
- (UIImage*)checkBoxImage
{
    static UIImage* checkedTemplate;
    static UIImage* uncheckedTemplate;
    if (!self.enabled && self.checkBoxTintColor != nil)
    {
        if (self.isChecked)
        {
            if (_checkedDisabledAndTinted == nil)
            {
                _checkedDisabledAndTinted = [[self createCheckBoxImageWithCheck:[MauiCheckBox createCheckMark]]
                    imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
            }
            return _checkedDisabledAndTinted;
        }
        if (_uncheckedDisabledAndTinted == nil)
        {
            _uncheckedDisabledAndTinted =
                [[self createCheckBoxImageWithCheck:nil] imageWithRenderingMode:UIImageRenderingModeAlwaysOriginal];
        }
        return _uncheckedDisabledAndTinted;
    }
    if (checkedTemplate == nil)
    {
        checkedTemplate = [[self createCheckBoxImageWithCheck:[MauiCheckBox createCheckMark]]
            imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
    }
    if (uncheckedTemplate == nil)
    {
        uncheckedTemplate =
            [[self createCheckBoxImageWithCheck:nil] imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
    }
    return self.isChecked ? checkedTemplate : uncheckedTemplate;
}

- (void)updateDisplay
{
    [self setImage:[self checkBoxImage] forState:UIControlStateNormal];
    [self setNeedsDisplay];
}

// SizeThatFits: floor both axes at MinimumViewSize, then square down to the smaller.
- (CGSize)sizeThatFits:(CGSize)size
{
    const CGSize result = [super sizeThatFits:size];
    const CGFloat height = std::max(self.minimumViewSize, result.height);
    const CGFloat width = std::max(self.minimumViewSize, result.width);
    const CGFloat final_side = std::min(width, height);
    return CGSizeMake(final_side, final_side);
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    [self updateDisplay];
    // Re-frame the clip mask to the new bounds (WrapperView.LayoutSubviews re-runs SetClip): the mask is
    // sized at map time, before the first layout, when bounds is 0×0. No-op when no clip is set.
    maui::platform::ios::reapply_clip((__bridge void*)self);
}

// AccessibilityValue: "1"/"0" mirrors the C# override (VoiceOver reads the checked state).
- (NSString*)accessibilityValue
{
    return self.isChecked ? @"1" : @"0";
}
@end

namespace
{
    MauiCheckBox* as_check_box(void* native)
    {
        return (__bridge MauiCheckBox*)native;
    }

    using maui::platform::ios::to_ui_color;
} // namespace

namespace maui::core
{
    check_box_platform::~check_box_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void check_box_platform::update_visibility(maui::core::visibility value)
    {
        as_check_box(native).hidden = value != maui::core::visibility::visible;
    }

    void check_box_platform::update_opacity(double value)
    {
        as_check_box(native).alpha = value;
    }

    void check_box_platform::update_is_enabled(bool value)
    {
        // C#'s MauiCheckBox.IsEnabled gates UserInteractionEnabled (and refreshes the image through the
        // Enabled override).
        as_check_box(native).enabled = static_cast<BOOL>(value);
        as_check_box(native).userInteractionEnabled = static_cast<BOOL>(value);
    }

    void check_box_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_check_box(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    // ViewHandler.MapClip → WrapperView.SetClip: mask the native view's layer to the clip
    // geometry, sized to the view's CURRENT bounds (0×0 before the first layout — the layout hook
    // re-frames it). apply_and_store_clip both applies and stashes the borrow for that re-frame.
    void check_box_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const CGRect bounds = ((__bridge UIView*)native).bounds;
        maui::platform::ios::apply_and_store_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    std::unique_ptr<check_box_platform> check_box_handler::create_platform_view()
    {
        auto platform = std::make_unique<check_box_platform>();
        // CheckBoxHandler.iOS.CreatePlatformView: MinimumViewSize = MinimumSize (44pt touch target).
        MauiCheckBox* const native = [[MauiCheckBox alloc] initWithFrame:CGRectZero];
        native.minimumViewSize = 44.0;
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void check_box_handler::on_connect_handler(check_box_platform& platform)
    {
        // CheckBoxHandler.iOS.ConnectHandler: subscribe CheckedChanged → write the native state back
        // (OnCheckedChanged: VirtualView.IsChecked = platformView.IsChecked).
        MauiCheckBox* const native = as_check_box(platform.native);
        const check_box_handler* const handler = this; // the block only reads through the handler
        native.onCheckedChanged = ^{
          auto* view = handler->virtual_view();
          auto* platform_view = handler->typed_platform_view();
          if (view != nullptr && platform_view != nullptr)
          {
              const bool native_checked = as_check_box(platform_view->native).isChecked;
              if (view->is_checked() != native_checked)
              {
                  view->send_is_checked(native_checked);
              }
          }
        };
    }

    void check_box_handler::on_disconnect_handler(check_box_platform& platform)
    {
        as_check_box(platform.native).onCheckedChanged = nil;
    }

    void check_box_handler::map_is_checked(check_box_handler& handler, i_check_box& view)
    {
        // CheckBoxExtensions.UpdateIsChecked.
        if (auto* platform = handler.typed_platform_view())
        {
            as_check_box(platform->native).isChecked = static_cast<BOOL>(view.is_checked());
        }
    }

    void check_box_handler::map_foreground(check_box_handler& handler, i_check_box& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // CheckBoxExtensions.UpdateForeground: solid paints only ("For the moment, we're only
        // supporting solid color Paint for the iOS Checkbox"); null resets the default tint.
        const maui::graphics::paint* foreground = view.foreground();
        as_check_box(platform->native).checkBoxTintColor =
            foreground != nullptr ? to_ui_color(foreground->background_color()) : nil;
    }

    maui::graphics::size check_box_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_check_box(platform->native) sizeThatFits:CGSizeMake(width, height)];
        maui::graphics::size result{fitting.width, fitting.height};

        // CheckBoxHandler.iOS.GetDesiredSize: a zero-measured axis under a free (non-positive or
        // infinite) constraint substitutes the 44pt MinimumSize.
        constexpr double minimum_size = 44.0;
        bool substituted = false;
        double substituted_width = width_constraint;
        double substituted_height = height_constraint;
        if (result.width == 0 && (width_constraint <= 0 || std::isinf(width_constraint)))
        {
            substituted_width = minimum_size;
            substituted = true;
        }
        if (result.height == 0 && (height_constraint <= 0 || std::isinf(height_constraint)))
        {
            substituted_height = minimum_size;
            substituted = true;
        }
        if (substituted)
        {
            result = {substituted_width, substituted_height};
        }
        return result;
    }

    void check_box_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_check_box(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
