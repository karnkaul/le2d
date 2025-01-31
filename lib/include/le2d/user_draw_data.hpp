#pragma once
#include <kvf/buffer_write.hpp>

namespace le {
class ITexture;

struct UserDrawData {
	kvf::BufferWrite ssbo{};
	ITexture const* texture{};
};
} // namespace le
