---
title: "Images"
description: ".NET MAUI graphics includes functionality to load, save, resize, and downsize images."
tags:
  - conceptual
  - area/user-interface
ms_date: "09/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/graphics/images?view=net-maui-10.0"
---

# Images

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/userinterface-graphicsview)

.NET Multi-platform App UI (.NET MAUI) graphics includes functionality to load, save, resize, and downsize images. Supported image formats are dependent on the underlying platform.

Images are represented by the [[IImage (Graphics)|IImage]] type, which defines the following properties:

- [[IImage (Graphics).Width|Width]], of type `float`, that defines the width of an image.
- [[IImage (Graphics).Height|Height]], of type `float`, that defines the height of an image.

An optional [[ImageFormat|ImageFormat]] argument can be specified when loading and saving images. The [[ImageFormat|ImageFormat]] enumeration defines `Png`, `Jpeg`, `Gif`, `Tiff`, and `Bmp` members. However, this argument is only used when the image format is supported by the underlying platform.

> [!NOTE]
> .NET MAUI contains two different `IImage` interfaces. `Microsoft.Maui.Graphics.IImage` is used for image display, manipulation, and persistence in when displaying graphics in a [[GraphicsView|GraphicsView]]. `Microsoft.Maui.IImage` is the interface that abstracts the [[Image (Controls)|Image]] control.

## Load an image

Image loading functionality is provided by the [[PlatformImage|PlatformImage]] class. You can load images from a stream by the `FromStream%2A` method, or from a byte array using the [[PlatformImage|PlatformImage]] constructor.

The following example shows how to load an image:

```csharp
using Microsoft.Maui.Graphics.Platform;
using System.Reflection;
using IImage = Microsoft.Maui.Graphics.IImage;

IImage image;
Assembly assembly = GetType().GetTypeInfo().Assembly;
using (Stream stream = assembly.GetManifestResourceStream("GraphicsViewDemos.Resources.Images.dotnet_bot.png"))
{
    image = PlatformImage.FromStream(stream);
}

if (image != null)
{
    canvas.DrawImage(image, 10, 10, image.Width, image.Height);
}
```

In this example, the image is retrieved from the assembly, loaded as a stream, and displayed.

> [!IMPORTANT]
> Loading an image that's embedded in an assembly requires the image to have its build action set to **Embedded Resource** rather than **MauiImage**.

## Resize an image

Images can be resized using the `Resize%2A` method, which requires `width` and `height` arguments, of type `float`, which represent the target dimensions of the image. The `Resize%2A` method also accepts two optional arguments:

- A [[ResizeMode|ResizeMode]] argument that controls how the image is resized to fit its target dimensions.
- A `bool` argument that controls whether the source image will be disposed after performing the resize operation. This argument defaults to `false`, indicating that the source image isn't disposed.

The [[ResizeMode|ResizeMode]] enumeration defines the following members, which specify how to resize the image to the target size:

- `Fit`, which letterboxes the image so that it fits its target size.
- `Bleed`, which clips the image so that it fits its target size, while preserving its aspect ratio.
- `Stretch`, which stretches the image so it fills the available space. This can result in a change in the image aspect ratio.

The following example shows how to resize an image:

```csharp
using Microsoft.Maui.Graphics.Platform;
using System.Reflection;
using IImage = Microsoft.Maui.Graphics.IImage;

IImage image;
Assembly assembly = GetType().GetTypeInfo().Assembly;
using (Stream stream = assembly.GetManifestResourceStream("GraphicsViewDemos.Resources.Images.dotnet_bot.png"))
{
    image = PlatformImage.FromStream(stream);
}

if (image != null)
{
    IImage newImage = image.Resize(100, 60, ResizeMode.Stretch, true);
    canvas.DrawImage(newImage, 10, 10, newImage.Width, newImage.Height);
}
```

In this example, the image is retrieved from the assembly and loaded as a stream. The image is resized using the `Resize%2A` method, with its arguments specifying the new size, and that it should be stretched to fill the available space. In addition, the source image is disposed. The resized image is then drawn at actual size at (10,10).

## Downsize an image

You can downsize images using one of the `Downsize%2A` overloads. The first overload requires a single `float` value that represents the maximum width or height of the image, and downsizes the image while maintaining its aspect ratio. The second overload requires two `float` arguments that represent the maximum width and maximum height of the image.

The `Downsize%2A` overloads also accept an optional `bool` argument that controls whether the source image should be disposed after performing the downsizing operation. This argument defaults to `false`, indicating that the source image isn't be disposed.

The following example shows how to downsize an image:

```csharp
using Microsoft.Maui.Graphics.Platform;
using System.Reflection;
using IImage = Microsoft.Maui.Graphics.IImage;

IImage image;
Assembly assembly = GetType().GetTypeInfo().Assembly;
using (Stream stream = assembly.GetManifestResourceStream("GraphicsViewDemos.Resources.Images.dotnet_bot.png"))
{
    image = PlatformImage.FromStream(stream);
}

if (image != null)
{
    IImage newImage = image.Downsize(100, true);
    canvas.DrawImage(newImage, 10, 10, newImage.Width, newImage.Height);
}
```

In this example, the image is retrieved from the assembly and loaded as a stream. The image is downsized using the `Downsize%2A` method, with the argument specifying that its largest dimension should be set to 100 pixels. In addition, the source image is disposed. The downsized image is then drawn at actual size at (10,10).

## Save an image

You can save images using the `Save%2A` and `SaveAsync%2A` methods. Each method saves the [[IImage (Graphics)|IImage]] to a `Stream`, and enables optional [[ImageFormat|ImageFormat]] and quality values to be specified.

> [!NOTE]
> The `Save%2A` and `SaveAsync%2A` methods on Android and iOS can save images to JPEG and PNG format.

The following example shows how to save an image:

```csharp
using Microsoft.Maui.Graphics.Platform;
using System.Reflection;
using IImage = Microsoft.Maui.Graphics.IImage;

IImage image;
Assembly assembly = GetType().GetTypeInfo().Assembly;
using (Stream stream = assembly.GetManifestResourceStream("GraphicsViewDemos.Resources.Images.dotnet_bot.png"))
{
    image = PlatformImage.FromStream(stream);
}

// Save image to a memory stream
if (image != null)
{
    IImage newImage = image.Downsize(150, true);
    using (MemoryStream memStream = new MemoryStream())
    {
        newImage.Save(memStream);
        // Reset destination stream position to 0 if saving to a file
    }
}
```

In this example, the image is retrieved from the assembly and loaded as a stream. The image is downsized using the `Downsize%2A` method, with the argument specifying that its largest dimension should be set to 150 pixels. In addition, the source image is disposed. The downsized image is then saved to a stream.

> [!IMPORTANT]
> The `Save%2A` method doesn't reset the stream position to 0. Therefore, if you want to save the stream to a file you should use the `Seek%2A` method to reset the destination stream position to 0 before copying it to a file.
