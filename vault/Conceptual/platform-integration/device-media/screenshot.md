---
title: "Screenshot"
description: "Learn how to use the IScreenshot interface in the Microsoft.Maui.Media namespace, to capture of the current displayed screen of the app."
tags:
  - conceptual
  - area/platform-integration
ms_date: "02/02/2023"
source: "https://learn.microsoft.com/dotnet/maui/platform-integration/device-media/screenshot?view=net-maui-10.0"
---

# Screenshot

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/platformintegration-essentials)

This article describes how you can use the .NET Multi-platform App UI (.NET MAUI) [[IScreenshot|IScreenshot]] interface. This interface lets you take a capture of the current displayed screen of the app.

The default implementation of the `IScreenshot` interface is available through the [[Screenshot.Default|Screenshot.Default]] property. Both the `IScreenshot` interface and `Screenshot` class are contained in the `Microsoft.Maui.Media` namespace.

## Capture a screenshot

To capture a screenshot of the current app, use the [[IScreenshot.CaptureAsync|CaptureAsync]] method. This method returns a [[IScreenshotResult|IScreenshotResult]], which contains information about the capture, such as the width and height of the screenshot. The following example demonstrates a method that captures a screenshot and returns it as an `ImageSource`.

:::code language="csharp" source="../snippets/shared_1/MediaPage.cs" id="screenshot":::
