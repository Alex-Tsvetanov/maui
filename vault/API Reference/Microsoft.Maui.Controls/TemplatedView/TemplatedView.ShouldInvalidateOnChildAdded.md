---
title: "TemplatedView.ShouldInvalidateOnChildAdded"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.TemplatedView.ShouldInvalidateOnChildAdded"
declaring_type: "TemplatedView"
member_kind: method
---

# TemplatedView.ShouldInvalidateOnChildAdded

> [!abstract] Method of [[TemplatedView|TemplatedView]]
> Namespace: `Microsoft.Maui.Controls`

If you want to influence invalidation override InvalidateMeasureOverride. This method will no longer work on .NET 10 and later.

## Signature

```csharp
bool override ShouldInvalidateOnChildAdded(Microsoft.Maui.Controls.View! child)
```

## Parameters

| Parameter | Description |
|---|---|
| `child` | The child for which to specify whether or not to track invalidation. |

## Returns

`true` if `child` should call `InvalidateMeasure`, otherwise `false`.

## See also

- Declaring type: [[TemplatedView|TemplatedView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
