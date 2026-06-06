---
title: "Layout (Controls).IgnoreSafeArea"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Layout.IgnoreSafeArea"
declaring_type: "Layout (Controls)"
member_kind: property
---

# Layout (Controls).IgnoreSafeArea

> [!abstract] Property of [[Layout (Controls)|Layout (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the safe area edges to obey for this layout. The default value is SafeAreaEdges.Default (None - edge to edge).

## Signature

```csharp
bool IgnoreSafeArea { get; set; }
```

## Remarks

This property controls which edges of the layout should obey safe area insets. Use SafeAreaRegions.None for edge-to-edge content, SafeAreaRegions.All to obey all safe area insets, SafeAreaRegions.Container for content that flows under keyboard but stays out of bars/notch, or SafeAreaRegions.SoftInput for keyboard-aware behavior.

## See also

- Declaring type: [[Layout (Controls)|Layout (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
