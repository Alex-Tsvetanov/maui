---
title: "RootComponent (WindowsForms).RootComponent"
tags:
  - api
  - member/constructor
  - ns/Microsoft-AspNetCore-Components-WebView-WindowsForms
aliases:
  - "Microsoft.AspNetCore.Components.WebView.WindowsForms.RootComponent.RootComponent"
declaring_type: "RootComponent (WindowsForms)"
member_kind: constructor
---

# RootComponent (WindowsForms).RootComponent

> [!abstract] Constructor of [[RootComponent (WindowsForms)|RootComponent (WindowsForms)]]
> Namespace: `Microsoft.AspNetCore.Components.WebView.WindowsForms`

Constructs an instance of `RootComponent`.

## Signature

```csharp
void RootComponent(string! selector, System.Type! componentType, System.Collections.Generic.IDictionary<string!, object?>? parameters)
```

## Parameters

| Parameter | Description |
|---|---|
| `selector` | The CSS selector string that specifies where in the document the component should be placed. This must be unique among the root components within the `BlazorWebView`. |
| `componentType` | The type of the root component. This type must implement `IComponent`. |
| `parameters` | An optional dictionary of parameters to pass to the root component. |

## See also

- Declaring type: [[RootComponent (WindowsForms)|RootComponent (WindowsForms)]]
- [[_Microsoft.AspNetCore.Components.WebView.WindowsForms|Microsoft.AspNetCore.Components.WebView.WindowsForms namespace]]
