# Library Concepts

## Vulkan

Vulkan is loaded dynamically at runtime, it is not linked to during a build. This allows applications to fail gracefully if Vulkan is not present (or if the driver is incompatible with Vulkan 1.3).

### Validation Layers

There is no validation layer code in the library or its dependencies. Users are expected to use Vulkan Configurator during development, and ensure that the layers are being loaded correctly. `le2d` is **NOT** designed to "evade" / wrap validation errors, it is **highly recommended** to always have the layers turned on during development. Even basic use of the engine warrants order-correct destruction of resources like `le::Texture` vs `le::RenderWindow`, where errors can slip by unless validation layers are enabled.

## Object Ownership

All class-type objects returned by the library use RAII and clean up on destruction. There must be only a single `le::Context` object and it must outlive all `le::IResource` objects created since (eg `le::ITexture`). If a type requires observation of another as part of its invariant, it will take that as a `gsl::not_null<T*>` parameter in its constructor to indicate that the passed pointer will be stored and used later, thus must remain address-stable.

### Shared Resources

Subtypes of `le::IResource` represent resources that are shared between the CPU (user) and the graphics or audio device. Such objects have two constraints in terms of lifetime:

1. Must not outlive `le::Context`
2. Must outlive usage by device

The first is an application-level concern, where `le::Context` must not be destroyed before all `le::IResource`s have been destroyed. The second is a per-resource concern, eg an `le::ITexture` must not be destroyed while it's being read by the GPU in a draw call; similarly, an `le::IAudioBuffer` must outlive playback. The simplest way to ensure this is to call `le::Context::wait_idle()` before any resource's destruction. `le::Context::Waiter` is an RAII wrapper that calls this in its destructor, an active instance as the last member of a type will ensure this is called before any other members are destroyed.

## Multi-threading

The engine is designed to trivially support multi-threaded resource creation / asset loading, there is no external synchronization required for this. There is no support for multi-threaded rendering, with the main noticeable consequence being that if event polling gets blocked (eg by dragging the corner of a window on Windows), so will rendering. Similarly, while double-buffering will pipeline sequential renders, if every frame takes too long / frame time is too high, expect drops in framerate.
