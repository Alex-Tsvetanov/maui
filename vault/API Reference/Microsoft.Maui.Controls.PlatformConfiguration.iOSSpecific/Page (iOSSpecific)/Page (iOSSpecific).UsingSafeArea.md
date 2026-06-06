---
title: "Page (iOSSpecific).UsingSafeArea"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-PlatformConfiguration-iOSSpecific
aliases:
  - "Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific.Page.UsingSafeArea"
declaring_type: "Page (iOSSpecific)"
member_kind: method
---

# Page (iOSSpecific).UsingSafeArea

> [!abstract] Method of [[Page (iOSSpecific)|Page (iOSSpecific)]]
> Namespace: `Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific`

Gets a value that represents whether the padding is overridden with the safe area.

## Signature

```csharp
bool static UsingSafeArea(this Microsoft.Maui.Controls.IPlatformElementConfiguration<Microsoft.Maui.Controls.PlatformConfiguration.iOS, Microsoft.Maui.Controls.Page> config)
```

## Parameters

| Parameter | Description |
|---|---|
| `config` | The element whose safe area behavior to get. |

## Returns

`true` if the padding is overridden with the safe area; otherwise, `false`.

## Remarks

This API is deprecated. Use SafeAreaEdges attached property instead for per-edge safe area control.

## See also

- Declaring type: [[Page (iOSSpecific)|Page (iOSSpecific)]]
- [[_Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific|Microsoft.Maui.Controls.PlatformConfiguration.iOSSpecific namespace]]
