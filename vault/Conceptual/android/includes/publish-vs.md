---
title: "Publish Vs"
tags:
  - conceptual
  - area/android
ms_date: "05/09/2023"
source: "https://learn.microsoft.com/dotnet/maui/android/includes/publish-vs?view=net-maui-10.0"
---

To build and sign your app in Visual Studio:

1. In the Visual Studio toolbar, use the **Debug Target** drop-down to select **Android Emulators** and then your chosen emulator:

    ![](../deployment/media/publish/vs/select-android-deployment.png)

1. In the Visual Studio toolbar, use the **Solutions Configuration** drop-down to change from the debug configuration to the release configuration:

    ![](../deployment/media/publish/vs/release-configuration.png)

1. In **Solution Explorer**, right-click on your .NET MAUI app project and select **Publish...**:

    ![](../deployment/media/publish/vs/publish-menu-item.png)

    The **Archive Manager** will open and Visual Studio will begin to archive your app bundle:

    ![](../deployment/media/publish/vs/archive-manager.png)

1. In the **Archive Manager**, once archiving has successfully completed, ensure your archive is selected and then select the **Distribute ...** button to begin the process of distributing your app:

    ![](../deployment/media/publish/vs/archive-manager-distribute.png)

    The **Distribute - Select Channel** dialog will appear.
