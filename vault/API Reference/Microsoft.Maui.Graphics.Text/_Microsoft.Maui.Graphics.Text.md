---
title: "Microsoft.Maui.Graphics.Text"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Graphics-Text
---

# Microsoft.Maui.Graphics.Text

> [!info] Namespace
> `Microsoft.Maui.Graphics.Text` — 21 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics.text)

## Overview

`Microsoft.Maui.Graphics.Text` provides the model and supporting machinery for *attributed text* — runs of text that carry formatting attributes (such as styling and color hints) on top of plain character content. It underpins rich-text scenarios in the MAUI Graphics layer, where text needs more structure than a flat string but is decoupled from any specific UI control.

At the center is the attributed-text abstraction exposed through [[IAttributedText|IAttributedText]] and its base implementation [[AbstractAttributedText|AbstractAttributedText]], with concrete forms in [[AttributedText|AttributedText]] and the editable [[MutableAttributedText|MutableAttributedText]]. Text is composed of runs ([[IAttributedTextRun|IAttributedTextRun]] / [[AttributedTextRun|AttributedTextRun]]), each pairing a span of text with a set of attributes ([[TextAttribute|TextAttribute]], [[TextAttributes|TextAttributes]], and the [[ITextAttributes (Text)|ITextAttributes]] contract). Helper types such as [[TextColors|TextColors]] and [[MarkerType|MarkerType]] supply common values used while building attributed content.

The namespace also handles serialization and conversion between formats. Readers and writers move attributed text to and from XML via [[XmlAttributedTextReader|XmlAttributedTextReader]] and [[XmlAttributedTextWriter|XmlAttributedTextWriter]], while [[MarkdownAttributedTextReader|MarkdownAttributedTextReader]] converts Markdown source into attributed text. A family of extension classes ([[AttributedTextExtensions (Text)|AttributedTextExtensions]], [[TextAttributeExtensions|TextAttributeExtensions]], and related helpers) adds convenience operations over these core types, so applications can construct, transform, and emit styled text without reimplementing the underlying run model.

## Key types

- [[IAttributedText|IAttributedText]] — Contract for text that carries formatting attributes across one or more runs.
- [[AbstractAttributedText|AbstractAttributedText]] — Base class providing shared attributed-text behavior for concrete implementations.
- [[AttributedText|AttributedText]] — Standard immutable attributed-text representation.
- [[MutableAttributedText|MutableAttributedText]] — Editable attributed text supporting in-place construction and modification.
- [[IAttributedTextRun|IAttributedTextRun]] — Contract for a single run: a span of text paired with its attributes.
- [[AttributedTextRun|AttributedTextRun]] — Concrete attributed-text run implementation.
- [[TextAttribute|TextAttribute]] — Represents an individual formatting attribute applied to text.
- [[TextAttributes|TextAttributes]] — A collection of text attributes (implements [[ITextAttributes (Text)|ITextAttributes]]).
- [[MarkdownAttributedTextReader|MarkdownAttributedTextReader]] — Converts Markdown text into attributed text.
- [[XmlAttributedTextReader|XmlAttributedTextReader]] — Reads attributed text from an XML representation.
- [[XmlAttributedTextWriter|XmlAttributedTextWriter]] — Writes attributed text to an XML representation.
- [[TextColors|TextColors]] — Predefined color values used when authoring attributed text.


## Classes

| Type | Summary |
|---|---|
| [[AbstractAttributedText\|AbstractAttributedText]] |  |
| [[AttributedText\|AttributedText]] |  |
| [[AttributedTextBlock\|AttributedTextBlock]] |  |
| [[AttributedTextExtensions (Text)\|AttributedTextExtensions (Text)]] |  |
| [[AttributedTextRun\|AttributedTextRun]] |  |
| [[AttributedTextRunComparer\|AttributedTextRunComparer]] |  |
| [[AttributedTextRunExtensions\|AttributedTextRunExtensions]] |  |
| [[CountingWriter\|CountingWriter]] |  |
| [[MarkdownAttributedTextReader\|MarkdownAttributedTextReader]] | Provides functionality to convert markdown text into attributed text. |
| [[MarkerType\|MarkerType]] |  |
| [[MutableAttributedText\|MutableAttributedText]] |  |
| [[TextAttribute\|TextAttribute]] |  |
| [[TextAttributeExtensions\|TextAttributeExtensions]] |  |
| [[TextAttributes\|TextAttributes]] |  |
| [[TextAttributesExtensions\|TextAttributesExtensions]] |  |
| [[TextColors\|TextColors]] |  |
| [[XmlAttributedTextReader\|XmlAttributedTextReader]] |  |
| [[XmlAttributedTextWriter\|XmlAttributedTextWriter]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IAttributedText\|IAttributedText]] |  |
| [[IAttributedTextRun\|IAttributedTextRun]] |  |
| [[ITextAttributes (Text)\|ITextAttributes (Text)]] |  |

## See also

- [[_API Reference]]
