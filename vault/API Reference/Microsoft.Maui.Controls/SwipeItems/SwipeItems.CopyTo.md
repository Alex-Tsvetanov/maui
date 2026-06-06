---
title: "SwipeItems.CopyTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.SwipeItems.CopyTo"
declaring_type: "SwipeItems"
member_kind: method
---

# SwipeItems.CopyTo

> [!abstract] Method of [[SwipeItems|SwipeItems]]
> Namespace: `Microsoft.Maui.Controls`

Copies a specified number of bytes from the current seek pointer in the stream to the current seek pointer in another stream.

## Signature

```csharp
void CopyTo(Microsoft.Maui.Controls.ISwipeItem[] array, int arrayIndex)
```

## Parameters

| Parameter | Description |
|---|---|
| `pstm` | A reference to the destination stream. |
| `cb` | The number of bytes to copy from the source stream. |
| `pcbRead` | On successful return, contains the actual number of bytes read from the source. (Note the native signature is to a ULARGE_INTEGER*, so 64 bits are written to this parameter on success.) |
| `pcbWritten` | On successful return, contains the actual number of bytes written to the destination. (Note the native signature is to a ULARGE_INTEGER*, so 64 bits are written to this parameter on success.) |

## See also

- Declaring type: [[SwipeItems|SwipeItems]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
