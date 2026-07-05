namespace MauiReference;

// Resolves a parity page key (snake_case, the shared-XAML filename in ../pages/) to its compiled page.
// The naming triple is mechanical and lint-enforced (port/maui-reference/docs/AUTHORING.md):
//     key "activity_indicator"  <->  pages/activity_indicator.xaml
//                               <->  x:Class="MauiReference.Pages.ActivityIndicatorPage"
// Unlike the retired ~/maui-compare app there is NO silent fallback page: an unknown key renders a loud
// red error page, so a typo'd or missing page can never masquerade as a valid capture.
public static class PageDispatch
{
    public static ContentPage Create(string key)
    {
        var pascal = string.Concat(key.Split('_', StringSplitOptions.RemoveEmptyEntries)
                                      .Select(w => char.ToUpperInvariant(w[0]) + w[1..]));
        var type = typeof(PageDispatch).Assembly.GetType($"MauiReference.Pages.{pascal}Page");
        if (type is not null && Activator.CreateInstance(type) is ContentPage page)
            return page;

        return new ContentPage
        {
            Title = "UNKNOWN PAGE",
            BackgroundColor = Colors.DarkRed,
            Content = new VerticalStackLayout
            {
                Spacing = 12,
                Padding = 24,
                Children =
                {
                    new Label
                    {
                        Text = "UNKNOWN PAGE",
                        TextColor = Colors.White,
                        FontSize = 32,
                        FontAttributes = FontAttributes.Bold,
                    },
                    new Label
                    {
                        Text = $"No page registered for key '{key}' " +
                               $"(expected class MauiReference.Pages.{pascal}Page from pages/{key}.xaml).",
                        TextColor = Colors.White,
                        FontSize = 18,
                    },
                },
            },
        };
    }
}
