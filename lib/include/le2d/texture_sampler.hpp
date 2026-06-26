#pragma once
#include <vulkan/vulkan.hpp>

namespace le {
/// \brief Texture Sampler metadata.
struct TextureSampler {
	auto operator==(TextureSampler const&) const -> bool = default;

	vk::SamplerAddressMode wrap{vk::SamplerAddressMode::eClampToEdge};
	vk::Filter filter{vk::Filter::eLinear};
	vk::BorderColor border{vk::BorderColor::eIntOpaqueBlack};
};
} // namespace le
