---
title: "Routing.GetOrCreateContent"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.Routing.GetOrCreateContent"
declaring_type: "Routing"
member_kind: method
---

# Routing.GetOrCreateContent

> [!abstract] Method of [[Routing|Routing]]
> Namespace: `Microsoft.Maui.Controls`

Gets or creates content for the specified route using dependency injection services.

## Signature

```csharp
Microsoft.Maui.Controls.Element static GetOrCreateContent(string route, System.IServiceProvider services = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `route` | The route string to get or create content for. |
| `services` | Optional service provider for dependency injection when creating new content instances. |

## Returns

The Element associated with the route, or null if not found and cannot be created.

## See also

- Declaring type: [[Routing|Routing]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
