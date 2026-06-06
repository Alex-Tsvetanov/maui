---
title: "Graphics"
description: ".NET MAUI includes cross-platform 2D graphics functionality that targets iOS, Android, Windows, macOS, Tizen, and Linux."
tags:
  - conceptual
  - area/user-interface
ms_date: "09/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/graphics?view=net-maui-10.0"
---

# Graphics

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/userinterface-graphicsview)

.NET Multi-platform App UI (.NET MAUI) provides a cross-platform graphics canvas on which 2D graphics can be drawn using types from the `Graphics` namespace. This canvas supports drawing and painting shapes and images, compositing operations, and graphical object transforms.

There are many similarities between the functionality provided by `Graphics`, and the functionality provided by .NET MAUI shapes and brushes. However, each is aimed at different scenarios:

- `Graphics` functionality must be consumed on a drawing canvas, enables performant graphics to be drawn, and provides a convenient approach for writing graphics-based controls. For example, a control that replicates the GitHub contribution profile can be more easily implemented using `Graphics` than by using .NET MAUI shapes.
- .NET MAUI shapes can be consumed directly on a page, and brushes can be consumed by all controls. This functionality is provided to help you produce an attractive UI.

For more information about .NET MAUI shapes, see [[shapes|Shapes]].

<!-- ## Platform abstractions

The following table lists the graphics abstractions that are supported on each platform:

| Platform | Graphics abstractions |
| -- | -- |
| .NET MAUI | Platform support as shown per platform below. |
| .NET for iOS | CoreGraphics, SkiaSharp |
| .NET for Android | Android.Graphics, SkiaSharp |
| .NET for macOS | CoreGraphics, SkiaSharp |
| Windows Presentation Foundation | SharpDX, XAML, GDI, SkiaSharp |
| Universal Windows Platform | SharpDX, Win2D, XAML, SkiaSharp |
| Windows Forms | SharpDX, GDI, SkiaSharp |
| Tizen | SkiaSharp |
| Linux | SkiaSharp |

By default, .NET MAUI uses the native graphics capabilities of each platform. -->

## Draw graphics

In .NET MAUI, the [[GraphicsView|GraphicsView]] enables consumption of `Graphics` functionality. [[GraphicsView|GraphicsView]] defines the `Drawable` property, of type `IDrawable`, which specifies the content that will be drawn by the control. To specify the content that will be drawn you must create an object that derives from `IDrawable`, and implement its `Draw` method:

```csharp
namespace MyMauiApp
{
    public class GraphicsDrawable : IDrawable
    {
        public void Draw(ICanvas canvas, RectF dirtyRect)
        {
            // Drawing code goes here
        }      
    }
}
```

The `Draw` method has [[ICanvas|ICanvas]] and `RectF` arguments. The [[ICanvas|ICanvas]] argument is the drawing canvas on which you draw graphical objects. The `RectF` argument is a `struct` that contains data about the size and location of the drawing canvas.

In XAML, the `IDrawable` object can be declared as a resource and then consumed by a [[GraphicsView|GraphicsView]] by specifying its key as the value of the `Drawable` property:

```xaml
<ContentPage xmlns=http://schemas.microsoft.com/dotnet/2021/maui
             xmlns:x=http://schemas.microsoft.com/winfx/2009/xaml
             xmlns:drawable="clr-namespace:MyMauiApp"
             x:Class="MyMauiApp.MainPage">
    <ContentPage.Resources>
        <drawable:GraphicsDrawable x:Key="drawable" />
    </ContentPage.Resources>
    <VerticalStackLayout>
        <GraphicsView Drawable="{StaticResource drawable}"
                      HeightRequest="300"
                      WidthRequest="400" />
    </VerticalStackLayout>
</ContentPage>
```

For more information about the [[GraphicsView|GraphicsView]], see [[graphicsview|GraphicsView]].

## Drawing canvas

The [[GraphicsView|GraphicsView]] control provides access to an [[ICanvas|ICanvas]] object, via its `IDrawable` object, on which properties can be set and methods invoked to draw graphical objects. For information about drawing on an [[ICanvas|ICanvas]], see [[draw|Draw graphical objects]].

[[ICanvas|ICanvas]] defines the following properties that affect the appearance of objects that are drawn on the canvas:

<!-- Todo: Font properties being renamed. Some property types may change -->

- [[ICanvas.Alpha|Alpha]], of type `float`, indicates the opacity of an object.
- [[ICanvas.Antialias|Antialias]], of type `bool`, specifies whether anti-aliasing is enabled.
- [[ICanvas.BlendMode|BlendMode]], of type [[BlendMode|BlendMode]], defines the blend mode, which determines what happens when an object is rendered on top of an existing object.
- [[ICanvas.DisplayScale|DisplayScale]], of type `float`, represents the scaling factor to scale the UI by on a canvas.
- [[ICanvas.FillColor|FillColor]], of type [[Color|Color]], indicates the color used to paint an object's interior.
- [[ICanvas.Font|Font]], of type [[IFont|IFont]], defines the font when drawing text.
- [[ICanvas.FontColor|FontColor]], of type [[Color|Color]], specifies the font color when drawing text.
- [[ICanvas.FontSize|FontSize]], of type `float`, defines the size of the font when drawing text.
- [[ICanvas.MiterLimit|MiterLimit]], of type `float`, specifies the limit of the miter length of line joins in an object.
- [[ICanvas.StrokeColor|StrokeColor]], of type [[Color|Color]], indicates the color used to paint an object's outline.
- [[ICanvas.StrokeDashOffset|StrokeDashOffset]], of type `float`, specifies the distance within the dash pattern where a dash begins.
- [[ICanvas.StrokeDashPattern|StrokeDashPattern]], of type `float[]`, specifies the pattern of dashes and gaps that are used to outline an object.
- [[ICanvas.StrokeLineCap|StrokeLineCap]], of type [[LineCap|LineCap]], describes the shape at the start and end of a line.
- [[ICanvas.StrokeLineJoin|StrokeLineJoin]], of type [[LineJoin|LineJoin]], specifies the type of join that is used at the vertices of a shape.
- [[ICanvas.StrokeSize|StrokeSize]], of type `float`, indicates the width of an object's outline.

By default, an [[ICanvas|ICanvas]] sets [[ICanvas.StrokeSize|StrokeSize]] to 1, [[ICanvas.StrokeColor|StrokeColor]] to black, [[ICanvas.StrokeLineJoin|StrokeLineJoin]] to `LineJoin.Miter`, and [[ICanvas.StrokeLineCap|StrokeLineCap]] to `LineJoin.Cap`.

### Drawing canvas state

The drawing canvas on each platform has the ability to maintain its state. This enables you to persist the current graphics state, and restore it when required.

However, not all elements of the canvas are elements of the graphics state. The graphics state does not include drawing objects, such as paths, and paint objects, such as gradients. Typical elements of the graphics state on each platform include stroke and fill data, and font data.

The graphics state of each [[ICanvas|ICanvas]] can be manipulated with the following methods:

- `SaveState%2A`, which saves the current graphics state.
- `RestoreState%2A`, which sets the graphics state to the most recently saved state.
- `ResetState%2A`, which resets the graphics state to its default values.

> [!NOTE]
> The state that's persisted by these methods is platform dependent.
