---
title: "CornerRadius.Deconstruct"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.CornerRadius.Deconstruct"
declaring_type: "CornerRadius"
member_kind: method
---

# CornerRadius.Deconstruct

> [!abstract] Method of [[CornerRadius|CornerRadius]]
> Namespace: `Microsoft.Maui`

Compares two `CornerRadius` values for inequality.

## Signature

```csharp
void Deconstruct(out double topLeft, out double topRight, out double bottomLeft, out double bottomRight)
```

## Parameters

| Parameter | Description |
|---|---|
| `left` | The first corner radius to compare. |
| `right` | The second corner radius to compare. |
| `topLeft` | The radius of the top left corner. |
| `topRight` | The radius of the top right corner. |
| `bottomLeft` | The radius of the bottom left corner. |
| `bottomRight` | The radius of the bottom right corner. |

## Returns

`true` if `left` and `right` have different corner values; otherwise, `false`.

## See also

- Declaring type: [[CornerRadius|CornerRadius]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
