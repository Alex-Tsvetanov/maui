---
title: "RelativeBindingSource.RelativeBindingSource"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.RelativeBindingSource.RelativeBindingSource"
declaring_type: "RelativeBindingSource"
member_kind: constructor
---

# RelativeBindingSource.RelativeBindingSource

> [!abstract] Constructor of [[RelativeBindingSource|RelativeBindingSource]]
> Namespace: `Microsoft.Maui.Controls`

Creates a new `RelativeBindingSource` with the specified mode, ancestor type, and level.

## Signature

```csharp
void RelativeBindingSource(Microsoft.Maui.Controls.RelativeBindingSourceMode mode, System.Type ancestorType = null, int ancestorLevel = 1)
```

## Parameters

| Parameter | Description |
|---|---|
| `mode` | The relative binding source mode. |
| `ancestorType` | The type of ancestor to find. Required for FindAncestor modes. |
| `ancestorLevel` | The level of ancestor to find (1-based). Default is 1. |

## See also

- Declaring type: [[RelativeBindingSource|RelativeBindingSource]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
