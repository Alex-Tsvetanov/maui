---
title: "Pin.Equals"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls-Maps
aliases:
  - "Microsoft.Maui.Controls.Maps.Pin.Equals"
declaring_type: "Pin"
member_kind: method
---

# Pin.Equals

> [!abstract] Method of [[Pin|Pin]]
> Namespace: `Microsoft.Maui.Controls.Maps`

Compares this MemberAccess with another IPathPart for equality. Note: This implementation uses a non-standard equality contract to support test scenarios. Extended metadata fields (ContainingType, MemberType, Kind, accessibility flags) are only compared when both sides have them populated. This allows tests to create simplified MemberAccess instances (e.g., new MemberAccess("Name")) that will match production instances with full metadata, as long as core properties (MemberName, IsValueType) match. This violates the standard transitive property of equality but is intentional for backward compatibility with existing tests.

## Signature

```csharp
bool override Equals(object? obj)
```

## See also

- Declaring type: [[Pin|Pin]]
- [[_Microsoft.Maui.Controls.Maps|Microsoft.Maui.Controls.Maps namespace]]
