---
title: "AppLinkEntry.IsLinkActive"
tags:
  - api
  - member/property
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.AppLinkEntry.IsLinkActive"
declaring_type: "AppLinkEntry"
member_kind: property
---

# AppLinkEntry.IsLinkActive

> [!abstract] Property of [[AppLinkEntry|AppLinkEntry]]
> Namespace: `Microsoft.Maui.Controls`

Gets or sets a value that tells whether the item that is identified by the link entry is currently open.

## Signature

```csharp
bool IsLinkActive { get; set; }
```

## Remarks

Application developers can set this value in `PageAppearing` and `PageDisappearing` methods to control whether the app link is shown for indexing or Handoff.

## See also

- Declaring type: [[AppLinkEntry|AppLinkEntry]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
