---
title: "Application (AppCompat).GetShouldPreserveKeyboardOnResume"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-AndroidSpecific-AppCompat
aliases:
  - "Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.AppCompat.Application.GetShouldPreserveKeyboardOnResume"
declaring_type: "Application (AppCompat)"
member_kind: method
---

# Application (AppCompat).GetShouldPreserveKeyboardOnResume

> [!abstract] Method of [[Application (AppCompat)|Application (AppCompat)]]
> Namespace: `Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.AppCompat`

Returns a Boolean value that controls whether the keyboard state should be preserved when the application resumes.

## Signatures

```csharp
bool static GetShouldPreserveKeyboardOnResume(Microsoft.Maui.Controls.BindableObject element)
bool static GetShouldPreserveKeyboardOnResume(this Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.Android, Microsoft.Maui.Controls.Application> config)
```

## Parameters

| Parameter | Description |
|---|---|
| `element` | The platform specific element on which to perform the operation. |

## Returns

`true` if the keyboard state should be preserved when the application resumes; otherwise, `false`.

## See also

- Declaring type: [[Application (AppCompat)|Application (AppCompat)]]
- [[_Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.AppCompat|Microsoft.Maui.Controls.PlatformConfiguration.AndroidSpecific.AppCompat namespace]]
