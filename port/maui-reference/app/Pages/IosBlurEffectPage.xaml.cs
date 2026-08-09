// Hand-written partial for pages/ios_blur_effect.xaml — the GENERATED marker is deliberately ABSENT so
// `e2e.py gen` leaves this file alone (see docs/AUTHORING.md).
//
// WHY THIS EXISTS — the same omission ChromePage.xaml.cs and GesturesPage.xaml.cs were written to close.
// The twin declared four buttons and a readout and wired nothing, while the code-first builder
// (port/cpp/examples/gallery/pages/ios_blur_effect_page.hpp:54-60) connects all four to
// `apply_blur_and_readout(style)`. At rest every column reads "BlurEffect: ExtraLight", so the STILL
// comparison never saw it — but this is one of the board's ANIMATED pages, and on a driven frame only
// the code-first column would move, which the board would report as the PORT's defect.
//
// Measured before this change: ios_blur_effect scored INVALID/`no-scenario` on all four lanes x both
// columns — 8 of the 80 cells in that bucket.
//
// THIS PAGE IS DETERMINISTIC AND PHASE-FREE, which is why it was done ahead of `animation`. A tap flips
// a stored enum and rewrites a Label; there is no clock in it. `animation`'s Start button, by contrast,
// launches a chained 1000+1000+2000ms translate, so any captured frame samples a random point along a
// four-second path — comparable only with phase alignment, and INCONCLUSIVE at best.
//
// THE READOUT IS READ BACK, NOT ASSUMED, and that is deliberate rather than decorative. The code-first
// page's update_readout() (line 141) calls GetBlurEffect and formats whatever it finds, so if the
// platform ignores the set the readout says so. Hard-coding the string here would make this twin CLAIM
// a state the platform may have refused — on Android/Windows the blur itself is inert, and the readout
// is then the ONLY evidence the tap did anything. It has to be evidence, not decoration.
//
// THE STRINGS ARE A CONTRACT: ios_blur_effect_page.hpp:146-161 is `"BlurEffect: " + name_of(style)`
// over exactly {None, ExtraLight, Light, Dark}.
using Microsoft.Maui.Controls.PlatformConfiguration;
using Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific;

namespace MauiReference.Pages;

public partial class IosBlurEffectPage : ContentPage
{
    public IosBlurEffectPage()
    {
        InitializeComponent();

        NoBlurButton.Clicked += (_, _) => Apply(BlurEffectStyle.None);
        ExtraLightButton.Clicked += (_, _) => Apply(BlurEffectStyle.ExtraLight);
        LightButton.Clicked += (_, _) => Apply(BlurEffectStyle.Light);
        DarkButton.Clicked += (_, _) => Apply(BlurEffectStyle.Dark);
    }

    void Apply(BlurEffectStyle style)
    {
        BlurTarget.On<iOS>().UseBlurEffect(style);
        // Read back rather than echo `style` — see the header note.
        Readout.Text = "BlurEffect: " + Name(BlurTarget.On<iOS>().GetBlurEffect());
    }

    static string Name(BlurEffectStyle style) => style switch
    {
        BlurEffectStyle.ExtraLight => "ExtraLight",
        BlurEffectStyle.Light => "Light",
        BlurEffectStyle.Dark => "Dark",
        _ => "None",
    };
}
