---
title: "PathBuilder.ParseFloat"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathBuilder.ParseFloat"
declaring_type: "PathBuilder"
member_kind: method
---

# PathBuilder.ParseFloat

> [!abstract] Method of [[PathBuilder|PathBuilder]]
> Namespace: `Microsoft.Maui.Graphics`

Parses a string value as a float using invariant culture.

## Signature

```csharp
float static ParseFloat(string value)
```

## Parameters

| Parameter | Description |
|---|---|
| `value` | The string representation of a number. |

## Returns

The float value parsed from the string.

## Remarks

Handles special cases like Illustrator's malformed number formats (e.g., "5.96.88").

## See also

- Declaring type: [[PathBuilder|PathBuilder]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
