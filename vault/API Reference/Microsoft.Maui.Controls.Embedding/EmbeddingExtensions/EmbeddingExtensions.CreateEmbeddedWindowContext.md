---
title: "EmbeddingExtensions.CreateEmbeddedWindowContext"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Embedding
aliases:
  - "Microsoft.Maui.Controls.Embedding.EmbeddingExtensions.CreateEmbeddedWindowContext"
declaring_type: "EmbeddingExtensions"
member_kind: method
---

# EmbeddingExtensions.CreateEmbeddedWindowContext

> [!abstract] Method of [[EmbeddingExtensions|EmbeddingExtensions]]
> Namespace: `Microsoft.Maui.Controls.Embedding`

Creates a window-scoped `IMauiContext` for the provided native platform window.

## Signatures

```csharp
Microsoft.Maui.IMauiContext! static CreateEmbeddedWindowContext(this Microsoft.Maui.Hosting.MauiApp! mauiApp, Android.App.Activity! platformWindow)
Microsoft.Maui.IMauiContext! static CreateEmbeddedWindowContext(this Microsoft.Maui.Hosting.MauiApp! mauiApp, UIKit.UIWindow! platformWindow)
Microsoft.Maui.IMauiContext! static CreateEmbeddedWindowContext(this Microsoft.Maui.Hosting.MauiApp! mauiApp, Microsoft.UI.Xaml.Window! platformWindow)
```

## Parameters

| Parameter | Description |
|---|---|
| `mauiApp` | The `MauiApp` instance. |
| `platformWindow` | The native platform window instance to create the context for. |

## Returns

The window-scoped `IMauiContext` instance.

## Remarks

In addition to the context being created, a new Window instance is created and attached to the app.

## See also

- Declaring type: [[EmbeddingExtensions|EmbeddingExtensions]]
- [[_Microsoft.Maui.Controls.Embedding|Microsoft.Maui.Controls.Embedding namespace]]
