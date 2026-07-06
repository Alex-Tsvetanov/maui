// Hand-written code-behind partial for pages/gap_carousel_position_binding.xaml (not GENERATED -- the
// marker line is intentionally dropped so `e2e.py gen` leaves this file alone). Position="{Binding
// CurrentPosition}" needs a BindingContext exposing CurrentPosition (see
// GapCarouselPositionBindingModel.cs), assigned in code-behind per docs/AUTHORING.md.
namespace MauiReference.Pages;

public partial class GapCarouselPositionBindingPage : ContentPage
{
    public GapCarouselPositionBindingPage()
    {
        InitializeComponent();

        BindingContext = new GapCarouselPositionBindingViewModel();
    }
}
