---
title: "PropertyMapperExtensions.ReplaceMapping<TVirtualView, TViewHandler>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.PropertyMapperExtensions.ReplaceMapping<TVirtualView, TViewHandler>"
declaring_type: "PropertyMapperExtensions"
member_kind: method
---

# PropertyMapperExtensions.ReplaceMapping<TVirtualView, TViewHandler>

> [!abstract] Method of [[PropertyMapperExtensions|PropertyMapperExtensions]]
> Namespace: `Microsoft.Maui`

Replace a property mapping in place but call the previous mapping if the types do not match.

## Signature

```csharp
void static ReplaceMapping<TVirtualView, TViewHandler>(this Microsoft.Maui.IPropertyMapper<Microsoft.Maui.IElement!, Microsoft.Maui.IElementHandler!>! propertyMapper, string! key, System.Action<TViewHandler, TVirtualView>! method)
```

## Parameters

| Parameter | Description |
|---|---|
| `propertyMapper` | The property mapper in which to change the mapping. |
| `key` | The name of the property. |
| `method` | The modified method to call when the property is updated. |

## See also

- Declaring type: [[PropertyMapperExtensions|PropertyMapperExtensions]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
