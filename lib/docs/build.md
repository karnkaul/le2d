## Build

### Requirements

- CMake 3.24
- C++23 compiler (and stdlib)

### Integration

`le2d` uses CMake, and expects to be built and linked as a static library. The suggested workflow is to use `FetchContent` or `git clone` / vendor the repository + use `add_subdirectory()`.

Once imported into the build tree, link to the `le2d::le2d` target. All headers will be accessible via an `le2d/` prefix, eg:

```cpp
#include <le2d/context.hpp>
```
