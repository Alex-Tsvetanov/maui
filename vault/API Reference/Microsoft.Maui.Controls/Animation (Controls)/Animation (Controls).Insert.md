---
title: "Animation (Controls).Insert"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Animation.Insert"
declaring_type: "Animation (Controls)"
member_kind: method
---

# Animation (Controls).Insert

> [!abstract] Method of [[Animation (Controls)|Animation (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Adds an `Animation` object to this `Animation` that begins at `beginAt` and finishes at `finishAt`.

## Signature

```csharp
Microsoft.Maui.Controls.Animation Insert(double beginAt, double finishAt, Microsoft.Maui.Controls.Animation animation)
```

## Parameters

| Parameter | Description |
|---|---|
| `beginAt` | The fraction into this animation at which the added child animation will begin animating. |
| `finishAt` | The fraction into this animation at which the added child animation will stop animating. |
| `animation` | The animation to add. |

## See also

- Declaring type: [[Animation (Controls)|Animation (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
