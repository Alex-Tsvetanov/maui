---
title: "ProgressBar (Controls).ProgressTo"
tags:
  - api
  - member/method
  - ns/Microsoft-Maui-Controls
aliases:
  - "Microsoft.Maui.Controls.ProgressBar.ProgressTo"
declaring_type: "ProgressBar (Controls)"
member_kind: method
---

# ProgressBar (Controls).ProgressTo

> [!abstract] Method of [[ProgressBar (Controls)|ProgressBar (Controls)]]
> Namespace: `Microsoft.Maui.Controls`

Animates the `Progress` property from its current value to the specified value.

## Signature

```csharp
System.Threading.Tasks.Task<bool> ProgressTo(double value, uint length, Microsoft.Maui.Easing easing)
```

## Returns

A task that completes when the animation finishes, with a result indicating whether the animation completed successfully.

## Parameters

| Parameter | Description |
|---|---|
| `value` | The target progress value (0.0 to 1.0). |
| `length` | The length of the animation in milliseconds. |
| `easing` | The easing function to use for the animation. |

## See also

- Declaring type: [[ProgressBar (Controls)|ProgressBar (Controls)]]
- [[_Microsoft.Maui.Controls|Microsoft.Maui.Controls namespace]]
