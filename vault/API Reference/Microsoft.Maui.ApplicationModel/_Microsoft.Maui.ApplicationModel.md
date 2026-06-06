---
title: "Microsoft.Maui.ApplicationModel"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-ApplicationModel
---

# Microsoft.Maui.ApplicationModel

> [!info] Namespace
> `Microsoft.Maui.ApplicationModel` — 74 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel)

## Overview

`Microsoft.Maui.ApplicationModel` is the cross-platform surface for interacting with the host application and the device platform around it. It groups together the everyday "app-level" services a .NET MAUI app needs: querying app metadata, responding to the operating system, requesting runtime permissions, and launching out to other apps and system experiences. Rather than writing per-platform code, you use these abstractions and MAUI maps them to the underlying Android, iOS, Windows, and Mac Catalyst APIs.

Several distinct concerns live here. App identity and lifecycle are covered by [[IAppInfo|AppInfo]], [[IVersionTracking|VersionTracking]], and the activity/window managers [[IActivityStateManager|ActivityStateManager]] and [[IWindowStateManager|WindowStateManager]]. UI-thread marshalling is handled by [[MainThread]], which lets you run code on the main thread and check whether you are already on it. App icon shortcuts are modeled by [[IAppActions|AppActions]] and [[AppAction]].

The largest cluster is permissions. [[BasePermission]] (and its platform counterpart [[BasePlatformPermission]]) is the abstract base for a family of concrete permission types — [[Camera]], [[LocationWhenInUse]], [[LocationAlways]], [[Bluetooth]], [[ContactsRead]], [[CalendarRead]], and many more — that you check and request before touching protected device capabilities. Finally, "open something" helpers like [[ILauncher|Launcher]], [[IBrowser|Browser]], and [[IMap (ApplicationModel)|Map]] launch URIs, web pages, and the installed maps app. Failures surface through [[FeatureNotSupportedException]] and [[FeatureNotEnabledException]].

## Key types

- [[IAppInfo|AppInfo]] — Information about the application, such as package name/identifier, name, and version.
- [[MainThread]] — Run code on the application's main thread and determine whether the current code is already executing on it.
- [[IAppActions|AppActions]] — Create and respond to app shortcuts launched from the app icon.
- [[ILauncher|Launcher]] — Open a URI with the system, often used for deep linking into another app's custom scheme.
- [[IBrowser|Browser]] — Display a web page inside the app or in the system browser.
- [[IMap (ApplicationModel)|Map]] — Open the installed maps application to a specific location or placemark.
- [[IVersionTracking|VersionTracking]] — Track an app's version and build history on a device.
- [[BasePermission|Permissions]] — Abstract base for the permission types used to check and request access to protected capabilities.
- [[Camera]] — Permission to access the device camera.
- [[LocationWhenInUse|Location]] — Permission to access device location while the app is in use.
- [[IActivityStateManager|ActivityStateManager]] — Track and respond to platform activity state changes.
- [[FeatureNotSupportedException]] — Thrown when a feature is used on a platform that does not support it.


## Classes

| Type | Summary |
|---|---|
| [[ActivityStateChangedEventArgs\|ActivityStateChangedEventArgs]] | The activity is created. |
| [[ActivityStateManager\|ActivityStateManager]] | Initializes the `ActivityStateManager` for the given `Application`. |
| [[AppAction\|AppAction]] | The `AppAction` class lets you create and respond to app shortcuts from the app icon. |
| [[AppActionEventArgs\|AppActionEventArgs]] | Event arguments containing data that is used when the app started through an `AppAction`. |
| [[AppActions\|AppActions]] | The lifecycle event that is triggered when this app is launched. |
| [[AppActionsExtensions\|AppActionsExtensions]] | Supporting extension methods for the AppActions API. |
| [[AppInfo\|AppInfo]] | Gets the application package name or identifier. |
| [[BasePermission\|BasePermission]] | Represents the abstract base class for all permissions. |
| [[BasePlatformPermission\|BasePlatformPermission]] | Represents the platform-specific abstract base class for all permissions on this platform. |
| [[Battery (ApplicationModel)\|Battery (ApplicationModel)]] | Represents permission to access the device battery information. |
| [[Bluetooth\|Bluetooth]] | Represents permission to communicate via Bluetooth (scanning, connecting and/or advertising). |
| [[Browser\|Browser]] | Open the browser to specified URI. |
| [[BrowserExtensions\|BrowserExtensions]] | This class contains static extension methods for use with `IBrowser`. |
| [[BrowserLaunchOptions\|BrowserLaunchOptions]] | Optional setting to open the browser with. |
| [[CalendarRead\|CalendarRead]] | Represents permission to read the device calendar information. |
| [[CalendarWrite\|CalendarWrite]] | Represents permission to write to the device calendar data. |
| [[Camera\|Camera]] | Represents permission to access the device camera. |
| [[ContactsRead\|ContactsRead]] | Represents permission to read the device contacts information. |
| [[ContactsWrite\|ContactsWrite]] | Represents permission to write to the device contacts data. |
| [[EventPermissions\|EventPermissions]] | Represents permission to access events. |
| [[FeatureNotEnabledException\|FeatureNotEnabledException]] | Exception that occurs when an attempt is made to use a feature on a platform that does not have this feature enabled. |
| [[FeatureNotSupportedException\|FeatureNotSupportedException]] | Exception that occurs when an attempt is made to use a feature on a platform that does not support it. |
| [[Flashlight (ApplicationModel)\|Flashlight (ApplicationModel)]] | Represents permission to access the device flashlight. |
| [[Intent\|Intent]] | A static class that contains Android specific information about `Intent`. |
| [[LaunchApp\|LaunchApp]] | Represents permission to launch other apps on the device. |
| [[Launcher\|Launcher]] | Queries if the device supports opening the given URI scheme. |
| [[LauncherExtensions\|LauncherExtensions]] | Static class with extension methods for the `ILauncher` APIs. |
| [[LocationAlways\|LocationAlways]] | Represents permission to access the device location, always. |
| [[LocationWhenInUse\|LocationWhenInUse]] | Represents permission to access the device location, only when the app is in use. |
| [[MainThread\|MainThread]] | The MainThread class allows applications to run code on the main thread of execution, and to determine if a particular block of code is currently running on … |
| [[Map (ApplicationModel)\|Map (ApplicationModel)]] | Open the installed application to a specific location with launch options. |
| [[MapExtensions (ApplicationModel)\|MapExtensions (ApplicationModel)]] | Static class with extension methods for the `IMap` APIs. |
| [[MapLaunchOptions\|MapLaunchOptions]] | Launch options for opening the installed map application. |
| [[Maps\|Maps]] | Represents permission to access the device maps application. |
| [[Media\|Media]] | Represents permission to access media from the device gallery. |
| [[Microphone\|Microphone]] | Represents permission to access the device microphone. |
| [[NearbyWifiDevices\|NearbyWifiDevices]] | Represents permission to access nearby WiFi devices. |
| [[NetworkState\|NetworkState]] | Represents permission to access the device network state information. |
| [[OpenFileRequest\|OpenFileRequest]] | Represents a request for opening a file in another application. |
| [[PermissionException\|PermissionException]] | Exception that occurs when calling an API that requires a specific permission. |
| [[PermissionResult\|PermissionResult]] |  |
| [[Permissions\|Permissions]] | The Permissions API provides the ability to check and request runtime permissions. |
| [[Phone\|Phone]] | Represents permission to access the device phone data. |
| [[Photos\|Photos]] | Represents permission to access photos from the device gallery. |
| [[PhotosAddOnly\|PhotosAddOnly]] | Represents permission to add photos (not read) to the device gallery. |
| [[Platform\|Platform]] | A static class that contains platform-specific helper methods. |
| [[PostNotifications\|PostNotifications]] | Represents permission to post notifications |
| [[Reminders\|Reminders]] | Represents permission to access the device reminders data. |
| [[Sensors\|Sensors]] | Represents permission to access the device sensors. |
| [[Sms (ApplicationModel)\|Sms (ApplicationModel)]] | Represents permission to access the device SMS data. |
| [[Speech\|Speech]] | Represents permission to access the device speech capabilities. |
| [[StorageRead\|StorageRead]] | Represents permission to read the device storage. |
| [[StorageWrite\|StorageWrite]] | Represents permission to write to the device storage. |
| [[VersionTracking\|VersionTracking]] | Starts tracking version information. |
| [[Vibrate\|Vibrate]] | Represents permission to access the device vibration motor. |
| [[WindowStateManager\|WindowStateManager]] | Occurs when the application's active window changed. |

## Interfaces

| Type | Summary |
|---|---|
| [[IActivityStateManager\|IActivityStateManager]] | Represents a manager object that can handle `Activity` states. |
| [[IAppActions\|IAppActions]] | The AppActions API lets you create and respond to app shortcuts from the app icon. |
| [[IAppInfo\|IAppInfo]] | Represents information about the application. |
| [[IBrowser\|IBrowser]] | Provides a way to display a web page inside an app. |
| [[ILauncher\|ILauncher]] | The Launcher API enables an application to open a URI by the system. This is often used when deep linking into another application's custom URI schemes. |
| [[IMap (ApplicationModel)\|IMap (ApplicationModel)]] | The Map API enables an application to open the installed map application to a specific location or placemark. |
| [[IPlatformAppActions\|IPlatformAppActions]] | Gets if app actions are supported on this device. |
| [[IVersionTracking\|IVersionTracking]] | The VersionTracking API provides an easy way to track an app's version on a device. |
| [[IWindowStateManager\|IWindowStateManager]] | Manager object that manages window states on Windows. |

## Enums

| Type | Summary |
|---|---|
| [[ActivityState\|ActivityState]] | Represents states that a `Activity` can have. |
| [[AppPackagingModel\|AppPackagingModel]] | Describes packaging options for a Windows app. |
| [[AppTheme\|AppTheme]] | Enumerates different themes an operating system or application can show. |
| [[BrowserLaunchFlags\|BrowserLaunchFlags]] | Additional flags that can be set to control how the browser opens. |
| [[BrowserLaunchMode\|BrowserLaunchMode]] | Launch type of the browser. |
| [[BrowserTitleMode\|BrowserTitleMode]] | Mode for the in-app browser title. |
| [[LayoutDirection\|LayoutDirection]] | Enumerates possible layout directions. |
| [[NavigationMode\|NavigationMode]] | Represents various modes of navigation that can be passed to the operating system's Maps app. |
| [[PermissionStatus\|PermissionStatus]] | Possible statuses of a permission. |

## See also

- [[_API Reference]]
