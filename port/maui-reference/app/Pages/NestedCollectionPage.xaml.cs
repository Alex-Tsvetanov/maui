// Hand-written code-behind partial for pages/nested_collection.xaml — not GENERATED (do not overwrite
// with e2e.py gen's trivial stub). The outer CollectionView's ItemsSource comes via {Binding Items} off
// the page's BindingContext; the C# markup sets no ItemsSource inline either (BindingContext =
// NestedCollectionViewModel is assigned in code-behind), so wire it here per docs/AUTHORING.md
// (code-behind data wiring is allowed; only XAML event attributes are forbidden).
namespace MauiReference.Pages;

public partial class NestedCollectionPage : ContentPage
{
    public NestedCollectionPage()
    {
        InitializeComponent();

        BindingContext = new NestedCollectionViewModel();
        CollectionView.SetBinding(Microsoft.Maui.Controls.ItemsView.ItemsSourceProperty, new Binding("Items"));
    }
}
