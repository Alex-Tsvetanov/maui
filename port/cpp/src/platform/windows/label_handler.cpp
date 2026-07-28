// label_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.TextBlock, the same native
// type LabelHandler.Windows.cs creates. Ported from LabelHandler.Windows.cs + the TextBlock half of
// src/Core/src/Platform/Windows/TextBlockExtensions.cs (the oracle for every Update* below).
//
// Not ported yet (they need infrastructure this first Windows slice does not have): NeedsContainer /
// SetupContainer (VerticalTextAlignment and Background go through a WrapperView container on Windows —
// the port has no container seam on this backend yet, so vertical alignment is applied directly to the
// TextBlock), FormattedText (Inlines), and the HTML text path (LabelHtmlHelper).

#include "maui/core/label_handler.hpp"

#include <winrt/Microsoft.UI.Text.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.UI.Text.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/i_label.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/text_decorations.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using text_block = winui::Controls::TextBlock;

    // The void* slot boxes the BASE UIElement, not the TextBlock — see winui_interop.hpp's note on
    // native_view(): the layout panel has to host any child generically, and a projected derived type is
    // a distinct C++ class, so storing TextBlock would make the generic upcast a reinterpret_cast. The
    // QueryInterface below is what makes it a real (checked) downcast; it is trivial next to XAML layout.
    text_block as_text_block(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<text_block>();
    }

    // "Was this property explicitly set?" — see the twin in button_handler.cpp for why this must not be a
    // value comparison ([[cpp-unset-color-sentinel-collision]]).
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // TextAlignmentExtensions.ToPlatform(isLtr: true) — MAUI passes true because Windows has no
    // FlowDirection wired through here yet (its own TODO in TextBlockExtensions).
    winui::TextAlignment to_platform(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return winui::TextAlignment::Center;
            case maui::core::text_alignment::end:
                return winui::TextAlignment::Right;
            case maui::core::text_alignment::justify:
                return winui::TextAlignment::Justify;
            case maui::core::text_alignment::start:
            default:
                return winui::TextAlignment::Left;
        }
    }

    winui::VerticalAlignment to_platform_vertical(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return winui::VerticalAlignment::Center;
            case maui::core::text_alignment::end:
                return winui::VerticalAlignment::Bottom;
            case maui::core::text_alignment::start:
            default:
                return winui::VerticalAlignment::Top;
        }
    }

    // FontExtensions.ToFontWeight: the named MAUI weights map onto the Microsoft.UI.Text.FontWeights
    // constants, and anything else passes its numeric value straight through. The port's font_weight enum
    // already carries the OpenType numbers, so the whole switch collapses to the fallthrough case — with
    // one exception worth keeping: C# maps Heavy (800) to ExtraBold, which is also 800, so the numeric
    // path is faithful rather than merely convenient.
    winrt::Windows::UI::Text::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return winrt::Windows::UI::Text::FontWeight{static_cast<std::uint16_t>(weight)};
    }

    winrt::Windows::UI::Text::FontStyle to_font_style(maui::core::font_slant slant)
    {
        switch (slant)
        {
            case maui::core::font_slant::italic:
                return winrt::Windows::UI::Text::FontStyle::Italic;
            case maui::core::font_slant::oblique:
                return winrt::Windows::UI::Text::FontStyle::Oblique;
            case maui::core::font_slant::normal:
            default:
                return winrt::Windows::UI::Text::FontStyle::Normal;
        }
    }

    // LineBreakMode has no single WinUI property: it resolves into TextWrapping + TextTrimming, the same
    // split ScrollViewerExtensions/LabelHandler do on the other desktop backends.
    void apply_line_break_mode(const text_block& block, maui::core::line_break_mode mode, int max_lines)
    {
        using maui::core::line_break_mode;
        switch (mode)
        {
            case line_break_mode::no_wrap:
                block.TextWrapping(winui::TextWrapping::NoWrap);
                block.TextTrimming(winui::TextTrimming::None);
                break;
            case line_break_mode::character_wrap:
                block.TextWrapping(winui::TextWrapping::Wrap);
                block.TextTrimming(winui::TextTrimming::None);
                break;
            case line_break_mode::head_truncation:
            case line_break_mode::middle_truncation:
                // WinUI can only trim at the END (TextTrimming has no head/middle member), so both of these
                // degrade to a tail ellipsis. Documented deviation, not an oversight: the alternative would be
                // measuring and rewriting the string ourselves, which would diverge from MAUI's own render.
                block.TextWrapping(winui::TextWrapping::NoWrap);
                block.TextTrimming(winui::TextTrimming::CharacterEllipsis);
                break;
            case line_break_mode::tail_truncation:
                block.TextWrapping(winui::TextWrapping::NoWrap);
                block.TextTrimming(winui::TextTrimming::CharacterEllipsis);
                break;
            case line_break_mode::word_wrap:
            default:
                block.TextWrapping(winui::TextWrapping::Wrap);
                block.TextTrimming(winui::TextTrimming::None);
                break;
        }
        // MaxLines 0 is WinUI's "unbounded"; the port's unset sentinel is -1.
        block.MaxLines(max_lines > 0 ? max_lines : 0);
    }
} // namespace

namespace maui::core
{
    label_platform::~label_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<label_platform> label_handler::create_platform_view()
    {
        auto platform = std::make_unique<label_platform>();
        platform->native = maui::platform::windows::take<winui::UIElement>(text_block{});
        return platform;
    }

    void label_handler::map_text(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text = std::string(view.text());
        as_text_block(platform->native).Text(maui::platform::windows::to_hstring(platform->text));
    }

    void label_handler::map_text_color(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        // C#'s UpdateProperty(ForegroundProperty, color) CLEARS the local value when the color is null,
        // letting the theme brush show through. The port's equivalent of "null" is an unset property, so
        // an all-zero (default-constructed) color is left to the theme rather than painted transparent
        // black — see [[cpp-unset-color-sentinel-collision]] for why this must not test `!= color{}`.
        if (!is_set(view, "text_color"))
        {
            as_text_block(platform->native).ClearValue(winui::Controls::TextBlock::ForegroundProperty());
            return;
        }
        as_text_block(platform->native)
            .Foreground(winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform->text_color)});
    }

    void label_handler::map_font(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        const text_block block = as_text_block(platform->native);
        const font& f = platform->text_font;
        if (f.size() > 0)
        {
            block.FontSize(f.size());
        }
        if (!f.family().empty())
        {
            block.FontFamily(winui::Media::FontFamily{maui::platform::windows::to_hstring(f.family())});
        }
        block.FontStyle(to_font_style(f.slant()));
        block.FontWeight(to_font_weight(f.weight()));
        block.IsTextScaleFactorEnabled(f.auto_scaling_enabled());
        // LineHeight is expressed as a MULTIPLE of the font size (UpdateLineHeight multiplies by
        // FontSize), so it has to be re-applied whenever the size changes — not only from map_line_height.
        if (platform->line_height >= 0)
        {
            block.LineHeight(platform->line_height * block.FontSize());
        }
    }

    void label_handler::map_horizontal_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment();
        as_text_block(platform->native).TextAlignment(to_platform(platform->horizontal_alignment));
    }

    void label_handler::map_vertical_text_alignment(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->vertical_alignment = view.vertical_text_alignment();
        as_text_block(platform->native).VerticalAlignment(to_platform_vertical(platform->vertical_alignment));
    }

    void label_handler::map_character_spacing(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // CharacterSpacingExtensions.ToEm: pt * 0.0624 * 1000, in 1/1000 em units.
        const auto em = static_cast<std::int32_t>(std::lround(platform->character_spacing * 0.0624 * 1000.0));
        as_text_block(platform->native).CharacterSpacing(em);
    }

    void label_handler::map_text_decorations(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->decorations = view.text_decorations();
        using winrt::Windows::UI::Text::TextDecorations;
        // maui::core::text_decorations is a scoped enum with NO bitwise operators (it mirrors C#'s
        // [Flags] enum in VALUES only), so the flag test has to go through the underlying type.
        const auto bits = static_cast<std::uint8_t>(platform->decorations);
        auto flags = TextDecorations::None;
        if ((bits & static_cast<std::uint8_t>(maui::core::text_decorations::underline)) != 0)
        {
            flags |= TextDecorations::Underline;
        }
        if ((bits & static_cast<std::uint8_t>(maui::core::text_decorations::strikethrough)) != 0)
        {
            flags |= TextDecorations::Strikethrough;
        }
        as_text_block(platform->native).TextDecorations(flags);
    }

    void label_handler::map_formatted_text(label_handler& handler, i_label& view)
    {
        // Mirror-only for now: the WinUI path is TextBlock.Inlines (a Run per span), which needs the
        // per-span font/color resolution the Inlines API expects. Recorded so the mirror stays truthful
        // and the plain-text path above keeps working.
        if (auto* platform = handler.typed_platform_view())
        {
            platform->formatted_text_runs = view.formatted_text_runs();
        }
    }

    void label_handler::map_line_height(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->line_height = view.line_height();
        // C# only ASSIGNS when >= 0; a negative (unset) LineHeight leaves the TextBlock's own default.
        if (platform->line_height >= 0)
        {
            const text_block block = as_text_block(platform->native);
            block.LineHeight(platform->line_height * block.FontSize());
        }
    }

    void label_handler::map_padding(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->padding = view.padding();
        // "Label padding values do not support negative values" — TextBlockExtensions clamps each edge.
        const maui::core::thickness& p = platform->padding;
        as_text_block(platform->native)
            .Padding(winui::Thickness{std::max(0.0, p.left), std::max(0.0, p.top), std::max(0.0, p.right),
                                     std::max(0.0, p.bottom)});
    }

    void label_handler::map_line_break_mode(label_handler& handler, i_label& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->line_break_mode_value = view.line_break_mode();
        platform->max_lines = view.max_lines();
        apply_line_break_mode(as_text_block(platform->native), platform->line_break_mode_value, platform->max_lines);
    }

    void label_handler::map_max_lines(label_handler& handler, i_label& view)
    {
        // Same pair, same push: LineBreakMode and MaxLines resolve TOGETHER into TextWrapping/TextTrimming
        // /MaxLines, so either key re-applies both (mirroring the headless partial's contract).
        map_line_break_mode(handler, view);
    }

    maui::graphics::size label_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // The REAL native measure, which is the whole point of having a native backend: WinUI lays the
        // text out with the actual font and returns DesiredSize. Infinite constraints pass straight
        // through — UIElement.Measure accepts infinity and treats it as unconstrained, exactly like the
        // cross-platform measure contract.
        const text_block block = as_text_block(platform->native);
        block.Measure(winrt::Windows::Foundation::Size{static_cast<float>(width_constraint),
                                                       static_cast<float>(height_constraint)});
        const auto desired = block.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void label_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // A WinUI child is positioned by its PARENT panel; the port's panel is a Canvas, so the position
        // is Canvas.Left/Top and the size is Width/Height. See layout_handler.cpp's arrange_child, which
        // this delegates to via the same convention (frame is parent-relative, like a UIView frame).
        const text_block block = as_text_block(platform->native);
        winui::Controls::Canvas::SetLeft(block, frame.x);
        winui::Controls::Canvas::SetTop(block, frame.y);
        block.Width(frame.width);
        block.Height(frame.height);
    }
} // namespace maui::core
