# Library Concepts

## Vulkan

Vulkan is loaded dynamically at runtime, it is not linked to during a build. This allows applications to fail gracefully if Vulkan is not present (or if the driver is incompatible with Vulkan 1.3).

### Validation Layers

There is no validation layer code in the library or its dependencies. Users are expected to use Vulkan Configurator during development, and ensure that the layers are being loaded correctly. `le2d` is **NOT** designed to "evade" / wrap validation errors, it is **highly recommended** to always have the layers turned on during development. Even basic use of the engine warrants order-correct destruction of resources like `le::Texture` vs `le::RenderWindow`, where errors can slip by unless validation layers are enabled.

## Object Ownership

All class-type objects returned by the library use RAII and clean up on destruction. Some objects, eg `le::RenderWindow` and/or `le::Context`, must outlive any objects created using them, eg `le::Texture`. If a type requires observation of another as part of its invariant, it will take that as a `gsl::not_null<T*>` parameter in its constructor to indicate that the passed pointer will be stored and used later, thus must remain address-stable.

### Buffered Resources

Rendering is double-buffered, with commands for frame N+1 being recorded on the CPU while the commands for frame N are executed on the GPU (after which the image is submitted to the display engine / compositor for presentation). Since the user owns Vulkan resources associated with a `le::Texture` object, they must ensure it does not get destroyed while in use by the GPU. The simplest way to prevent this is to use `kvf::DeviceBlock` to block the calling thread (in its destructor) or to call `le::Context::wait_idle()` before the texture's destruction. Such blocking will also be needed for `le::Context` and/or `le::RenderPass` objects, since they own Vulkan images as offscreen render targets.

Note that while interaction with the Audio Device is not explicitly double-buffered, the basic concept of "don't destroy the AudioBuffer before the AudioSouce that's playing it" still applies.

Most of the time this will simply boil down to the order of members in your own classes, and perhaps a `kvf::DeviceBlock` member at the end.

## Multi-threading

The engine is designed to trivially support multi-threaded resource creation / asset loading, there is no external synchronization required for this. There is no support for multi-threaded rendering, with the main noticeable consequence being that if event polling gets blocked (eg by dragging the corner of a window on Windows), so will rendering. Similarly, while double-buffering will pipeline sequential renders, if every frame takes too long / frame time is too high, expect drops in framerate.
