# Advanced Usage

## Service Locator

## Assets

### Storage

`le::AssetMap` provides storage and safe destruction of `le::IAsset` subtypes, each instance mapped to a `le::Uri`. This is not a concurrent map, the correct way to load assets on multiple threads is described later.

### Locating Assets

Binary and text (including JSON) data can be loaded from the filesystem using `le::FileDataLoader`. The specification of the path is decoupled from the actual filesystem by using relative URIs that any custom `le::IDataLoader` can resolve (eg by treating it as a path inside a ZIP archive or a remote URL). `le::FileDataLoader` uses a prefix / mounted directory which it prefixes to incoming URIs to form the full filesystem path. This prefix must be chosen and set carefully, `le2d` provides two helpers for this purpose:

- `le::util::exe_path()` to obtain the path to the running executable (Windows / Linux compatible)
- `le::FileDataLoader::upfind()` to locate a subdirectory in a given directory or its parent directories

### Multi-threaded Loading

The high-level approach to loading assets on multiple threads is to do exactly that: perform isolated loads on worker threads, and at the end transfer all loaded assets to an external `le::AssetMap`. Three types help with this:

1. `le::ManifestEntry` is the encapsulation of a concrete `le::IAsset` type and its `le::Uri`.
1. `le::AssetManifest` is a list of `le::ManifestEntry`s.
1. `le::IManifestLoader` consumes an `le::AssetManifest` and loads the assets on worker threads (after which they are expected to be transferred to an external `le::AssetMap`).

See the `demo::LoadAssets` scene for a demonstration.

## Input System

WIP

### Router and Actions

For gameplay. One-way. Stack with single active receiver.

### Dispatch and Listeners

For UI / scenes. Two-way. Cascading stack.

## Console

## RenderPass and RenderTexture

## Animations
