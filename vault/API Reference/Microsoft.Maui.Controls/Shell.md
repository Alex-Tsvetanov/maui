---
title: "Shell"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Shell"
namespace: "Microsoft.Maui.Controls"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - Controls
---

# Shell

> [!abstract] Class in `Microsoft.Maui.Controls`
> Full name: `Microsoft.Maui.Controls.Shell`

The main navigation container for .NET MAUI apps, providing flyout and tab-based navigation.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[Shell.Shell\|Shell]] | Initializes a new instance of the `Shell` class. |

## Properties

| Name | Summary |
|---|---|
| [[Shell.Current\|Current]] |  |
| [[Shell.CurrentItem\|CurrentItem]] |  |
| [[Shell.CurrentPage\|CurrentPage]] | The currently presented page. |
| [[Shell.CurrentState\|CurrentState]] | Gets or sets the icon that, when pressed, opens the flyout. |
| [[Shell.FlyoutBackdrop\|FlyoutBackdrop]] |  |
| [[Shell.FlyoutBackground\|FlyoutBackground]] |  |
| [[Shell.FlyoutBackgroundColor\|FlyoutBackgroundColor]] |  |
| [[Shell.FlyoutBackgroundImage\|FlyoutBackgroundImage]] |  |
| [[Shell.FlyoutBackgroundImageAspect\|FlyoutBackgroundImageAspect]] |  |
| [[Shell.FlyoutBehavior\|FlyoutBehavior]] |  |
| [[Shell.FlyoutContent\|FlyoutContent]] |  |
| [[Shell.FlyoutContentTemplate\|FlyoutContentTemplate]] |  |
| [[Shell.FlyoutFooter\|FlyoutFooter]] |  |
| [[Shell.FlyoutFooterTemplate\|FlyoutFooterTemplate]] |  |
| [[Shell.FlyoutHeader\|FlyoutHeader]] |  |
| [[Shell.FlyoutHeaderBehavior\|FlyoutHeaderBehavior]] |  |
| [[Shell.FlyoutHeaderTemplate\|FlyoutHeaderTemplate]] |  |
| [[Shell.FlyoutHeight\|FlyoutHeight]] |  |
| [[Shell.FlyoutIcon\|FlyoutIcon]] |  |
| [[Shell.FlyoutIsPresented\|FlyoutIsPresented]] |  |
| [[Shell.FlyoutItems\|FlyoutItems]] |  |
| [[Shell.FlyoutVerticalScrollMode\|FlyoutVerticalScrollMode]] |  |
| [[Shell.FlyoutWidth\|FlyoutWidth]] |  |
| [[Shell.ItemTemplate\|ItemTemplate]] |  |
| [[Shell.Items\|Items]] |  |
| [[Shell.MenuItemTemplate\|MenuItemTemplate]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Shell.GetBackButtonBehavior\|GetBackButtonBehavior]] | Gets the `BackButtonBehavior` for the specified `obj`. |
| [[Shell.GetBackgroundColor\|GetBackgroundColor]] | Gets the background color in the Shell chrome. |
| [[Shell.GetDisabledColor\|GetDisabledColor]] | Gets the color to shade text and icons that are disabled. |
| [[Shell.GetFlyoutBackdrop\|GetFlyoutBackdrop]] | Gets the backdrop of the flyout, which is the appearance of the flyout overlay. |
| [[Shell.GetFlyoutBehavior\|GetFlyoutBehavior]] | Gets the behavior used to open the flyout when the given `obj` is presented. |
| [[Shell.GetFlyoutHeight\|GetFlyoutHeight]] | Gets the height of the flyout when the given `obj` is active. |
| [[Shell.GetFlyoutItemIsVisible\|GetFlyoutItemIsVisible]] |  |
| [[Shell.GetFlyoutWidth\|GetFlyoutWidth]] | Gets the width of the flyout. |
| [[Shell.GetForegroundColor\|GetForegroundColor]] | Gets the foreground color for the tab bar. |
| [[Shell.GetItemTemplate\|GetItemTemplate]] | Gets the applied to each object managed by Shell. |
| [[Shell.GetMenuItemTemplate\|GetMenuItemTemplate]] | Gets the applied to objects in the MenuItems collection. |
| [[Shell.GetNavBarHasShadow\|GetNavBarHasShadow]] | Gets a value that represents if the navigation bar has a shadow when the given `obj` is active. |
| [[Shell.GetNavBarIsVisible\|GetNavBarIsVisible]] | Gets a value indicating if the navigation bar is visible when when the given `obj` is active. |
| [[Shell.GetNavBarVisibilityAnimationEnabled\|GetNavBarVisibilityAnimationEnabled]] | Gets a value indicating whether the navigation bar visibility change is animated for the given `obj`. |
| [[Shell.GetPresentationMode\|GetPresentationMode]] | Gets the navigation animation that occurs when a page is navigated to with the method. |
| [[Shell.GetSearchHandler\|GetSearchHandler]] | Gets the integrated search functionality. |
| [[Shell.GetTabBarBackgroundColor\|GetTabBarBackgroundColor]] | Gets the background color for the tab bar. |
| [[Shell.GetTabBarDisabledColor\|GetTabBarDisabledColor]] | Gets the color of the tab bar when it's disabled. |
| [[Shell.GetTabBarForegroundColor\|GetTabBarForegroundColor]] | Gets the foreground color for the tab bar. |
| [[Shell.GetTabBarIsVisible\|GetTabBarIsVisible]] | Gets the tabs visibility when the given `obj` is active. |
| [[Shell.GetTabBarTitleColor\|GetTabBarTitleColor]] | Gets the title color for the tab bar. |
| [[Shell.GetTabBarUnselectedColor\|GetTabBarUnselectedColor]] | Gets the unselected color for the tab bar. If the property is unset, the UnselectedColor property value is used. |
| [[Shell.GetTitleColor\|GetTitleColor]] | Gets the color used for the title of the current page. |
| [[Shell.GetTitleView\|GetTitleView]] | Gets any to be displayed in the navigation bar when the given `obj` is active. |
| [[Shell.GetUnselectedColor\|GetUnselectedColor]] | Gets the color for unselected text and icons in the Shell chrome. |
| [[Shell.GoToAsync\|GoToAsync]] | Asynchronously navigates to the specified `state`. |
| [[Shell.LayoutChildren\|LayoutChildren]] |  |
| [[Shell.OnBackButtonPressed\|OnBackButtonPressed]] |  |
| [[Shell.OnBindingContextChanged\|OnBindingContextChanged]] | Gets or sets applied to each of the Items. |
| [[Shell.OnNavigated\|OnNavigated]] |  |
| [[Shell.OnNavigating\|OnNavigating]] |  |
| [[Shell.OnPropertyChanged\|OnPropertyChanged]] |  |
| [[Shell.SetBackButtonBehavior\|SetBackButtonBehavior]] | Sets the back button behavior when the given `obj` is presented. |
| [[Shell.SetBackgroundColor\|SetBackgroundColor]] | Sets the background color in the Shell chrome. The color won't fill in behind the Shell content. |
| [[Shell.SetDisabledColor\|SetDisabledColor]] | Sets the color to shade text and icons that are disabled. |
| [[Shell.SetFlyoutBackdrop\|SetFlyoutBackdrop]] | Sets the backdrop of the flyout, which is the appearance of the flyout overlay. |
| [[Shell.SetFlyoutBehavior\|SetFlyoutBehavior]] | Sets the behavior used to open the flyout when the given `obj` is presented. |
| [[Shell.SetFlyoutHeight\|SetFlyoutHeight]] | Sets the height of the flyout. |
| [[Shell.SetFlyoutItemIsVisible\|SetFlyoutItemIsVisible]] | Sets a value that determines if an object has a visible in the flyout menu. Flyout items are visible in the flyout by default. However, an item can be hidden… |
| [[Shell.SetFlyoutWidth\|SetFlyoutWidth]] | Sets the width of the flyout when the given `obj` is active. This enables scenarios such as expanding the flyout across the entire screen. |
| [[Shell.SetForegroundColor\|SetForegroundColor]] | Defines the foreground color for the tab bar. If the property is unset, the value is used. |
| [[Shell.SetItemTemplate\|SetItemTemplate]] | Sets the applied to each object managed by Shell. |
| [[Shell.SetMenuItemTemplate\|SetMenuItemTemplate]] | Sets the applied to objects in the MenuItems collection. Shell provides the Text and IconImageSource properties to the BindingContext of the . |
| [[Shell.SetNavBarHasShadow\|SetNavBarHasShadow]] | Controls whether the navigation bar has a shadow when the given `obj` is active. By default the value of the property is `true` on Android, and `false` on ot… |
| [[Shell.SetNavBarIsVisible\|SetNavBarIsVisible]] | Controls if the navigation bar is visible when the given `obj` is presented. By default the value of the property is `true`. |
| [[Shell.SetNavBarVisibilityAnimationEnabled\|SetNavBarVisibilityAnimationEnabled]] | Sets whether the navigation bar visibility change should be animated for the given `obj`. By default, the value of the property is `true`. |
| [[Shell.SetPresentationMode\|SetPresentationMode]] | Sets the navigation animation that plays when a `Page` is navigated to with the method. |
| [[Shell.SetSearchHandler\|SetSearchHandler]] | Sets the handler responsible for implementing the integrated search functionality for when the given `obj` is active. Enabling this property results in a sea… |
| [[Shell.SetTabBarBackgroundColor\|SetTabBarBackgroundColor]] | Sets the background color for the tab bar. If the property is unset, the BackgroundColor property value is used. |
| [[Shell.SetTabBarDisabledColor\|SetTabBarDisabledColor]] | Sets the disabled color for the tab bar. If the property is unset, the value is used. |
| [[Shell.SetTabBarForegroundColor\|SetTabBarForegroundColor]] | Sets the foreground color for the tab bar. If the property is unset, the ForegroundColor property value is used. |
| [[Shell.SetTabBarIsVisible\|SetTabBarIsVisible]] | Sets the tabs visibility when the given `obj` is active. |
| [[Shell.SetTabBarTitleColor\|SetTabBarTitleColor]] | Sets the title color for the tab bar. If the property is unset, the `TitleColorProperty` value will be used. |
| [[Shell.SetTabBarUnselectedColor\|SetTabBarUnselectedColor]] | Sets the unselected color for the tab bar. If the property is unset, the UnselectedColor property value is used. |
| [[Shell.SetTitleColor\|SetTitleColor]] | Sets the color used for the title of the current page. |
| [[Shell.SetTitleView\|SetTitleView]] | Sets any to be displayed in the navigation bar when the given `obj` is active. |
| [[Shell.SetUnselectedColor\|SetUnselectedColor]] | Sets the color for unselected text and icons in the Shell chrome. |

## Events

| Name | Summary |
|---|---|
| [[Shell.Navigated\|Navigated]] |  |
| [[Shell.Navigating\|Navigating]] |  |

## Fields

| Name | Summary |
|---|---|
| [[Shell.BackButtonBehaviorProperty\|BackButtonBehaviorProperty]] | Controls the behavior of the page's back button. |
| [[Shell.BackgroundColorProperty\|BackgroundColorProperty]] | Defines the background color in the Shell chrome. The color won't fill in behind the Shell content. |
| [[Shell.CurrentItemProperty\|CurrentItemProperty]] | The currently selected ShellItem. |
| [[Shell.CurrentStateProperty\|CurrentStateProperty]] | Bindable property for `CurrentState`. |
| [[Shell.DisabledColorProperty\|DisabledColorProperty]] | Defines the color to shade text and icons that are disabled. |
| [[Shell.FlyoutBackdropProperty\|FlyoutBackdropProperty]] | The backdrop of the flyout, which is the appearance of the flyout overlay. |
| [[Shell.FlyoutBackgroundColorProperty\|FlyoutBackgroundColorProperty]] | The background color of the Shell Flyout. |
| [[Shell.FlyoutBackgroundImageAspectProperty\|FlyoutBackgroundImageAspectProperty]] | The aspect ratio of the background image. |
| [[Shell.FlyoutBackgroundImageProperty\|FlyoutBackgroundImageProperty]] | Sets the flyout background image, of type ImageSource, to a file, embedded resource, URI, or stream. |
| [[Shell.FlyoutBackgroundProperty\|FlyoutBackgroundProperty]] | Bindable property for `FlyoutBackground`. |
| [[Shell.FlyoutBehaviorProperty\|FlyoutBehaviorProperty]] | Manages the behavior used to open the flyout. |
| [[Shell.FlyoutContentProperty\|FlyoutContentProperty]] | Flyout items, which represent the flyout content. |
| [[Shell.FlyoutContentTemplateProperty\|FlyoutContentTemplateProperty]] | The flyout content can be defined by setting a . A flyout header can optionally be displayed above your flyout content, and a flyout footer can optionally be… |
| [[Shell.FlyoutFooterProperty\|FlyoutFooterProperty]] | The flyout footer appearance. The flyout footer is the content that optionally appears at the bottom of the flyout. |
| [[Shell.FlyoutFooterTemplateProperty\|FlyoutFooterTemplateProperty]] | The flyout footer appearance can be defined by setting a . |
| [[Shell.FlyoutHeaderBehaviorProperty\|FlyoutHeaderBehaviorProperty]] | Bindable property for `FlyoutHeaderBehavior`. |
| [[Shell.FlyoutHeaderProperty\|FlyoutHeaderProperty]] | The flyout header appearance. The flyout header is the content that optionally appears at the top of the flyout. |
| [[Shell.FlyoutHeaderTemplateProperty\|FlyoutHeaderTemplateProperty]] | The flyout header appearance can be defined by setting a . |
| [[Shell.FlyoutHeightProperty\|FlyoutHeightProperty]] | The height of the flyout. This enables scenarios such as reducing the height of the flyout so that it doesn't obscure the tab bar. |
| [[Shell.FlyoutIconProperty\|FlyoutIconProperty]] | By default, Shell applications have a hamburger icon which, when pressed, opens the flyout. This icon can be changed by setting the FlyoutIcon property. |
| [[Shell.FlyoutIsPresentedProperty\|FlyoutIsPresentedProperty]] | The flyout can be programmatically opened and closed by setting the FlyoutIsPresented property to a boolean value that indicates whether the flyout is curren… |
| [[Shell.FlyoutItemIsVisibleProperty\|FlyoutItemIsVisibleProperty]] | The visibility. Flyout items are visible in the flyout by default. |
| [[Shell.FlyoutVerticalScrollModeProperty\|FlyoutVerticalScrollModeProperty]] | Modifies the behavior of the flyout scroll. By default, a flyout can be scrolled vertically when the flyout items don't fit in the flyout. |
| [[Shell.FlyoutWidthProperty\|FlyoutWidthProperty]] | The width of the flyout. This enables scenarios such as expanding the flyout across the entire screen. |
| [[Shell.ForegroundColorProperty\|ForegroundColorProperty]] | Defines the color to shade text and icons. |
| [[Shell.ItemTemplateProperty\|ItemTemplateProperty]] | The applied to each object managed by Shell. |
| [[Shell.ItemsProperty\|ItemsProperty]] | Bindable property for `Items`. |
| [[Shell.MenuItemTemplateProperty\|MenuItemTemplateProperty]] | Customizes the appearance of each . |
| [[Shell.NavBarHasShadowProperty\|NavBarHasShadowProperty]] | Controls whether the navigation bar has a shadow. |
| [[Shell.NavBarIsVisibleProperty\|NavBarIsVisibleProperty]] | Manages if the navigation bar is visible when a page is presented. |
| [[Shell.NavBarVisibilityAnimationEnabledProperty\|NavBarVisibilityAnimationEnabledProperty]] | Determines if the navigation bar visibility change should be animated. |
| [[Shell.PresentationModeProperty\|PresentationModeProperty]] | Defines the navigation animation that occurs when a page is navigated to with the `GoToAsync` method. Also controls if the content is presented in a modal wa… |
| [[Shell.SearchHandlerProperty\|SearchHandlerProperty]] | Controls the search functionality. |
| [[Shell.TabBarBackgroundColorProperty\|TabBarBackgroundColorProperty]] | Defines the background color for the tab bar. If the property is unset, the value is used. |
| [[Shell.TabBarDisabledColorProperty\|TabBarDisabledColorProperty]] | Defines the disabled color for the tab bar. If the property is unset, the value is used. |
| [[Shell.TabBarForegroundColorProperty\|TabBarForegroundColorProperty]] | Bindable property for attached property TabBarForegroundColor . |
| [[Shell.TabBarIsVisibleProperty\|TabBarIsVisibleProperty]] | Manages the bottom tab bar visibility. |
| [[Shell.TabBarTitleColorProperty\|TabBarTitleColorProperty]] | Defines the title color for the tab bar. If the property is unset, the value will be used. |
| [[Shell.TabBarUnselectedColorProperty\|TabBarUnselectedColorProperty]] | Defines the unselected color for the tab bar. If the property is unset, the value is used. |
| [[Shell.TitleColorProperty\|TitleColorProperty]] | Defines the title color for the tab bar. If the property is unset, the value will be used. |
| [[Shell.TitleViewProperty\|TitleViewProperty]] | Enables any to be displayed in the navigation bar. |
| [[Shell.UnselectedColorProperty\|UnselectedColorProperty]] | Defines the unselected color for the tab bar. If the property is unset, the value is used. |

## Guide

- 📖 Conceptual: [[shell]]

## See also

- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.shell)
