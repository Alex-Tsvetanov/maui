---
title: "PathF.Close"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Graphics
aliases:
  - "Microsoft.Maui.Graphics.PathF.Close"
declaring_type: "PathF"
member_kind: method
---

# PathF.Close

> [!abstract] Method of [[PathF|PathF]]
> Namespace: `Microsoft.Maui.Graphics`

Closes the current sub-path by appending a close segment if it is not already closed.

## Signature

```csharp
void Close()
```

## Remarks

Closing a path is typically required for fill operations to work correctly. Attempting to fill an unclosed path may result in undefined behavior or exceptions in some graphics implementations. A closed path ensures that the shape is properly defined for filling operations.

## See also

- Declaring type: [[PathF|PathF]]
- [[_Microsoft.Maui.Graphics|Microsoft.Maui.Graphics namespace]]
