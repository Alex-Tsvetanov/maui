---
title: "Publish a .NET MAUI iOS app for App Store distribution"
description: "Learn how to provision and publish a .NET MAUI iOS app for App Store distribution."
tags:
  - conceptual
  - area/ios
ms_date: "08/27/2024"
source: "https://learn.microsoft.com/dotnet/maui/ios/deployment/publish-app-store?view=net-maui-10.0"
---

# Publish an iOS app for App Store distribution

> [!div class="op_single_selector"]
>
> - [[publish-in-house|Publish for in-house distribution]]
> - [[publish-ad-hoc|Publish for ad-hoc distribution]]
> - [[publish-cli|Publish using the command line]]

The most common approach to distributing iOS apps to users is through the App Store. Apps are submitted to the App Store through an online tool called *App Store Connect*. Only developers who belong to the Apple Developer Program have access to this tool. Members of the Apple Developer Enterprise Program do not have access. All apps submitted to the App Store require approval from Apple.

Distributing an iOS app requires that the app is provisioned using a *provisioning profile*. Provisioning profiles are files that contain code signing information, as well as the identity of the app and its intended distribution mechanism.

To distribute a .NET Multi-platform App UI (.NET MAUI) iOS app, you'll need to build a *distribution provisioning profile* specific to it. This profile enables the app to be digitally signed for release so that it can be installed on an iOS device. A distribution provisioning profile contains an App ID and a distribution certificate. You can use the same App ID that you used when deploying your app to a device for testing. However, you'll need to create a distribution certificate to identify yourself or your organization, if you don't already have one.

The process for creating an App Store distribution provisioning profile is as follows:

1. Create a distribution certificate. For more information, see [Create a distribution certificate](#create-a-distribution-certificate).
1. Create an App ID. For more information, see [Create an App ID](#create-an-app-id).
1. Create a provisioning profile. For more information, see [Create a provisioning profile](#create-a-provisioning-profile).

> [!IMPORTANT]
> Apple has a privacy policy for apps that target iOS on the App Store. It requires the app to include a privacy manifest file in the app bundle, that lists the types of data your .NET MAUI app or any third-party SDKs and packages collect, and the reasons for using any required reason APIs. If your use of the required reason APIs, or third-party SDKs, isn’t declared in the privacy manifest, your app might be rejected by the App Store. For more information, see [[privacy-manifest|Apple privacy manifest]].

![[distribution-certificate]]

## Create a distribution profile

A distribution provisioning profile enables your .NET MAUI iOS app to be digitally signed for release, so that it can be installed on an iOS device. A distribution provisioning profile contains an App ID and a distribution certificate.

![[app-id]]

### Create a provisioning profile

Once the App ID has been created, you should create a distribution provisioning profile. This profile enables the app to be digitally signed for release so that it can be installed on an iOS device.

To create a provisioning profile for App Store distribution:

1. In the Certificates, Identifiers & Profiles page of your Apple Developer Account, select the **Profiles** tab.
1. In the **Profiles** tab, click the **+** button to create a new profile.
1. In the **Register a New Provisioning Profile** page, select the **App Store** radio button before clicking the **Continue** button:

    ![](media/publish/appstore-register-provisioning-profile.png)

1. In the **Generate a Provisioning Profile** page, in the **App ID** drop-down, select the App ID that you previously created before clicking the **Continue** button:

    ![](media/publish/select-app-id.png)

1. In the **Generate a Provisioning Profile** page, select the radio button that corresponds to your distribution certificate before clicking the **Continue** button:

    ![](media/publish/appstore-select-certificate.png)

1. In the **Generate a Provisioning Profile** page, enter a name for the provisioning profile before clicking the **Generate** button:

    ![](media/publish/appstore-generate-profile.png)

    > [!NOTE]
    > Make a note of the provisioning profile name, as it will be required when signing your app.

1. In the **Generate a Provisioning Profile** page, optionally click the **Download** button to download your provisioning profile.

    > [!NOTE]
    > It's not necessary to download your provisioning profile now. Instead, you will do this in Visual Studio.

![[download-profiles]]

## Publish the app

Visual Studio can publish a .NET MAUI iOS app for App Store distribution and upload it to the App Store. However, before you can upload an app to the App Store you must:

- Create a record for the app you want to upload to the App Store in App Store Connect. This record includes all the information about the app as it will appear in the App Store and all of the information needed to manage the app through the distribution process. For more information, see [Create an app record](https://developer.apple.com/help/app-store-connect/create-an-app-record/add-a-new-app) on developer.apple.com.
- Create an app-specific password. For information about generating an app-specific password, see [Sign in to apps with your Apple ID using app-specific passwords](https://support.apple.com/HT204397) on support.apple.com.

To publish a .NET MAUI iOS app for App Store distribution:

![[publish-vs]]

<!-- markdownlint-disable MD029 -->
7. In the **Distribute - Select Channel** dialog, select the **App Store** button:

    ![](media/publish/vs/distribution-select-channel-app-store.png)
    <!-- markdownlint-enable MD029 -->

1. In the **Distribute - Signing Identity** dialog, select your signing identity and provisioning profile:

    ![](media/publish/vs/distribution-signing-identity-app-store.png)

    > [!NOTE]
    > You should use the signing identity and provisioning profile that were created for your app and the selected distribution channel.

1. In the **Distribute - Signing Identity** dialog, select either the **Save As** button or the **Upload to Store** button. The **Save As** button will enable you to enter a filename, before re-signing your app and publishing it to an *.ipa* file on your file system for later upload to the App Store through an app such as [Transporter](https://apps.apple.com/us/app/transporter/id1450874784?mt=12). The **Upload to Store** button will re-sign your app and publish it to an *.ipa* file, before uploading it to the App Store.

    1. If you select the **Upload to Store** button, the **Upload to store credentials** dialog will appear after Visual Studio has created the *.ipa* file. In the **Upload to store credentials** dialog enter your Apple ID and app-specific password and select the **OK** button:

        ![](media/publish/vs/upload-to-app-store.png)

      Visual Studio will validate your app package and upload it to the App Store, provided you've created an application record in App Store Connect.

For information about publishing an iOS app using the Command Line Interface (CLI), see [[publish-cli|Publish an iOS app using the command line]].

## Troubleshoot

[Transporter](https://apps.apple.com/us/app/transporter/id1450874784?mt=12) can be used to help identify errors with app packages that stop successful submission to the App Store.
