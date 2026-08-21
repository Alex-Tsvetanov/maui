// radio_button_handler — iOS (UIKit) platform recipe: the NATIVE DEFAULT FALLBACK (documented
// deviation). C# renders RadioButton on iOS through a ControlTemplate (RadioButtonHandler.iOS.cs hosts
// a plain ContentView and leaves the text/stroke maps [MissingMapper]); the port's templated content
// is deferred, so this partial renders the DefaultTemplate's essentials natively instead: a
// UIButton(System) whose Normal/Selected images are the circle / filled-circle SF-symbol pair (the
// template's Ellipse indicator + CheckedIndicator), the string content on the title, IsChecked riding
// UIButton.selected — plus C#'s MapIsChecked AccessibilityValue "1"/"0" push, kept verbatim. A tap
// flows back through a target-action proxy to send_is_checked(true) (RadioButton.SelectRadioButton —
// a radio tap SELECTS; the cross-platform RadioButtonGroup unchecks the others). The text style uses
// the ButtonExtensions recipe (title colors + kerned attributed title + TitleLabel font); the
// stroke/corner ride the layer. Compiled as Objective-C++ with ARC only for the `ios` backend.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_text_ops.hpp"
#include "ios_visual_ops.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_radio_button.hpp"
#include "maui/core/radio_button_handler.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

// Obj-C trampoline: forwards the UIButton's touch-up-inside to the C++ handler's virtual view.
@interface MauiIosRadioButtonProxy : NSObject
@property(nonatomic) maui::core::radio_button_handler* handler;
- (void)onSelect:(id)sender;
@end

@implementation MauiIosRadioButtonProxy
- (void)onSelect:(id)sender
{
    (void)sender;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            // RadioButton.SelectRadioButton: a tap checks the button (the from-handler write).
            view->send_is_checked(true);
        }
    }
}
@end

namespace
{
    // Key for the associated MauiIosRadioButtonProxy kept alive by the UIButton (targets are weak).
    const char k_proxy_key = 0;

    UIButton* as_button(void* native)
    {
        return (__bridge UIButton*)native;
    }

    // The DefaultTemplate Grid's ColumnSpacing (RadioButton.cs:536) — the gap between the ring and the
    // title. Opened via the image/title/content inset split (see apply_template_chrome).
    constexpr CGFloat k_gap = 6;

    // C# BorderElement.BorderWidthProperty's "not set" default (BorderElement.cs:17). MAUI feeds this raw
    // -1 into Border.CrossPlatformMeasure's `Padding + StrokeThickness`, so it is a real measurement input,
    // not just a flag — see template_pad below.
    constexpr double k_unset_border_width = -1.0;

    // ONE SIDE of the DefaultTemplate's chrome, in points, as a function of RadioButton.BorderWidth.
    //
    // BuildDefaultTemplate wraps the indicator + content in Border(Padding = 6) > Grid(Padding = 2)
    // (RadioButton.cs:520 and :534) and binds the Border's StrokeThickness to the RadioButton's
    // BorderWidth (RadioButton.cs:528). Border.CrossPlatformMeasure insets its content by
    // `Padding + StrokeThickness` (Border.cs:354 — Thickness + double adds to all four sides), so one
    // side of chrome is (6 + BorderWidth) + 2, the Grid's own Padding.
    //
    // The subtlety that makes this 7 and not the 8 the template's literals suggest: BorderWidth's
    // default is the -1d "not set" SENTINEL (BorderElement.cs:17), and CrossPlatformMeasure adds that
    // raw -1 straight into the sum — it does not clamp. So an unset radio's chrome is 6 + (-1) + 2 = 7pt
    // per side (14pt of total row chrome over the 21pt ring = the 35pt row the reference draws), and a
    // BorderWidth = 4 radio's is 6 + 4 + 2 = 12pt per side (45pt row). Both are confirmed against
    // radio_button_border_light @3x: the reference's unset rings start at x=69 and its BorderWidth=4
    // rings at x=84 — exactly 5pt = 15px apart, and its yellow rows measure 35.0pt and 45.0pt.
    //
    // This supersedes the earlier pair of constants that carried 7 as a blind calibration; 7 is what
    // the oracle's own arithmetic yields for the default, which is why it measured right.
    CGFloat template_pad(double border_width)
    {
        return static_cast<CGFloat>(8.0 + border_width);
    }

    // Push the template chrome onto the button for the given BorderWidth. contentEdgeInsets is what
    // feeds sizeThatFits, so this is also the measurement path; imageEdgeInsets/titleEdgeInsets only
    // move the two pieces relative to each other and net to zero on both axes.
    void apply_template_chrome(UIButton* button, double border_width)
    {
        const CGFloat pad = template_pad(border_width);
        // MAUI's template Label is TOP-aligned in the Grid row — Label.VerticalTextAlignment defaults to
        // TextAlignment.Start (Label.cs:24), which MauiLabel.AlignVertical turns into a rect pinned to
        // the top of the frame (MauiLabel.cs:60-77) — whereas a plain UIButton vertically CENTERS its
        // titleLabel in the content box. MEASURED on radio_button_group_gallery_light and
        // radio_button_border_light @3x: with the ring ink landing on the reference's ring to the pixel,
        // the port's title glyphs are byte-identical to the reference's but sit exactly 6px = 2pt lower
        // (a whole-region shift search bottoms out at dy=+6 with residual 0.0). The port renders the
        // radio natively rather than through the template, so the template's top-alignment has to be
        // expressed as a title offset here; 2pt is calibrated to the shipped render (RENDER-BREAKS-TIES)
        // because UIButton's internal titleLabel metrics are not derivable from the C# geometry.
        constexpr CGFloat k_title_rise = 2;
        button.imageEdgeInsets = UIEdgeInsetsMake(0, -k_gap / 2, 0, k_gap / 2);
        button.titleEdgeInsets = UIEdgeInsetsMake(-k_title_rise, k_gap / 2, k_title_rise, -k_gap / 2);
        button.contentEdgeInsets = UIEdgeInsetsMake(pad, pad + k_gap / 2, pad, pad + k_gap / 2);
    }

    using maui::platform::ios::to_ui_color;
    using maui::platform::ios::to_ui_font;

    // The kerned-title rebuild — the button partial's refresh_button_title_formatting twin over the
    // i_text_style face (see button_handler.mm for the plain-vs-attributed title rules).
    void refresh_radio_title_formatting(UIButton* button, const maui::core::i_text_style& view)
    {
        const double spacing = view.character_spacing();
        if (spacing == 0)
        {
            [button setAttributedTitle:nil forState:UIControlStateNormal];
            return;
        }
        // is-set discriminator (see label_handler.mm map_text_color): a value compare cannot tell an
        // explicit TextColor=Black from the default-constructed sentinel, so key off BindableObject.IsSet.
        const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
        UIColor* const foreground = color_is_set ? to_ui_color(view.text_color()) : UIColor.labelColor;
        NSString* const plain_title = [button titleForState:UIControlStateNormal];
        [button setAttributedTitle:maui::platform::ios::kern_attributed(plain_title, spacing, foreground)
                          forState:UIControlStateNormal];
    }
} // namespace

namespace maui::core
{
    // The teardown that must run whether the handler is DISCONNECTED or merely DESTROYED. The native
    // view outlives the handler in any real app (a superview retains it) and the trampolines it keeps
    // in its associated objects carry RAW handler pointers; nothing calls disconnect_handler() when a
    // handler is destroyed (there is no ~view_handler doing it), so the platform dtor has to run this
    // too or the next native callback dereferences freed memory. Idempotent: disconnect_handler()
    // destroys the platform right after calling it, so both paths run on the same object.
    namespace
    {
        void detach_trampolines(radio_button_platform& platform)
        {
            UIButton* const button = as_button(platform.native);
            if (auto* const proxy = (MauiIosRadioButtonProxy*)objc_getAssociatedObject(button, &k_proxy_key))
            {
                [button removeTarget:proxy action:@selector(onSelect:) forControlEvents:UIControlEventTouchUpInside];
                proxy.handler = nullptr; // the back-pointer live_view re-reads after user code
            }
            objc_setAssociatedObject(button, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
    } // namespace

    radio_button_platform::~radio_button_platform()
    {
        detach_trampolines(*this); // before any CFRelease: the void* slot holds the last retain
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void radio_button_platform::update_visibility(maui::core::visibility value)
    {
        as_button(native).hidden = value != maui::core::visibility::visible;
    }

    void radio_button_platform::update_opacity(double value)
    {
        as_button(native).alpha = value;
    }

    void radio_button_platform::update_is_enabled(bool value)
    {
        as_button(native).enabled = static_cast<BOOL>(value);
    }

    void radio_button_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_button(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    // VisualElement.Background → the RadioButton's fill (e.g. RadioButtonBorder's yellow Option 1/2). Unlike
    // the System UIButton (which ignores backgroundColor — see button_handler.mm), a Custom UIButton draws
    // its backgroundColor directly, and the layer cornerRadius set by map_corner_radius clips it to the
    // rounded card. Gradient/image paints defer to the shared layer applier; a null paint clears the fill.
    void radio_button_platform::update_background(const maui::graphics::paint* value)
    {
        UIButton* const button = as_button(native);
        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            button.backgroundColor = to_ui_color(solid->color());
        }
        else if (value != nullptr)
        {
            maui::platform::ios::apply_background(native, value);
        }
        else
        {
            button.backgroundColor = nil;
        }
    }

    std::unique_ptr<radio_button_platform> radio_button_handler::create_platform_view()
    {
        auto platform = std::make_unique<radio_button_platform>();
        // Custom (not System): the iOS-26 system button paints a tint-colored fill behind the SELECTED
        // state (a white box once the tint is the dynamic label color), which MAUI's templated radio has
        // no analog for. A custom button still tints the template SF-symbol indicators via tintColor.
        UIButton* const button = [UIButton buttonWithType:UIButtonTypeCustom];
        // The DefaultTemplate's indicator pair, DRAWN to the oracle's exact geometry rather than borrowed
        // from SF Symbols. RadioButton.cs:546-556 specifies the outer Ellipse as WidthRequest =
        // HeightRequest = 21 with StrokeThickness = 2, and the CheckedIndicator (:558-566) as an 11pt
        // filled Ellipse whose opacity flips 0 -> 1 on check.
        //
        // This USED to be [UIImage systemImageNamed:@"circle"] / @"smallcircle.filled.circle", which is
        // close enough to look right and wrong enough to never score green: an SF symbol renders at its
        // own natural size, not the requested one. MEASURED on radio_button_group_gallery_light (iOS @3x):
        // the reference ring is 63px tall with a 6px stroke (21.0pt / 2.0pt — the oracle's numbers exactly)
        // while the port drew 51px with a 4px stroke (17.0pt / 1.33pt). That 4pt shortfall is what kept
        // seven cells non-green across iOS and Catalyst.
        //
        // Drawn with UIGraphicsImageRenderer at the requested point size so the geometry is exact and
        // resolution-independent, and rendered as a TEMPLATE image so the existing tintColor path
        // (map_text_color below, and the DefaultTemplate's theme-aware stroke) still colours it.
        const CGFloat ring_diameter = 21;  // RadioButton.cs:552-553
        const CGFloat ring_stroke = 2;     // RadioButton.cs:554
        const CGFloat check_diameter = 11; // RadioButton.cs:562-563
        // The canvas is the ring PLUS one stroke width, and the path's bounding box is the full 21pt: a
        // centred stroke then straddles that path, spending its outer half in the extra margin instead of
        // being clipped at the canvas edge. Drawing the path INSET by stroke/2 inside a 21pt canvas —
        // the arithmetically tidy reading of "21pt with a 2pt stroke" — measured 57px against the
        // reference's 63px @3x, because the solid ink tracks the PATH diameter (19pt there) and the
        // stroke's outer half antialiases away. Sizing the path itself to 21pt is what reproduces the
        // reference's ink.
        const CGFloat canvas = ring_diameter;
        const CGRect ring_rect =
            CGRectInset(CGRectMake(0, 0, ring_diameter, ring_diameter), ring_stroke / 2, ring_stroke / 2);
        UIGraphicsImageRenderer* const renderer =
            [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(canvas, canvas)];

        // The ring alone: stroked on the INSET rect so the 2pt stroke sits inside the 21pt box (UIKit
        // strokes centred on the path, so a full-bounds circle would bleed 1pt out on every side and
        // measure 23pt).
        UIImage* const unchecked = [renderer imageWithActions:^(UIGraphicsImageRendererContext* ctx) {
          (void)ctx;
          UIBezierPath* const ring = [UIBezierPath bezierPathWithOvalInRect:ring_rect];
          ring.lineWidth = ring_stroke;
          [UIColor.labelColor setStroke];
          [ring stroke];
        }];
        // The ring PLUS the filled 11pt check mark, centred — the CheckedIndicator at opacity 1.
        UIImage* const checked = [renderer imageWithActions:^(UIGraphicsImageRendererContext* ctx) {
          (void)ctx;
          UIBezierPath* const ring = [UIBezierPath bezierPathWithOvalInRect:ring_rect];
          ring.lineWidth = ring_stroke;
          [UIColor.labelColor setStroke];
          [ring stroke];
          const CGFloat inset = (canvas - check_diameter) / 2;
          UIBezierPath* const mark =
              [UIBezierPath bezierPathWithOvalInRect:CGRectMake(inset, inset, check_diameter, check_diameter)];
          [UIColor.labelColor setFill];
          [mark fill];
        }];
        [button setImage:[unchecked imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate]
                forState:UIControlStateNormal];
        [button setImage:[checked imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate]
                forState:UIControlStateSelected];
        // Center, NOT the default scale-to-fill: UIButton fits its image into the content box, and with
        // the template chrome insets below that box is slightly under 21pt — so the exactly-sized image
        // was being scaled down to ~0.905 and landed at 19pt with a 1.81pt stroke. MEASURED: the drawn
        // image is right (2.0pt stroke) while the RENDER was 57px instead of the reference's 63px @3x, so
        // the defect was the button resizing the indicator, not the geometry. Centering pins it at its
        // natural 21pt.
        button.imageView.contentMode = UIViewContentModeCenter;
        // MAUI's DefaultTemplate renders the indicator + content as a LEFT-aligned row with a gap between
        // the ring and the label; a plain UIButton centers its content and butts the title flush against the
        // image. Left-align the content, then hand the template chrome to apply_template_chrome above — it
        // opens the Grid ColumnSpacing gap via the canonical image/title/content inset split (image left by
        // gap/2, title right by gap/2, content box grown by gap/2 each side so the title is NOT clipped) and
        // pads both axes by the Border(Padding) + BorderWidth + Grid(Padding) chrome. These insets are
        // deprecated in the UIButtonConfiguration era but remain functional + correct for this image+title
        // layout on a Custom button (the same tolerated deprecation as button_handler.mm's contentEdgeInsets),
        // and a configuration would lose the per-state Normal/Selected ring images this fallback relies on.
        //
        // Seeded at the BorderWidth SENTINEL (-1d, BorderElement.cs:17) because the virtual view is not
        // attached yet; map_stroke_thickness re-applies it with the real value on connect and on every
        // change, which is what makes a bordered radio grow instead of squeezing its own fill inward.
        button.contentHorizontalAlignment = UIControlContentHorizontalAlignmentLeft;
        apply_template_chrome(button, k_unset_border_width);
        platform->native = (__bridge_retained void*)button; // the void* slot owns one reference
        return platform;
    }

    void radio_button_handler::on_connect_handler(radio_button_platform& platform)
    {
        UIButton* const button = as_button(platform.native);
        MauiIosRadioButtonProxy* const proxy = [[MauiIosRadioButtonProxy alloc] init];
        proxy.handler = this;
        // UIControl targets are weak, so the proxy is kept alive via an associated object.
        [button addTarget:proxy action:@selector(onSelect:) forControlEvents:UIControlEventTouchUpInside];
        objc_setAssociatedObject(button, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void radio_button_handler::on_disconnect_handler(radio_button_platform& platform)
    {
        detach_trampolines(platform);
    }

    void radio_button_handler::map_is_checked(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->is_checked = view.is_checked();
        UIButton* const button = as_button(platform->native);
        button.selected = static_cast<BOOL>(view.is_checked());
        // RadioButtonHandler.iOS MapIsChecked: AccessibilityValue mirrors the checked state.
        button.accessibilityValue = view.is_checked() ? @"1" : @"0";
    }

    void radio_button_handler::map_content(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string content(view.content_as_string());
        platform->content = content;
        NSString* const raw = [NSString stringWithUTF8String:content.c_str()];
        UIButton* const button = as_button(platform->native);
        [button setTitle:(raw != nil ? raw : @"") forState:UIControlStateNormal];
        // Long titles WRAP rather than truncate (MAUI wraps the View-fallback RadioButton's text to
        // multiple lines); the default UIButton titleLabel is single-line + tail-truncated. The matching
        // multi-line height is reported by get_desired_size so the control grows to fit.
        button.titleLabel.numberOfLines = 0;
        button.titleLabel.lineBreakMode = NSLineBreakByWordWrapping;
        refresh_radio_title_formatting(button, view);
    }

    void radio_button_handler::map_text_color(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // The ButtonExtensions.UpdateTextColor recipe applies TextColor to the TITLE only. The SF-symbol
        // radio indicator (the fallback's stand-in for the template's themed Ellipse) must NOT track the
        // content TextColor: MAUI keeps the ring at the default label color, so e.g. a red-text option still
        // shows a black ring. Keying off is_property_set so an explicit TextColor=Black is not misread as
        // unset. Explicit wins.
        UIButton* const button = as_button(platform->native);
        const auto* const bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
        UIColor* const color = color_is_set ? to_ui_color(view.text_color()) : UIColor.labelColor;
        [button setTitleColor:color forState:UIControlStateNormal];
        [button setTitleColor:color forState:UIControlStateHighlighted];
        [button setTitleColor:color forState:UIControlStateDisabled];
        // Ring indicator pinned to the dynamic system label color (NOT the content TextColor) — matches MAUI,
        // and still visible on both light and dark backgrounds.
        button.tintColor = UIColor.labelColor;
        refresh_radio_title_formatting(button, view);
    }

    void radio_button_handler::map_font(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // MAUI renders RadioButton via a ControlTemplate whose text is a Label; RadioButton's
            // FontSizeDefaultValueCreator() (RadioButton.cs:353) returns this.GetDefaultFontSize() ==
            // IFontManager.DefaultFontSize == UIFont.SystemFontSize (14pt on iOS), so an unset-FontSize
            // radio renders at SystemFontSize — NOT UIFont.ButtonFontSize (18pt). The port renders the
            // radio natively through the ButtonExtensions recipe (TitleLabel), so it must supply the same
            // SystemFontSize default the creator would; buttonFontSize (18pt) made the title ~1.29× too
            // large (18/14), matching the Button map_font fix. See ios_conversions.hpp's
            // default_text_font_size() note.
            as_button(platform->native).titleLabel.font =
                to_ui_font(view.font(), maui::platform::ios::default_text_font_size());
        }
    }

    void radio_button_handler::map_character_spacing(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            refresh_radio_title_formatting(as_button(platform->native), view);
        }
    }

    void radio_button_handler::map_stroke_color(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).layer.borderColor = to_ui_color(view.stroke_color()).CGColor;
        }
    }

    void radio_button_handler::map_stroke_thickness(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            UIButton* const button = as_button(platform->native);
            // The PAINT: clamp the -1d sentinel to 0 the way every C# UpdateStrokeThickness does (it keys
            // off `>= 0` before overriding the native default) — CALayer has no sentinel and a negative
            // borderWidth is undefined.
            const double stroke = view.stroke_thickness();
            button.layer.borderWidth = static_cast<CGFloat>(stroke > 0 ? stroke : 0.0);
            // The MEASURE: BorderWidth is an input to the template's own chrome, NOT just a paint. The
            // DefaultTemplate's Border insets its content by `Padding + StrokeThickness` (Border.cs:354),
            // so a bordered radio is BOTH taller and wider than an unset one — MAUI grows the row around
            // the stroke. The port paints the stroke on the layer, which is drawn INSIDE the view bounds
            // and contributes nothing on its own, so without this the fill was squeezed inward by the
            // stroke instead. Feed the RAW value (sentinel included): see template_pad.
            apply_template_chrome(button, stroke);
        }
    }

    void radio_button_handler::map_corner_radius(radio_button_handler& handler, i_radio_button& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            as_button(platform->native).layer.cornerRadius = static_cast<CGFloat>(view.corner_radius());
        }
    }

    maui::graphics::size radio_button_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        UIButton* const button = as_button(platform->native);
        const CGSize fitting = [button sizeThatFits:CGSizeMake(width, height)];
        maui::graphics::size result{fitting.width, fitting.height};
        // UIButton's sizeThatFits underestimates a WRAPPED multi-line title (the legacy setTitle path stays
        // single-line). When the width is bounded and the title is wider than the available text column,
        // recompute its wrapped height via boundingRect so the radio grows to fit instead of truncating —
        // the titleLabel is configured numberOfLines=0/WordWrap in map_content.
        NSString* const title = [button titleForState:UIControlStateNormal];
        if (std::isfinite(width_constraint) && title.length > 0)
        {
            UIFont* const font = button.titleLabel.font != nil ? button.titleLabel.font
                                                               : [UIFont systemFontOfSize:UIFont.buttonFontSize];
            NSDictionary* const attrs = @{NSFontAttributeName : font};
            const CGSize one_line = [title sizeWithAttributes:attrs];
            // `fitting` is the single-line button width; what it adds over the bare title is the chrome
            // (indicator + content/title insets). Subtract that to get the text column at this constraint.
            const CGFloat reserved = std::max<CGFloat>(0, fitting.width - one_line.width);
            const CGFloat text_width = std::max<CGFloat>(1, width - reserved);
            const CGRect wrapped = [title boundingRectWithSize:CGSizeMake(text_width, CGFLOAT_MAX)
                                                       options:NSStringDrawingUsesLineFragmentOrigin
                                                    attributes:attrs
                                                       context:nil];
            // Size the row from MAUI's DefaultTemplate geometry DIRECTLY: the outer Ellipse measures 21pt
            // (63px @3x) and the chrome around it is 2 * template_pad(BorderWidth) — 14pt for an unset
            // radio (the 35pt row the reference draws), 24pt at BorderWidth = 4 (a 45pt row). Thus
            // row = max(21pt ring, text) + 2 * template_pad. UIButton's own sizeThatFits height is NOT used: it adds its
            // internal title metrics and over-measures a large-font (e.g. 18pt) title by ~2pt, and that
            // per-large-radio excess accumulated into the vertical drift that reddened the content-heavy radio
            // pages (radio_content_properties / radio_button_group_gallery / radio_button_content). Prior tries
            // with chrome=16 (too tall) or ring-floor=16 (too short) both missed; this pair matches MAUI at
            // both 14pt (35) and 18pt (35.5). The ring still RENDERS at the SF-symbol's ~16.7pt (a separate
            // ring-size residual), but the row heights — hence the vertical alignment — now match.
#if TARGET_OS_MACCATALYST
            // Mac Catalyst renders the radio differently (a smaller ~15.5pt ring) and its pages are already
            // fine, so keep its measurement on UIButton's native height (max with the wrapped multi-line
            // height) — the iOS ring/chrome calibration below is tuned to the iOS shipped render only.
            const CGFloat vertical_chrome = button.contentEdgeInsets.top + button.contentEdgeInsets.bottom;
            result.height = std::max<double>(result.height, std::ceil(wrapped.size.height) + vertical_chrome);
#else
            constexpr CGFloat k_ring_pt = 21.0; // Ellipse HeightRequest, shipped render
            const CGFloat chrome = button.contentEdgeInsets.top + button.contentEdgeInsets.bottom;
            result.height = std::max<double>(k_ring_pt, std::ceil(wrapped.size.height)) + chrome;
#endif
        }
        // NOTE: BorderWidth is NOT added here. It is already in the row via template_pad, which
        // map_stroke_thickness folds into contentEdgeInsets — the single input both the width (through
        // sizeThatFits) and the height (through `chrome` above) are read from. It used to be a separate
        // `+= 2 * BorderWidth` on top of a fixed 7pt pad, which grew the row by 8pt at BorderWidth = 4
        // where the oracle grows it by 10, and never moved the ring's x at all.
        return result;
    }

    void radio_button_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_button(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }

    // Render transform pushed to the native UIView via the shared ios apply_transform helper
    // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
    void radio_button_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::ios::apply_transform(native, value);
    }

} // namespace maui::core
