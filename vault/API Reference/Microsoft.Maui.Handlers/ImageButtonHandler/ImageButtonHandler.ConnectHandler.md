---
title: "ImageButtonHandler.ConnectHandler"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Handlers
aliases:
  - "Microsoft.Maui.Handlers.ImageButtonHandler.ConnectHandler"
declaring_type: "ImageButtonHandler"
member_kind: method
---

# ImageButtonHandler.ConnectHandler

> [!abstract] Method of [[ImageButtonHandler|ImageButtonHandler]]
> Namespace: `Microsoft.Maui.Handlers`

Connects the handler to the native image button control, wiring up event handlers and platform state.

## Signatures

```csharp
void override ConnectHandler(Google.Android.Material.ImageView.ShapeableImageView! platformView)
void override ConnectHandler(UIKit.UIButton! platformView)
void override ConnectHandler(Microsoft.Maui.Platform.MauiImageButton! platformView)
void override ConnectHandler(Microsoft.UI.Xaml.Controls.Button! platformView)
```

## See also

- Declaring type: [[ImageButtonHandler|ImageButtonHandler]]
- [[_Microsoft.Maui.Handlers|Microsoft.Maui.Handlers namespace]]
