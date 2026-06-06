---
title: "EmbeddingExtensions.ToPlatformEmbedded"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Embedding
aliases:
  - "Microsoft.Maui.Controls.Embedding.EmbeddingExtensions.ToPlatformEmbedded"
declaring_type: "EmbeddingExtensions"
member_kind: method
---

# EmbeddingExtensions.ToPlatformEmbedded

> [!abstract] Method of [[EmbeddingExtensions|EmbeddingExtensions]]
> Namespace: `Microsoft.Maui.Controls.Embedding`

Similar to `ToPlatform`, but also adds the element as a logical child to the embedded window.

## Signatures

```csharp
Android.Views.View! static ToPlatformEmbedded(this Microsoft.Maui.IElement! element, Microsoft.Maui.Hosting.MauiApp! mauiApp, Android.App.Activity! platformWindow)
Android.Views.View! static ToPlatformEmbedded(this Microsoft.Maui.IElement! element, Microsoft.Maui.IMauiContext! context)
UIKit.UIView! static ToPlatformEmbedded(this Microsoft.Maui.IElement! element, Microsoft.Maui.Hosting.MauiApp! mauiApp, UIKit.UIWindow! platformWindow)
Microsoft.UI.Xaml.FrameworkElement! static ToPlatformEmbedded(this Microsoft.Maui.IElement! element, Microsoft.Maui.Hosting.MauiApp! mauiApp, Microsoft.UI.Xaml.Window! platformWindow)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | The element to use when creating the native platform view. |
| `context` | The context to use when creating the native platform view. |

## Returns

The native platform view that represents the element.

## Remarks

Only if the window is an embedded window and the element is a `VisualElement` will the element be added as a logical child of that window.

## See also

- Declaring type: [[EmbeddingExtensions|EmbeddingExtensions]]
- [[_Microsoft.Maui.Controls.Embedding|Microsoft.Maui.Controls.Embedding namespace]]
