---
title: "ContentPage.SafeAreaEdges"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ContentPage.SafeAreaEdges"
declaring_type: "ContentPage"
member_kind: property
---

# ContentPage.SafeAreaEdges

> [!abstract] Property of [[ContentPage|ContentPage]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the safe area edges to obey for this content page. The default value is SafeAreaEdges.Default (None - edge to edge).

## Signature

```csharp
Microsoft.Maui.SafeAreaEdges SafeAreaEdges { get; set; }
```

## Remarks

This property controls which edges of the content page should obey safe area insets. Use SafeAreaRegions.None for edge-to-edge content, SafeAreaRegions.All to obey all safe area insets, SafeAreaRegions.Container for content that flows under keyboard but stays out of bars/notch, or SafeAreaRegions.SoftInput for keyboard-aware behavior.

## See also

- Declaring type: [[ContentPage|ContentPage]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
