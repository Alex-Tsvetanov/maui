---
title: "Microsoft.Maui.LifecycleEvents"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-LifecycleEvents
---

# Microsoft.Maui.LifecycleEvents

> [!info] Namespace
> `Microsoft.Maui.LifecycleEvents` — 95 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.lifecycleevents)

## Overview

`Microsoft.Maui.LifecycleEvents` provides the API surface a .NET MAUI app uses to hook into the native platform lifecycle of each operating system it runs on. Because Android, iOS/Mac Catalyst, Windows, and Tizen each expose their own lifecycle (activity callbacks, app delegate messages, scene events, window events, and so on), this namespace gives a single, strongly-typed place to register handlers for those moments without dropping down to platform-specific entry points by hand.

The configuration entry point is the lifecycle builder, surfaced through [[ILifecycleBuilder|ILifecycleBuilder]] and the per-platform builders [[IAndroidLifecycleBuilder|IAndroidLifecycleBuilder]], [[IiOSLifecycleBuilder|IiOSLifecycleBuilder]], [[IWindowsLifecycleBuilder|IWindowsLifecycleBuilder]], and [[ITizenLifecycleBuilder|ITizenLifecycleBuilder]]. You typically reach these through `ConfigureLifecycleEvents` on the app host builder (see [[MauiAppHostBuilderExtensions|MauiAppHostBuilderExtensions]] and the platform builder extensions), then call into per-platform groups exposed by [[AndroidLifecycle|AndroidLifecycle]], [[iOSLifecycle|iOSLifecycle]], [[WindowsLifecycle|WindowsLifecycle]], and [[TizenLifecycle|TizenLifecycle]].

Registered handlers are described as named delegate types — for example [[OnCreate (Microsoft.Maui.LifecycleEvents)|OnCreate]], [[OnResumed|OnResumed]], [[OnStop|OnStop]], [[FinishedLaunching|FinishedLaunching]], [[OnLaunched|OnLaunched]], and the iOS scene events such as [[SceneWillConnect|SceneWillConnect]]. At runtime the registered events are tracked and dispatched through the lifecycle event service ([[ILifecycleEventService|ILifecycleEventService]] / [[LifecycleEventService|LifecycleEventService]]), which the framework invokes at the corresponding native lifecycle stage. This lets cross-platform code respond to startup, foreground/background transitions, configuration changes, memory pressure, and input/URL activation in a consistent way.

## Key types

- [[ILifecycleBuilder|ILifecycleBuilder]] — root builder for registering lifecycle event handlers across platforms.
- [[IAndroidLifecycleBuilder|IAndroidLifecycleBuilder]] — registers handlers for Android activity and application lifecycle events.
- [[IiOSLifecycleBuilder|IiOSLifecycleBuilder]] — registers handlers for iOS/Mac Catalyst app delegate and scene lifecycle events.
- [[IWindowsLifecycleBuilder|IWindowsLifecycleBuilder]] — registers handlers for Windows app and window lifecycle events.
- [[ITizenLifecycleBuilder|ITizenLifecycleBuilder]] — registers handlers for Tizen application lifecycle events.
- [[ILifecycleEventService|ILifecycleEventService]] — runtime service that stores and dispatches registered lifecycle events.
- [[LifecycleEventService|LifecycleEventService]] — default implementation of the lifecycle event service.
- [[MauiAppHostBuilderExtensions|MauiAppHostBuilderExtensions]] — `ConfigureLifecycleEvents` host builder extension that wires lifecycle handlers into the app.
- [[AndroidLifecycle|AndroidLifecycle]] — container of Android lifecycle event delegate types (e.g. create, resume, pause, destroy).
- [[iOSLifecycle|iOSLifecycle]] — container of iOS/Mac Catalyst lifecycle and scene event delegate types.
- [[WindowsLifecycle|WindowsLifecycle]] — container of Windows lifecycle event delegate types.
- [[LifecycleEventRegistration|LifecycleEventRegistration]] — represents a single registered lifecycle event handler.


## Classes

| Type | Summary |
|---|---|
| [[AndroidLifecycle\|AndroidLifecycle]] |  |
| [[AndroidLifecycleBuilderExtensions\|AndroidLifecycleBuilderExtensions]] |  |
| [[AndroidLifecycleExtensions\|AndroidLifecycleExtensions]] |  |
| [[AppHostBuilderExtensions (LifecycleEvents)\|AppHostBuilderExtensions (LifecycleEvents)]] |  |
| [[ApplicationSignificantTimeChange\|ApplicationSignificantTimeChange]] |  |
| [[ContinueUserActivity\|ContinueUserActivity]] |  |
| [[DidEnterBackground\|DidEnterBackground]] |  |
| [[FinishedLaunching\|FinishedLaunching]] |  |
| [[IiOSLifecycleBuilder\|IiOSLifecycleBuilder]] |  |
| [[LifecycleBuilderExtensions\|LifecycleBuilderExtensions]] |  |
| [[LifecycleEventRegistration\|LifecycleEventRegistration]] |  |
| [[LifecycleEventService\|LifecycleEventService]] |  |
| [[LifecycleEventServiceExtensions\|LifecycleEventServiceExtensions]] |  |
| [[MauiAppHostBuilderExtensions\|MauiAppHostBuilderExtensions]] |  |
| [[OnActivated (LifecycleEvents)\|OnActivated (LifecycleEvents)]] |  |
| [[OnActivated (Microsoft.Maui.LifecycleEvents)\|OnActivated (Microsoft.Maui.LifecycleEvents)]] |  |
| [[OnActivityResult\|OnActivityResult]] |  |
| [[OnAppControlReceived\|OnAppControlReceived]] |  |
| [[OnApplicationConfigurationChanged\|OnApplicationConfigurationChanged]] |  |
| [[OnApplicationCreate\|OnApplicationCreate]] |  |
| [[OnApplicationCreating\|OnApplicationCreating]] |  |
| [[OnApplicationLowMemory\|OnApplicationLowMemory]] |  |
| [[OnApplicationTrimMemory\|OnApplicationTrimMemory]] |  |
| [[OnBackPressed\|OnBackPressed]] |  |
| [[OnClosed\|OnClosed]] |  |
| [[OnConfigurationChanged\|OnConfigurationChanged]] |  |
| [[OnCreate (LifecycleEvents)\|OnCreate (LifecycleEvents)]] |  |
| [[OnCreate (Microsoft.Maui.LifecycleEvents)\|OnCreate (Microsoft.Maui.LifecycleEvents)]] |  |
| [[OnDestroy\|OnDestroy]] |  |
| [[OnDeviceOrientationChanged\|OnDeviceOrientationChanged]] |  |
| [[OnKeyDown\|OnKeyDown]] |  |
| [[OnKeyLongPress\|OnKeyLongPress]] |  |
| [[OnKeyMultiple\|OnKeyMultiple]] |  |
| [[OnKeyShortcut\|OnKeyShortcut]] |  |
| [[OnKeyUp\|OnKeyUp]] |  |
| [[OnLaunched\|OnLaunched]] |  |
| [[OnLaunching\|OnLaunching]] |  |
| [[OnLocaleChanged\|OnLocaleChanged]] |  |
| [[OnLowBattery\|OnLowBattery]] |  |
| [[OnLowMemory\|OnLowMemory]] |  |
| [[OnNewIntent\|OnNewIntent]] |  |
| [[OnPause (LifecycleEvents)\|OnPause (LifecycleEvents)]] |  |
| [[OnPause (Microsoft.Maui.LifecycleEvents)\|OnPause (Microsoft.Maui.LifecycleEvents)]] |  |
| [[OnPlatformMessage\|OnPlatformMessage]] |  |
| [[OnPlatformWindowSubclassed\|OnPlatformWindowSubclassed]] |  |
| [[OnPostCreate\|OnPostCreate]] |  |
| [[OnPostResume\|OnPostResume]] |  |
| [[OnPreCreate\|OnPreCreate]] |  |
| [[OnRegionFormatChanged\|OnRegionFormatChanged]] |  |
| [[OnRequestPermissionsResult\|OnRequestPermissionsResult]] |  |
| [[OnResignActivation\|OnResignActivation]] |  |
| [[OnRestart\|OnRestart]] |  |
| [[OnRestoreInstanceState\|OnRestoreInstanceState]] |  |
| [[OnResume (LifecycleEvents)\|OnResume (LifecycleEvents)]] |  |
| [[OnResume (Microsoft.Maui.LifecycleEvents)\|OnResume (Microsoft.Maui.LifecycleEvents)]] |  |
| [[OnResumed\|OnResumed]] |  |
| [[OnSaveInstanceState\|OnSaveInstanceState]] |  |
| [[OnStart\|OnStart]] |  |
| [[OnStop\|OnStop]] |  |
| [[OnTerminate\|OnTerminate]] |  |
| [[OnVisibilityChanged\|OnVisibilityChanged]] |  |
| [[OnWindowCreated\|OnWindowCreated]] |  |
| [[OpenUrl\|OpenUrl]] |  |
| [[PerformActionForShortcutItem\|PerformActionForShortcutItem]] |  |
| [[PerformFetch\|PerformFetch]] |  |
| [[SceneContinueUserActivity\|SceneContinueUserActivity]] |  |
| [[SceneDidDisconnect\|SceneDidDisconnect]] |  |
| [[SceneDidEnterBackground\|SceneDidEnterBackground]] |  |
| [[SceneDidFailToContinueUserActivity\|SceneDidFailToContinueUserActivity]] |  |
| [[SceneDidUpdateUserActivity\|SceneDidUpdateUserActivity]] |  |
| [[SceneOnActivated\|SceneOnActivated]] |  |
| [[SceneOnResignActivation\|SceneOnResignActivation]] |  |
| [[SceneOpenUrl\|SceneOpenUrl]] |  |
| [[SceneRestoreInteractionState\|SceneRestoreInteractionState]] |  |
| [[SceneWillConnect\|SceneWillConnect]] |  |
| [[SceneWillContinueUserActivity\|SceneWillContinueUserActivity]] |  |
| [[SceneWillEnterForeground\|SceneWillEnterForeground]] |  |
| [[TizenLifecycle\|TizenLifecycle]] |  |
| [[TizenLifecycleBuilderExtensions\|TizenLifecycleBuilderExtensions]] |  |
| [[TizenLifecycleExtensions\|TizenLifecycleExtensions]] |  |
| [[WillEnterForeground\|WillEnterForeground]] |  |
| [[WillFinishLaunching\|WillFinishLaunching]] |  |
| [[WillTerminate\|WillTerminate]] |  |
| [[WindowSceneDidUpdateCoordinateSpace\|WindowSceneDidUpdateCoordinateSpace]] |  |
| [[WindowsLifecycle\|WindowsLifecycle]] |  |
| [[WindowsLifecycleBuilderExtensions\|WindowsLifecycleBuilderExtensions]] |  |
| [[WindowsLifecycleExtensions\|WindowsLifecycleExtensions]] |  |
| [[iOSLifecycle\|iOSLifecycle]] |  |
| [[iOSLifecycleBuilderExtensions\|iOSLifecycleBuilderExtensions]] |  |
| [[iOSLifecycleExtensions\|iOSLifecycleExtensions]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IAndroidLifecycleBuilder\|IAndroidLifecycleBuilder]] |  |
| [[ILifecycleBuilder\|ILifecycleBuilder]] |  |
| [[ILifecycleEventService\|ILifecycleEventService]] |  |
| [[ITizenLifecycleBuilder\|ITizenLifecycleBuilder]] |  |
| [[IWindowsLifecycleBuilder\|IWindowsLifecycleBuilder]] |  |

## See also

- [[_API Reference]]
