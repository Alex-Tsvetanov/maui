---
title: "GridExtensions.AddWithSpan"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.GridExtensions.AddWithSpan"
declaring_type: "GridExtensions"
member_kind: method
---

# GridExtensions.AddWithSpan

> [!abstract] Method of [[GridExtensions|GridExtensions]]
> Namespace: `Microsoft.Maui.Controls`

Adds an `IView` to the the `Grid` at the specified row and column with the specified row and column spans.

## Signature

```csharp
void static AddWithSpan(this Microsoft.Maui.Controls.Grid grid, Microsoft.Maui.IView view, int row = 0, int column = 0, int rowSpan = 1, int columnSpan = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `grid` | The `Grid` to which the `IView` will be added. |
| `view` | The `IView` to add. |
| `row` | The top row in which to place the `IView`. Defaults to 0. |
| `column` | The left column in which to place the `IView`. Defaults to 0. |
| `rowSpan` | The number of rows the `IView` should span. Defaults to 1. |
| `columnSpan` | The number of columns the `IView` should span. Defaults to 1. |

## Remarks

If the `Grid` does not have enough rows/columns to encompass the specified spans, they will be added.

## See also

- Declaring type: [[GridExtensions|GridExtensions]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
