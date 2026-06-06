---
title: "SearchBar (Controls).SearchCommand"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.SearchBar.SearchCommand"
declaring_type: "SearchBar (Controls)"
member_kind: property
---

# SearchBar (Controls).SearchCommand

> [!abstract] Property of [[SearchBar (Controls)|SearchBar (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets the command to execute when the user performs a search. This is a bindable property.

## Signature

```csharp
System.Windows.Input.ICommand SearchCommand { get; set; }
```

## Remarks

This command is executed when the user presses the search button on the keyboard or when `OnSearchButtonPressed` is called. The `SearchCommandParameter` is passed as the command parameter.

## See also

- Declaring type: [[SearchBar (Controls)|SearchBar (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
