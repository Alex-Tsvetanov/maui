---
title: "PointerGestureRecognizer.PointerReleasedCommand"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.PointerGestureRecognizer.PointerReleasedCommand"
declaring_type: "PointerGestureRecognizer"
member_kind: property
---

# PointerGestureRecognizer.PointerReleasedCommand

> [!abstract] Property of [[PointerGestureRecognizer|PointerGestureRecognizer]]
> Namespace: `Microsoft.Maui.Controls`

Identifies the PointerReleasedCommand bindable property.

## Signature

```csharp
System.Windows.Input.ICommand! PointerReleasedCommand { get; set; }
```

## Remarks

See the remarks on `PointerReleased` for platform-specific behavior when handling secondary (right) button interactions. Command handlers may receive a release immediately after a press for secondary clicks on iOS/Mac Catalyst.

## See also

- Declaring type: [[PointerGestureRecognizer|PointerGestureRecognizer]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
