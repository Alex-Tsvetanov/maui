---
title: "VisualElement (Controls).BatchCommit"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.VisualElement.BatchCommit"
declaring_type: "VisualElement (Controls)"
member_kind: method
---

# VisualElement (Controls).BatchCommit

> [!abstract] Method of [[VisualElement (Controls)|VisualElement (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Signals the end of a batch of commands to the element and that those commands should now be committed.

## Signature

```csharp
void BatchCommit()
```

## Remarks

This method only ensures that updates sent during the batch have been committed. It does not ensure that they were not committed before calling this.

## See also

- Declaring type: [[VisualElement (Controls)|VisualElement (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
