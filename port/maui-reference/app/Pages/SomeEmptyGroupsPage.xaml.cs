// Hand-written code-behind partial for pages/some_empty_groups.xaml — not GENERATED (do not overwrite
// with e2e.py gen's trivial stub). CollectionView.IsGrouped="True" needs a real grouped source; the C#
// markup has none inline, matching the original sample (SomeEmptyGroups.xaml.cs), so wire it here per
// docs/AUTHORING.md (code-behind data wiring is allowed; only XAML event attributes are forbidden).
// Faithfully reproduces the original's five teams (two deliberately empty: "Thundercats", "Bionic Six")
// so the page demonstrates that empty groups still render their header/footer.
namespace MauiReference.Pages;

public partial class SomeEmptyGroupsPage : ContentPage
{
    public SomeEmptyGroupsPage()
    {
        InitializeComponent();

        var teams = new List<Team>
        {
            new Team("Avengers", new List<Member>
            {
                new Member("Thor"),
                new Member("Captain America")
            }),

            new Team("Thundercats", new List<Member>()),

            new Team("Avengers", new List<Member>
            {
                new Member("Thor"),
                new Member("Captain America")
            }),

            new Team("Bionic Six", new List<Member>()),

            new Team("Fantastic Four", new List<Member>
            {
                new Member("The Thing"),
                new Member("The Human Torch"),
                new Member("The Invisible Woman"),
                new Member("Mr. Fantastic"),
            })
        };

        CollectionView.ItemsSource = teams;
    }
}
