#pragma once
#include "klib/ptr.hpp"
#include "kvf/buffer_write.hpp"

namespace le {
class ITextureBase;

struct UserDrawData {
	kvf::BufferWrite ssbo{};
	klib::Ptr<ITextureBase const> texture{};
};
} // namespace le
