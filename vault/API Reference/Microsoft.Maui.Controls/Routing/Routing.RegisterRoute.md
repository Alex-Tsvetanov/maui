---
title: "Routing.RegisterRoute"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Routing.RegisterRoute"
declaring_type: "Routing"
member_kind: method
---

# Routing.RegisterRoute

> [!abstract] Method of [[Routing|Routing]]
> Namespace: `Microsoft.Maui.Controls`

Registers a route with a custom factory for creating navigation content.

## Signatures

```csharp
void static RegisterRoute(string route, Microsoft.Maui.Controls.RouteFactory factory)
void static RegisterRoute(string route, System.Type type)
```

## Parameters

| Parameter | Description |
|---|---|
| `route` | The route string to register. |
| `factory` | The factory that creates elements for this route. |

## See also

- Declaring type: [[Routing|Routing]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
