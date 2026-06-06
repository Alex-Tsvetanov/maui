---
title: "Layout (Compatibility).ShouldInvalidateOnChildRemoved"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Compatibility
aliases:
  - "Microsoft.Maui.Controls.Compatibility.Layout.ShouldInvalidateOnChildRemoved"
declaring_type: "Layout (Compatibility)"
member_kind: method
---

# Layout (Compatibility).ShouldInvalidateOnChildRemoved

> [!abstract] Method of [[Layout (Compatibility)|Layout (Compatibility)]]
> Namespace: `Microsoft.Maui.Controls.Compatibility`

When implemented, should return `true` if `child` should call `InvalidateMeasure` when removed, and should return `false` if it should not call `InvalidateMeasure`. The default value is `true`.

## Signature

```csharp
bool virtual ShouldInvalidateOnChildRemoved(Microsoft.Maui.Controls.View child)
```

## Parameters

| Parameter | Description |
|---|---|
| `child` | The child for which to specify whether or not to track invalidation. |

## Returns

`true` if `child` should call `InvalidateMeasure`, otherwise `false`.

## See also

- Declaring type: [[Layout (Compatibility)|Layout (Compatibility)]]
- [[_Microsoft.Maui.Controls.Compatibility|Microsoft.Maui.Controls.Compatibility namespace]]
