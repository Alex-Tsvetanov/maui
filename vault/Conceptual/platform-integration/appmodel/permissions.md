---
title: "Permissions"
description: "Learn how to use the .NET MAUI Permissions class, to check and request permissions. This class is in the Microsoft.Maui.ApplicationModel namespace."
tags:
  - conceptual
  - area/platform-integration
ms_date: "07/11/2025"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/appmodel/permissions?view=net-maui-10.0"
---

# Permissions

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[Permissions|Permissions]] class. This class allows you to check and request permissions at run-time. The `Permissions` type is available in the `Microsoft.Maui.ApplicationModel` namespace.

## Available permissions

.NET MAUI attempts to abstract as many permissions as possible. However, each operating system has a different set of permissions. Even though the API allows access to a common permission, there may be differences between operating systems related to that permission. The following table describes the available permissions:

The following table uses ✔️ to indicate that the permission is supported and ❌ to indicate the permission isn't supported or isn't required:

| Permission                                                                              | Android | iOS | Windows | tvOS |
|------------------------------------------------------------------------------------------|:-------:|:---:|:-------:|:----:|
| [[Battery (ApplicationModel)|Battery]]                     | ✔️     | ❌  | ❌     | ❌    |
| [[Bluetooth|Bluetooth]]                 | ✔️     | ❌  | ❌     | ❌    |
| [[CalendarRead|CalendarRead]]           | ✔️     | ✔️  | ❌      | ❌    |
| [[CalendarWrite|CalendarWrite]]         | ✔️     | ✔️  | ❌      | ❌    |
| [[Camera|Camera]]                       | ✔️     | ✔️  | ❌      | ❌    |
| [[ContactsRead|ContactsRead]]           | ✔️     | ✔️  | ❌      | ❌    |
| [[ContactsWrite|ContactsWrite]]         | ✔️     | ✔️  | ❌      | ❌    |
| [[Flashlight (ApplicationModel)|Flashlight]]               | ✔️     | ❌   | ❌      | ❌    |
| [[LocationWhenInUse|LocationWhenInUse]] | ✔️     | ✔️  | ❌      | ✔️   |
| [[LocationAlways|LocationAlways]]       | ✔️     | ✔️  | ❌      | ❌    |
| [[Media|Media]]                         | ❌      | ✔️  | ❌      | ❌    |
| [[Microphone|Microphone]]               | ✔️     | ✔️  | ❌      | ❌    |
| [[NearbyWifiDevices|NearbyWifiDevices]] | ✔️     | ❌  | ❌     | ❌    |
| [[NetworkState|NetworkState]]           | ✔️     | ❌  | ❌      | ❌   |
| [[Phone|Phone]]                         | ✔️     | ✔️  | ❌      | ❌    |
| [[Photos|Photos]]                       | ❌     | ✔️  | ❌      | ✔️   |
| [[PhotosAddOnly|PhotosAddOnly]]         | ❌     | ✔️  | ❌       | ✔️   |
| [[PostNotifications|PostNotifications]] | ✔️     | ✔️  | ❌      | ❌   |
| [[Reminders|Reminders]]                 | ❌      | ✔️  | ❌      | ❌    |
| [[Sensors|Sensors]]                     | ✔️     | ✔️  | ❌      | ❌    |
| [[Sms (ApplicationModel)|Sms]]                             | ✔️     | ✔️  | ❌      | ❌    |
| [[Speech|Speech]]                       | ✔️     | ✔️  | ❌      | ❌    |
| [[StorageRead|StorageRead]]             | ✔️     | ❌   | ❌      | ❌    |
| [[StorageWrite|StorageWrite]]           | ✔️     | ❌   | ❌      | ❌    |
| [[Vibrate|Vibrate]]                     | ✔️     | ❌   | ❌      | ❌    |

> [!IMPORTANT]
> The [[StorageRead|StorageRead]] and [[StorageWrite|StorageWrite]] permissions will always return [[PermissionStatus.Granted|Granted]] on Android API 33+. This is because the underlying Android `READ_EXTERNAL_STORAGE` and `WRITE_EXTERNAL_STORAGE` permissions are no longer available from API 33.

If a permission is marked as ❌, it will always return [[PermissionStatus.Granted|Granted]] when checked or requested.


> [!NOTE]
> Starting with .NET MAUI in .NET 11, `PostNotifications` is also supported on iOS and Mac Catalyst. On Apple platforms, this permission uses `UNUserNotificationCenter.RequestAuthorization` to request notification authorization from the user. Unlike most iOS permissions, `PostNotifications` does not require an `Info.plist` usage description string.


## Checking permissions

To check the current status of a permission, use the `Permissions.CheckStatusAsync%2A` method along with the specific permission to get the status for. The following example checks the status of the [[LocationWhenInUse|`LocationWhenInUse`]] permission:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="permission_check":::

A [[PermissionException|PermissionException]] is thrown if the required permission isn't declared.

It's best to check the status of the permission before requesting it. Each operating system returns a different default state, if the user has never been prompted. iOS returns [[PermissionStatus.Unknown|Unknown]], while others return [[PermissionStatus.Denied|Denied]]. If the status is [[PermissionStatus.Granted|Granted]] then there's no need to make other calls. On iOS if the status is [[PermissionStatus.Denied|Denied]] you should prompt the user to change the permission in the settings. On Android, you can call `ShouldShowRationale%2A` to detect if the user has already denied the permission in the past.

### Permission status

When using `CheckStatusAsync%2A` or `RequestAsync%2A`, a [[PermissionStatus|PermissionStatus]] is returned that can be used to determine the next steps:

- [[PermissionStatus.Unknown|Unknown]]\
The permission is in an unknown state, or on iOS, the user has never been prompted.

- [[PermissionStatus.Denied|Denied]]\
The user denied the permission request.

- [[PermissionStatus.Disabled|Disabled]]\
The feature is disabled on the device.

- [[PermissionStatus.Granted|Granted]]\
The user granted permission or is automatically granted.

- [[PermissionStatus.Restricted|Restricted]]\
In a restricted state.

- [[PermissionStatus.Limited|Limited]]\
In a limited state. Only iOS returns this status.

## Requesting permissions

To request a permission from the users, use the `RequestAsync%2A` method along with the specific permission to request. If the user previously granted permission, and hasn't revoked it, then this method will return [[PermissionStatus.Granted|Granted]] without showing a dialog to the user. Permissions shouldn't be requested from your `MauiProgram` or `App` class, and should only be requested once the first page of the app has appeared.

The following example requests the [[LocationWhenInUse|`LocationWhenInUse`]] permission:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="permission_request":::

A [[PermissionException|PermissionException]] is thrown if the required permission isn't declared.

> [!IMPORTANT]
> On some platforms, a permission request can only be activated a single time. Further prompts must be handled by the developer to check if a permission is in the [[PermissionStatus.Denied|Denied]] state, and then ask the user to manually turn it on.


### Requesting notification permissions

To request notification posting permissions on Android, iOS, and Mac Catalyst, use the [[PostNotifications|`PostNotifications`]] permission:

```csharp
var status = await Permissions.RequestAsync<Permissions.PostNotifications>();
```

On Android, this maps to the `POST_NOTIFICATIONS` manifest permission. On iOS and Mac Catalyst, this uses `UNUserNotificationCenter.RequestAuthorization` to request notification authorization from the user. If the user has previously denied the notification permission on iOS, the method returns [[PermissionStatus.Denied|Denied]] and the user must enable notifications manually through the Settings app.


## Explain why permission is needed

It's best practice to explain to your user why your application needs a specific permission. On iOS, you must specify a string that is displayed to the user. Android doesn't have this ability, and also defaults permission status to [[PermissionStatus.Disabled|Disabled]]. This limits the ability to know if the user denied the permission or if it's the first time the permission is being requested. The `ShouldShowRationale%2A` method can be used to determine if an informative UI should be displayed. If the method returns `true`, this is because the user has denied or disabled the permission in the past. Other platforms always return `false` when calling this method.

## Example

The following code presents the general usage pattern for determining whether a permission has been granted, and then requesting it if it hasn't.

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="permission_check_and_request":::

## Extending permissions

The Permissions API was created to be flexible and extensible for applications that require more validation or permissions that aren't included in .NET MAUI. Create a class that inherits from [[BasePermission|BasePermission]], and implement the required abstract methods. The following example code demonstrates the basic abstract members, but without implementation:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="permission_class":::

When implementing a permission in a specific platform, the [[BasePlatformPermission|BasePlatformPermission]] class can be inherited from. This class provides extra platform helper methods to automatically check the permission declarations. This helps when creating custom permissions that do groupings, for example requesting both **Read** and **Write** access to storage on Android. The following code example demonstrates requesting **Read** and **Write** storage access:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="permission_readwrite":::

You then check the permission in the same way as any other permission type provided by .NET MAUI:

:::code language="csharp" source="../snippets/shared_1/AppModelPage.xaml.cs" id="permission_readwrite_request":::

If you wanted to call this API from your cross-platform code, you could create an interface and register the custom permission as a dependency in the app's service container. The following example shows the `IReadWritePermission` interface:

```csharp
public interface IReadWritePermission
{        
    Task<PermissionStatus> CheckStatusAsync();
    Task<PermissionStatus> RequestAsync();
}
```

Then implement the interface in your custom permission:

```csharp
public class ReadWriteStoragePermission : Permissions.BasePlatformPermission, IReadWritePermission
{
    public override (string androidPermission, bool isRuntime)[] RequiredPermissions => new List<(string androidPermission, bool isRuntime)>
    {
        (Android.Manifest.Permission.ReadExternalStorage, true),
        (Android.Manifest.Permission.WriteExternalStorage, true)
    }.ToArray();
}
```

In the `MauiProgram` class you should then register the interface and its concrete type, and the type that will consume the custom permission, in the app's service container:

```csharp
builder.Services.AddTransient<MyViewModel>();
builder.Services.AddSingleton<IReadWritePermission, ReadWriteStoragePermission>();
```

The custom permission implementation can then be resolved and invoked from one of your types, such as a viewmodel:

```csharp
public class MyViewModel
{
    IReadWritePermission _readWritePermission;

    public MyViewModel(IReadWritePermission readWritePermission)
    {
        _readWritePermission = readWritePermission;
    }

    public async Task CheckPermissionAsync()
    {
        var status = await _readWritePermission.CheckStatusAsync();
        if (status != PermissionStatus.Granted)
        {
            status = await _readWritePermission.RequestAsync();
        }
    }
}
```

## Platform differences

This section describes the platform-specific differences with the permissions API.

<!-- markdownlint-disable MD025 -->
# [Android](#tab/android)

Permissions must have the matching attributes set in the Android Manifest file. Permission status defaults to [[PermissionStatus.Denied|Denied]].

<!-- TODO For more information, see [[permissions|Permissions in .NET MAUI for Android]]. -->

# [iOS/Mac Catalyst](#tab/macios)

Permissions must have a matching string in the _Info.plist_ file. Once a permission is requested and denied, a pop-up will no longer appear if you request the permission a second time. You must prompt your user to manually adjust the setting in the applications settings screen in iOS. Permission status defaults to [[PermissionStatus.Unknown|Unknown]].

<!-- TODO For more information, see [[security-privacy|iOS Security and Privacy Features]]. -->

# [Windows](#tab/windows)

No platform differences.

-----
<!-- markdownlint-enable MD025 -->
