---
title: "Publishing Output"
tags:
  - conceptual
  - area/mac-catalyst
ms_date: "03/23/2023"
source: "https://learn.microsoft.com/dotnet/maui/mac-catalyst/includes/publishing-output?view=net-maui-10.0"
---

Publishing builds, signs, and packages the app, and then copies the *.pkg* to the *bin/Release/net8.0-maccatalyst/publish/* folder. If you publish the app using only a single architecture, it will be published to the *bin/Release/net8.0-maccatalyst/{architecture}/publish/* folder.

During the signing process it maybe necessary to enter your login password and allow `codesign` and `productbuild` to run:

![](../deployment/media/codesign.png)
![](../deployment/media/productbuild.png)

For more information about the `dotnet publish` command, see [dotnet publish](/dotnet/core/tools/dotnet-publish).

<!-- Todo: It's possible to re-sign an existing app bundle with a different certificate to change the distribution channel -->
