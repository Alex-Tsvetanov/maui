---
title: "PlatformDragStartingEventArgs.SetPrefersFullSizePreviews"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PlatformDragStartingEventArgs.SetPrefersFullSizePreviews"
declaring_type: "PlatformDragStartingEventArgs"
member_kind: method
---

# PlatformDragStartingEventArgs.SetPrefersFullSizePreviews

> [!abstract] Method of [[PlatformDragStartingEventArgs|PlatformDragStartingEventArgs]]
> Namespace: `Microsoft.Maui.Controls`

Sets the func that requests to keep drag previews full-sized when dragging begins.

## Signature

```csharp
void SetPrefersFullSizePreviews(System.Func<UIKit.UIDragInteraction!, UIKit.IUIDragSession!, bool>? prefersFullSizePreviews)
```

## Parameters

| Parameter | Description |
|---|---|
| `prefersFullSizePreviews` | Func that returns whether to request full size previews. |

## Remarks

The default behavior on iOS is to reduce the size of the drag shadow if not requested here. Even if requested, it is up to the system whether or not to fulfill the request. This method exists inside `PlatformDragStartingEventArgs` since the preview must have this value set when dragging begins.

## See also

- Declaring type: [[PlatformDragStartingEventArgs|PlatformDragStartingEventArgs]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
