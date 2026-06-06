---
title: "AsyncValueExtensions.AsAsyncValue<T>"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Internals
aliases:
  - "Microsoft.Maui.Controls.Internals.AsyncValueExtensions.AsAsyncValue<T>"
declaring_type: "AsyncValueExtensions"
member_kind: method
---

# AsyncValueExtensions.AsAsyncValue<T>

> [!abstract] Method of [[AsyncValueExtensions|AsyncValueExtensions]]
> Namespace: `Microsoft.Maui.Controls.Internals`

Wraps the specified task in an AsyncValue that exposes the result once the task completes, using the given default until then.

## Signature

```csharp
Microsoft.Maui.Controls.Internals.AsyncValue<T> static AsAsyncValue<T>(this System.Threading.Tasks.Task<T> valueTask, T defaultValue = default(T))
```

## See also

- Declaring type: [[AsyncValueExtensions|AsyncValueExtensions]]
- [[_Microsoft.Maui.Controls.Internals|Microsoft.Maui.Controls.Internals namespace]]
