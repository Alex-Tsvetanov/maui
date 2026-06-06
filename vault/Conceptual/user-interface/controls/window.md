---
title: "Window"
description: "Learn how to use the .NET MAUI Window class to create, configure, show, and manage multi-window apps."
tags:
  - conceptual
  - area/user-interface
ms_date: "08/19/2025"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/controls/window?view=net-maui-10.0"
---

# Window

The .NET Multi-platform App UI (.NET MAUI) [[Window|Window]] class provides the ability to create, configure, show, and manage multiple windows.

[[Window|Window]] defines the following properties:

- [[Window.FlowDirection|FlowDirection]], of type `FlowDirection`, defines the direction in which the UI element of the window are laid out.
- [[Window.Height|Height]], of type `double`, specifies the height of the window on Windows.
- [[Window.MaximumHeight|MaximumHeight]], of type `double`, represents the maximum height of the window on desktop platforms. Valid values are between 0 and `double.PositiveInfinity`.
- [[Window.MaximumWidth|MaximumWidth]], of type `double`, represents the maximum width of the window on desktop platforms. Valid values are between 0 and `double.PositiveInfinity`.
- [[Window.MinimumHeight|MinimumHeight]], of type `double`, represents the minimum height of the window on desktop platforms. Valid values are between 0 and `double.PositiveInfinity`.
- [[Window.MinimumWidth|MinimumWidth]], of type `double`, represents the minimum width of the window on desktop platforms. Valid values are between 0 and `double.PositiveInfinity`.
- [[Window.Overlays|Overlays]], of type `IReadOnlyCollection<IWindowOverlay>`, represents the collection of window overlays.
- [[Page (Controls)|Page]], of type [[Page (Controls)|Page]], indicates the page being displayed by the window. This property is the content property of the [[Window|Window]] class, and therefore does not need to be explicitly set.
- [[Window.Title|Title]], of type `string`, represents the title of the window.
- [[Window.Width|Width]], of type `double`, specifies the width of the window on Windows.
- [[Window.X|X]], of type `double`, specifies the X coordinate of the window on Windows.
- [[Window.Y|Y]], of type `double`, specifies the Y coordinate of the window on Windows.

These properties, with the exception of the `Overlays` property, are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

<!-- Todo: Is/will Title be shown on desktop platforms? -->

The [[Window|Window]] class defines the following events:

- [[Window.Created|Created]], which is raised when the window is created.
- [[Window.Resumed|Resumed]], which is raised when the window is resumed from a sleeping state.
- [[Window.Activated|Activated]], which is raised when the window is activated.
- [[Window.Deactivated|Deactivated]], which is raised when the window is deactivated.
- [[Window.Stopped|Stopped]], which is raised when the window is stopped.
- [[Window.Destroying|Destroying]], which is raised when the window is destroyed.
- [[Window.SizeChanged|SizeChanged]], which is raised on desktop platforms when the window changes size.
- [[Window.Backgrounding|Backgrounding]], with an accompanying `BackgroundingEventArgs` object, which is raised on iOS and Mac Catalyst when the window is closed or enters a background state. This event can be used to persist any `string` state to the `State` property of the `BackgroundingEventArgs` object, which the OS will preserve until it's time to resume the window. When the window is resumed the state is provided via the `IActivationState` argument to the `CreateWindow` method.
- [[Window.DisplayDensityChanged|DisplayDensityChanged]], with an accompanying `DisplayDensityChangedEventArgs` object, which is raised on Android and Windows when the effective dots per inch (DPI) for the window has changed.

For more information about the lifecycle events, and their associated overrides, see [[app-lifecycle|App lifecycle]].

The [[Window|Window]] class also defines the following modal navigation events:

- [[Window.ModalPopped|ModalPopped]], with `ModalPoppedEventArgs`, which is raised when a view has been popped modally.
- [[Window.ModalPopping|ModalPopping]], with `ModalPoppingEventArgs`, which is raised when a view is modally popped.
- [[Window.ModalPushed|ModalPushed]], with `ModalPushedEventArgs`, which is raised after a view has been pushed modally.
- [[Window.ModalPushing|ModalPushing]], with `ModalPushingEventArgs`, which is raised when a view is modally pushed.
- [[Window.PopCanceled|PopCanceled]], which is raised when a modal pop is cancelled.

The [[VisualElement (Controls)|VisualElement]] class has a `Window` property that exposes the parent [[Window|Window]] object. This property can be accessed from any page, layout, or view, to manipulate [[Window|Window]] objects.

## Create a Window


By default, .NET MAUI creates a [[Window|Window]] object when you set the `MainPage` property to a [[Page (Controls)|Page]] object in your `App` class. However, you can also override the `CreateWindow` method in your `App` class to create a [[Window|Window]] object:

```csharp
namespace MyMauiApp
{
    public partial class App : Application
    {
        public App()
        {
            InitializeComponent();

            MainPage = new MainPage();
        }

        protected override Window CreateWindow(IActivationState activationState)
        {
            Window window = base.CreateWindow(activationState);

            // Manipulate Window object

            return window;
        }
    }
}
```

While the [[Window|Window]] class has a default constructor and a constructor that accepts a [[Page (Controls)|Page]] argument, which represents the root page of the app, you can also call the base `CreateWindow` method to return the .NET MAUI created [[Window|Window]] object.



By default, your .NET MAUI app overrides the `CreateWindow` method in your `App` class to create a [[Window|Window]] object:

```csharp
namespace MyMauiApp
{
    public partial class App : Application
    {
        public App()
        {
            InitializeComponent();
        }

        protected override Window CreateWindow(IActivationState? activationState)
        {
            return new Window(new AppShell());
        }
    }
}
```


The [[Window|Window]] class has a default constructor and a constructor that accepts a [[Page (Controls)|Page]] argument, which represents the root page of the app.

In addition, you can also create your own [[Window|Window]]-derived object:

```csharp
namespace MyMauiApp
{
    public class MyWindow : Window
    {
        public MyWindow() : base()
        {
        }

        public MyWindow(Page page) : base(page)
        {
        }

        // Override Window methods
    }
}
```

The [[Window|Window]]-derived class can then be consumed by creating a `MyWindow` object in the `CreateWindow` override in your `App` class.

Regardless of how your [[Window|Window]] object is created, it will be the parent of the root page in your app.

## Multi-window support

Multiple windows can be simultaneously opened on Android, iOS on iPad (iPadOS), Mac Catalyst, and Windows. This can be achieved by creating a [[Window|Window]] object and opening it using the `OpenWindow` method on the `Application` object:

```csharp
Window secondWindow = new Window(new MyPage());
Application.Current?.OpenWindow(secondWindow);
```

The `Application.Current.Windows` collection, of type `IReadOnlyList<Window>` maintains references to all [[Window|Window]] objects that are registered with the `Application` object.


A specific window can be brought to the front on Mac Catalyst and Windows with the `Application.Current.ActivateWindow` method:

```csharp
Application.Current?.ActivateWindow(secondWindow);
```


Windows can be closed with the `Application.Current.CloseWindow` method:

```csharp
// Close a specific window
Application.Current?.CloseWindow(secondWindow);

// Close the active window
Application.Current?.CloseWindow(GetParentWindow());
```

> [!IMPORTANT]
> Multi-window support works on Windows without additional configuration. However, additional configuration is required on Android, iPadOS and Mac Catalyst.

### Android configuration

To use multi-window support on Android, you must change the `MainActivity` launch mode in *Platforms > Android > MainActivity.cs* from `LaunchMode.SingleTop` to `LaunchMode.Multiple`:

```csharp
using Android.App;
using Android.Content.PM;
using Android.OS;

namespace MyMauiApp;

[Activity(Theme = "@style/Maui.SplashTheme", MainLauncher = true, LaunchMode = LaunchMode.Multiple, ...)]
public class MainActivity : MauiAppCompatActivity
{
}
```

### iPadOS and macOS configuration

To use multi-window support on iPadOS and Mac Catalyst, add a class named `SceneDelegate` to the **Platforms > iOS** and **Platforms > MacCatalyst** folders:

```csharp
using Foundation;
using Microsoft.Maui;
using UIKit;

namespace MyMauiApp;

[Register("SceneDelegate")]
public class SceneDelegate : MauiUISceneDelegate
{
}
```

Then, in the XML editor, open the **Platforms > iOS > Info.plist** file and the **Platforms > MacCatalyst > Info.plist** file and add the following XML to the end of each file:

```xml
<key>UIApplicationSceneManifest</key>
<dict>
  <key>UIApplicationSupportsMultipleScenes</key>
  <true/>
  <key>UISceneConfigurations</key>
  <dict>
    <key>UIWindowSceneSessionRoleApplication</key>
    <array>
      <dict>
        <key>UISceneConfigurationName</key>
        <string>__MAUI_DEFAULT_SCENE_CONFIGURATION__</string>
        <key>UISceneDelegateClassName</key>
        <string>SceneDelegate</string>
      </dict>
    </array>
  </dict>
</dict>
```

> [!IMPORTANT]
> Multi-window support doesn't work on iOS for iPhone.

## Position and size a Window


The position and size of a window can be programmatically defined for a .NET MAUI app on Windows by setting the `X`, `Y`, `Width`, and `Height` properties on a [[Window|Window]] object.

> [!WARNING]
> Mac Catalyst doesn't support resizing or repositioning windows programmatically by setting the `X`, `Y`, `Width`, and `Height` properties.



The position and size of a window can be programmatically defined for a .NET MAUI app on Mac Catalyst and Windows by setting the `X`, `Y`, `Width`, and `Height` properties on a [[Window|Window]] object.


For example, to set the window position and size on launch you should override the `CreateWindow` method in your `App` class and set the `X`, `Y`, `Width`, and `Height` properties on a [[Window|Window]] object:

```csharp
public partial class App : Application
{
    public App()
    {
        InitializeComponent();
    }

    protected override Window CreateWindow(IActivationState activationState) =>
        new Window(new AppShell())
        {
            Width = 700,
            Height = 500,
            X = 100,
            Y = 100
        };
}
```

Alternatively, a window can be positioned and sized by accessing the `Window` property from any page, layout, or view. For example, the following code shows how to position a window in the center of the screen:

```csharp
// Get display size
var displayInfo = DeviceDisplay.Current.MainDisplayInfo;

// Center the window
Window.X = (displayInfo.Width / displayInfo.Density - Window.Width) / 2;
Window.Y = (displayInfo.Height / displayInfo.Density - Window.Height) / 2;
```

For information about obtaining the device's screen metrics, see [[display|Device display information]].


### Mac Catalyst

Mac Catalyst doesn't support resizing or repositioning windows programmatically. However, a workaround to enable resizing is to set the `MinimumWidth` and `MaximumWidth` properties to the desired width of the window, and the `MinimumHeight` and `MaximumHeight` properties to the desired height of the window. This will trigger a resize, and you can then revert the properties back to their original values:

```csharp
Window.MinimumWidth = 700;
Window.MaximumWidth = 700;
Window.MinimumHeight = 500;
Window.MaximumHeight = 500;

// Give the Window time to resize
Dispatcher.Dispatch(() =>
{
    Window.MinimumWidth = 0;
    Window.MinimumHeight = 0;
    Window.MaximumWidth = double.PositiveInfinity;
    Window.MaximumHeight = double.PositiveInfinity;
});
```


## Decouple window management from the App class

Window management can be decoupled from the `App` class by creating a class that implements the [[IWindowCreator|IWindowCreator]] interface, and adding your window management code in the `CreateWindow%2A` method:

```csharp
public class WindowCreator : IWindowCreator
{
    public Window CreateWindow(Application app, IActivationState activationState)
    {
        var window = new Window(new ContentPage
        {
            Content = new Grid
            {
                new Label
                {
                    Text = "Hello from IWindowCreator",
                    HorizontalOptions = LayoutOptions.Center,
                    VerticalOptions = LayoutOptions.Center
                }
            }
        });

        return window;
    }
}
```

Then, in the `MauiProgram` class you should register your window management type as a dependency in the app's service container:

```csharp
builder.Services.AddSingleton<IWindowCreator, WindowCreator>();
```

> [!IMPORTANT]
> Ensure that your registration code specifies the `IWindowCreator` interface as well as its concrete type.

Then, ensure that your `App` class doesn't set the `MainPage` property:

```csharp
public partial class App : Application
{
    public App()
    {
        InitializeComponent();
    }
}
```

Provided that the [[IWindowCreator|IWindowCreator]] interface and its concrete type have been registered with the app's service container, and that the [[Application (Controls).MainPage|MainPage]] property of the [[Application (Controls)|Application]] class isn't set, your registered type will be used to create the [[Window|Window]].


## Enable or disable minimize and maximize buttons on Windows

On Windows, you can enable or disable the minimize and maximize buttons on a window. This can be accomplished by retrieving the underlying WinUI window and adjusting its presenter properties.

The following example disables both the minimize and maximize buttons:

```csharp
#if WINDOWS
using Microsoft.Maui.Platform; // MauiWinUIWindow
using Microsoft.UI.Windowing;  // AppWindow, OverlappedPresenter

public static void ConfigureWindowButtons(Window window)
{
    var nativeWindow = (MauiWinUIWindow)window.Handler.PlatformView;
    var appWindow = nativeWindow.AppWindow;

    if (appWindow.Presenter is OverlappedPresenter presenter)
    {
        presenter.IsMinimizable = false;
        presenter.IsMaximizable = false;
    }
}
#endif
```

Call `ConfigureWindowButtons` during app startup (for example, after creating the `Window` in `CreateWindow`) or when showing a secondary window.

> [!NOTE]
> These properties only apply on Windows desktop. Other platforms ignore this code path. To re-enable buttons, set `IsMinimizable` and `IsMaximizable` back to `true`.

