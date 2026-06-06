---
title: "PanUpdatedEventArgs.PanUpdatedEventArgs"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PanUpdatedEventArgs.PanUpdatedEventArgs"
declaring_type: "PanUpdatedEventArgs"
member_kind: constructor
---

# PanUpdatedEventArgs.PanUpdatedEventArgs

> [!abstract] Constructor of [[PanUpdatedEventArgs|PanUpdatedEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Initializes a new instance of the `PanUpdatedEventArgs` class with translation data.

## Signatures

```csharp
void PanUpdatedEventArgs(Microsoft.Maui.GestureStatus type, int gestureId, double totalx, double totaly)
void PanUpdatedEventArgs(Microsoft.Maui.GestureStatus type, int gestureId)
```

## Parameters

| Parameter | Description |
|---|---|
| `type` | The gesture status. |
| `gestureId` | The unique identifier for this gesture. |
| `totalx` | The total X translation since the gesture started. |
| `totaly` | The total Y translation since the gesture started. |

## See also

- Declaring type: [[PanUpdatedEventArgs|PanUpdatedEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
