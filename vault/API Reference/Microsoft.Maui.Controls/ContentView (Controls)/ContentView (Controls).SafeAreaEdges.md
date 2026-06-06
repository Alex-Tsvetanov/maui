---
title: "ContentView (Controls).SafeAreaEdges"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ContentView.SafeAreaEdges"
declaring_type: "ContentView (Controls)"
member_kind: property
---

# ContentView (Controls).SafeAreaEdges

> [!abstract] Property of [[ContentView (Controls)|ContentView (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the safe area edges to obey for this content view. The default value is SafeAreaEdges.Default (None - edge to edge).

## Signature

```csharp
Microsoft.Maui.SafeAreaEdges SafeAreaEdges { get; set; }
```

## Remarks

This property controls which edges of the content view should obey safe area insets. Use SafeAreaRegions.None for edge-to-edge content, SafeAreaRegions.All to obey all safe area insets, SafeAreaRegions.Container for content that flows under keyboard but stays out of bars/notch, or SafeAreaRegions.Keyboard for keyboard-aware behavior.

## See also

- Declaring type: [[ContentView (Controls)|ContentView (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
