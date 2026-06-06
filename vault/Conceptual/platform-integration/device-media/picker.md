---
title: "Media picker for photos and videos"
description: "Learn how to use the IMediaPicker interface in the Microsoft.Maui.Media namespace, to prompt the user to select or take a photo or video"
tags:
  - conceptual
  - area/platform-integration
ms_date: "12/16/2024"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/device-media/picker?view=net-maui-10.0"
---

# Media picker for photos and videos

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[IMediaPicker|IMediaPicker]] interface. This interface lets a user pick or take a photo or video on the device.

The default implementation of the `IMediaPicker` interface is available through the [[MediaPicker.Default|MediaPicker.Default]] property. Both the `IMediaPicker` interface and `MediaPicker` class are contained in the `Microsoft.Maui.Media` namespace.

## Get started

To access the media picker functionality, the following platform-specific setup is required.

<!-- markdownlint-disable MD025 -->
# [Android](#tab/android)

The `CAMERA` permission is required and must be configured in the Android project. In addition:

- If your app targets Android 12 or lower, you must request the `READ_EXTERNAL_STORAGE` and `WRITE_EXTERNAL_STORAGE` permissions.
- If your app targets Android 13 or higher and needs access to media files that other apps have created, you must request one or more of the following granular media permissions instead of the `READ_EXTERNAL_STORAGE` permission:

  - `READ_MEDIA_IMAGES`
  - `READ_MEDIA_VIDEO`
  - `READ_MEDIA_AUDIO`

These permissions can be added in the following ways:

- Add the assembly-based permissions:

  Open the _Platforms/Android/MainApplication.cs_ file and add the following assembly attributes after `using` directives:

  :::code language="csharp" source="../snippets/shared_1/Platforms/Android/MainApplication.cs" id="media_picker":::

  \- or -

- Update the Android Manifest:

  Open the _Platforms/Android/AndroidManifest.xml_ file and add the following in the `manifest` node:

  ```xml
  <!-- Needed for Picking photo/video -->
  <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" android:maxSdkVersion="32" />
  <uses-permission android:name="android.permission.READ_MEDIA_AUDIO" />
  <uses-permission android:name="android.permission.READ_MEDIA_IMAGES" />
  <uses-permission android:name="android.permission.READ_MEDIA_VIDEO" />

  <!-- Needed for Taking photo/video -->
  <uses-permission android:name="android.permission.CAMERA" />
  <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" android:maxSdkVersion="32" />

  <!-- Add these properties if you would like to filter out devices that do not have cameras, or set to false to make them optional -->
  <uses-feature android:name="android.hardware.camera" android:required="true" />
  <uses-feature android:name="android.hardware.camera.autofocus" android:required="true" />
  ```

  \- or -

- Update the Android Manifest in the manifest editor:

  In Visual Studio double-click on the *Platforms/Android/AndroidManifest.xml* file to open the Android manifest editor. Then, under **Required permissions** check the permissions listed above. This will automatically update the *AndroidManifest.xml* file.

If your project's Target Android version is set to **Android 11 (R API 30)** or higher, you must update your _Android Manifest_ with queries that use Android's [package visibility requirements](https://developer.android.com/preview/privacy/package-visibility).

In the _Platforms/Android/AndroidManifest.xml_ file, add the following `queries/intent` nodes in the `manifest` node:

```xml
<queries>
  <intent>
    <action android:name="android.media.action.IMAGE_CAPTURE" />
  </intent>
</queries>
```

# [iOS/Mac Catalyst](#tab/macios)

In the _Platforms/iOS/Info.plist_ and _Platforms/MacCatalyst/Info.plist_ files, add the following keys and values:

```xml
<key>NSCameraUsageDescription</key>
<string>This app needs access to the camera to take photos.</string>
<key>NSMicrophoneUsageDescription</key>
<string>This app needs access to microphone for taking videos.</string>
<key>NSPhotoLibraryAddUsageDescription</key>
<string>This app needs access to the photo gallery for picking photos and videos.</string>
<key>NSPhotoLibraryUsageDescription</key>
<string>This app needs access to photos gallery for picking photos and videos.</string>
```

Each `<string>` element represents the reason the app is requesting access to that specific permission. This text is shown to the user.

# [Windows](#tab/windows)

No setup is required.

-----
<!-- markdownlint-enable MD025 -->

## Using media picker


The [[IMediaPicker|IMediaPicker]] interface has the following methods that return a [[FileResult|FileResult]], which can be used to get the file's location or read it.

- `PickPhotoAsync%2A` \
Opens the media browser to select a photo.

- `CapturePhotoAsync%2A` \
Opens the camera to take a photo.

- `PickVideoAsync%2A` \
Opens the media browser to select a video.

- `CaptureVideoAsync%2A` \
Opens the camera to take a video.

Each method optionally takes a [[MediaPickerOptions|MediaPickerOptions]] parameter that allows the [[MediaPickerOptions.Title|Title]] to be set on some operating systems, which is displayed to the user.



In .NET 10, the media picker adds multi-select support and new processing options. Use the following methods:

- `PickPhotosAsync%2A` (returns `List<FileResult>`) \
Opens the media browser to select one or more photos.

- `CapturePhotoAsync%2A` (returns `FileResult?`) \
Opens the camera to take a photo.

- `PickVideosAsync%2A` (returns `List<FileResult>`) \
Opens the media browser to select one or more videos.

- `CaptureVideoAsync%2A` (returns `FileResult?`) \
Opens the camera to take a video.

The [[MediaPickerOptions|MediaPickerOptions]] parameter exposes additional fields such as [[MediaPickerOptions.SelectionLimit|SelectionLimit]], [[MediaPickerOptions.MaximumWidth|MaximumWidth]], [[MediaPickerOptions.MaximumHeight|MaximumHeight]], [[MediaPickerOptions.CompressionQuality|CompressionQuality]], [[MediaPickerOptions.RotateImage|RotateImage]], and [[MediaPickerOptions.PreserveMetaData|PreserveMetaData]].

> [!IMPORTANT]
> When the user cancels a multi-select operation, the returned list is empty. On Android, some picker UIs may not enforce [[MediaPickerOptions.SelectionLimit|SelectionLimit]]; on Windows, `SelectionLimit` isn't supported. Implement your own logic to enforce limits or notify the user on these platforms.

### Pick multiple photos

```csharp
var results = await MediaPicker.PickPhotosAsync(new MediaPickerOptions
{
  // Default is 1; set to 0 for no limit
  SelectionLimit = 10,
  // Optional processing for images
  MaximumWidth = 1024,
  MaximumHeight = 768,
  CompressionQuality = 85,
  RotateImage = true,
  PreserveMetaData = true,
});

foreach (var file in results)
{
  using var stream = await file.OpenReadAsync();
  // Process the stream
}
```

### Pick multiple videos

```csharp
var results = await MediaPicker.PickVideosAsync(new MediaPickerOptions
{
  SelectionLimit = 3,
  Title = "Select up to 3 videos",
});

foreach (var file in results)
{
  using var stream = await file.OpenReadAsync();
  // Process the stream
}
```

> [!TIP]
> For single selection, prefer `PickPhotosAsync`/`PickVideosAsync` as well. Set `SelectionLimit = 1` (the default) and read the first item if present.


> [!IMPORTANT]
> All methods must be called on the UI thread because permission checks and requests are automatically handled by .NET MAUI.

## Take a photo

Call the `CapturePhotoAsync%2A` method to open the camera and let the user take a photo. If the user takes a photo, the return value of the method will be a non-null value. The following code sample uses the media picker to take a photo and save it to the cache directory:

:::code language="csharp" source="../snippets/shared_1/MediaPage.cs" id="photo_take_and_save":::

![[tip-file-result]]
