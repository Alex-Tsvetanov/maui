---
title: "Download Profiles"
tags:
  - conceptual
  - area/ios
ms_date: "02/24/2023"
source: "https://learn.microsoft.com/dotnet/maui/ios/includes/download-profiles?view=net-maui-10.0"
---

## Download provisioning profiles in Visual Studio

After you create a distribution provisioning profile in your Apple Developer Account, Visual Studio can download it so that it's available for signing your app:

1. In Visual Studio, go to **Tools > Options > Xamarin > Apple Accounts**.
1. In the **Apple Developer Accounts** dialog, select your team and click **View Details**.
1. In the **Details** dialog, verify that the new profile appears in the **Provisioning Profiles** list. You may need to restart Visual Studio to refresh the list.
1. In the **Details** dialog, click **Download All Profiles**.

The provisioning profiles are downloaded on Windows, and exported to your Mac build host if the IDE is paired to it. For more information, see [[pair-to-mac|Pair to Mac for iOS development]].
