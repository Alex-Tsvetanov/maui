// Hand-written code-behind partial for pages/grouping_no_templates.xaml — not GENERATED (do not overwrite
// with e2e.py gen's trivial stub). CollectionView.IsGrouped="True" needs a real grouped source; the C#
// markup has none inline, matching the original sample (GroupingNoTemplates.xaml.cs), so wire it here per
// docs/AUTHORING.md (code-behind data wiring is allowed; only XAML event attributes are forbidden).
namespace MauiReference.Pages;

public partial class GroupingNoTemplatesPage : ContentPage
{
    public GroupingNoTemplatesPage()
    {
        InitializeComponent();

        CollectionView.ItemsSource = new SuperTeams();
    }
}
