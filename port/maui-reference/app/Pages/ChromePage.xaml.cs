// Hand-written partial for pages/chrome.xaml — the GENERATED marker is deliberately ABSENT so
// `e2e.py gen` leaves this file alone (see docs/AUTHORING.md; the generated stub it replaced said so).
//
// WHY THIS EXISTS — the same failure GesturesPage.xaml.cs was written to prevent, on another page.
// The twin declares a Button and a "Ready" readout but wired no handler, while the code-first builder
// (port/cpp/examples/gallery/pages/chrome_page.hpp:75) connects it: `action_button_.clicked.connect(
// [this] { stamp("Button pressed"); })`. At rest all three columns read "Ready", so the STILL
// comparison never saw the gap — but `chrome` is one of the board's ANIMATED pages, and the moment a
// scenario drives it, only the port's code-first column would change. The board would report MOTION
// MISMATCH and blame the port for this file's omission.
//
// Measured before this change: chrome scored INVALID/`no-scenario` on all four lanes x both columns —
// 8 of the 80 cells in that bucket. It had no scenario because a scenario would have been pointless
// while two of the three columns could not react to it.
//
// WIRED VIA x:Name, NOT VIA A XAML EVENT ATTRIBUTE, and that is a hard constraint rather than a style
// choice: pages/chrome.xaml is the SAME file the C++ loader hydrates for the cpp_xaml column, and that
// loader refuses event attributes loudly. A `Clicked="OnPressed"` here would break that column outright
// (docs/AUTHORING.md rule 3, which `e2e.py lint` enforces by regex).
//
// THE STRING IS A CONTRACT, not a description. chrome_page.hpp:126-129 defines
// `stamp(what) => readout_.set_text("Last: " + what)` and passes "Button pressed", so the readout must
// become exactly "Last: Button pressed". Any other wording diverges the driven frame for a reason that
// has nothing to do with either renderer — which is the failure this whole file exists to prevent.
namespace MauiReference.Pages;

public partial class ChromePage : ContentPage
{
    public ChromePage()
    {
        InitializeComponent();

        // Only the Button press is reproducible here. chrome_page.hpp also stamps the readout from its
        // toolbar items, menu-bar items and context flyout — but this twin is DEGRADED by construction
        // (see chrome.xaml's own comment: none of ToolbarItem / MenuBarItem / MenuFlyout* are registered
        // on the XAML loader, because they are chrome attached to the Page/Window rather than view
        // children). The button is the page's one at-rest VISIBLE interactive surface, and so the only
        // one a coordinate-driven scenario can reach on every lane.
        ActionButton.Clicked += (_, _) => Readout.Text = "Last: Button pressed";
    }
}
