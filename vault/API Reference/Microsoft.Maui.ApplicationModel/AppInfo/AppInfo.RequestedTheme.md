---
title: "AppInfo.RequestedTheme"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-ApplicationModel
aliases:
  - "Microsoft.Maui.ApplicationModel.AppInfo.RequestedTheme"
declaring_type: "AppInfo"
member_kind: property
---

# AppInfo.RequestedTheme

> [!abstract] Property of [[AppInfo|AppInfo]]
> Namespace: `Microsoft.Maui.ApplicationModel`

Gets the detected theme of the system or application.

## Signature

```csharp
static Microsoft.Maui.ApplicationModel.AppTheme RequestedTheme { get; }
```

## Remarks

For platforms or platform versions which do not support themes, `Unspecified` is returned.

## See also

- Declaring type: [[AppInfo|AppInfo]]
- [[_Microsoft.Maui.ApplicationModel|Microsoft.Maui.ApplicationModel namespace]]
