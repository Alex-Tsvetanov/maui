#pragma once
// maui::core::label_handler  <=  Microsoft.Maui.Handlers.LabelHandler
//
// The handler for a text label — the second concrete control, proving the handler recipe generalizes.
// Display-only: no inbound event channel (unlike button), so the mapper is a single table keyed on
// i_label (which already exposes the text/appearance/alignment via its bases — no chaining needed).
// Ported from LabelHandler.cs (+ LabelHandler.iOS.cs, translated to AppKit's NSTextField).
//
// Same partial-class split + single cross-platform label_platform struct as button_handler.

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/label_run.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto
    // it (headless keeps the base mirrors; Apple overrides update_* to push to the NSTextField).
    struct label_platform : view_platform_base
    {
        label_platform() = default;
        ~label_platform() override; // backend-defined: releases the retained native label on Apple
        label_platform(const label_platform&) = delete;
        label_platform(label_platform&&) = delete;
        label_platform& operator=(const label_platform&) = delete;
        label_platform& operator=(label_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of the mapped properties (the Apple build writes to `native` instead). The
        // generic IView mirrors (hidden/alpha/enabled/automation_id) come from view_platform_base.
        std::string text;
        maui::graphics::color text_color;
        font text_font;
        text_alignment horizontal_alignment = text_alignment::start;
        text_alignment vertical_alignment = text_alignment::start; // C# Label default Start
        double character_spacing = 0;
        // Headless mirror of LineHeight (Label.LineHeight default -1 = "unset", no paragraph-style
        // multiple) and Padding (ILabel.Padding → the native TextInsets / cell inset). The Apple/iOS
        // builds push these onto the NSTextField cell / MauiLabel TextInsets + paragraph style instead.
        double line_height = -1;
        maui::core::thickness padding;
        maui::core::text_decorations decorations = maui::core::text_decorations::none;
        // Headless mirror of Label.LineBreakMode (default WordWrap) + Label.MaxLines (default -1 = unset).
        // The Apple/iOS builds push these onto the UILabel.lineBreakMode + numberOfLines / NSTextFieldCell
        // (SetLineBreakMode resolves the two into the platform wrap mode + line count); headless just keeps
        // the raw view values so the cross-platform mapper contract is observable.
        maui::core::line_break_mode line_break_mode_value = maui::core::line_break_mode::word_wrap;
        int max_lines = -1;
        // Headless mirror of the resolved attributed runs (Label.FormattedText). Empty = plain text path.
        // The Apple/iOS builds turn these into an NSAttributedString instead; the headless build keeps the
        // run list so tests can assert per-span attributes flowed to the platform mirror.
        std::vector<maui::core::label_run> formatted_text_runs;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSTextField (defined in
        // src/platform/apple/label_handler.mm). Omitted on headless, which keeps the base mirrors; the
        // class layout is identical and a build only ever sees one backend, so there is no ODR mismatch.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        // Accessibility metadata + the input-transparent flag pushed to the NSTextField (M5d native a11y /
        // hit-test): semantics → accessibilityLabel/Help/heading role, input_transparent → -hitTest: gate.
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend (M6 fan-out): push the four fundamental IView properties to the UILabel (defined
        // in src/platform/ios/label_handler.mm). transform IS pushed via the shared ios apply_transform
        // helper (the generic-IView ViewMapper widening). The remaining generic-IView pushes —
        // flow_direction / semantics / input_transparent — keep the view_platform_base mirrors
        // until the shared ios view/visual/semantics op helpers land (the coordinator's retrofit; see
        // port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // MapBackground (LabelHandler is a VisualElement → a Label CAN carry a Background, e.g. the
        // AbsoluteLayout "AutoSized" demo paints white-on-blue): push the solid/gradient/image paint onto
        // the UILabel's backing layer via apply_background. Without it the paint fell through to the no-op
        // view_platform_base mirror and never rendered (a transparent label, invisible on a light page).
        void update_background(const maui::graphics::paint* value) override;
        // MapShadow → ShadowExtensions.SetShadow on the UILabel's layer (the shared apply_shadow). A
        // Label IS a VisualElement and CAN carry a Shadow (e.g. the shadow_playground "Label with a
        // Shadow" target casts a red blur behind its glyphs); without this push the shadow fell through
        // to the no-op view_platform_base mirror and never rendered, while the BoxView (shape_view) twin
        // showed it — the asymmetry the parity sweep caught.
        void update_shadow(const maui::core::i_shadow* value) override;
        // Clip IS pushed: WrapperView.SetClip masks the MauiIosLabel (UILabel)'s layer (the shared
        // apply_and_store_clip; MauiIosLabel.layoutSubviews re-frames the mask to the live bounds, the
        // 0×0-at-map-time fix).
        void update_clip(const maui::graphics::i_shape* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (M-android fan-out): push the generic IView properties to the real
        // android.widget.TextView over JNI (defined in src/platform/android/label_handler.cpp). Each
        // override calls the view_platform_base body FIRST — the android preset also runs the pure-native
        // cross-platform suite on the emulator WITHOUT a Java VM, and that suite observes the headless
        // mirrors — then pushes to the widget when one exists. Background pushes a solid fill; transform /
        // flow-direction / semantics route through the shared android ops (W4-34e). Shadow is pushed
        // NATIVELY (setElevation + a colored spot/ambient outline shadow, android_visual_ops apply_shadow —
        // the API-28 colored-elevation expression of IView.Shadow); InputTransparent keeps ONLY the base
        // mirror (no plain-View analog), as the button partial documents.
        //
        // Clip IS pushed for the label (unlike button): a CONVEX outline clip via the shared
        // apply_outline_clip (android_clip_ops.hpp) — setOutlineProvider + setClipToOutline(true). A Label
        // CAN carry a Clip (the chat_example bubble stages a RoundRectangle{CornerRadius=12} on the cell
        // Label to round its background fill, the single-root reduction of the oracle's rounded Border), and
        // a round-rect / ellipse / rectangle is convex so the framework clips it exactly. Without this push
        // the bubble's staged background painted a SQUARE fill (no rounding) — the Android chat_example RED.
        // The geometry is bounds-dependent (resolved against the view's live size), so update_clip is a
        // best-effort push at map time (0×0 before the first layout) and platform_arrange re-installs it once
        // the label has its final bounds — the image_handler / iOS reapply_clip pattern.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_semantics(const maui::core::semantics* value) override;
        // NOTE: no captured-default-text-color field. MAUI's TextViewExtensions.UpdateTextColor is a no-op
        // when Label.TextColor is null, so map_text_color leaves the freshly-created TextView's theme
        // ColorStateList (textColorPrimary, ~0xDE000000 gray — which also encodes the disabled dimming)
        // untouched on the unset branch. Re-applying a captured getCurrentTextColor() int would flatten that
        // ColorStateList and break the disabled appearance (layout_is_enabled), so it is deliberately gone.
#endif

#ifdef MAUI_PLATFORM_WINDOWS
        // Windows (WinUI 3) backend: push the fundamental IView properties to the real
        // Microsoft.UI.Xaml.Controls.TextBlock (defined in src/platform/windows/label_handler.cpp). Each
        // override calls the view_platform_base body FIRST — the windows preset also runs the
        // cross-platform suite on the host WITHOUT a XAML runtime (create_platform_view degrades to a
        // null native there), and that suite observes the headless mirrors — then pushes to the control
        // when one exists (the android partial's dual-drive pattern). is_enabled is intentionally NOT
        // overridden: a TextBlock is not a Control, and C#'s ViewExtensions.UpdateIsEnabled
        // (`platformView as Control`) no-ops on it, so the base mirror is the faithful behavior.
        // background keeps the base mirror too: a TextBlock has no Background property — C# paints a
        // label background through the WrapperView container (LabelHandler.Windows.MapBackground →
        // ContainerView), which this first cut defers alongside the container infra. transform /
        // flow_direction / semantics / shadow / clip / input_transparent also keep the base mirrors
        // (deferred — see the partial's header).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
#endif
    };

    class label_handler : public view_handler<label_handler, i_label, label_platform>
    {
    public:
        label_handler();

        static property_mapper<i_label, label_handler>& mapper();
        static command_mapper<i_label, label_handler>& command_mapper();

        static std::unique_ptr<label_platform> create_platform_view();

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe).
        static void map_text(label_handler& handler, i_label& view);
        static void map_text_color(label_handler& handler, i_label& view);
        static void map_font(label_handler& handler, i_label& view);
        static void map_horizontal_text_alignment(label_handler& handler, i_label& view);
        static void map_vertical_text_alignment(label_handler& handler, i_label& view);
        static void map_character_spacing(label_handler& handler, i_label& view);
        // LabelHandler.MapTextDecorations → LabelExtensions.UpdateTextDecorations (underline /
        // strikethrough on the attributed text; headless mirrors the flags).
        static void map_text_decorations(label_handler& handler, i_label& view);
        // LabelHandler.MapLineHeight → LabelExtensions.UpdateLineHeight (a paragraph-style
        // lineHeightMultiple over the attributed string; -1 leaves it unset). Headless mirrors the value.
        static void map_line_height(label_handler& handler, i_label& view);
        // LabelHandler.MapPadding → LabelExtensions.UpdatePadding (MauiLabel.TextInsets on iOS / the cell
        // title-rect inset on AppKit, with an RTL flip). Headless mirrors the thickness.
        static void map_padding(label_handler& handler, i_label& view);
        // The port's gap-closure G1 map: Label.FormattedText → LabelExtensions.UpdateText's FormattedText
        // branch. Non-empty runs build the native attributed string (NSAttributedString on apple/ios; the
        // headless run mirror); empty runs revert to the plain text path (clearing the attributed string).
        static void map_formatted_text(label_handler& handler, i_label& view);
        // LabelHandler.MapLineBreakMode / MapMaxLines (Label.iOS.cs) — BOTH call UILabel.SetLineBreakMode
        // (the wrap-mode + numberOfLines resolution from TextExtensions.SetLineBreakMode), so both port map
        // fns delegate to the same platform refresh. Headless mirrors the raw view values.
        static void map_line_break_mode(label_handler& handler, i_label& view);
        static void map_max_lines(label_handler& handler, i_label& view);
    };
} // namespace maui::core
