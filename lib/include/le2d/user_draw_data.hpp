#pragma once
#include <kvf/buffer_write.hpp>

namespace le {
class ITextureBase;

struct UserDrawData {
	kvf::BufferWrite ssbo{};
	ITextureBase const* texture{};
};
} // namespace le
