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

`le::Context::event_queue()` returns an ordered view into the window `le::Event`s for the current frame. This is generally fine for simple applications, but gets complicated with multiple scenes and dynamic + blocking UI, for which routing and action-binding systems are provided.

Use `le::input::KeyTrigger`, `le::input::MouseButtonTrigger`, `le::input::KeyChord`, and `le::input::MouseButtonChord` to reduce boilerplate. Triggers engage on key/button press and disengage on release, chords engage on a matching key/button action and support key modifiers.

### Mappings, Actions, and Router

An `le::input::IMapping` represents a consumer of events (and gamepad states). An incoming event can be observed, or "consumed" - in which case it will not propagate any further. (This is irrelevant unless multiple mappings are active and bound simultaneously.) Mappings can combine multiple input events into a higher-level construct (eg a 2D axis), which is why `le::input::IMapping::dispatch_events()` exists, and is called after all events have been processed.

The most direct kind of mapping is `le::input::ListenerMapping`, which exposes optional callbacks for each `le::input::IMapping` customization point.

An `input::action` type represents one of various kinds of device inputs (eg digital for off/on, axis for 0-1). An action can be bound to a callback via an `le::input::ActionMapping` instance. Each callback is invoked with an `input::action::Value` which can be cast to a `bool` (digital), `float` (1D axis), or `glm::vec2` (2D axis).

An `le::input::Router` stores a mutable stack of weak-pointers to input mappings. The stack is cycled (in reverse) for each event, until a mapping consumes it. Instead of looping over the event queue, pass it to the router every frame via `le::input::Router::dispatch()`.

Most of the time a single persistent mapping will be sufficient, the stack is useful for eg having a different mapping for popup/modal navigation or multiple active (non-overlapping) device controllers.

See the `demo::InputActions` scene for a demonstration.

## Console

`le::console::ITerminal` is a interface to a basic command-line terminal with text input. A monospace font is recommended for use with it. Bind its input mapping via `le::console::ITerminal::get_mapping()` to `le::input::Router::terminal_mapping` for automatic event routing between the console and other bound mappings.

See the `demo::TextBoxConsole` scene for a demonstration.

## RenderPass and RenderTexture

WIP

## Animations
