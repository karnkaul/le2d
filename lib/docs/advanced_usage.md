# Advanced Usage

## Assets

### Locating Assets

Binary and text (including JSON) data can be loaded from the filesystem using `le::FileDataLoader`. The specification of the path is decoupled from the actual filesystem by using relative URIs that any custom `le::IDataLoader` can resolve (eg by treating it as a path inside a ZIP archive or a remote URL). `le::FileDataLoader` uses a prefix / mounted directory which it prefixes to incoming URIs to form the full filesystem path. This prefix must be chosen and set carefully, `le2d` provides two helpers for this purpose:

- `le::util::exe_path()` to obtain the path to the running executable (Windows / Linux compatible)
- `le::FileDataLoader::upfind()` to locate a subdirectory in a given directory or its parent directories

### Multi-threaded Loading

WIP

## Input System

### Listener and Dispatch

### Controls

## Service Locator

## Animations

## Console
