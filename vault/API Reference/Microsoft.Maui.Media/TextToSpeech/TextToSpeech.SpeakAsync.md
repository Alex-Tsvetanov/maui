---
title: "TextToSpeech.SpeakAsync"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Media
aliases:
  - "Microsoft.Maui.Media.TextToSpeech.SpeakAsync"
declaring_type: "TextToSpeech"
member_kind: method
---

# TextToSpeech.SpeakAsync

> [!abstract] Method of [[TextToSpeech|TextToSpeech]]
> Namespace: `Microsoft.Maui.Media`

Speaks the given text through the device's speech-to-text.

## Signatures

```csharp
System.Threading.Tasks.Task! static SpeakAsync(string! text, Microsoft.Maui.Media.SpeechOptions? options, System.Threading.CancellationToken cancelToken = default(System.Threading.CancellationToken))
System.Threading.Tasks.Task! static SpeakAsync(string! text, System.Threading.CancellationToken cancelToken = default(System.Threading.CancellationToken))
```

## Parameters

| Parameter | Description |
|---|---|
| `text` | The text to speak. |
| `cancelToken` | Optional cancellation token to stop speaking. |

## Returns

A `Task` object with the current status of the asynchronous operation.

## See also

- Declaring type: [[TextToSpeech|TextToSpeech]]
- [[_Microsoft.Maui.Media|Microsoft.Maui.Media namespace]]
