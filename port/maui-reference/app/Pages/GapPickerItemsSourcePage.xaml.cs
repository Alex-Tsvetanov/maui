// Hand-written code-behind partial for pages/gap_picker_items_source.xaml (not GENERATED -- the marker
// line is intentionally dropped so `e2e.py gen` leaves this file alone). ItemsSource="{Binding Fruits}"
// needs a BindingContext exposing Fruits (see GapPickerItemsSourceModel.cs), assigned in code-behind per
// docs/AUTHORING.md (code-behind data wiring is allowed; only XAML event attributes are forbidden).
namespace MauiReference.Pages;

public partial class GapPickerItemsSourcePage : ContentPage
{
    public GapPickerItemsSourcePage()
    {
        InitializeComponent();

        BindingContext = new GapPickerItemsSourceViewModel();
    }
}
