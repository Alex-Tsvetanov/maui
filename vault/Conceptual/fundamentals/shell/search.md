---
title: ".NET MAUI Shell search"
description: "Learn how .NET MAUI Shell apps can use integrated search functionality that's provided by a search box that can be added to the top of each page."
tags:
  - conceptual
  - area/fundamentals
ms_date: "08/20/2025"
source: "https://learn.microsoft.com/dotnet/maui/fundamentals/shell/search?view=net-maui-10.0"
---

# .NET MAUI Shell search

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/fundamentals-shell)

.NET Multi-platform App UI (.NET MAUI) Shell includes integrated search functionality that's provided by the [[SearchHandler|SearchHandler]] class. Search capability can be added to a page by setting the `Shell.SearchHandler` attached property to a subclassed [[SearchHandler|SearchHandler]] object. This results in a search box being added at the top of the page:

![](media/search/searchhandler.png)

When a query is entered into the search box, the `Query` property is updated, and on each update the `OnQueryChanged` method is executed. This method can be overridden to populate the search suggestions area with data:

![](media/search/search-suggestions.png)

Then, when a result is selected from the search suggestions area, the `OnItemSelected` method is executed. This method can be overridden to respond appropriately, such as by navigating to a detail page.

## Create a SearchHandler

Search functionality can be added to a Shell app by subclassing the [[SearchHandler|SearchHandler]] class, and overriding the `OnQueryChanged` and `OnItemSelected` methods:

```csharp
public class AnimalSearchHandler : SearchHandler
{
    public IList<Animal> Animals { get; set; }
    public Type SelectedItemNavigationTarget { get; set; }

    protected override void OnQueryChanged(string oldValue, string newValue)
    {
        base.OnQueryChanged(oldValue, newValue);

        if (string.IsNullOrWhiteSpace(newValue))
        {
            ItemsSource = null;
        }
        else
        {
            ItemsSource = Animals
                .Where(animal => animal.Name.ToLower().Contains(newValue.ToLower()))
                .ToList<Animal>();
        }
    }

    protected override async void OnItemSelected(object item)
    {
        base.OnItemSelected(item);

        Animal animal = item as Animal;
        string navigationTarget = GetNavigationTarget();

        if (navigationTarget.Equals("catdetails") || navigationTarget.Equals("dogdetails"))
        {
            // Navigate, passing a string
            await Shell.Current.GoToAsync($"{navigationTarget}?name={((Animal)item).Name}");
        }
        else
        {
            string lowerCasePropertyName = navigationTarget.Replace("details", string.Empty);
            // Capitalise the property name
            string propertyName = char.ToUpper(lowerCasePropertyName[0]) + lowerCasePropertyName.Substring(1);

            var navigationParameters = new Dictionary<string, object>
            {
                { propertyName, animal }
            };

            // Navigate, passing an object
            await Shell.Current.GoToAsync($"{navigationTarget}", navigationParameters);
        }
    }

    string GetNavigationTarget()
    {
        return (Shell.Current as AppShell).Routes.FirstOrDefault(route => route.Value.Equals(SelectedItemNavigationTarget)).Key;
    }
}
```

The `OnQueryChanged` override has two arguments: `oldValue`, which contains the previous search query, and `newValue`, which contains the current search query. The search suggestions area can be updated by setting the `SearchHandler.ItemsSource` property to an `IEnumerable` collection that contains items that match the current search query.

When a search result is selected by the user, the `OnItemSelected` override is executed and the `SelectedItem` property is set. In this example, the method navigates to another page that displays data about the selected `Animal`. For more information about navigation, see [[navigation|Shell navigation]].

> [!NOTE]
> Additional [[SearchHandler|SearchHandler]] properties can be set to control the search box appearance.

## Consume a SearchHandler

The subclassed [[SearchHandler|SearchHandler]] can be consumed by setting the `Shell.SearchHandler` attached property to an object of the subclassed type, on the consuming page:

```xaml
<ContentPage ...
             xmlns:controls="clr-namespace:Xaminals.Controls"
             xmlns:data="clr-namespace:Xaminals.Data">
    <Shell.SearchHandler>
        <controls:AnimalSearchHandler Placeholder="Enter search term"
                                      ShowsResults="true"
                                      DisplayMemberName="Name"
                                      Animals="{x:Static data:CatData.Cats}" />
    </Shell.SearchHandler>
    ...
</ContentPage>
```

The equivalent C# code is:

```csharp
Shell.SetSearchHandler(this, new AnimalSearchHandler
{
    Placeholder = "Enter search term",
    ShowsResults = true,
    DisplayMemberName = "Name",
    Animals = CatData.Cats
});
```

The `AnimalSearchHandler.OnQueryChanged` method returns a `List` of `Animal` objects. The `DisplayMemberName` property is set to the `Name` property of each `Animal` object, and so the data displayed in the suggestions area will be each animal name.


> [!WARNING]
> `SearchHandler.DisplayMemberName` isn't trim safe and shouldn't be used with full trimming or NativeAOT. Instead, you should provide an `ItemTemplate` to define the appearance of `SearchHandler` results. For more information, see [Define search results item appearance](#define-search-results-item-appearance), [[trimming|Trim a .NET MAUI app]] and [[nativeaot|Native AOT deployment]].



> [!TIP]
> In .NET 10, trimming defaults and feature switches mean `SearchHandler.DisplayMemberName` is ignored when full trimming or Native AOT is enabled. Prefer using `ItemTemplate` for results, which is trim-safe and supported across platforms. If you must use `DisplayMemberName` in partially trimmed scenarios, you can opt back in by setting the `MauiShellSearchResultsRendererDisplayMemberNameSupported` feature switch to `true` in your project file. For details and tradeoffs, see [[trimming#trimming-feature-switches|Trimming feature switches]].


The `ShowsResults` property is set to `true`, so that search suggestions are displayed as the user enters a search query:

![](media/search/search-results.png)

As the search query changes, the search suggestions area is updated:

![](media/search/search-results-change.png)

When a search result is selected, the `MonkeyDetailPage` is navigated to, and a detail page about the selected monkey is displayed:

![](media/search/detailpage.png)

## Define search results item appearance

In addition to displaying `string` data in the search results, the appearance of each search result item can be defined by setting the `SearchHandler.ItemTemplate` property to a [[DataTemplate|DataTemplate]]:

```xaml
<ContentPage ...
             xmlns:controls="clr-namespace:Xaminals.Controls"
             xmlns:data="clr-namespace:Xaminals.Data"
             xmlns:models="clr-namespace:Xaminals.Models">
    <Shell.SearchHandler>
        <controls:AnimalSearchHandler Placeholder="Enter search term"
                                      ShowsResults="true"
                                      Animals="{x:Static data:CatData.Cats}">
            <controls:AnimalSearchHandler.ItemTemplate>
                <DataTemplate x:DataType="models:Animal">
                    <Grid Padding="10"
                          ColumnDefinitions="0.15*,0.85*">
                        <Image Source="{Binding ImageUrl}"
                               HeightRequest="40"
                               WidthRequest="40" />
                        <Label Grid.Column="1"
                               Text="{Binding Name}"
                               FontAttributes="Bold"
                               VerticalOptions="Center" />
                    </Grid>
                </DataTemplate>
            </controls:AnimalSearchHandler.ItemTemplate>
       </controls:AnimalSearchHandler>
    </Shell.SearchHandler>
    ...
</ContentPage>
```

The elements specified in the [[DataTemplate|DataTemplate]] define the appearance of each item in the suggestions area. In this example, layout within the [[DataTemplate|DataTemplate]] is managed by a [[Grid (Controls)|Grid]]. The [[Grid (Controls)|Grid]] contains an [[Image (Controls)|Image]] object, and a [[Label (Controls)|Label]] object, that both bind to properties of each `Monkey` object.

The following screenshot shows the result of templating each item in the suggestions area:

![](media/search/search-results-template.png)

For more information about data templates, see [[datatemplate|Data templates]].

## Search box visibility

By default, when a [[SearchHandler|SearchHandler]] is added at the top of a page, the search box is visible and fully expanded. However, this behavior can be changed by setting the `SearchHandler.SearchBoxVisibility` property to one of the `SearchBoxVisibility` enumeration members:

- `Hidden` – the search box is not visible or accessible.
- `Collapsible` – the search box is hidden until the user performs an action to reveal it. On iOS the search box is revealed by vertically bouncing the page content, and on Android the search box is revealed by tapping the question mark icon.
- `Expanded` – the search box is visible and fully expanded. This is the default value of the `SearchBoxVisibility` property.

> [!IMPORTANT]
> On iOS, a collapsible search box requires iOS 11 or greater.

The following example shows to how to hide the search box:

```xaml
<ContentPage ...
             xmlns:controls="clr-namespace:Xaminals.Controls">
    <Shell.SearchHandler>
        <controls:AnimalSearchHandler SearchBoxVisibility="Hidden"
                                      ... />
    </Shell.SearchHandler>
    ...
</ContentPage>
```

## Search box focus

Tapping in a search box invokes the onscreen keyboard, with the search box gaining input focus. This can also be achieved programmatically by calling the `Focus` method, which attempts to set input focus on the search box, and returns `true` if successful. When a search box gains focus, the `Focused` event is fired and the overridable `OnFocused` method is called.

When a search box has input focus, tapping elsewhere on the screen dismisses the onscreen keyboard, and the search box loses input focus. This can also be achieved programmatically by calling the `Unfocus` method. When a search box loses focus, the `Unfocused` event is fired and the overridable `OnUnfocus` method is called.

The focus state of a search box can be retrieved through the `IsFocused` property, which returns `true` if a [[SearchHandler|SearchHandler]] currently has input focus.

## SearchHandler keyboard

The keyboard that's presented when users interact with a [[SearchHandler|SearchHandler]] can be set programmatically via the `Keyboard` property, to one of the following properties from the `Keyboard` class:

- `Chat` – used for texting and places where emoji are useful.
- `Default` – the default keyboard.
- `Email` – used when entering email addresses.
- `Numeric` – used when entering numbers.
- `Plain` – used when entering text, without any `KeyboardFlags` specified.
- `Telephone` – used when entering telephone numbers.
- `Text` – used when entering text.
- `Url` – used for entering file paths & web addresses.

This can be accomplished in XAML as follows:

```xaml
<SearchHandler Keyboard="Email" />
```

The `Keyboard` class also has a `Create` factory method that can be used to customize a keyboard by specifying capitalization, spellcheck, and suggestion behavior. `KeyboardFlags` enumeration values are specified as arguments to the method, with a customized `Keyboard` being returned. The `KeyboardFlags` enumeration contains the following values:

- `None` – no features are added to the keyboard.
- `CapitalizeSentence` – indicates that the first letter of the first word of each entered sentence will be automatically capitalized.
- `Spellcheck` – indicates that spellcheck will be performed on entered text.
- `Suggestions` – indicates that word completions will be offered on entered text.
- `CapitalizeWord` – indicates that the first letter of each word will be automatically capitalized.
- `CapitalizeCharacter` – indicates that every character will be automatically capitalized.
- `CapitalizeNone` – indicates that no automatic capitalization will occur.
- `All` – indicates that spellcheck, word completions, and sentence capitalization will occur on entered text.

The following XAML code example shows how to customize the default `Keyboard` to offer word completions and capitalize every entered character:

```xaml
<SearchHandler Placeholder="Enter search terms">
    <SearchHandler.Keyboard>
        <Keyboard x:FactoryMethod="Create">
            <x:Arguments>
                <KeyboardFlags>Suggestions,CapitalizeCharacter</KeyboardFlags>
            </x:Arguments>
        </Keyboard>
    </SearchHandler.Keyboard>
</SearchHandler>
```

## Hide and show the soft input keyboard


In .NET 10, [[SearchHandler|SearchHandler]] gains methods to programmatically control the soft input keyboard:

- [[SearchHandler.ShowSoftInputAsync|ShowSoftInputAsync]]
- [[SearchHandler.HideSoftInputAsync|HideSoftInputAsync]]

These methods let you bring up the keyboard when the user navigates to a page with search, and dismiss it once a result is chosen, without relying on focus changes.

For example, you can auto-open the keyboard when the page appears:

```csharp
// In a ContentPage code-behind
protected override void OnAppearing()
{
    base.OnAppearing();

    var handler = Shell.GetSearchHandler(this);
    handler?.ShowSoftInputAsync();
}
```

And you can hide the keyboard after a selection is made in your custom SearchHandler:

```csharp
public class AnimalSearchHandler : SearchHandler
{
    protected override async void OnItemSelected(object item)
    {
        base.OnItemSelected(item);

        // Dismiss the soft input keyboard
        await HideSoftInputAsync();

        // Perform navigation or other actions
        await Shell.Current.GoToAsync("monkeydetails", new Dictionary<string, object>
        {
            { "Monkey", item }
        });
    }
}
```

Notes:

- On platforms without a software keyboard (for example, desktop with a hardware keyboard), these methods may be no-ops.
- If you need to direct focus to the search box, you can continue to use [[SearchHandler.Focus|Focus]] and [[SearchHandler.Unfocus|Unfocus]]. Showing or hiding the soft input is complementary to focus.

