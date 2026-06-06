---
title: "Publish Ad Hoc"
tags:
  - conceptual
  - area/android
ms_date: "05/12/2023"
source: "https://learn.microsoft.com/dotnet/maui/android/includes/publish-ad-hoc?view=net-maui-10.0"
---

<!-- markdownlint-disable MD029 -->
5. In the **Distribute - Select Channel** dialog, select the **Ad Hoc** button:

    ![](../deployment/media/publish/vs/distribution-select-channel-ad-hoc.png)
    <!-- markdownlint-enable MD029 -->

1. In the **Distribute - Signing Identity** dialog, select the **+** button to create a new signing identity:

    ![](../deployment/media/publish/vs/create-new-ad-hoc-signing-identity.png)

    The **Create Android Keystore** dialog will appear.

    > [!NOTE]
    > Alternatively, an existing signing identity can be used by selecting the **Import** button.

1. In the **Create Android Keystore** dialog, enter the required information to create a new signing identity, known as a *keystore*, and then select the **Create** button:

    - Alias. Enter an identifying name for your key.
    - Password. Create and confirm a secure password for your key.
    - Validity. Set the length of time, in years, that your key will be valid.
    - Full name, organization unit, organization, city or locality, state or province, and country code. This information is not displayed in your app, but is included in your certificate.

    ![](../deployment/media/publish/vs/create-android-keystore.png)

    A new keystore, which contains a new certificate, will be saved to **C:\Users\{Username}\AppData\Local\Xamarin\Mono for Android\Keystore\{Alias}\{Alias}.keystore**.

    > [!IMPORTANT]
    > The keystore and password isn't saved to your Visual Studio solution. Therefore, ensure you back up this data. If you lose it you'll be unable to sign your app with the same signing identity.  

1. In the **Distribute - Signing Identity** dialog, select your newly created signing identity and select the **Save As** button:

    ![](../deployment/media/publish/vs/save-ad-hoc.png)

    The *Archive Manager* displays the publishing process.

1. In the **Save As** dialog, confirm the location and file name for your package is correct and select the **Save** button.
1. In the **Signing Password** dialog, enter your signing identity password and select the **OK** button:

    ![](../deployment/media/publish/vs/keystore-password.png)

1. In the *Archive Manager*, select the **Open Distribution** button once the publishing process completes:

    ![](../deployment/media/publish/vs/ad-hoc-open-distribution.png)

    Visual Studio will open the folder containing the published app.
