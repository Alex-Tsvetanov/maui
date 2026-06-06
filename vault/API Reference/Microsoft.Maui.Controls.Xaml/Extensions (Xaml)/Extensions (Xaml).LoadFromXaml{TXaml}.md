---
title: "Extensions (Xaml).LoadFromXaml<TXaml>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Xaml
aliases:
  - "Microsoft.Maui.Controls.Xaml.Extensions.LoadFromXaml<TXaml>"
declaring_type: "Extensions (Xaml)"
member_kind: method
---

# Extensions (Xaml).LoadFromXaml<TXaml>

> [!abstract] Method of [[Extensions (Xaml)|Extensions (Xaml)]]
> Namespace: `Microsoft.Maui.Controls.Xaml`

Loads the XAML associated with the specified type into the view.

## Signatures

```csharp
TXaml static LoadFromXaml<TXaml>(this TXaml view, string xaml)
TXaml static LoadFromXaml<TXaml>(this TXaml view, System.Type callingType)
```

## Returns

The view with XAML loaded.

## Parameters

| Parameter | Description |
|---|---|
| `view` | The view to load XAML into. |
| `callingType` | The type used to locate the XAML resource. |

## See also

- Declaring type: [[Extensions (Xaml)|Extensions (Xaml)]]
- [[_Microsoft.Maui.Controls.Xaml|Microsoft.Maui.Controls.Xaml namespace]]
