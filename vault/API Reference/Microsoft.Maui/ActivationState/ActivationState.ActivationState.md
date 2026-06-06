---
title: "ActivationState.ActivationState"
tags:
  - api
  - member/constructor
  - ns/Microsoft-Maui
aliases:
  - "Microsoft.Maui.ActivationState.ActivationState"
declaring_type: "ActivationState"
member_kind: constructor
---

# ActivationState.ActivationState

> [!abstract] Constructor of [[ActivationState|ActivationState]]
> Namespace: `Microsoft.Maui`

Initializes a new instance of the ActivationState class with the specified MAUI context and, depending on the overload, persisted or platform-specific launch state.

## Signatures

```csharp
void ActivationState(Microsoft.Maui.IMauiContext! context, Android.OS.Bundle? savedInstance)
void ActivationState(Microsoft.Maui.IMauiContext! context, Microsoft.Maui.IPersistedState! state)
void ActivationState(Microsoft.Maui.IMauiContext! context)
void ActivationState(Microsoft.Maui.IMauiContext! context, Foundation.NSDictionary![]? states)
void ActivationState(Microsoft.Maui.IMauiContext! context, Tizen.Applications.Bundle? savedInstance)
void ActivationState(Microsoft.Maui.IMauiContext! context, Microsoft.UI.Xaml.LaunchActivatedEventArgs? launchActivatedEventArgs)
```

## See also

- Declaring type: [[ActivationState|ActivationState]]
- [[_Microsoft.Maui|Microsoft.Maui namespace]]
