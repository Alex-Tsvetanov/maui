---
title: "PlatformDragEventArgs.SetDropProposal"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PlatformDragEventArgs.SetDropProposal"
declaring_type: "PlatformDragEventArgs"
member_kind: method
---

# PlatformDragEventArgs.SetDropProposal

> [!abstract] Method of [[PlatformDragEventArgs|PlatformDragEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Sets the drop proposal when dragging over a view.

## Signature

```csharp
void SetDropProposal(UIKit.UIDropProposal! dropProposal)
```

## Parameters

| Parameter | Description |
|---|---|
| `dropProposal` | The custom drop proposal to use. |

## Remarks

`PlatformDragEventArgs` is used for DragOver and DragLeave events, but this method only has an effect with DragOver events.

## See also

- Declaring type: [[PlatformDragEventArgs|PlatformDragEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
