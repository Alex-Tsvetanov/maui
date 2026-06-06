---
title: "IndicatorView.OnMeasure"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.IndicatorView.OnMeasure"
declaring_type: "IndicatorView"
member_kind: method
---

# IndicatorView.OnMeasure

> [!abstract] Method of [[IndicatorView|IndicatorView]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the shape of the indicators.

## Signature

```csharp
Microsoft.Maui.SizeRequest override OnMeasure(double widthConstraint, double heightConstraint)
```

## Parameters

| Parameter | Description |
|---|---|
| `widthConstraint` | The available width. |
| `heightConstraint` | The available height. |

## Returns

A `SizeRequest` indicating the desired size of the indicator view.

## Remarks

This property is set automatically when `IndicatorTemplate` is specified. Use this as the content property when defining custom indicator layouts in XAML.

## See also

- Declaring type: [[IndicatorView|IndicatorView]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
