---
title: "DependencyResolver.ResolveUsing"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Internals
aliases:
  - "Microsoft.Maui.Controls.Internals.DependencyResolver.ResolveUsing"
declaring_type: "DependencyResolver"
member_kind: method
---

# DependencyResolver.ResolveUsing

> [!abstract] Method of [[DependencyResolver|DependencyResolver]]
> Namespace: `Microsoft.Maui.Controls.Internals`

Sets a resolver function that takes a type and constructor arguments.

## Signatures

```csharp
void static ResolveUsing(System.Func<System.Type, object[], object> resolver)
void static ResolveUsing(System.Func<System.Type, object> resolver)
```

## Parameters

| Parameter | Description |
|---|---|
| `resolver` | The resolver function. |

## See also

- Declaring type: [[DependencyResolver|DependencyResolver]]
- [[_Microsoft.Maui.Controls.Internals|Microsoft.Maui.Controls.Internals namespace]]
