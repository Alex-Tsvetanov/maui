---
title: "ImageSource.FromResource"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ImageSource.FromResource"
declaring_type: "ImageSource"
member_kind: method
---

# ImageSource.FromResource

> [!abstract] Method of [[ImageSource|ImageSource]]
> Namespace: `Microsoft.Maui.Controls`

Creates an `ImageSource` from an embedded resource in the assembly containing the specified type.

## Signatures

```csharp
Microsoft.Maui.Controls.ImageSource static FromResource(string resource, System.Reflection.Assembly sourceAssembly = null)
Microsoft.Maui.Controls.ImageSource static FromResource(string resource, System.Type resolvingType)
```

## Parameters

| Parameter | Description |
|---|---|
| `resource` | The name of the embedded resource. |
| `resolvingType` | A type whose assembly contains the resource. |

## Returns

A `StreamImageSource` for the embedded resource.

## See also

- Declaring type: [[ImageSource|ImageSource]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
