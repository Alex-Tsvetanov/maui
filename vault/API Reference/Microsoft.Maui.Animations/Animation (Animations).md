---
title: "Animation (Animations)"
tags:
  - api
  - kind/class
  - ns/Microsoft-Maui-Animations
aliases:
  - "Microsoft.Maui.Animations.Animation"
namespace: "Microsoft.Maui.Animations"
kind: class
platforms:
  - All platforms (.NET)
  - Android
  - iOS
  - Mac Catalyst
  - Windows
  - Tizen
  - .NET Standard
  - .NET Standard 2.0
assemblies:
  - src
---

# Animation (Animations)

> [!abstract] Class in `Microsoft.Maui.Animations`
> Full name: `Microsoft.Maui.Animations.Animation`

Represents an animation.

## Platforms

| Platform | Available |
|---|---|
| All platforms (.NET) | ✅ |
| Android | ✅ |
| iOS | ✅ |
| Mac Catalyst | ✅ |
| Windows | ✅ |
| Tizen | ✅ |
| .NET Standard | ✅ |
| .NET Standard 2.0 | ✅ |


## Constructors

| Name | Summary |
|---|---|
| [[Animation (Animations).Animation\|Animation]] | Instantiate a new `Animation` object. |

## Properties

| Name | Summary |
|---|---|
| [[Animation (Animations).AnimationManager\|AnimationManager]] | A reference to the `IAnimationManager` that manages this animation. |
| [[Animation (Animations).CurrentTime\|CurrentTime]] | The current timestamp (in seconds) of the animation. |
| [[Animation (Animations).Duration\|Duration]] | The duration of this animation in seconds. |
| [[Animation (Animations).Easing\|Easing]] | The `Easing` function that is applied to this animation. |
| [[Animation (Animations).Finished\|Finished]] | A callback that is invoked when this animation finishes. |
| [[Animation (Animations).HasFinished\|HasFinished]] | Specifies whether this animation has finished. |
| [[Animation (Animations).IsDisposed\|IsDisposed]] | Gets a value that specifies if this animation has been disposed. |
| [[Animation (Animations).IsPaused\|IsPaused]] | Specifies whether this animation is currently paused. |
| [[Animation (Animations).Name\|Name]] | The name of this animation. |
| [[Animation (Animations).Progress\|Progress]] | Progress of this animation in percentage. |
| [[Animation (Animations).Repeats\|Repeats]] | Specifies whether this animation should repeat. |
| [[Animation (Animations).StartDelay\|StartDelay]] | The delay (in seconds) taken into account before the animation starts. |
| [[Animation (Animations).Step\|Step]] |  |

## Methods

| Name | Summary |
|---|---|
| [[Animation (Animations).Add\|Add]] | Adds a new child animation to this animation with the specified parameters. |
| [[Animation (Animations).Commit\|Commit]] | Sets the `IAnimationManager` for this animation. |
| [[Animation (Animations).CreateAutoReversing\|CreateAutoReversing]] | Creates an animation that includes both the original animation and a reversed version of the same animation. |
| [[Animation (Animations).CreateReverse\|CreateReverse]] | Creates a reversed version of the current animation, including reversing the child animations. |
| [[Animation (Animations).Dispose\|Dispose]] |  |
| [[Animation (Animations).GetEnumerator\|GetEnumerator]] | Provides an `IEnumerator` of the child animations. |
| [[Animation (Animations).OnTick\|OnTick]] | Executes the logic to update all animations within this animation. |
| [[Animation (Animations).Pause\|Pause]] | Pauses the animation. |
| [[Animation (Animations).RemoveFromParent\|RemoveFromParent]] | Removes this animation from it's parent. If there is no parent, nothing will happen. |
| [[Animation (Animations).Reset\|Reset]] | Resets the animation (and all child animations) to its initial state. |
| [[Animation (Animations).Resume\|Resume]] | Resumes the animation. |
| [[Animation (Animations).Tick\|Tick]] | Method to trigger an update for this animation. |
| [[Animation (Animations).Update\|Update]] | Updates this animation by updating `Progress` and invoking `Step`. |

## Fields

| Name | Summary |
|---|---|
| [[Animation (Animations).animationManger\|animationManger]] | A reference to the `IAnimationManager` that manages this animation. |
| [[Animation (Animations).childrenAnimations\|childrenAnimations]] |  |

## See also

- [[_Microsoft.Maui.Animations|Microsoft.Maui.Animations namespace]]
- [Online API docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.animations.animation)
