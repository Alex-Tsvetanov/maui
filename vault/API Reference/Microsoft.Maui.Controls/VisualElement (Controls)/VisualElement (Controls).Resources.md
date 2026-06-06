---
title: "VisualElement (Controls).Resources"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.Resources"
declaring_type: "VisualElement (Controls)"
member_kind: property
---

# VisualElement (Controls).Resources

> [!abstract] Property of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the local resource dictionary.

## Signature

```csharp
Microsoft.Maui.Controls.ResourceDictionary Resources { get; set; }
```

## Remarks

In XAML, resource dictionaries are filled with key/value pairs that are specified in XAML and consequently created at runtime. The keys in the resource dictionary are specified with the x:Key attribute of the XAML tag for the type to create. An object of that type is created, and is initialized with the property and field values that are specified either by additional attributes or by nested tags, both of which, when present, are simply string representations of the property or field names. The object is then inserted into the `ResourceDictionary` for the enclosing type at runtime. Resource dictionaries and their associated XML provide the application developer with a convenient method to reuse code inside the XAML compile-time and runtime engines. For more information, see: Resource Dictionaries (Microsoft Learn).

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
