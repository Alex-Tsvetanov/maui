---
title: "Publish Vs"
tags:
  - conceptual
  - area/ios
ms_date: "02/24/2023"
source: "https://learn.microsoft.com/dotnet/maui/ios/includes/publish-vs?view=net-maui-10.0"
---

1. In Visual Studio, ensure that the IDE is paired to a Mac Build host. For more information, see [[pair-to-mac|Pair to Mac for iOS development]].

1. In the Visual Studio toolbar, use the **Debug Target** drop-down to select **iOS Remote Devices** and then **Remote Device**:

    ![](../deployment/media/publish/vs/select-remote-device.png)

1. In the Visual Studio toolbar, use the **Solutions Configuration** drop-down to change from the debug configuration to the release configuration:

    ![](../deployment/media/publish/vs/release-configuration.png)

1. In **Solution Explorer**, right-click on your .NET MAUI app project and select **Properties**. Then, navigate to the **iOS Bundle Signing** tab and ensure that **Scheme** is set to **Manual Provisioning**, **Signing identity** is set to **Distribution (Automatic)**, and **Provisioning profile** is set to **Automatic**:

    ![](../deployment/media/publish/vs/bundle-signing.png)

    These settings will ensure that Visual Studio will select the correct provisioning profile based on the bundle identifier in *Info.plist* (which is identical to the value of the **Application ID** property in your project file). Alternatively, set the **Signing identity** to the appropriate distribution certificate and **Provisioning profile** to the profile you created in your Apple Developer Account.

1. In **Solution Explorer**, right-click on your .NET MAUI app project and select **Publish...**:

    ![](../deployment/media/publish/vs/publish-menu-item.png)

    The **Archive Manager** will open and Visual Studio will begin to archive your app bundle:

    ![](../deployment/media/publish/vs/archive-manager.png)

    The archiving process signs the app with the certificate and provisioning profiles that you specified in the **iOS Bundle Signing** tab, for the selected solution configuration.

1. In the **Archive Manager**, once archiving has successfully completed, ensure your archive is selected and then select the **Distribute ...** button to begin the process of packaging your app for distribution:

    ![](../deployment/media/publish/vs/archive-manager-distribute.png)

    The **Distribute - Select Channel** dialog will appear.
