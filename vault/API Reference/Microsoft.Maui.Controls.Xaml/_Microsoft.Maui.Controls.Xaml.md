---
title: "Microsoft.Maui.Controls.Xaml"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Controls-Xaml
---

# Microsoft.Maui.Controls.Xaml

> [!info] Namespace
> `Microsoft.Maui.Controls.Xaml` — 35 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.controls.xaml)

## Overview

`Microsoft.Maui.Controls.Xaml` is the namespace that powers XAML in .NET MAUI. It supplies the markup extensions, parser infrastructure, and compiler attributes that turn declarative XAML markup into live object graphs at runtime (or precompiled IL at build time). When you write XAML in a MAUI app, the types here are what interpret the `{...}` markup expressions, resolve types and resources, and report problems.

The bulk of the namespace is **markup extensions** — the classes behind the curly-brace syntax you use inside XAML attributes. [[StaticResourceExtension|StaticResourceExtension]] and [[DynamicResourceExtension|DynamicResourceExtension]] pull values from a `ResourceDictionary`, [[BindingExtension|BindingExtension]] and [[TemplateBindingExtension|TemplateBindingExtension]] create data bindings, and platform/idiom-aware extensions like [[OnPlatformExtension|OnPlatformExtension]] and [[OnIdiomExtension|OnIdiomExtension]] return different values per device. [[AppThemeBindingExtension|AppThemeBindingExtension]] swaps values between light and dark themes. Each of these implements the markup-extension contract defined by [[IMarkupExtension{T}|IMarkupExtension]] / [[IMarkupExtension{T}|IMarkupExtension&lt;T&gt;]].

The namespace also exposes the **XAML processing pipeline**: service-provider interfaces such as [[IXamlTypeResolver|IXamlTypeResolver]], [[IProvideValueTarget|IProvideValueTarget]], [[IRootObjectProvider|IRootObjectProvider]], and [[IValueProvider|IValueProvider]] that markup extensions consume while resolving their values, plus diagnostics like [[XamlParseException|XamlParseException]] and [[XmlLineInfo|XmlLineInfo]] for reporting and locating errors. Finally, compiler-facing attributes — [[XamlCompilationAttribute|XamlCompilationAttribute]] with [[XamlCompilationOptions|XamlCompilationOptions]], [[XamlResourceIdAttribute|XamlResourceIdAttribute]], and [[XamlFilePathAttribute|XamlFilePathAttribute]] — control XAML compilation (XAMLC) and link compiled XAML back to its source.

## Key types

- [[BindingExtension|BindingExtension]] — markup extension that creates a `Binding` from a XAML attribute value.
- [[StaticResourceExtension|StaticResourceExtension]] — resolves a resource from a `ResourceDictionary`.
- [[DynamicResourceExtension|DynamicResourceExtension]] — creates a `DynamicResource` for dynamic resource lookup.
- [[OnPlatformExtension|OnPlatformExtension]] — returns different values depending on the platform the app runs on.
- [[OnIdiomExtension|OnIdiomExtension]] — returns different values depending on the device idiom.
- [[AppThemeBindingExtension|AppThemeBindingExtension]] — binds different values for light and dark themes.
- [[StaticExtension|StaticExtension]] — returns the value of a static field or property.
- [[TemplateBindingExtension|TemplateBindingExtension]] — creates a binding to the templated parent.
- [[IMarkupExtension{T}|IMarkupExtension]] — the contract every markup extension implements to provide its value.
- [[IXamlTypeResolver|IXamlTypeResolver]] — service used to resolve a CLR type from a XAML type name.
- [[XamlCompilationAttribute|XamlCompilationAttribute]] — enables or disables XAML compilation (XAMLC) via [[XamlCompilationOptions|XamlCompilationOptions]].
- [[XamlParseException|XamlParseException]] — raised when the XAML parser encounters an error.


## Classes

| Type | Summary |
|---|---|
| [[AcceptEmptyServiceProviderAttribute\|AcceptEmptyServiceProviderAttribute]] | Tells the XAML parser and compiler that they may ignore supplied service providers in methods and constructors in the attributed class. |
| [[AppThemeBindingExtension\|AppThemeBindingExtension]] | Provides a XAML markup extension that creates a binding with different values for light and dark themes. |
| [[ArrayExtension\|ArrayExtension]] | Provides a XAML markup extension that creates an array of objects. |
| [[BindingExtension\|BindingExtension]] | Provides a XAML markup extension that creates a `Binding` from a XAML attribute value. |
| [[DataTemplateExtension\|DataTemplateExtension]] | Provides a XAML markup extension that creates a `DataTemplate` for a specified type. |
| [[DynamicResourceExtension\|DynamicResourceExtension]] | Provides a XAML markup extension that creates a `DynamicResource` for dynamic resource lookup. |
| [[Extensions (Xaml)\|Extensions (Xaml)]] | Provides extension methods for loading XAML into objects. |
| [[FontImageExtension\|FontImageExtension]] | Provides a XAML markup extension that creates a font image. Use `FontImageSource` instead. |
| [[NullExtension\|NullExtension]] | Provides a XAML markup extension that returns `null`. |
| [[OnIdiomExtension\|OnIdiomExtension]] | Provides a XAML markup extension that returns different values depending on the device idiom. |
| [[OnPlatformExtension\|OnPlatformExtension]] | Provides a XAML markup extension that returns different values depending on the platform the app is running on. |
| [[ReferenceExtension\|ReferenceExtension]] | Provides a XAML markup extension that returns an object by its x:Name from the current XAML namescope. |
| [[RelativeSourceExtension\|RelativeSourceExtension]] | Provides a XAML markup extension that returns a `RelativeBindingSource` for relative bindings. |
| [[RequireServiceAttribute\|RequireServiceAttribute]] |  |
| [[ResourceDictionaryHelpers\|ResourceDictionaryHelpers]] |  |
| [[StaticExtension\|StaticExtension]] | Provides a XAML markup extension that returns the value of a static field or property. |
| [[StaticResourceExtension\|StaticResourceExtension]] | Provides a XAML markup extension that resolves a resource from a `ResourceDictionary`. |
| [[StyleSheetExtension\|StyleSheetExtension]] | Provides a XAML markup extension that loads a CSS style sheet from a source or inline content. |
| [[TemplateBindingExtension\|TemplateBindingExtension]] | Provides a XAML markup extension that creates a binding to the templated parent. |
| [[TypeExtension\|TypeExtension]] | Provides a XAML markup extension that returns a `Type` object for a specified type name. |
| [[XamlCompilationAttribute\|XamlCompilationAttribute]] |  |
| [[XamlCompilationOptions\|XamlCompilationOptions]] |  |
| [[XamlFilePathAttribute\|XamlFilePathAttribute]] | Specifies the file path of the XAML file associated with a type. |
| [[XamlParseException\|XamlParseException]] | Exception that is raised when the XAML parser encounters a XAML error. |
| [[XamlResourceIdAttribute\|XamlResourceIdAttribute]] | Maps a XAML resource ID to its path and associated type. |
| [[XmlLineInfo\|XmlLineInfo]] | Provides line and position information for XAML parsing. |

## Interfaces

| Type | Summary |
|---|---|
| [[IMarkupExtension\|IMarkupExtension]] |  |
| [[IMarkupExtension{T}\|IMarkupExtension<T>]] |  |
| [[IProvideValueTarget\|IProvideValueTarget]] |  |
| [[IReferenceProvider\|IReferenceProvider]] |  |
| [[IRootObjectProvider\|IRootObjectProvider]] |  |
| [[IValueProvider\|IValueProvider]] |  |
| [[IXamlDataTypeProvider\|IXamlDataTypeProvider]] |  |
| [[IXamlTypeResolver\|IXamlTypeResolver]] |  |
| [[IXmlLineInfoProvider\|IXmlLineInfoProvider]] |  |

## See also

- [[_API Reference]]
