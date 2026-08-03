// winui_visual_ops — see the header for what these are and why they are free functions.

#include "winui_visual_ops.hpp"

#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>

#include <algorithm>
#include <vector>

#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/graphics/system_background_paint.hpp" // the legacy Frame's theme-aware default fill
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias.
    namespace winui = winrt::Microsoft::UI::Xaml;

    winui::UIElement element_of(void* slot)
    {
        if (slot == nullptr)
        {
            return nullptr;
        }
        return maui::platform::windows::ref<winui::UIElement>(slot);
    }

    // Append the paint's stops, ORDERED BY OFFSET: C#'s BrushExtensions walks them in offset order,
    // and an unordered list would render in declaration order instead, silently reversing a gradient
    // authored high-offset-first.
    //
    // FILLS the brush's own collection rather than returning one to assign: GradientStops is a
    // READ-ONLY property on both gradient brushes (there is no setter in the projection), so the stops
    // have to be appended into the collection the brush already owns.
    // Takes IVector<GradientStop>, not GradientStopCollection: LinearGradientBrush::GradientStops()
    // returns the collection type but RadialGradientBrush::GradientStops() returns an
    // IObservableVector<GradientStop>. IVector is the interface both satisfy.
    void fill_stops(const winrt::Windows::Foundation::Collections::IVector<winui::Media::GradientStop>& collection,
                    const maui::graphics::gradient_paint& paint)
    {
        std::vector<maui::graphics::gradient_stop> stops = paint.gradient_stops();
        std::ranges::sort(stops, [](const auto& a, const auto& b) { return a.offset() < b.offset(); });
        for (const auto& s : stops)
        {
            winui::Media::GradientStop stop;
            stop.Offset(s.offset());
            stop.Color(maui::platform::windows::to_ui_color(s.color()));
            collection.Append(stop);
        }
    }

} // namespace

namespace maui::platform::windows
{
    // A Brush for the paint, or a null projected object when the paint kind is not translated yet
    // (image and pattern paints - they need an ImageBrush fed by the image-source services). Exported
    // (see the header) so a control whose oracle remaps Background away from apply_background below
    // (Slider, on Windows) can reuse the same translation instead of duplicating it.
    winui::Media::Brush brush_for(const maui::graphics::paint& paint)
    {
        if (const auto* linear = dynamic_cast<const maui::graphics::linear_gradient_paint*>(&paint))
        {
            winui::Media::LinearGradientBrush brush;
            fill_stops(brush.GradientStops(), *linear);
            // MappingMode Relative: MAUI's gradient points are RELATIVE (0..1) coordinates, and
            // LinearGradientBrush defaults to RelativeToBoundingBox, so the points pass straight through.
            brush.StartPoint(
                {static_cast<float>(linear->start_point().x), static_cast<float>(linear->start_point().y)});
            brush.EndPoint({static_cast<float>(linear->end_point().x), static_cast<float>(linear->end_point().y)});
            return brush;
        }
        if (const auto* radial = dynamic_cast<const maui::graphics::radial_gradient_paint*>(&paint))
        {
            winui::Media::RadialGradientBrush brush;
            fill_stops(brush.GradientStops(), *radial);
            brush.Center({static_cast<float>(radial->center().x), static_cast<float>(radial->center().y)});
            // XAML's radial brush takes SEPARATE x/y radii; MAUI's paint carries one relative radius,
            // so both axes get it (a circular gradient in relative space, which is what MAUI draws).
            brush.RadiusX(radial->radius());
            brush.RadiusY(radial->radius());
            brush.GradientOrigin(brush.Center());
            return brush;
        }
        // The legacy Frame's default fill. controls/frame.cpp injects a system_background_paint marker
        // when the developer sets no Background, and each backend is expected to resolve it to the
        // platform's DYNAMIC system background — ios_visual_ops.hpp, apple_visual_ops.hpp and
        // android_visual_ops.hpp all carry that branch. Windows never got one, so it fell through to the
        // marker's static value, which is an opaque WHITE light-mode fallback
        // (graphics/system_background_paint.cpp). The result was a Frame painted white in BOTH themes.
        //
        // Invisible until the board started capturing the OS theme: MAUI's own Windows oracle is the
        // compatibility FrameRenderer, which fills SystemAltHighColor — a THEME-AWARE WinUI system colour
        // (Compatibility/Handlers/Windows/FrameRenderer.cs:113) — and UserAppTheme does NOT drive WinUI's
        // theme dictionaries. So under the old forced-override capture the REFERENCE was white too and
        // the pair scored as a match; under a real system-dark desktop MAUI turns black and the port did
        // not. Measured on radio_button_content_dark: reference (0,0,0), port (255,255,255) at (500,245),
        // whiting out the card and hiding its white label text entirely.
        //
        // Checked BEFORE the solid fallthrough below because system_background_paint DERIVES solid_paint,
        // so the fallthrough would otherwise swallow it. One producer exists repo-wide (frame.cpp), so
        // this cannot leak into any other Windows control.
        if (dynamic_cast<const maui::graphics::system_background_paint*>(&paint) != nullptr)
        {
            const auto app = winui::Application::Current();
            if (app != nullptr)
            {
                const auto key = winrt::box_value(winrt::hstring{L"SystemAltHighColor"});
                if (app.Resources().HasKey(key))
                {
                    return winui::Media::SolidColorBrush{
                        winrt::unbox_value<winrt::Windows::UI::Color>(app.Resources().Lookup(key))};
                }
                // A miss is a live path, not a defect: the oracle reads the RENDERER element's dictionary,
                // which is a different lookup root than Application.Current.Resources. Fall back to the two
                // known SystemAltHighColor values, keyed off the app theme host_run.cpp seeded from the OS.
                const bool dark = app.RequestedTheme() == winui::ApplicationTheme::Dark;
                return winui::Media::SolidColorBrush{
                    maui::platform::windows::to_ui_color(dark ? maui::graphics::color(0.0F, 0.0F, 0.0F, 1.0F)
                                                              : maui::graphics::color(1.0F, 1.0F, 1.0F, 1.0F))};
            }
        }
        // Everything else resolves through the base contract's background_color(), which solid_paint,
        // pattern_paint and the system paint all implement. That is C#'s fallback too.
        return winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(paint.background_color())};
    }

    void apply_visibility(void* slot, maui::core::visibility value)
    {
        const winui::UIElement element = element_of(slot);
        if (element == nullptr)
        {
            return;
        }
        element.Visibility(value == maui::core::visibility::visible ? winui::Visibility::Visible
                                                                    : winui::Visibility::Collapsed);
    }

    void apply_opacity(void* slot, double value)
    {
        if (const winui::UIElement element = element_of(slot))
        {
            element.Opacity(value);
        }
    }

    void apply_is_enabled(void* slot, bool value)
    {
        const winui::UIElement element = element_of(slot);
        if (element == nullptr)
        {
            return;
        }
        if (const auto control = element.try_as<winui::Controls::Control>())
        {
            control.IsEnabled(value);
        }
    }

    void apply_automation_id(void* slot, std::string_view value)
    {
        const winui::UIElement element = element_of(slot);
        if (element == nullptr)
        {
            return;
        }
        winui::Automation::AutomationProperties::SetAutomationId(element, to_hstring(value));
    }

    void apply_background(void* slot, const maui::graphics::paint* value)
    {
        const winui::UIElement element = element_of(slot);
        if (element == nullptr)
        {
            return;
        }
        // Three unrelated WinUI types expose Background and they share no common interface: Panel (the
        // layout hosts), Control (Button and friends) and Border (the label's wrapper). Hence the
        // three-way try_as rather than one cast.
        const auto panel = element.try_as<winui::Controls::Panel>();
        const auto control = element.try_as<winui::Controls::Control>();
        const auto border = element.try_as<winui::Controls::Border>();
        if (value == nullptr)
        {
            // CLEAR, do not paint transparent: a themed control (a Button) draws its own chrome from the
            // theme brush, and stamping a local Transparent over it erases that chrome entirely.
            if (panel)
            {
                panel.ClearValue(winui::Controls::Panel::BackgroundProperty());
            }
            if (control)
            {
                control.ClearValue(winui::Controls::Control::BackgroundProperty());
            }
            if (border)
            {
                border.ClearValue(winui::Controls::Border::BackgroundProperty());
            }
            return;
        }
        const winui::Media::Brush brush = brush_for(*value);
        if (brush == nullptr)
        {
            return;
        }
        if (panel)
        {
            panel.Background(brush);
        }
        if (control)
        {
            control.Background(brush);
        }
        if (border)
        {
            border.Background(brush);
        }
    }
} // namespace maui::platform::windows
