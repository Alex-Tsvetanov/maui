// Shared data model for pages/gap_picker_items_source.xaml: a plain fixed string list bound to
// Picker.ItemsSource, so real MAUI renders a populated Picker (the P3 gap corpus acceptance gate).
namespace MauiReference.Pages;

public class GapPickerItemsSourceViewModel
{
    public List<string> Fruits { get; } = new()
    {
        "Apple",
        "Banana",
        "Cherry",
        "Date",
    };
}
