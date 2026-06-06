---
title: "ICrossPlatformLayoutBacking.CrossPlatformLayout"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.ICrossPlatformLayoutBacking.CrossPlatformLayout"
declaring_type: "ICrossPlatformLayoutBacking"
member_kind: property
---

# ICrossPlatformLayoutBacking.CrossPlatformLayout

> [!abstract] Property of [[ICrossPlatformLayoutBacking|ICrossPlatformLayoutBacking]]
> Namespace: `Microsoft.Maui`

Gets or sets the implementation of cross-platform layout operations to be carried out by this control

## Signature

```csharp
Microsoft.Maui.ICrossPlatformLayout? CrossPlatformLayout { get; set; }
```

## Remarks

This property is the bridge between the platform-level backing control and the cross-platform-level layout. It is typically connected by the handler, which may add additional logic to normalize layout and content behaviors across the various platforms.

## See also

- Declaring type: [[ICrossPlatformLayoutBacking|ICrossPlatformLayoutBacking]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
