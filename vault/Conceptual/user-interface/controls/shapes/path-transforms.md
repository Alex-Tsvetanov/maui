---
title: "Path transforms"
description: "A .NET MAUI transform defines how to transform a Path object from one coordinate space to another coordinate space."
tags:
  - conceptual
  - area/user-interface
ms_date: "08/30/2024"
source: "https://learn.microsoft.com/dotnet/maui/user-interface/controls/shapes/path-transforms?view=net-maui-10.0"
---

# Path transforms

[![Browse sample.](~/media/code-sample.png) Browse the sample](/samples/dotnet/maui-samples/userinterface-shapes)

A .NET Multi-platform App UI (.NET MAUI) [[Transform|Transform]] defines how to transform a [[Path|Path]] object from one coordinate space to another coordinate space. When a transform is applied to a [[Path|Path]] object, it changes how the object is rendered in the UI.

Transforms can be categorized into four general classifications: rotation, scaling, skew, and translation. .NET MAUI defines a class for each of these transform classifications:

- [[RotateTransform|RotateTransform]], which rotates a [[Path|Path]] by a specified `Angle`.
- [[ScaleTransform|ScaleTransform]], which scales a [[Path|Path]] object by specified `ScaleX` and `ScaleY` amounts.
- [[SkewTransform|SkewTransform]], which skews a [[Path|Path]] object by specified `AngleX` and `AngleY` amounts.
- [[TranslateTransform|TranslateTransform]], which moves a [[Path|Path]] object by specified `X` and `Y` amounts.

.NET MAUI also provides the following classes for creating more complex transformations:

- [[TransformGroup|TransformGroup]], which represents a composite transform composed of multiple transform objects.
- [[CompositeTransform|CompositeTransform]], which applies multiple transform operations to a [[Path|Path]] object.
- [[MatrixTransform|MatrixTransform]], which creates custom transforms that are not provided by the other transform classes.

All of these classes derive from the [[Transform|Transform]] class, which defines a `Value` property of type [[Matrix|Matrix]], which represents the current transformation as a [[Matrix|Matrix]] object. This property is backed by a [[BindableProperty|BindableProperty]] object, which means that it can be the target of data bindings, and styled. For more information about the [[Matrix|Matrix]] struct, see [Transform matrix](#transform-matrix).

To apply a transform to a [[Path|Path]], you create a transform class and set it as the value of the `Path.RenderTransform` property.

## Rotation transform

A rotate transform rotates a [[Path|Path]] object clockwise about a specified point in a 2D x-y coordinate system.

The [[RotateTransform|RotateTransform]] class, which derives from the [[Transform|Transform]] class, defines the following properties:

- [[RotateTransform.Angle|Angle]], of type `double`, represents the angle, in degrees, of clockwise rotation. The default value of this property is 0.0.
- [[RotateTransform.CenterX|CenterX]], of type `double`, represents the x-coordinate of the rotation center point. The default value of this property is 0.0.
- [[RotateTransform.CenterY|CenterY]], of type `double`, represents the y-coordinate of the rotation center point. The default value of this property is 0.0.

These properties are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

The [[RotateTransform.CenterX|CenterX]] and [[RotateTransform.CenterY|CenterY]] properties specify the point about which the [[Path|Path]] object is rotated. This center point is expressed in the coordinate space of the object that's transformed. By default, the rotation is applied to (0,0), which is the upper-left corner of the [[Path|Path]] object.

The following example shows how to rotate a [[Path|Path]] object:

```xaml
<Path Stroke="Black"
      Aspect="Uniform"
      HorizontalOptions="Center"
      HeightRequest="100"
      WidthRequest="100"
      Data="M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z">
    <Path.RenderTransform>
        <RotateTransform CenterX="0"
                         CenterY="0"
                         Angle="45" />
    </Path.RenderTransform>
</Path>
```

In this example, the [[Path|Path]] object is rotated 45 degrees about its upper-left corner.

## Scale transform

A scale transform scales a [[Path|Path]] object in the 2D x-y coordinate system.

The [[ScaleTransform|ScaleTransform]] class, which derives from the [[Transform|Transform]] class, defines the following properties:

- [[ScaleTransform.ScaleX|ScaleX]], of type `double`, which represents the x-axis scale factor. The default value of this property is 1.0.
- [[ScaleTransform.ScaleY|ScaleY]], of type `double`, which represents the y-axis scale factor. The default value of this property is 1.0.
- [[ScaleTransform.CenterX|CenterX]], of type `double`, which represents the x-coordinate of the center point of this transform. The default value of this property is 0.0.
- [[ScaleTransform.CenterY|CenterY]], of type `double`, which represents the y-coordinate of the center point of this transform. The default value of this property is 0.0.

These properties are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

The value of [[ScaleTransform.ScaleX|ScaleX]] and [[ScaleTransform.ScaleY|ScaleY]] have a huge impact on the resulting scaling:

- Values between 0 and 1 decrease the width and height of the scaled object.
- Values greater than 1 increase the width and height of the scaled object.
- Values of 1 indicate that the object is not scaled.
- Negative values flip the scale object horizontally and vertically.
- Values between 0 and -1 flip the scale object and decrease its width and height.
- Values less than -1 flip the object and increase its width and height.
- Values of -1 flip the scaled object but do not change its horizontal or vertical size.

The [[ScaleTransform.CenterX|CenterX]] and [[ScaleTransform.CenterY|CenterY]] properties specify the point about which the [[Path|Path]] object is scaled. This center point is expressed in the coordinate space of the object that's transformed. By default, scaling is applied to (0,0), which is the upper-left corner of the [[Path|Path]] object. This has the effect of moving the [[Path|Path]] object and making it appear larger, because when you apply a transform you change the coordinate space in which the [[Path|Path]] object resides.

The following example shows how to scale a [[Path|Path]] object:

```xaml
<Path Stroke="Black"
      Aspect="Uniform"
      HorizontalOptions="Center"
      HeightRequest="100"
      WidthRequest="100"
      Data="M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z">
    <Path.RenderTransform>
        <ScaleTransform CenterX="0"
                        CenterY="0"
                        ScaleX="1.5"
                        ScaleY="1.5" />
    </Path.RenderTransform>
</Path>
```

In this example, the [[Path|Path]] object is scaled to 1.5 times the size.

## Skew transform

A skew transform skews a [[Path|Path]] object in the 2D x-y coordinate system, and is useful for creating the illusion of 3D depth in a 2D object.

The [[SkewTransform|SkewTransform]] class, which derives from the [[Transform|Transform]] class, defines the following properties:

- [[SkewTransform.AngleX|AngleX]], of type `double`, which represents the x-axis skew angle, which is measured in degrees counterclockwise from the y-axis. The default value of this property is 0.0.
- [[SkewTransform.AngleY|AngleY]], of type `double`, which represents the y-axis skew angle, which is measured in degrees counterclockwise from the x-axis. The default value of this property is 0.0.
- [[SkewTransform.CenterX|CenterX]], of type `double`, which represents the x-coordinate of the transform center. The default value of this property is 0.0.
- [[SkewTransform.CenterY|CenterY]], of type `double`, which represents the y-coordinate of the transform center. The default value of this property is 0.0.

These properties are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

To predict the effect of a skew transformation, consider that [[SkewTransform.AngleX|AngleX]] skews x-axis values relative to the original coordinate system. Therefore, for an [[SkewTransform.AngleX|AngleX]] of 30, the y-axis rotates 30 degrees through the origin and skews the values in x by 30 degrees from that origin. Similarly, an [[SkewTransform.AngleY|AngleY]] of 30 skews the y values of the [[Path|Path]] object by 30 degrees from the origin.

> [!NOTE]
> To skew a [[Path|Path]] object in place, set the `CenterX` and `CenterY` properties to the object's center point.

The following example shows how to skew a [[Path|Path]] object:

```xaml
<Path Stroke="Black"
      Aspect="Uniform"
      HorizontalOptions="Center"
      HeightRequest="100"
      WidthRequest="100"
      Data="M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z">
    <Path.RenderTransform>
        <SkewTransform CenterX="0"
                       CenterY="0"
                       AngleX="45"
                       AngleY="0" />
    </Path.RenderTransform>
</Path>
```

In this example, a horizontal skew of 45 degrees is applied to the [[Path|Path]] object, from a center point of (0,0).

## Translate transform

A translate transform moves an object in the 2D x-y coordinate system.

The [[TranslateTransform|TranslateTransform]] class, which derives from the [[Transform|Transform]] class, defines the following properties:

- [[TranslateTransform.X|X]], of type `double`, which represents the distance to move along the x-axis. The default value of this property is 0.0.
- [[TranslateTransform.Y|Y]], of type `double`, which represents the distance to move along the y-axis. The default value of this property is 0.0.

These properties are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

Negative [[TranslateTransform.X|X]] values move an object to the left, while positive values move an object to the right. Negative [[TranslateTransform.Y|Y]] values move an object up, while positive values move an object down.

The following example shows how to translate a [[Path|Path]] object:

```xaml
<Path Stroke="Black"
      Aspect="Uniform"
      HorizontalOptions="Center"
      HeightRequest="100"
      WidthRequest="100"
      Data="M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z">
    <Path.RenderTransform>
        <TranslateTransform X="50"
                            Y="50" />
    </Path.RenderTransform>
</Path>
```

In this example, the [[Path|Path]] object is moved 50 device-independent units to the right, and 50 device-independent units down.

## Multiple transforms

.NET MAUI has two classes that support applying multiple transforms to a [[Path|Path]] object. These are [[TransformGroup|TransformGroup]], and [[CompositeTransform|CompositeTransform]]. A [[TransformGroup|TransformGroup]] performs transforms in any desired order, while a [[CompositeTransform|CompositeTransform]] performs transforms in a specific order.

### Transform groups

Transform groups represent composite transforms composed of multiple [[Transform|Transform]] objects.

The [[TransformGroup|TransformGroup]] class, which derives from the [[Transform|Transform]] class, defines a [[TransformGroup.Children|Children]] property, of type [[TransformCollection|TransformCollection]], which represents a collection of [[Transform|Transform]] objects. This property is backed by a [[BindableProperty|BindableProperty]] object, which means that it can be the target of data bindings, and styled.

The order of transformations is important in a composite transform that uses the [[TransformGroup|TransformGroup]] class. For example, if you first rotate, then scale, then translate, you get a different result than if you first translate, then rotate, then scale. One reason order is significant is that transforms like rotation and scaling are performed respect to the origin of the coordinate system. Scaling an object that is centered at the origin produces a different result to scaling an object that has been moved away from the origin. Similarly, rotating an object that is centered at the origin produces a different result than rotating an object that has been moved away from the origin.

The following example shows how to perform a composite transform using the [[TransformGroup|TransformGroup]] class:

```xaml
<Path Stroke="Black"
      Aspect="Uniform"
      HorizontalOptions="Center"
      HeightRequest="100"
      WidthRequest="100"
      Data="M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z">
    <Path.RenderTransform>
        <TransformGroup>
            <ScaleTransform ScaleX="1.5"
                            ScaleY="1.5" />
            <RotateTransform Angle="45" />
        </TransformGroup>
    </Path.RenderTransform>
</Path>
```

In this example, the [[Path|Path]] object is scaled to 1.5 times its size, and then rotated by 45 degrees.

## Composite transforms

A composite transform applies multiple transforms to an object.

The [[CompositeTransform|CompositeTransform]] class, which derives from the [[Transform|Transform]] class, defines the following properties:

- [[CompositeTransform.CenterX|CenterX]], of type `double`, which represents the x-coordinate of the center point of this transform. The default value of this property is 0.0.
- [[CompositeTransform.CenterY|CenterY]], of type `double`, which represents the y-coordinate of the center point of this transform. The default value of this property is 0.0.
- [[CompositeTransform.ScaleX|ScaleX]], of type `double`, which represents the x-axis scale factor. The default value of this property is 1.0.
- [[CompositeTransform.ScaleY|ScaleY]], of type `double`, which represents the y-axis scale factor. The default value of this property is 1.0.
- [[CompositeTransform.SkewX|SkewX]], of type `double`, which represents the x-axis skew angle, which is measured in degrees counterclockwise from the y-axis. The default value of this property is 0.0.
- [[CompositeTransform.SkewY|SkewY]], of type `double`, which represents the y-axis skew angle, which is measured in degrees counterclockwise from the x-axis. The default value of this property is 0.0.
- [[CompositeTransform.Rotation|Rotation]], of type `double`, represents the angle, in degrees, of clockwise rotation. The default value of this property is 0.0.
- [[CompositeTransform.TranslateX|TranslateX]], of type `double`, which represents the distance to move along the x-axis. The default value of this property is 0.0.
- [[CompositeTransform.TranslateY|TranslateY]], of type `double`, which represents the distance to move along the y-axis. The default value of this property is 0.0.

These properties are backed by [[BindableProperty|BindableProperty]] objects, which means that they can be targets of data bindings, and styled.

A [[CompositeTransform|CompositeTransform]] applies transforms in this order:

1. Scale ([[CompositeTransform.ScaleX|ScaleX]] and [[CompositeTransform.ScaleY|ScaleY]]).
1. Skew ([[CompositeTransform.SkewX|SkewX]] and [[CompositeTransform.SkewY|SkewY]]).
1. Rotate ([[CompositeTransform.Rotation|Rotation]]).
1. Translate ([[CompositeTransform.TranslateX|TranslateX]], [[CompositeTransform.TranslateY|TranslateY]]).

If you want to apply multiple transforms to an object in a different order, you should create a [[TransformGroup|TransformGroup]] and insert the transforms in your intended order.

> [!IMPORTANT]
> A [[CompositeTransform|CompositeTransform]] uses the same center points, `CenterX` and `CenterY`, for all transformations. If you want to specify different center points per transform, use a [[TransformGroup|TransformGroup]],

The following example shows how to perform a composite transform using the [[CompositeTransform|CompositeTransform]] class:

```xaml
<Path Stroke="Black"
      Aspect="Uniform"
      HorizontalOptions="Center"
      HeightRequest="100"
      WidthRequest="100"
      Data="M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z">
    <Path.RenderTransform>
        <CompositeTransform ScaleX="1.5"
                            ScaleY="1.5"
                            Rotation="45"
                            TranslateX="50"
                            TranslateY="50" />
    </Path.RenderTransform>
</Path>
```

In this example, the [[Path|Path]] object is scaled to 1.5 times its size, then rotated by 45 degrees, and then translated by 50 device-independent units.

## Transform matrix

A transform can be described in terms of a 3x3 affine transformation matrix, that performs transformations in 2D space. This 3x3 matrix is represented by the [[Matrix|Matrix]] struct, which is a collection of three rows and three columns of `double` values.

The [[Matrix|Matrix]] struct defines the following properties:

- [[Matrix.Determinant|Determinant]], of type `double`, which gets the determinant of the matrix.
- [[Matrix.HasInverse|HasInverse]], of type `bool`, which indicates whether the matrix is invertible.
- [[Matrix.Identity|Identity]], of type [[Matrix|Matrix]], which gets an identity matrix.
- [[Matrix.IsIdentity|IsIdentity]], of type `bool`, which indicates whether the matrix is an identity matrix.
- [[Matrix.M11|M11]], of type `double`, which represents the value of the first row and first column of the matrix.
- [[Matrix.M12|M12]], of type `double`, which represents the value of the first row and second column of the matrix.
- [[Matrix.M21|M21]], of type `double`, which represents the value of the second row and first column of the matrix.
- [[Matrix.M22|M22]], of type `double`, which represents the value of the second row and second column of the matrix.
- [[Matrix.OffsetX|OffsetX]], of type `double`, which represents the value of the third row and first column of the matrix.
- [[Matrix.OffsetY|OffsetY]], of type `double`, which represents the value of the third row and second column of the matrix.

The [[Matrix.OffsetX|OffsetX]] and [[Matrix.OffsetY|OffsetY]] properties are so named because they specify the amount to translate the coordinate space along the x-axis, and y-axis, respectively.

In addition, the [[Matrix|Matrix]] struct exposes a series of methods that can be used to manipulate the matrix values, including `Append%2A`, `Invert%2A`, `Multiply%2A`, `Prepend%2A` and many more.

The following table shows the structure of a .NET MAUI matrix:


        M11


        M12


        0.0




        M21


        M22


        0.0




        OffsetX


        OffsetY


        1.0



> [!NOTE]
> An affine transformation matrix has its final column equal to (0,0,1), so only the members in the first two columns need to be specified.

By manipulating matrix values, you can rotate, scale, skew, and translate [[Path|Path]] objects. For example, if you change the [[Matrix.OffsetX|OffsetX]] value to 100, you can use it move a [[Path|Path]] object 100 device-independent units along the x-axis. If you change the [[Matrix.M22|M22]] value to 3, you can use it to stretch a [[Path|Path]] object to three times its current height. If you change both values, you move the [[Path|Path]] object 100 device-independent units along the x-axis and stretch its height by a factor of 3. In addition, affine transformation matrices can be multiplied to form any number of linear transformations, such as rotation and skew, followed by translation.

## Custom transforms

The [[MatrixTransform|MatrixTransform]] class, which derives from the [[Transform|Transform]] class, defines a [[Matrix|Matrix]] property, of type [[Matrix|Matrix]], which represents the matrix that defines the transformation. This property is backed by a [[BindableProperty|BindableProperty]] object, which means that it can be the target of data bindings, and styled.

Any transform that you can describe with a [[TranslateTransform|TranslateTransform]], [[ScaleTransform|ScaleTransform]], [[RotateTransform|RotateTransform]], or [[SkewTransform|SkewTransform]] object can equally be described by a [[MatrixTransform|MatrixTransform]]. However, the [[TranslateTransform|TranslateTransform]], [[ScaleTransform|ScaleTransform]], [[RotateTransform|RotateTransform]], and [[SkewTransform|SkewTransform]] classes are easier to conceptualize than setting the vector components in a [[Matrix|Matrix]]. Therefore, the [[MatrixTransform|MatrixTransform]] class is typically used to create custom transformations that aren't provided by the [[RotateTransform|RotateTransform]], [[ScaleTransform|ScaleTransform]], [[SkewTransform|SkewTransform]], or [[TranslateTransform|TranslateTransform]] classes.

The following example shows how to transform a [[Path|Path]] object using a [[MatrixTransform|MatrixTransform]]:

```xaml
<Path Stroke="Black"
      Aspect="Uniform"
      HorizontalOptions="Center"
      Data="M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z">
    <Path.RenderTransform>
        <MatrixTransform>
            <MatrixTransform.Matrix>
                <!-- M11 stretches, M12 skews -->
                <Matrix OffsetX="10"
                        OffsetY="100"
                        M11="1.5"
                        M12="1" />
            </MatrixTransform.Matrix>
        </MatrixTransform>
    </Path.RenderTransform>
</Path>
```

In this example, the [[Path|Path]] object is stretched, skewed, and offset in both the X and Y dimensions.

Alternatively, this can be written in a simplified form that uses a type converter that's built into .NET MAUI:

```xaml
<Path Stroke="Black"
      Aspect="Uniform"
      HorizontalOptions="Center"
      Data="M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z">
    <Path.RenderTransform>
        <MatrixTransform Matrix="1.5,1,0,1,10,100" />
    </Path.RenderTransform>
</Path>
```

In this example, the [[Matrix|Matrix]] property is specified as a comma-delimited string consisting of six members: `M11`, `M12`, `M21`, `M22`, `OffsetX`, `OffsetY`. While the members are comma-delimited in this example, they can also be delimited by one or more spaces.

In addition, the previous example can be simplified even further by specifying the same six members as the value of the [[Path.RenderTransform|RenderTransform]] property:

```xaml
<Path Stroke="Black"
      Aspect="Uniform"
      HorizontalOptions="Center"
      RenderTransform="1.5 1 0 1 10 100"
      Data="M13.908992,16.207977L32.000049,16.207977 32.000049,31.999985 13.908992,30.109983z" />
```
