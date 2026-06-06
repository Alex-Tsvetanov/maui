---
title: "DevFlow overview"
description: "Learn about DevFlow, a testing, automation, and debugging toolkit for .NET MAUI applications that provides visual tree inspection, a CLI, Blazor WebView debugging, and AI agent integration."
tags:
  - conceptual
  - area/developer-tools
ms_date: "05/08/2026"
source: "https://learn.microsoft.com/dotnet/maui/developer-tools/devflow?view=net-maui-10.0"
---

# DevFlow overview

DevFlow is a comprehensive testing, automation, and debugging toolkit for .NET MAUI applications. It provides an in-app HTTP agent for visual tree inspection and element interaction, a CLI with 50+ commands, a Blazor WebView CDP bridge, and an MCP server for AI agent integration.

> [!IMPORTANT]
> DevFlow is experimental and will change between releases.

## Architecture

DevFlow consists of three components that work together:

- **Agent**: In-app HTTP server that exposes the visual tree, element interactions, screenshots, and profiling. Runs inside the MAUI app process.
- **CLI**: Command-line interface (`maui devflow`) that communicates with the agent. Integrated into the .NET MAUI CLI.
- **Broker**: Lightweight background daemon that coordinates port assignment and agent discovery when multiple apps run simultaneously. See [[broker|Broker architecture]].

## Get started

### Step 1: Install the NuGet packages

Add the DevFlow agent package to your .NET MAUI project. If your app uses Blazor Hybrid, also add the Blazor package:

```xml
<PackageReference Include="Microsoft.Maui.DevFlow.Agent" />
<PackageReference Include="Microsoft.Maui.DevFlow.Blazor" />  <!-- If using Blazor Hybrid -->
```

### Step 2: Register the agent in MauiProgram.cs

Register the DevFlow agent in your app's startup code:

```csharp
using Microsoft.Maui.DevFlow.Agent;

public static MauiApp CreateMauiApp()
{
    var builder = MauiApp.CreateBuilder();
    builder.UseMauiApp<App>();

    #if DEBUG
    builder.AddMauiDevFlowAgent();
    #endif

    return builder.Build();
}
```

> [!NOTE]
> Wrap the `AddMauiDevFlowAgent` call in `#if DEBUG` to ensure the agent isn't included in release builds.

### Step 3: Install the CLI tool

Install the DevFlow CLI as a global .NET tool:

```bash
dotnet tool install -g Microsoft.Maui.Cli --prerelease
```

### Step 4: Run your app and interact

With your app running, use the CLI to inspect and interact with it:

```bash
# View the visual tree
maui devflow MAUI tree

# Take a screenshot
maui devflow MAUI screenshot -o screenshot.png

# Tap an element
maui devflow agent interact tap --automationid "MyButton"

# Start MCP server for AI agents
maui devflow mcp
```

## Features

- [[visual-tree-screenshots|Visual tree inspection and screenshots]] — query the visual tree and capture PNG screenshots
- [[element-interaction|Element interaction and automation]] — tap, fill, scroll, navigate, and mutate element properties
- [[blazor-cdp|Blazor WebView debugging]] — Chrome DevTools Protocol bridge for Blazor Hybrid content
- [[mcp-server|MCP server for AI agents]] — 15 tool categories for AI agent integration
- [[network-profiling|Network monitoring and profiling]] — HTTP interception, CPU/memory/GC profiling, jank detection
- [[storage|File storage access]] — discover file storage roots and manage sandboxed app files remotely
- [[broker|Broker architecture]] — multi-app port assignment and agent discovery

## Packages

| Package | Description |
|---------|-------------|
| `Microsoft.Maui.DevFlow.Agent` | In-app agent for .NET MAUI apps. Exposes visual tree, element interactions, screenshots, and profiling via HTTP/JSON API. |
| `Microsoft.Maui.DevFlow.Agent.Core` | Platform-agnostic core: HTTP server, visual tree walker, CSS selector engine, network capture, profiling. |
| `Microsoft.Maui.DevFlow.Agent.Gtk` | GTK/Linux agent for Maui.Gtk apps. |
| `Microsoft.Maui.DevFlow.Blazor` | Blazor WebView CDP bridge. Enables Chrome DevTools Protocol access for Blazor Hybrid content. |
| `Microsoft.Maui.DevFlow.Blazor.Gtk` | Blazor CDP bridge for WebKitGTK on Linux. |
| `Microsoft.Maui.DevFlow.Driver` | Platform-aware app driver for iOS, Android, Mac Catalyst, Windows, and Linux. |
| `Microsoft.Maui.DevFlow.Logging` | Buffered rotating JSONL file logger. No MAUI dependency. |

## Platform support

| Platform | Agent status | Notes |
|----------|-------------|-------|
| Mac Catalyst | ✅ Supported | Direct localhost connectivity |
| iOS Simulator | ✅ Supported | Shares host network |
| Linux/GTK | ✅ Supported | Direct localhost connectivity |
| Android | 🔄 In progress | Requires `adb reverse` for port forwarding |
| Windows | 🔄 In progress | — |

## Platform setup

- [[setup-android|Android setup]]
- [[setup-apple|Apple platforms setup]]
- [[setup-windows|Windows setup]]

## See also

- [[cli|.NET MAUI CLI overview]]
