// Hand-written code-behind partial for pages/grouping_plus_selection.xaml — not GENERATED (do not
// overwrite with e2e.py gen's trivial stub). CollectionView.IsGrouped="True" needs a real grouped source;
// the C# markup has none inline, matching the original sample (GroupingPlusSelection.xaml.cs), so wire it
// here per docs/AUTHORING.md (code-behind data wiring is allowed; only XAML event attributes are
// forbidden).
namespace MauiReference.Pages;

public partial class GroupingPlusSelectionPage : ContentPage
{
    public GroupingPlusSelectionPage()
    {
        InitializeComponent();

        CollectionView.ItemsSource = new SuperTeams();
    }
}
