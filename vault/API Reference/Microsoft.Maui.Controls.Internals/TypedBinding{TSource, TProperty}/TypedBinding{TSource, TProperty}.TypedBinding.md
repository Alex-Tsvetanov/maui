---
title: "TypedBinding<TSource, TProperty>.TypedBinding"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui-Controls-Internals
aliases:
  - "Microsoft.Maui.Controls.Internals.TypedBinding<TSource, TProperty>.TypedBinding"
declaring_type: "TypedBinding<TSource, TProperty>"
member_kind: constructor
---

# TypedBinding<TSource, TProperty>.TypedBinding

> [!abstract] Constructor of [[TypedBinding{TSource, TProperty}|TypedBinding<TSource, TProperty>]]
> Namespace: `Microsoft.Maui.Controls.Internals`

Initializes a new strongly typed binding with the specified getter, setter, and property change handlers.

## Signature

```csharp
void Microsoft.Maui.Controls.Internals.TypedBinding<TSource, TProperty>.TypedBinding(System.Func<TSource, (TProperty value, bool success)> getter, System.Action<TSource, TProperty> setter, System.Tuple<System.Func<TSource, object>, string>[] handlers)
```

## See also

- Declaring type: [[TypedBinding{TSource, TProperty}|TypedBinding<TSource, TProperty>]]
- [[_Microsoft.Maui.Controls.Internals|Microsoft.Maui.Controls.Internals namespace]]
