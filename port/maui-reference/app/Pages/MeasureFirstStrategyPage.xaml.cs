// Hand-written code-behind partial for pages/measure_first_strategy.xaml — not GENERATED (do not
// overwrite with e2e.py gen's trivial stub). CollectionView.IsGrouped="True" needs a real grouped source;
// the C# markup has none inline, matching the original sample (MeasureFirstStrategy.xaml.cs), so wire it
// here per docs/AUTHORING.md (code-behind data wiring is allowed; only XAML event attributes are
// forbidden).
namespace MauiReference.Pages;

public partial class MeasureFirstStrategyPage : ContentPage
{
    public MeasureFirstStrategyPage()
    {
        InitializeComponent();

        CollectionView.ItemsSource = new SuperTeams();
    }
}
