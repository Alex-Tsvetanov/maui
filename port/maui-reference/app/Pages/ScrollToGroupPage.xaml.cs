// Hand-written code-behind partial for pages/scroll_to_group.xaml — not GENERATED (do not overwrite with
// e2e.py gen's trivial stub). CollectionView.IsGrouped="True" needs a real grouped source; the C# markup
// has none inline, matching the original sample (ScrollToGroup.xaml.cs), so wire it here per
// docs/AUTHORING.md (code-behind data wiring is allowed; only XAML event attributes are forbidden). The
// original's ScrollTo/ScrollToItem button Clicked handlers are omitted (event handlers are out of scope
// for this static twin) — only the ItemsSource assignment is reproduced.
namespace MauiReference.Pages;

public partial class ScrollToGroupPage : ContentPage
{
    public ScrollToGroupPage()
    {
        InitializeComponent();

        CollectionView.ItemsSource = new SuperTeams();
    }
}
