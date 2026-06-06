---
title: "ITextToSpeech.SpeakAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.ITextToSpeech.SpeakAsync"
declaring_type: "ITextToSpeech"
member_kind: method
---

# ITextToSpeech.SpeakAsync

> [!abstract] Method of [[ITextToSpeech|ITextToSpeech]]
> Namespace: `Microsoft.Maui.Media`

Speaks the given text through the device's speech-to-text.

## Signature

```csharp
System.Threading.Tasks.Task! SpeakAsync(string! text, Microsoft.Maui.Media.SpeechOptions? options = null, System.Threading.CancellationToken cancelToken = default(System.Threading.CancellationToken))
```

## Returns

A `Task` object with the current status of the asynchronous operation.

## Parameters

| Parameter | Description |
|---|---|
| `text` | The text to speak. |
| `options` | The options to use for speaking. |
| `cancelToken` | Optional cancellation token to stop speaking. |

## See also

- Declaring type: [[ITextToSpeech|ITextToSpeech]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
