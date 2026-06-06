---
title: "PlatformDragStartingEventArgs.SetPreviewProvider"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PlatformDragStartingEventArgs.SetPreviewProvider"
declaring_type: "PlatformDragStartingEventArgs"
member_kind: method
---

# PlatformDragStartingEventArgs.SetPreviewProvider

> [!abstract] Method of [[PlatformDragStartingEventArgs|PlatformDragStartingEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Sets the preview provider when dragging begins.

## Signature

```csharp
void SetPreviewProvider(System.Func<UIKit.UIDragPreview?>! previewProvider)
```

## Parameters

| Parameter | Description |
|---|---|
| `previewProvider` | The custom preview provider to use. |

## Remarks

This previewProvider will be applied to the MAUI generated dragItem.

## See also

- Declaring type: [[PlatformDragStartingEventArgs|PlatformDragStartingEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
