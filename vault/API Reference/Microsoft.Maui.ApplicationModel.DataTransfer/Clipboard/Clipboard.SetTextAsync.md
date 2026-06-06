---
title: "Clipboard.SetTextAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-ApplicationModel-DataTransfer
aliases:
  - "Microsoft.Maui.ApplicationModel.DataTransfer.Clipboard.SetTextAsync"
declaring_type: "Clipboard"
member_kind: method
---

# Clipboard.SetTextAsync

> [!abstract] Method of [[Clipboard|Clipboard]]
> Namespace: `Microsoft.Maui.ApplicationModel.DataTransfer`

Sets the contents of the clipboard to be the specified text.

## Signature

```csharp
System.Threading.Tasks.Task! static SetTextAsync(string? text)
```

## Parameters

| Parameter | Description |
|---|---|
| `text` | The text to put on the clipboard. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## Remarks

This method returns immediately and does not guarentee that the text is on the clipboard by the time this method returns.

## See also

- Declaring type: [[Clipboard|Clipboard]]
- [[_Microsoft.Maui.ApplicationModel.DataTransfer|Microsoft.Maui.ApplicationModel.DataTransfer namespace]]
