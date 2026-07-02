// swipe_item_menu_item_handler — Windows (WinUI 3) platform recipe: a REAL
// Microsoft.UI.Xaml.Controls.Button twin of the iOS UIButton / apple NSButton recipe. C#'s Windows
// SwipeView is a SwipeControl whose SwipeItems are DATA objects the control renders internally (there is
// no per-item platform view on Windows in MAUI) — so this partial follows the iOS
// SwipeItemMenuItemHandler recipe instead (the port's swipe machine activates the items through the
// contract, and this handler renders their button visual so a revealed item COULD render once the
// SwipeView's reveal lands): a non-hit-testable Button (the SwipeView's own pan drives activation, the
// UserInteractionEnabled=false analog) whose title / title colour / font / background the mapper pushes.
// The headless mirror (title / colours / font / source / visibility) is ALWAYS maintained beside the
// native pushes so the XAML-less cross-platform suite observes exactly the headless partial's behavior.
//
// DOCUMENTED DEVIATIONS (infrastructure gaps of this first cut, not behavior guesses):
//   - MapSource (the icon load + resize + the frame-observer re-resize) is deferred with the image
//     fan-out on this backend — the has_source mirror records the set, like the apple twin's stub.
//   - The C# SwipeItemButton frame observer (re-running MapSource when the button's frame changes) has
//     nothing to re-size without the icon, so connect()/disconnect() are no-ops (a SizeChanged token can
//     wire it when the icon lands).
//   - RestorationIdentifier = Text maps to AutomationProperties.AutomationId (the WinUI identity slot).
//
// XAML-less degradation: create_platform_view catches the construction failure and keeps native null;
// every apply_* still records the headless mirror.

#include "maui/core/swipe_item_menu_item_handler.hpp"

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // ButtonBase/ContentControl base-class consume methods
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include "maui/core/font.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_swipe_item_menu_item.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wut = winrt::Windows::UI::Text;
    namespace wnative = maui::platform::win;

    // FontManager.Windows.DefaultFontSize — the ControlContentThemeFontSize theme resource (14.0); read
    // as a constant here (no Application.Current on the XAML-less test host). Same constant as the
    // button/label partials.
    constexpr double k_default_font_size = 14.0;

    [[nodiscard]] muxc::Button button_of(const maui::core::swipe_item_menu_item_platform& platform)
    {
        return wnative::borrow<muxc::Button>(platform.native);
    }

    // FontExtensions.ToFontStyle: Slant → FontStyle (Italic / Oblique / Normal) — the button partial's
    // helper, kept local so the partial stays independently buildable.
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

    // FontExtensions.ToFontWeight — the port's font_weight enum values ARE the numeric OpenType weights.
    [[nodiscard]] wut::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return wut::FontWeight{static_cast<std::uint16_t>(weight)};
    }

    // The maui color from a packed ARGB uint (the mirror's storage) → a WinUI brush.
    [[nodiscard]] muxm::SolidColorBrush to_brush_argb(std::uint32_t argb)
    {
        return muxm::SolidColorBrush{winrt::Windows::UI::Color{
            .A = static_cast<uint8_t>((argb >> 24U) & 0xFFU),
            .R = static_cast<uint8_t>((argb >> 16U) & 0xFFU),
            .G = static_cast<uint8_t>((argb >> 8U) & 0xFFU),
            .B = static_cast<uint8_t>(argb & 0xFFU),
        }};
    }
} // namespace

namespace maui::core
{
    // Releases the one strong ref pinning the Button (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSButton + detaches the frame observer here — no observer
    // was wired on this backend, see the header deviations).
    swipe_item_menu_item_platform::~swipe_item_menu_item_platform()
    {
        wnative::release(native);
    }

    std::unique_ptr<swipe_item_menu_item_platform> swipe_item_menu_item_handler::create_platform_view()
    {
        auto platform = std::make_unique<swipe_item_menu_item_platform>();
        try
        {
            // SwipeItemMenuItemHandler.iOS CreatePlatformView: a UIButton with
            // UserInteractionEnabled=false (the SwipeView's own pan drives activation, not the button).
            // The WinUI analog is IsHitTestVisible=false — it drops pointer input without the grayed
            // disabled visual IsEnabled=false would paint.
            const muxc::Button button;
            button.IsHitTestVisible(false);
            button.IsTabStop(false);
            platform->native = wnative::store(button); // released in ~swipe_item_menu_item_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    // C# ConnectHandler / SwipeItemButtonProxy.Connect: the frame observer re-runs MapSource on a frame
    // change — deferred with the icon (header deviations), so nothing to wire (the headless twin's body).
    void swipe_item_menu_item_handler::connect()
    {
    }

    // C# DisconnectHandler / SwipeItemButtonProxy.Disconnect — no observer was wired (see connect).
    void swipe_item_menu_item_handler::disconnect() const
    {
    }

    // C# MapText: RestorationIdentifier = Text; SetTitle(Text). Content = the boxed string (the stock-
    // Button text slot, like the button partial) + AutomationProperties.AutomationId as the identity
    // slot (header deviations).
    void swipe_item_menu_item_handler::apply_text() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        platform->title = std::string(item_view_->text());
        if (auto button = button_of(*platform))
        {
            button.Content(winrt::box_value(wnative::to_hstring_utf8(item_view_->text())));
            wnative::apply_automation_id(platform->native, item_view_->text());
        }
    }

    // C# MapTextColor: SetTitleColor(view.GetTextColor()) — the luminosity-derived effective colour
    // (null leaves the title colour untouched, matching C#'s `if (color != null)` guard).
    void swipe_item_menu_item_handler::apply_text_color() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const auto color = get_text_color(*item_view_);
        platform->has_title_color = color.has_value();
        if (color.has_value())
        {
            platform->title_color_argb = color->to_uint();
            if (auto button = button_of(*platform))
            {
                button.Foreground(wnative::to_brush(*color));
            }
        }
    }

    // C# MapCharacterSpacing: UpdateCharacterSpacing(view) — Control.CharacterSpacing in 1/1000 em.
    void swipe_item_menu_item_handler::apply_character_spacing() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        platform->character_spacing = item_view_->character_spacing();
        if (auto button = button_of(*platform))
        {
            button.CharacterSpacing(wnative::to_em(item_view_->character_spacing()));
        }
    }

    // C# MapFont: UpdateFont(view, fontManager) — FontSize + FontFamily + FontStyle + FontWeight (the
    // button partial's ControlExtensions.UpdateFont body; the registrar lookup is skipped, same as
    // there).
    void swipe_item_menu_item_handler::apply_font() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const font value = item_view_->font();
        platform->item_font = value;
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        const double size = value.size();
        button.FontSize((size > 0 && !std::isnan(size)) ? size : k_default_font_size);
        if (!value.family().empty())
        {
            button.FontFamily(muxm::FontFamily{wnative::to_hstring_utf8(value.family())});
        }
        else
        {
            button.ClearValue(muxc::Control::FontFamilyProperty()); // C# null Family → the default family
        }
        button.FontStyle(to_font_style(value.slant()));
        button.FontWeight(to_font_weight(value.weight()));
    }

    // C# MapBackground: UpdateBackground(view.Background) — the coloured fill the revealed item shows.
    // A null paint restores the theme default (ClearValue), like the button partial's null branch.
    void swipe_item_menu_item_handler::apply_background() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const maui::graphics::paint* const paint = item_view_->background();
        platform->has_background = paint != nullptr;
        if (paint != nullptr)
        {
            platform->background_argb = paint->background_color().to_uint();
        }
        auto button = button_of(*platform);
        if (button == nullptr)
        {
            return;
        }
        if (paint != nullptr)
        {
            button.Background(to_brush_argb(platform->background_argb));
        }
        else
        {
            button.ClearValue(muxc::Control::BackgroundProperty());
        }
    }

    // C# MapSource: load + resize the icon. deferred: no image decode pipeline on this backend yet
    // (header deviations) — the mirror records whether a non-empty source was set, and a stale set is
    // cleared off the button content slot's icon (nothing was ever pushed, so nothing to clear).
    void swipe_item_menu_item_handler::apply_source() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const auto source = item_view_->source();
        platform->has_source = source != nullptr && !source->is_empty();
    }

    // C# MapVisibility: notify the parent MauiSwipeView (UpdateIsVisibleSwipeItem) + UpdateVisibility.
    // The port's swipe host is a plain Canvas and the shared swipe_machine reads each item's visibility
    // live (the apple twin's deviation), so this pushes the element visibility only.
    void swipe_item_menu_item_handler::apply_visibility() const
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || item_view_ == nullptr)
        {
            return;
        }
        const maui::core::visibility value = item_view_->visibility();
        platform->item_visibility = value;
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses. The
        // struct is not a view_platform_base (no alpha mirror), so the restore opacity is 1.0.
        wnative::apply_visibility(platform->native, value, 1.0);
    }
} // namespace maui::core
