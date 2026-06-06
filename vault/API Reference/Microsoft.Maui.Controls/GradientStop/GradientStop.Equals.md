---
title: "GradientStop.Equals"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.GradientStop.Equals"
declaring_type: "GradientStop"
member_kind: method
---

# GradientStop.Equals

> [!abstract] Method of [[GradientStop|GradientStop]]
> Namespace: `Microsoft.Maui.Controls`

Compares this MemberAccess with another IPathPart for equality. Note: This implementation uses a non-standard equality contract to support test scenarios. Extended metadata fields (ContainingType, MemberType, Kind, accessibility flags) are only compared when both sides have them populated. This allows tests to create simplified MemberAccess instances (e.g., new MemberAccess("Name")) that will match production instances with full metadata, as long as core properties (MemberName, IsValueType) match. This violates the standard transitive property of equality but is intentional for backward compatibility with existing tests.

## Signature

```csharp
bool override Equals(object obj)
```

## See also

- Declaring type: [[GradientStop|GradientStop]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
