// Hand-written partial for pages/button.xaml — the GENERATED marker is deliberately ABSENT so
// `e2e.py gen` leaves this file alone (see docs/AUTHORING.md).
//
// WHY THIS EXISTS — the third page in this batch with the same omission, and the one that had a
// SCORER EXEMPTION built for it. The twin carried a literal `<Label Text="Taps: 0"/>` beside
// `<!-- Clicked (handler omitted) -->`, while the code-first builder (examples/gallery/pages/
// button_page.hpp:63-66) connects the button to `++tap_count_; update_readout()`. That asymmetry was
// unfixable at the time, so motion_score grew a `twin_cannot_react` flag and button.toml set it — a
// scorer escape hatch whose own comment says "RETIRE THIS BY FIXING THE TWIN, not by deleting the
// scenario". This is that fix; the flag comes out with it.
//
// WIRED VIA x:Name, NOT VIA A XAML EVENT ATTRIBUTE: pages/button.xaml is the SAME file the C++ loader
// hydrates for the cpp_xaml column, and that loader refuses event attributes loudly (AUTHORING.md
// rule 3, enforced by `e2e.py lint`). A `Clicked="OnClicked"` here would break that column outright.
//
// THE FORMAT IS A CONTRACT and the code-first page is its ORIGIN: button_page.hpp:228-231 is
// `snprintf("Taps: %d", tap_count_)`, so the readout must read exactly "Taps: 1" after one click —
// not "Taps:1", not "1 tap". A driven frame diverges on any other spelling.
//
// ONLY the "Clicked" button is wired here. The page's other buttons (Command, BackgroundColor,
// BorderColor, CornerRadius, the image buttons, the spacing pair) drive behaviours the twin either
// expresses declaratively already or cannot express at all; adding handlers for them would change what
// the still comparison renders, which is a separate decision from making the page driveable.
namespace MauiReference.Pages;

public partial class ButtonPage : ContentPage
{
    int _taps;

    public ButtonPage()
    {
        InitializeComponent();
        ClickedButton.Clicked += (_, _) => Readout.Text = $"Taps: {++_taps}";
    }
}
