// Hand-written code-behind partial for pages/gap_event_attribute.xaml (not GENERATED -- the marker line
// is intentionally dropped so `e2e.py gen` leaves this file alone). The shared page's Button declares
// Clicked="OnClicked" -- XamlC requires that handler to exist on this partial class, so a trivial no-op
// is provided here (real MAUI compiles + runs it; the port never reaches this code at all since its
// loader rejects the Clicked attribute at parse time -- that rejection is the gap being pinned).
namespace MauiReference.Pages;

public partial class GapEventAttributePage : ContentPage
{
    public GapEventAttributePage()
    {
        InitializeComponent();
    }

    void OnClicked(object sender, EventArgs e)
    {
        // Deliberately empty: this page exists to prove the markup is well-formed, working real-MAUI
        // XAML, not to demonstrate any particular click behavior.
    }
}
