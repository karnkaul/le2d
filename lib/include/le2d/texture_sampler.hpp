#pragma once
#include <vulkan/vulkan.hpp>

namespace le {
struct TextureSampler {
	vk::SamplerAddressMode wrap_u{vk::SamplerAddressMode::eClampToEdge};
	vk::SamplerAddressMode wrap_v{vk::SamplerAddressMode::eClampToEdge};
	vk::Filter min_filter{vk::Filter::eLinear};
	vk::Filter mag_filter{vk::Filter::eLinear};
	vk::SamplerMipmapMode mip_map{vk::SamplerMipmapMode::eNearest};
	vk::BorderColor border_color{vk::BorderColor::eFloatTransparentBlack};

	auto operator==(TextureSampler const&) const -> bool = default;
};
} // namespace le
