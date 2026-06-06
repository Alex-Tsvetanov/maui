---
title: "Microsoft.Maui.Graphics"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Graphics
---

# Microsoft.Maui.Graphics

> [!info] Namespace
> `Microsoft.Maui.Graphics` — 83 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.graphics)

## Overview

`Microsoft.Maui.Graphics` is the cross-platform 2D drawing library at the heart of .NET MAUI. It provides a platform-agnostic abstraction for immediate-mode drawing, so the same drawing code renders consistently across every target platform. The central abstraction is [[ICanvas|ICanvas]], a surface onto which you draw lines, shapes, text, and images; you supply drawing logic by implementing [[IDrawable|IDrawable]], whose draw method receives a canvas and a bounding [[RectF|RectF]].

The namespace supplies a complete set of value types for describing geometry — [[PointF|PointF]], [[SizeF|SizeF]], and [[RectF|RectF]] (with double-precision [[Point|Point]], [[Size|Size]], and [[Rect (Graphics)|Rect]] counterparts) — plus [[PathF|PathF]] for building arbitrary lines, curves, and shapes. Color is modeled by [[Color|Color]] with named constants on [[Colors|Colors]].

Fills are described by the [[Paint|Paint]] hierarchy: [[SolidPaint|SolidPaint]] for flat color, [[LinearGradientPaint|LinearGradientPaint]] and [[RadialGradientPaint|RadialGradientPaint]] for gradients, and [[ImagePaint|ImagePaint]] / [[PatternPaint|PatternPaint]] for image and pattern fills. Drawing can also be recorded once and replayed via [[IPicture|IPicture]], or exported to bitmaps and PDFs through services such as [[IBitmapExportService|IBitmapExportService]] and [[IPdfPage|IPdfPage]]. Text rendering is supported through [[IFont|IFont]] and related text-attribute and sizing services.

## Key types

- [[ICanvas|ICanvas]] — platform-agnostic 2D surface for drawing lines, shapes, text, and images.
- [[IDrawable|IDrawable]] — defines an object that can draw itself onto a canvas.
- [[PathF|PathF]] — geometric path of lines, curves, and shapes using single-precision coordinates.
- [[Color|Color]] — an RGBA color with floating-point components from 0.0 to 1.0.
- [[Colors|Colors]] — the set of system-defined named colors.
- [[Paint|Paint]] — abstract base for fills used to paint shapes.
- [[SolidPaint|SolidPaint]] — fills shapes with a solid color.
- [[LinearGradientPaint|LinearGradientPaint]] — gradient fill transitioning colors along a line.
- [[RadialGradientPaint|RadialGradientPaint]] — gradient fill radiating outward from a center point.
- [[IImage (Graphics)|IImage]] — a drawable, loadable image abstraction.
- [[IPicture|IPicture]] — a recorded collection of drawing commands that can be replayed on a canvas.
- [[RectF|RectF]] — single-precision rectangle commonly used as a drawing bounds.


## Classes

| Type | Summary |
|---|---|
| [[AbstractCanvas{TState}\|AbstractCanvas<TState>]] | Provides an abstract base implementation of the `ICanvas` interface. |
| [[AbstractPattern\|AbstractPattern]] | Provides a base implementation for pattern types that can be used to fill shapes. |
| [[BitmapExportContext\|BitmapExportContext]] | Provides an abstract base class for bitmap export operations. |
| [[BitmapExportContextExtensions\|BitmapExportContextExtensions]] | Provides extension methods for the `BitmapExportContext` class. |
| [[CanvasDefaults\|CanvasDefaults]] | Provides default values for canvas-related properties. |
| [[CanvasExtensions (Graphics)\|CanvasExtensions (Graphics)]] | Provides extension methods for the `ICanvas` interface. |
| [[CanvasState\|CanvasState]] | Represents the state of a canvas, including transformation and stroke properties. |
| [[Color\|Color]] | Represents an RGBA color with floating-point components in the range of 0.0 to 1.0. |
| [[Colors\|Colors]] | Represents all the system-defined colors. |
| [[DrawingCommand\|DrawingCommand]] |  |
| [[FontWeights\|FontWeights]] | Defines constant values for standard font weights. |
| [[GeometryUtil\|GeometryUtil]] | Provides utility methods for geometric calculations. |
| [[GradientPaint\|GradientPaint]] | Represents an abstract base class for gradient paints that transition between multiple colors. |
| [[IFontExtensions\|IFontExtensions]] | Provides extension methods for the `IFont` interface. |
| [[ImageExtensions (Graphics)\|ImageExtensions (Graphics)]] | Provides extension methods for the `IImage` interface. |
| [[ImageLoadingServiceExtensions\|ImageLoadingServiceExtensions]] | Provides extension methods for the `IImageLoadingService` interface. |
| [[ImagePaint\|ImagePaint]] | Represents a paint that uses an image to fill shapes. |
| [[Insets\|Insets]] | Represents inset distances from the four edges of a rectangle. |
| [[InsetsF\|InsetsF]] | Represents inset distances from the four edges of a rectangle using single-precision floating-point values. |
| [[LayoutLine\|LayoutLine]] |  |
| [[LinearGradientPaint\|LinearGradientPaint]] | Represents a linear gradient paint that transitions colors along a line defined by start and end points. |
| [[MauiDrawable (Graphics)\|MauiDrawable (Graphics)]] |  |
| [[Paint\|Paint]] | Represents an abstract base class for different types of paints that can be used to fill shapes. |
| [[PaintExtensions\|PaintExtensions]] |  |
| [[PaintGradientStop\|PaintGradientStop]] | Represents a color stop in a gradient paint. |
| [[PaintPattern\|PaintPattern]] | Represents a pattern that applies a paint to a wrapped pattern for filling shapes. |
| [[PathArcExtensions\|PathArcExtensions]] | Provides extension methods for adding SVG-compatible arcs to a `PathF`. |
| [[PathBuilder\|PathBuilder]] | Provides functionality for constructing path objects from string definitions. |
| [[PathExtensions\|PathExtensions]] | Provides extension methods for working with `PathF` objects. |
| [[PathF\|PathF]] | Represents a geometric path consisting of lines, curves, and shapes using single-precision floating-point coordinates. |
| [[PatternExtensions\|PatternExtensions]] | Provides extension methods for the `IPattern` interface. |
| [[PatternPaint\|PatternPaint]] | Represents a paint that fills shapes with a repeating pattern. |
| [[PdfPageExtensions\|PdfPageExtensions]] | Provides extension methods for the `IPdfPage` interface. |
| [[PictureCanvas\|PictureCanvas]] | A canvas implementation that records drawing commands into a picture for later playback. |
| [[PictureExtensions\|PictureExtensions]] | Provides extension methods for the `IPicture` interface. |
| [[PicturePattern\|PicturePattern]] | Represents a pattern that draws a picture repeatedly. |
| [[PictureReaderExtensions\|PictureReaderExtensions]] | Provides extension methods for the `IPictureReader` interface. |
| [[PictureWriterExtensions\|PictureWriterExtensions]] | Provides extension methods for the `IPictureWriter` interface. |
| [[RadialGradientPaint\|RadialGradientPaint]] | Represents a radial gradient paint that transitions colors outward from a center point. |
| [[ScalingCanvas\|ScalingCanvas]] | A canvas implementation that applies scaling to drawing operations before delegating to a wrapped canvas. |
| [[ShapeDrawable\|ShapeDrawable]] |  |
| [[SolidPaint\|SolidPaint]] | Represents a paint that fills shapes with a solid color. |
| [[StandardPicture\|StandardPicture]] | Provides a standard implementation of the `IPicture` interface using drawing commands. |
| [[StandardTextAttributes\|StandardTextAttributes]] | Provides a standard implementation of the `ITextAttributes` interface for styling text. |
| [[XmlnsPrefixAttribute (Graphics)\|XmlnsPrefixAttribute (Graphics)]] |  |

## Interfaces

| Type | Summary |
|---|---|
| [[IBitmapExportService\|IBitmapExportService]] | Defines a service for exporting graphics content as bitmaps. |
| [[IBlurrableCanvas\|IBlurrableCanvas]] | Defines a canvas that supports blur effects. |
| [[ICanvas\|ICanvas]] | Represents a platform-agnostic canvas on which 2D graphics can be drawn using types from the `Graphics` namespace. |
| [[ICanvasStateService{TState}\|ICanvasStateService<TState>]] | Defines a service for creating and managing canvas state objects. |
| [[IDrawable\|IDrawable]] | Defines an object that can be drawn onto a canvas. |
| [[IFont\|IFont]] | Defines a font with name, weight, and style properties for text rendering. |
| [[IImage (Graphics)\|IImage (Graphics)]] | Preserves aspect ratio and ensures the image fits within the target dimensions. May leave empty space if the aspect ratios don't match. |
| [[IImageLoadingService\|IImageLoadingService]] | Defines a service for loading images from streams. |
| [[IPattern\|IPattern]] | Defines a pattern that can be used to fill shapes on a canvas. |
| [[IPdfPage\|IPdfPage]] | Represents a PDF page that can be drawn upon and saved to a stream. |
| [[IPdfRenderService\|IPdfRenderService]] | Defines a service for rendering PDF pages. |
| [[IPicture\|IPicture]] | Defines an interface for a picture, which is a collection of drawing commands that can be replayed on a canvas. |
| [[IPictureReader\|IPictureReader]] | Defines an interface for reading picture data from a byte array. |
| [[IPictureWriter\|IPictureWriter]] | Defines an interface for saving picture objects to streams. |
| [[IPlatformFonts\|IPlatformFonts]] | Defines an interface for accessing and managing platform-specific fonts. |
| [[IShape\|IShape]] | Provides a base definition class for shape elements, such as Ellipse, Polygon, or Rectangle. |
| [[IStringSizeService\|IStringSizeService]] | Defines a service for calculating the size of text strings with different fonts and alignments. |
| [[ITextAttributes (Graphics)\|ITextAttributes (Graphics)]] | Defines attributes for styling and positioning text in a graphics context. |

## Structs

| Type | Summary |
|---|---|
| [[Font (Graphics)\|Font (Graphics)]] | Represents a font with a name, weight, and style. |
| [[FontSource\|FontSource]] | Represents a font source with name, weight, and style information. |
| [[Point\|Point]] | Represents a point in 2D space using double-precision floating-point coordinates. |
| [[PointF\|PointF]] | Represents a point in 2D space using single-precision floating-point coordinates. |
| [[Rect (Graphics)\|Rect (Graphics)]] | Represents a rectangle with double-precision floating-point x, y coordinates and width and height. |
| [[RectF\|RectF]] | Represents a rectangle with single-precision floating-point x, y coordinates and width and height. |
| [[Size\|Size]] | Represents a size with double-precision floating-point width and height. |
| [[SizeF\|SizeF]] | Represents a size with single-precision floating-point width and height. |

## Enums

| Type | Summary |
|---|---|
| [[BlendMode\|BlendMode]] | Specifies the blend mode to use when compositing images or drawings. |
| [[FontStyleType\|FontStyleType]] | Specifies the style of a font. |
| [[HorizontalAlignment\|HorizontalAlignment]] | Specifies the horizontal alignment of text or elements. |
| [[ImageFormat\|ImageFormat]] | Specifies the format of an image. |
| [[LineCap\|LineCap]] | Specifies the style of line cap (ending) to use at the end of lines when drawing. |
| [[LineJoin\|LineJoin]] | Specifies the style used to join two line segments where they meet. |
| [[PathOperation\|PathOperation]] | Specifies the types of operations that can be performed on a path. |
| [[PictureCommand\|PictureCommand]] | Specifies the type of commands that can be recorded and played back in a picture. |
| [[ResizeMode\|ResizeMode]] | Specifies how an image should be resized to fit a target area. |
| [[TextFlow\|TextFlow]] | Specifies how text should flow when it exceeds the bounds of its container. |
| [[VerticalAlignment\|VerticalAlignment]] | Specifies the vertical alignment of text or elements. |
| [[WindingMode\|WindingMode]] | Specifies the algorithm used to determine which regions are inside or outside a path for filling. |

## See also

- [[_API Reference]]
