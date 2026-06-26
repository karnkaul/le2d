#pragma once
#include "klib/base_types.hpp"
#include "le2d/texture_sampler.hpp"

namespace le {
class ISamplerFactory : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_or_create(TextureSampler const& in) -> vk::Sampler = 0;
};
} // namespace le
