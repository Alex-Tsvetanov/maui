---
title: "RootComponentCollectionExtensions.Add<TComponent>"
tags:
  - api
  - member/method
  - ns/Microsoft-AspNetCore-Components-WebView-WindowsForms
aliases:
  - "Microsoft.AspNetCore.Components.WebView.WindowsForms.RootComponentCollectionExtensions.Add<TComponent>"
declaring_type: "RootComponentCollectionExtensions"
member_kind: method
---

# RootComponentCollectionExtensions.Add<TComponent>

> [!abstract] Method of [[RootComponentCollectionExtensions|RootComponentCollectionExtensions]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.WindowsForms`

Adds the component specified by `TComponent` to the collection specified by `components` to be associated with the selector specified by `selector` and to be instantiated with the parameters specified by `parameters`.

## Signature

```csharp
void static Add<TComponent>(this Microsoft.AspNetCore.Components.WebView.WindowsForms.RootComponentsCollection! components, string! selector, System.Collections.Generic.IDictionary<string!, object?>? parameters = null)
```

## Parameters

| Parameter | Description |
|---|---|
| `components` | The collection to which the component should be added. |
| `selector` | The selector to which the component will be associated. |
| `parameters` | The optional creation parameters for the component. |

## See also

- Declaring type: [[RootComponentCollectionExtensions|RootComponentCollectionExtensions]]
- [[_Microsoft.AspNetCore.Components.WebView.WindowsForms|Microsoft.AspNetCore.Components.WebView.WindowsForms namespace]]
