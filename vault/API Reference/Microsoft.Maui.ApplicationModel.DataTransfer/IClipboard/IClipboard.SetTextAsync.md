---
title: "IClipboard.SetTextAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-DataTransfer
aliases:
  - "Microsoft.Maui.ApplicationModel.DataTransfer.IClipboard.SetTextAsync"
declaring_type: "IClipboard"
member_kind: method
---

# IClipboard.SetTextAsync

> [!abstract] Method of [[IClipboard|IClipboard]]
> Namespace: `Microsoft.Maui.ApplicationModel.DataTransfer`

Sets the contents of the clipboard to be the specified text.

## Signature

```csharp
System.Threading.Tasks.Task! SetTextAsync(string? text)
```

## Remarks

This method returns immediately and does not guarentee that the text is on the clipboard by the time this method returns.

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `text` | The text to put on the clipboard. |

## See also

- Declaring type: [[IClipboard|IClipboard]]
- [[_Microsoft.Maui.ApplicationModel.DataTransfer|Microsoft.Maui.ApplicationModel.DataTransfer namespace]]
