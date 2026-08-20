// HAND-WRITTEN (the GENERATED marker is deliberately absent so e2e.py gen leaves this file alone).
//
// WHY THIS PAGE NEEDS A BINDING CONTEXT AT ALL. The original sample
// (src/Controls/samples/.../SelectionGalleries/SelectionSynchronization.xaml.cs) binds every Selected*
// slot to a SelectionSyncModel, and the multiple-selection ones are the point of the page. The twin
// used to inline them as <x:Array Type="{x:Type x:String}"> instead, and XamlC REFUSES to match a
// String[] to IList<object> (no CLR array covariance), so it added the array AS ONE OBJECT and the
// page rendered with NOTHING selected in six of its nine CollectionViews -- i.e. the twin silently
// demonstrated the opposite of what its own labels claim.
//
// The model below mirrors the original's, verbatim in values, so the shared XAML can bind exactly what
// the original binds. See port/CLAUDE.md rule 5 (PORT-MUST-EXPRESS-IT).
namespace MauiReference.Pages;

using System.Collections.ObjectModel;

public partial class SelectionSynchronizationPage : ContentPage
{
    public SelectionSynchronizationPage()
    {
        InitializeComponent();
        BindingContext = new SelectionSyncModel();
    }
}

// Twin of the original's SelectionSyncModel (…/SelectionSynchronization.xaml.cs:31-52).
public class SelectionSyncModel
{
    public SelectionSyncModel()
    {
        Items = new List<string> { "Item 1", "Item 2", "Item 3", "Item 4" };

        SelectedItem = "Item 2";
        SelectedItems = new ObservableCollection<object> { "Item 3", "Item 2" };

        // Deliberately absent from Items: these drive the four "(not in source)" CollectionViews, which
        // must resolve to NOTHING selected. That is the behaviour the page exists to demonstrate.
        SelectedItemNotInSource = "Foo";
        SelectedItemsNotInSource = new ObservableCollection<object> { "Foo", "Bar", "Baz" };
    }

    public List<string> Items { get; set; }

    public string SelectedItem { get; set; }
    public ObservableCollection<object> SelectedItems { get; set; }

    public string SelectedItemNotInSource { get; set; }
    public ObservableCollection<object> SelectedItemsNotInSource { get; set; }
}
