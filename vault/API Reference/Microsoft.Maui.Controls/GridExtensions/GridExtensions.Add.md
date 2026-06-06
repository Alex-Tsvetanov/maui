---
title: "GridExtensions.Add"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.GridExtensions.Add"
declaring_type: "GridExtensions"
member_kind: method
---

# GridExtensions.Add

> [!abstract] Method of [[GridExtensions|GridExtensions]]
> Namespace: `Microsoft.Maui.Controls`

Adds an `IView` to the `Grid` at the specified column and row with a row span of 1 and a column span of 1.

## Signatures

```csharp
void static Add(this Microsoft.Maui.Controls.Grid grid, Microsoft.Maui.IView view, int column = 0, int row = 0)
void static Add(this Microsoft.Maui.Controls.Grid grid, Microsoft.Maui.IView view, int left, int right, int top, int bottom)
```

## Parameters

| Parameter | Description |
|---|---|
| `grid` | The `Grid` to which the `IView` will be added. |
| `view` | The `IView` to add. |
| `column` | The column in which to place the `IView`. |
| `row` | The row in which to place the `IView`. |

## Remarks

If the `Grid` does not have enough rows/columns to encompass the specified location, they will be added.

## See also

- Declaring type: [[GridExtensions|GridExtensions]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
