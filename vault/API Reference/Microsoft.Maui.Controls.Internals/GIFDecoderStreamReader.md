---
title: "GIFDecoderStreamReader"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Controls-Internals
aliases:
  - "Microsoft.Maui.Controls.Internals.GIFDecoderStreamReader"
namespace: "Microsoft.Maui.Controls.Internals"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
assemblies:
  - Controls
---

# GIFDecoderStreamReader

> [!abstract] Class in `Microsoft.Maui.Controls.Internals`
> Full name: `Microsoft.Maui.Controls.Internals.GIFDecoderStreamReader`

Reads bytes from a stream for GIF decoding.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[GIFDecoderStreamReader.GIFDecoderStreamReader\|GIFDecoderStreamReader]] | Creates a new reader for the specified stream. |

## Properties

| Name | Summary |
|---|---|
| [[GIFDecoderStreamReader.CurrentBlockBuffer\|CurrentBlockBuffer]] |  |
| [[GIFDecoderStreamReader.CurrentBlockSize\|CurrentBlockSize]] |  |
| [[GIFDecoderStreamReader.CurrentPosition\|CurrentPosition]] |  |

## Methods

| Name | Summary |
|---|---|
| [[GIFDecoderStreamReader.Read\|Read]] | Gets the current position in the stream. |
| [[GIFDecoderStreamReader.ReadAsync\|ReadAsync]] |  |
| [[GIFDecoderStreamReader.ReadBlockAsync\|ReadBlockAsync]] |  |
| [[GIFDecoderStreamReader.ReadShort\|ReadShort]] | Reads a 16-bit little-endian integer from the stream. |
| [[GIFDecoderStreamReader.ReadString\|ReadString]] | Reads an ASCII string of the specified length from the stream. |
| [[GIFDecoderStreamReader.SkipBlockAsync\|SkipBlockAsync]] | Asynchronously skips over a GIF data block. |

## See also

- [[_Microsoft.Maui.Controls.Internals|Microsoft.Maui.Controls.Internals namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.internals.gifdecoderstreamreader)
