---
title: "BindingBase.EnableCollectionSynchronization"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.BindingBase.EnableCollectionSynchronization"
declaring_type: "BindingBase"
member_kind: method
---

# BindingBase.EnableCollectionSynchronization

> [!abstract] Method of [[BindingBase|BindingBase]]
> Namespace: `Microsoft.Maui.Controls`

Enables synchronized (thread-safe) access to `collection` using the supplied callback.

## Signature

```csharp
void static EnableCollectionSynchronization(System.Collections.IEnumerable collection, object context, Microsoft.Maui.Controls.CollectionSynchronizationCallback callback)
```

## Parameters

| Parameter | Description |
|---|---|
| `collection` | The collection that will be read or updated from multiple threads. |
| `context` | A context object (optionally a lock object) passed to `callback`; may be `null`. |
| `callback` | Delegate invoked by the framework to perform collection access under synchronization. |

## Remarks

The framework holds only a weak reference to the collection. The callback receives parameters indicating whether write access is required; implementers should perform appropriate locking (often on `context`) before invoking the supplied access delegate.

## See also

- Declaring type: [[BindingBase|BindingBase]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
