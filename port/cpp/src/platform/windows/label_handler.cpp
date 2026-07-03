// label_handler — Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.TextBlock.
// The windows twin of src/platform/apple/label_handler.mm (NSTextField) and the real-native sibling of
// the headless mirror partial (src/platform/headless/label_handler.cpp).
//
// Ported DIRECTLY from LabelHandler.Windows.cs + Platform/Windows/{TextBlockExtensions.cs,
// ViewExtensions.cs, AlignmentExtensions.cs, FontExtensions.cs, CharacterSpacingExtensions.cs} +
// Fonts/FontManager.Windows.cs + the Controls-layer
// src/Controls/src/Core/Platform/Windows/Extensions/TextBlockExtensions.cs (SetLineBreakMode).
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - FontManager's registrar/embedded-font lookups are skipped (the port has no font registrar on any
//     backend): a named family goes straight to a FontFamily(name); the default size is the constant 14
//     (the ControlContentThemeFontSize theme value FontManager.DefaultFontSize resolves — there is no
//     Application.Current on the XAML-less test host to read the resource from).
//   - map_vertical_text_alignment pushes TextBlock.VerticalAlignment faithfully
//     (AlignmentExtensions.ToPlatformVerticalAlignment), but WITHOUT C#'s WrapperView container (whose
//     SetupContainer sets the child Height to Auto inside a fixed-height container) the property has no
//     visual effect under the port's explicit-frame Canvas model — deferred with the container infra.
//   - map_formatted_text builds real Documents.Run inlines (text/color/font/kerning/decorations per
//     run); the per-run background_color and line_height stay mirror-only (a TextElement carries
//     neither — C# realizes them through TextHighlighters / paragraph styling; deferred).
//   - The Html TextType branch of UpdateText and TextHighlighters are out of scope (no TextType in the
//     port's i_label).
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained —
// so that suite observes exactly the headless partial's behavior (including its placeholder measure
// metric), and the real app additionally drives the real TextBlock.

#include "maui/core/label_handler.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Documents.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Inlines IVector consume methods
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxd = winrt::Microsoft::UI::Xaml::Documents;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wut = winrt::Windows::UI::Text;
    namespace wnative = maui::platform::win;

    // FontManager.Windows.DefaultFontSize — the ControlContentThemeFontSize theme resource (14.0 in the
    // WinUI generic theme). Read as a constant here: the resource needs Application.Current, which the
    // XAML-less test host does not have (header deviation).
    constexpr double k_default_font_size = 14.0;

    // The inner TextBlock every text map targets — `native` is the Border CONTAINER (WrapperView
    // stand-in) that carries the view background, so the text lives in `text_block`.
    [[nodiscard]] muxc::TextBlock text_block_of(const maui::core::label_platform& platform)
    {
        return wnative::borrow<muxc::TextBlock>(platform.text_block);
    }

    [[nodiscard]] muxc::Border border_of(const maui::core::label_platform& platform)
    {
        return wnative::borrow<muxc::Border>(platform.native);
    }

    // FontExtensions.ToFontStyle: Slant → FontStyle (Italic / Oblique / Normal).
    [[nodiscard]] wut::FontStyle to_font_style(maui::core::font_slant slant)
    {
        switch (slant)
        {
            case maui::core::font_slant::italic:
                return wut::FontStyle::Italic;
            case maui::core::font_slant::oblique:
                return wut::FontStyle::Oblique;
            case maui::core::font_slant::normal:
            default:
                return wut::FontStyle::Normal;
        }
    }

    // FontExtensions.ToFontWeight: every named C# weight maps to its numeric OpenType value, and the
    // fallback constructs FontWeight((ushort)weight) — the port's font_weight enum IS those numeric
    // values (100…900), so the whole switch collapses to the numeric constructor.
    [[nodiscard]] wut::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return wut::FontWeight{static_cast<std::uint16_t>(weight)};
    }

    // TextBlockExtensions.UpdateFont → FontManager.GetFontSize: size <= 0 / NaN → DefaultFontSize.
    [[nodiscard]] double resolved_font_size(const maui::core::font& value)
    {
        const double size = value.size();
        return (size > 0 && !std::isnan(size)) ? size : k_default_font_size;
    }

    // AlignmentExtensions.ToPlatform(alignment, isLtr: true) — the C# body carries the same
    // "no FlowDirection yet" TODO, so the LTR branch is the faithful port.
    [[nodiscard]] mux::TextAlignment to_text_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return mux::TextAlignment::Center;
            case maui::core::text_alignment::end:
                return mux::TextAlignment::Right;
            case maui::core::text_alignment::justify:
                return mux::TextAlignment::Justify;
            case maui::core::text_alignment::start:
            default:
                return mux::TextAlignment::Left;
        }
    }

    // AlignmentExtensions.ToPlatformVerticalAlignment: Center → Center, End → Bottom, else Top.
    [[nodiscard]] mux::VerticalAlignment to_vertical_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return mux::VerticalAlignment::Center;
            case maui::core::text_alignment::end:
                return mux::VerticalAlignment::Bottom;
            default:
                return mux::VerticalAlignment::Top;
        }
    }

    // TextBlockExtensions.UpdateTextDecorations' set/clear pair, collapsed: the two bits are rebuilt
    // whole from the view flags (same net effect as C#'s &=~ / |= dance).
    [[nodiscard]] wut::TextDecorations to_text_decorations(maui::core::text_decorations value)
    {
        auto result = wut::TextDecorations::None;
        if ((static_cast<unsigned>(value) & static_cast<unsigned>(maui::core::text_decorations::underline)) != 0U)
        {
            result |= wut::TextDecorations::Underline;
        }
        if ((static_cast<unsigned>(value) & static_cast<unsigned>(maui::core::text_decorations::strikethrough)) != 0U)
        {
            result |= wut::TextDecorations::Strikethrough;
        }
        return result;
    }

    // Controls TextBlockExtensions.SetLineBreakMode — MaxLines (>= 0, else 0 = unlimited) + the
    // LineBreakMode → TextTrimming/TextWrapping switch. DetermineTruncatedTextWrapping: a truncated
    // mode wraps only when MaxLines > 1.
    void apply_line_break_mode(const muxc::TextBlock& block, maui::core::line_break_mode mode, int max_lines)
    {
        block.MaxLines(max_lines >= 0 ? max_lines : 0);
        const auto truncated_wrapping = [&block] {
            return block.MaxLines() > 1 ? mux::TextWrapping::Wrap : mux::TextWrapping::NoWrap;
        };
        switch (mode)
        {
            case maui::core::line_break_mode::no_wrap:
                block.TextTrimming(mux::TextTrimming::Clip);
                block.TextWrapping(mux::TextWrapping::NoWrap);
                break;
            case maui::core::line_break_mode::word_wrap:
                block.TextTrimming(mux::TextTrimming::None);
                block.TextWrapping(mux::TextWrapping::Wrap);
                break;
            case maui::core::line_break_mode::character_wrap:
                block.TextTrimming(mux::TextTrimming::WordEllipsis);
                block.TextWrapping(mux::TextWrapping::Wrap);
                break;
            case maui::core::line_break_mode::head_truncation:
                // C# TODO: WinUI truncates at the end (no head ellipsis) — the same approximation.
                block.TextTrimming(mux::TextTrimming::WordEllipsis);
                block.TextWrapping(truncated_wrapping());
                break;
            case maui::core::line_break_mode::tail_truncation:
                block.TextTrimming(mux::TextTrimming::CharacterEllipsis);
                block.TextWrapping(truncated_wrapping());
                break;
            case maui::core::line_break_mode::middle_truncation:
                // C# TODO: WinUI truncates at the end (no middle ellipsis) — the same approximation.
                block.TextTrimming(mux::TextTrimming::WordEllipsis);
                block.TextWrapping(truncated_wrapping());
                break;
        }
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the TextBlock (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSTextField here).
    label_platform::~label_platform()
    {
        wnative::release(text_block); // the inner TextBlock's own ref (the Border also holds one via Child)
        wnative::release(native);     // the Border container
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST — the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) — then pushes to the real TextBlock when one exists.

    void label_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void label_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void label_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId (on the Border host).
        wnative::apply_automation_id(native, value);
    }

    void label_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value); // keep the mirror live for the XAML-less suite
        auto border = wnative::borrow<winrt::Microsoft::UI::Xaml::Controls::Border>(native);
        if (border == nullptr)
        {
            return;
        }
        // LabelHandler.Windows.MapBackground → the WrapperView/ContainerView carries the brush; the
        // port's Border container is that stand-in. A null paint clears back to transparent; solid and
        // gradient paints ride the shared Paint.ToPlatform bridge.
        if (value == nullptr)
        {
            border.ClearValue(winrt::Microsoft::UI::Xaml::Controls::Border::BackgroundProperty());
            return;
        }
        border.Background(wnative::to_paint_brush(value));
    }

    std::unique_ptr<label_platform> label_handler::create_platform_view()
    {
        auto platform = std::make_unique<label_platform>();
        try
        {
            // LabelHandler.CreatePlatformView: new TextBlock() — WRAPPED in a Border container (the port's
            // WrapperView stand-in) so a Label BackgroundColor has a surface to paint on (a TextBlock has
            // no Background) and VerticalTextAlignment has a fixed-height slot to align within. The child
            // stretches to fill the Border, so horizontal/vertical text alignment position the text
            // inside the arranged frame; the Border is transparent until update_background paints it.
            const muxc::TextBlock block;
            const muxc::Border border;
            border.Child(block);
            block.HorizontalAlignment(mux::HorizontalAlignment::Stretch);
            block.VerticalAlignment(mux::VerticalAlignment::Top);
            platform->text_block = wnative::store(block); // the inner TextBlock (freed in the dtor)
            platform->native = wnative::store(border);    // the Border container (freed in the dtor)
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
            platform->text_block = nullptr;
        }
        return platform;
    }

    void label_handler::map_text(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text = std::string(view.text());
        // TextBlockExtensions.UpdateText → UpdateTextPlainText: platformControl.Text = label.Text
        // (setting Text also clears any previous Inlines — the FormattedText ↔ Text exclusivity).
        if (auto block = text_block_of(*platform))
        {
            block.Text(wnative::to_hstring_utf8(view.text()));
        }
    }

    void label_handler::map_text_color(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        auto block = text_block_of(*platform);
        if (block == nullptr)
        {
            return;
        }
        // TextBlockExtensions.UpdateTextColor → UpdateProperty(ForegroundProperty, text.TextColor): a
        // null color CLEARS the value (back to the theme foreground). The port's color is a
        // non-nullable value type whose default equals opaque black, so the C# `null` branch is
        // discriminated on whether the property was explicitly SET (BindableObject.IsSet) — the same
        // stand-in the android twin documents.
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
        if (color_is_set)
        {
            block.Foreground(wnative::to_brush(view.text_color()));
        }
        else
        {
            block.ClearValue(muxc::TextBlock::ForegroundProperty());
        }
    }

    void label_handler::map_font(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        auto block = text_block_of(*platform);
        if (block == nullptr)
        {
            return;
        }
        // TextBlockExtensions.UpdateFont: FontSize + FontFamily + FontStyle + FontWeight +
        // IsTextScaleFactorEnabled (FontManager.GetFontSize/GetFontFamily — registrar skipped, header).
        const font value = view.font();
        block.FontSize(resolved_font_size(value));
        if (!value.family().empty())
        {
            block.FontFamily(muxm::FontFamily{wnative::to_hstring_utf8(value.family())});
        }
        else
        {
            block.ClearValue(muxc::TextBlock::FontFamilyProperty()); // C# null Family → the default family
        }
        block.FontStyle(to_font_style(value.slant()));
        block.FontWeight(to_font_weight(value.weight()));
        block.IsTextScaleFactorEnabled(value.auto_scaling_enabled());
    }

    void label_handler::map_horizontal_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment();
        // TextBlockExtensions.UpdateHorizontalTextAlignment: TextAlignment = alignment.ToPlatform(true)
        // (the C# body's FlowDirection TODO applies equally here — LTR assumed).
        if (auto block = text_block_of(*platform))
        {
            block.TextAlignment(to_text_alignment(view.horizontal_text_alignment()));
        }
    }

    void label_handler::map_vertical_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->vertical_alignment = view.vertical_text_alignment();
        // TextBlockExtensions.UpdateVerticalTextAlignment: VerticalAlignment =
        // alignment.ToPlatformVerticalAlignment(). deferred: without C#'s WrapperView container (child
        // Height Auto inside the fixed container) the property has no visual effect under the port's
        // explicit-frame Canvas model — see the header deviations.
        if (auto block = text_block_of(*platform))
        {
            block.VerticalAlignment(to_vertical_alignment(view.vertical_text_alignment()));
        }
    }

    void label_handler::map_character_spacing(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // LabelHandler.Windows.MapCharacterSpacing: skip the default-valued push while connecting
        // (IsConnectingHandler() && CharacterSpacing == 0) — the freshly-created TextBlock already has
        // spacing 0.
        if (handler.is_connecting() && view.character_spacing() == 0)
        {
            return;
        }
        // TextBlockExtensions.UpdateCharacterSpacing: CharacterSpacing = value.ToEm().
        if (auto block = text_block_of(*platform))
        {
            block.CharacterSpacing(wnative::to_em(view.character_spacing()));
        }
    }

    void label_handler::map_text_decorations(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->decorations = view.text_decorations();
        // TextBlockExtensions.UpdateTextDecorations: the Underline/Strikethrough bits.
        if (auto block = text_block_of(*platform))
        {
            block.TextDecorations(to_text_decorations(view.text_decorations()));
        }
    }

    void label_handler::map_formatted_text(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Mirror the resolved runs (the headless twin's contract — the XAML-less suite observes them).
        platform->formatted_text_runs = view.formatted_text_runs();
        auto block = text_block_of(*platform);
        if (block == nullptr)
        {
            return;
        }
        const auto& runs = view.formatted_text_runs();
        if (runs.empty())
        {
            // Empty runs revert to the plain text path (setting Text clears the Inlines) — the
            // FormattedText / Text exclusivity the label enforces.
            block.Text(wnative::to_hstring_utf8(view.text()));
            return;
        }
        // Controls TextBlockExtensions.UpdateInlines: one Documents.Run per resolved span, carrying the
        // effective text/font/color/kerning/decorations. The per-run background_color and line_height
        // stay mirror-only (deferred — a TextElement carries neither; see the header deviations).
        auto inlines = block.Inlines();
        inlines.Clear();
        for (const auto& run : runs)
        {
            muxd::Run native_run;
            native_run.Text(wnative::to_hstring_utf8(run.text));
            native_run.FontSize(resolved_font_size(run.run_font));
            if (!run.run_font.family().empty())
            {
                native_run.FontFamily(muxm::FontFamily{wnative::to_hstring_utf8(run.run_font.family())});
            }
            native_run.FontStyle(to_font_style(run.run_font.slant()));
            native_run.FontWeight(to_font_weight(run.run_font.weight()));
            if (run.text_color.has_value())
            {
                native_run.Foreground(wnative::to_brush(*run.text_color));
            }
            native_run.CharacterSpacing(wnative::to_em(run.character_spacing));
            native_run.TextDecorations(to_text_decorations(run.decorations));
            inlines.Append(native_run);
        }
    }

    void label_handler::map_line_height(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->line_height = view.line_height();
        // TextBlockExtensions.UpdateLineHeight: only a non-negative multiple is pushed — LineHeight =
        // label.LineHeight * FontSize (C# reads the TextBlock's current FontSize; -1 leaves it unset).
        if (view.line_height() >= 0)
        {
            if (auto block = text_block_of(*platform))
            {
                block.LineHeight(view.line_height() * block.FontSize());
            }
        }
    }

    void label_handler::map_padding(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->padding = view.padding();
        // TextBlockExtensions.UpdatePadding: negative components are replaced with zero.
        if (auto block = text_block_of(*platform))
        {
            const thickness pad = view.padding();
            block.Padding(wnative::to_thickness(thickness{std::max(0.0, pad.left), std::max(0.0, pad.top),
                                                          std::max(0.0, pad.right), std::max(0.0, pad.bottom)}));
        }
    }

    // LabelHandler.MapLineBreakMode / MapMaxLines — BOTH route through the Controls-layer
    // SetLineBreakMode (the MaxLines + TextTrimming/TextWrapping resolution), so both map fns delegate
    // to the same platform refresh; both mirror both fields (the headless twin's contract).
    void label_handler::map_line_break_mode(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->line_break_mode_value = view.line_break_mode();
        platform->max_lines = view.max_lines();
        if (auto block = text_block_of(*platform))
        {
            apply_line_break_mode(block, view.line_break_mode(), view.max_lines());
        }
    }

    void label_handler::map_max_lines(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->line_break_mode_value = view.line_break_mode();
        platform->max_lines = view.max_lines();
        if (auto block = text_block_of(*platform))
        {
            apply_line_break_mode(block, view.line_break_mode(), view.max_lines());
        }
    }

    maui::graphics::size label_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's placeholder metric (~7pt/char, 16pt line) so
            // the backend-agnostic size-request suites see consistent numbers. The FULL headless body
            // (keep in sync with src/platform/headless/label_handler.cpp get_desired_size), not just the
            // single-line product: the cross-platform suite also asserts its padding-inflation
            // (MauiLabel.SizeThatFits) and explicit-Width wrap (PreferredMaxLayoutWidth) branches
            // against this fallback (label_seam.padding_inflates_desired_size /
            // .explicit_width_wraps_to_multiple_lines).
            constexpr double per_char = 7.0;
            constexpr double line = 16.0;
            const thickness& pad = platform->padding;
            const double text_width = static_cast<double>(platform->text.size()) * per_char;
            const double fallback_virtual_width =
                virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
            double wrap_width =
                dimension::is_explicit_set(fallback_virtual_width) ? fallback_virtual_width : width_constraint;
            if (dimension::is_explicit_set(fallback_virtual_width) && std::isfinite(width_constraint) &&
                width_constraint < wrap_width)
            {
                wrap_width = width_constraint;
            }
            double content_width = text_width;
            double lines = 1.0;
            if (std::isfinite(wrap_width) && wrap_width > 0)
            {
                const double available = wrap_width - pad.left - pad.right;
                if (available > 0 && text_width > available)
                {
                    lines = std::ceil(text_width / available);
                    content_width = available;
                }
            }
            return {content_width + pad.left + pad.right, (lines * line) + pad.top + pad.bottom};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (the value C#'s MapWidth/MapHeight would have pushed — see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void label_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the TextBlock to
        // the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
