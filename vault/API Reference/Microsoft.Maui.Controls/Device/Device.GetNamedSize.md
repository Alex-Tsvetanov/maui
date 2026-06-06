---
title: "Device.GetNamedSize"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Device.GetNamedSize"
declaring_type: "Device"
member_kind: method
---

# Device.GetNamedSize

> [!abstract] Method of [[Device|Device]]
> Namespace: `Microsoft.Maui.Controls`

Returns the named font size for the specified element type.

## Signatures

```csharp
double static GetNamedSize(Microsoft.Maui.Controls.NamedSize size, Microsoft.Maui.Controls.Element targetElement)
double static GetNamedSize(Microsoft.Maui.Controls.NamedSize size, System.Type targetElementType, bool useOldSizes)
double static GetNamedSize(Microsoft.Maui.Controls.NamedSize size, System.Type targetElementType)
```

## Parameters

| Parameter | Description |
|---|---|
| `size` | The named size to retrieve. |
| `targetElement` | The element to get the named size for. |

## Returns

The font size value.

## See also

- Declaring type: [[Device|Device]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
