---
title: "Location.Equals"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Devices-Sensors
aliases:
  - "Microsoft.Maui.Devices.Sensors.Location.Equals"
declaring_type: "Location"
member_kind: method
---

# Location.Equals

> [!abstract] Method of [[Location|Location]]
> Namespace: `Microsoft.Maui.Devices.Sensors`

Compares this MemberAccess with another IPathPart for equality. Note: This implementation uses a non-standard equality contract to support test scenarios. Extended metadata fields (ContainingType, MemberType, Kind, accessibility flags) are only compared when both sides have them populated. This allows tests to create simplified MemberAccess instances (e.g., new MemberAccess("Name")) that will match production instances with full metadata, as long as core properties (MemberName, IsValueType) match. This violates the standard transitive property of equality but is intentional for backward compatibility with existing tests.

## Signature

```csharp
bool override Equals(object obj)
```

## See also

- Declaring type: [[Location|Location]]
- [[_Microsoft.Maui.Devices.Sensors|Microsoft.Maui.Devices.Sensors namespace]]
