// Hand-written code-behind partial for pages/switch_grouping.xaml — not GENERATED (do not overwrite with
// e2e.py gen's trivial stub). CollectionView.IsGrouped="True" needs a real grouped source; the C# markup
// has none inline, matching the original sample (SwitchGrouping.xaml.cs), so wire it here per
// docs/AUTHORING.md (code-behind data wiring is allowed; only XAML event attributes are forbidden).
namespace MauiReference.Pages;

public partial class SwitchGroupingPage : ContentPage
{
    public SwitchGroupingPage()
    {
        InitializeComponent();

        CollectionView.ItemsSource = new SuperTeams();
    }
}
