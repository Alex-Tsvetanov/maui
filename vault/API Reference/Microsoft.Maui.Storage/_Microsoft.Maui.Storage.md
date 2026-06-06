---
title: "Microsoft.Maui.Storage"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-Storage
---

# Microsoft.Maui.Storage

> [!info] Namespace
> `Microsoft.Maui.Storage` — 18 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.storage)

## Overview

`Microsoft.Maui.Storage` provides the cross-platform device storage primitives that .NET MAUI apps use to persist data and interact with the file system. It groups four related concerns behind small, focused APIs: lightweight settings, secrets, the device file system, and user-driven file picking — each abstracted so the same code runs across Android, iOS, macOS, and Windows.

For simple persisted settings, [[Preferences|Preferences]] (and its [[IPreferences|IPreferences]] contract) stores application preferences in a key/value store, ideal for flags, last-used values, and lightweight configuration. When the data is sensitive — tokens, passwords, or other credentials — [[SecureStorage|SecureStorage]] and [[ISecureStorage|ISecureStorage]] securely store simple key/value pairs using the platform's encrypted backing store, with [[SecureStorageExtensions|SecureStorageExtensions]] adding convenience helpers.

For file access, [[FileSystem|FileSystem]] and [[IFileSystem|IFileSystem]] expose the platform locations for app data and cached/temporary data, so you do not have to hard-code per-platform paths. To let users choose existing files, [[FilePicker|FilePicker]] and [[IFilePicker|IFilePicker]] open the native picker; [[PickOptions|PickOptions]] and [[FilePickerFileType|FilePickerFileType]] customize the prompt and constrain selectable types, and the chosen file comes back as a [[FileResult|FileResult]]. File content is modeled by [[FileBase|FileBase]] and the read-only [[ReadOnlyFile|ReadOnlyFile]], each carrying the file and its content type.

> [!tip] Use the static facades (`Preferences`, `SecureStorage`, `FileSystem`, `FilePicker`) for quick access, or inject the matching interfaces (`IPreferences`, `ISecureStorage`, `IFileSystem`, `IFilePicker`) when you want testable, dependency-injected code.

## Key types

- [[Preferences|Preferences]] — Key/value store for simple application preferences and settings.
- [[IPreferences|IPreferences]] — Abstraction for the Preferences key/value store.
- [[SecureStorage|SecureStorage]] — Securely stores simple key/value pairs in the platform's encrypted store.
- [[ISecureStorage|ISecureStorage]] — Abstraction for securely storing simple key/value pairs.
- [[FileSystem|FileSystem]] — Access to app data and temporary/cache storage locations.
- [[IFileSystem|IFileSystem]] — Abstraction for accessing device folder locations.
- [[FilePicker|FilePicker]] — Opens the default file picker so the user can pick a file.
- [[IFilePicker|IFilePicker]] — Abstraction that lets the user pick a file from device storage.
- [[PickOptions|PickOptions]] — Options that customize how the file picker behaves.
- [[FilePickerFileType|FilePickerFileType]] — Constrains the file types a user is allowed to pick.
- [[FileResult|FileResult]] — Represents a file returned from a pick action, with its content type.
- [[FileBase|FileBase]] — Base representation of a file and its content type.


## Classes

| Type | Summary |
|---|---|
| [[FileBase\|FileBase]] | A representation of a file and its content type. |
| [[FilePicker\|FilePicker]] | Opens the default file picker to allow the user to pick a single file. |
| [[FilePickerFileType\|FilePickerFileType]] | Represents the file types that are allowed to be picked by the user when using `IFilePicker`. |
| [[FileProvider\|FileProvider]] |  |
| [[FileProviderLocation\|FileProviderLocation]] |  |
| [[FileResult\|FileResult]] | A representation of a file, as a result of a pick action by the user, and its content type. |
| [[FileSystem\|FileSystem]] | Gets the location where temporary data can be stored. |
| [[FileSystemImplementation\|FileSystemImplementation]] | Concrete implementation of the `IFileSystem` APIs. |
| [[PickOptions\|PickOptions]] | Represents file picking options that can be used to customize the working of `IFilePicker`. |
| [[Preferences\|Preferences]] | Checks for the existence of a given key. |
| [[ReadOnlyFile\|ReadOnlyFile]] | A representation of a file, that is read-only, and its content type. |
| [[SecureStorage\|SecureStorage]] | Default `SecAccessible` to use for all Get/Set calls to KeyChain. Default value is `AfterFirstUnlock`. |
| [[SecureStorageExtensions\|SecureStorageExtensions]] | This class contains static extension methods for use with `ISecureStorage`. |

## Interfaces

| Type | Summary |
|---|---|
| [[IFilePicker\|IFilePicker]] | Lets the user pick a file from the device's storage. |
| [[IFileSystem\|IFileSystem]] | Provides an easy way to access the locations for device folders. |
| [[IPlatformSecureStorage\|IPlatformSecureStorage]] | Gets and decrypts the value for a given key. |
| [[IPreferences\|IPreferences]] | The Preferences API helps to store application preferences in a key/value store. |
| [[ISecureStorage\|ISecureStorage]] | The SecureStorage API helps securely store simple key/value pairs. |

## See also

- [[_API Reference]]
