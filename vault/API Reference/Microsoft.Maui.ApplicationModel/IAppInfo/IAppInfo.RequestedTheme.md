---
title: "IAppInfo.RequestedTheme"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.IAppInfo.RequestedTheme"
declaring_type: "IAppInfo"
member_kind: property
---

# IAppInfo.RequestedTheme

> [!abstract] Property of [[IAppInfo|IAppInfo]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Gets the detected theme of the system or application.

## Signature

```csharp
Microsoft.Maui.ApplicationModel.AppTheme RequestedTheme { get; }
```

## Remarks

For platforms or platform versions which do not support themes, `Unspecified` is returned.

## See also

- Declaring type: [[IAppInfo|IAppInfo]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
