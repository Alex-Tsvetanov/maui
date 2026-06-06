---
title: "KeyboardAcceleratorExtensions.ToPlatform"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Platform
aliases:
  - "Microsoft.Maui.Platform.KeyboardAcceleratorExtensions.ToPlatform"
declaring_type: "KeyboardAcceleratorExtensions"
member_kind: method
---

# KeyboardAcceleratorExtensions.ToPlatform

> [!abstract] Method of [[KeyboardAcceleratorExtensions|KeyboardAcceleratorExtensions]]
> Namespace: `Microsoft.Maui.Platform`

Converts a list of IKeyboardAccelerator to a list of KeyboardAccelerator. A KeyboardAccelerator represents a keyboard shortcut (or accelerator) that lets a user perform an action using the keyboard instead of navigating the app UI (directly or through access keys).

## Signatures

```csharp
Microsoft.UI.Xaml.Input.KeyboardAccelerator? static ToPlatform(this Microsoft.Maui.IKeyboardAccelerator! keyboardAccelerator)
System.Collections.Generic.IList<Microsoft.UI.Xaml.Input.KeyboardAccelerator!>? static ToPlatform(this System.Collections.Generic.IReadOnlyList<Microsoft.Maui.IKeyboardAccelerator!>! keyboardAccelerators)
```

## Returns

List of `KeyboardAccelerator`

## Parameters

| Parameter | Description |
|---|---|
| `keyboardAccelerators` | List of `IKeyboardAccelerator` |

## See also

- Declaring type: [[KeyboardAcceleratorExtensions|KeyboardAcceleratorExtensions]]
- [[_Microsoft.Maui.Platform|Microsoft.Maui.Platform namespace]]
