---
title: "Objective Sharpie Features"
description: "This document links to various guides that help describe Objective Sharpie, how to use it, and the output that it generates."
tags:
  - conceptual
  - area/ios
ms_date: "02/11/2026"
source: "https://learn.microsoft.com/dotnet/maui/ios/objective-sharpie/platform?view=net-maui-10.0"
---

# Objective Sharpie Features

Read through these pages to better understand Objective Sharpie's features:

## [[api-definition-structs-enums|**ApiDefinition.cs & StructsAndEnums.cs**]]

These two files are emitted by Objective Sharpie, to be included
in your binding project. Learn more about them in [[api-definition-structs-enums|ApiDefinition.cs & StructsAndEnums.cs]].

## [[native-frameworks|**Native Frameworks**]]

Some libraries are distributed as frameworks rather than as source.
Objective Sharpie lets you bind these frameworks directly by passing the
`.framework` directory to the `-f` option.

## [[verify|**Verify**]]

Objective Sharpie add `Verify` attributes to signal that you need to
manually inspect and update the generated binding.
