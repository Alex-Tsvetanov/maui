---
title: "Upgrade from Xamarin to .NET"
description: "Learn how to upgrade Xamarin apps to .NET."
tags:
  - conceptual
  - area/migration
ms_date: "08/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/migration?view=net-maui-10.0"
---

# Upgrade from Xamarin to .NET

> [!IMPORTANT]
> [Microsoft support for Xamarin](https://dotnet.microsoft.com/platform/support/policy/xamarin) will end on May 1, 2024 for all Xamarin SDKs including Xamarin.Forms. Help us improve your upgrade experience from Xamarin to .NET MAUI by completing this [short survey](https://www.surveymonkey.com/r/VXH5TM7).

Xamarin projects can run on .NET after completing an upgrade process. The following table lists the Xamarin project types that can be upgraded to .NET:

| Project type            | Upgrade | Guide                                             |
|-------------------------|---------|---------------------------------------------------|
| Xamarin.Android         | ✅       | [[native-projects|Upgrade Xamarin native projects]] |
| Xamarin.iOS             | ✅       | [[native-projects|Upgrade Xamarin native projects]] |
| Xamarin.Mac             | ✅       | [[native-projects|Upgrade Xamarin native projects]] |
| Xamarin.tvOS            | ✅       | [[native-projects|Upgrade Xamarin native projects]] |
| Xamarin.Forms           | ✅       | [[multi-project-to-multi-project|Upgrade a Xamarin.Forms app to a multi-project .NET MAUI app]] <br> [[multi-project-to-single-project|Upgrade a Xamarin.Forms app to a single project .NET MAUI app]] |
| Xamarin.Forms UWP       | ✅       | [[uwp-projects|Xamarin.Forms UWP project migration]] |
| iOS App Extensions      | ✅       | [[native-projects|Upgrade Xamarin native projects]] |
| Android Wear            | ✅       | [[native-projects|Upgrade Xamarin native projects]] |
| Android Binding Library | ✅       | [[android-binding-projects|Xamarin.Android binding project migration]] |
| iOS Binding Library     | ✅       | [[ios-binding-projects|Xamarin.iOS binding project migration]] |
| SpriteKit               | ✅       | [[native-projects|Upgrade Xamarin native projects]] |
| SceneKit                | ✅       | [[native-projects|Upgrade Xamarin native projects]] |
| Metal                   | ✅       | [[native-projects|Upgrade Xamarin native projects]] |
| OpenGL                  | ❌ (iOS) | Removed from iOS since OpenTK isn't available |
| Xamarin.watchOS         | ❌       | Recommendation: bundle Swift extensions with .NET for iOS apps |

<!-- markdownlint-disable MD032 -->
> [!IMPORTANT]
> To upgrade an app from Xamarin to .NET:
> - All projects **do** need to become SDK-style.
> - Projects **don't** need to be rewritten.
> - Multi-project solutions **don't** need to become a multi-targeted single project.
<!-- markdownlint-enable MD025 -->

To upgrade your Xamarin native projects to .NET, you'll first have to update the projects to be SDK-style projects and then update your dependencies to .NET 8. For more information, see [[native-projects|Upgrade Xamarin.Android, Xamarin.iOS, and Xamarin.Mac projects to .NET]].

The .NET Upgrade Assistant is a command-line tool that can help you upgrade multi-project Xamarin.Forms apps to multi-project .NET Multi-platform App UI (.NET MAUI) apps. After running the tool, in most cases the app will require additional effort to complete the upgrade. For more information, see [[upgrade-assistant|Upgrade a Xamarin.Forms app to a .NET MAUI app with the .NET Upgrade Assistant]].

You can also manually upgrade at Xamarin.Forms app to a multi-project .NET MAUI app with a two-step process:

1. Upgrade your Xamarin native projects, in your Xamarin.Forms solution, to .NET. For more information, see [[native-projects|Upgrade Xamarin.Android, Xamarin.iOS, and Xamarin.Mac apps to .NET]]. In addition, you can upgrade your Xamarin.Forms UWP project to a .NET MAUI WinUI 3 project. For more information, see [[uwp-projects|Xamarin.Forms UWP project migration]].
1. Upgrade your Xamarin.Forms library project to .NET Multi-platform App UI (.NET MAUI). For more information, see [[multi-project-to-multi-project|Manually upgrade a Xamarin.Forms app to a multi-project .NET MAUI app]].

Alternatively, you can manually upgrade a Xamarin.Forms app to a single-project .NET MAUI app. For more information, see [[multi-project-to-single-project|Manually upgrade a Xamarin.Forms app to a single project .NET MAUI app]].
