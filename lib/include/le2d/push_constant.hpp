#pragma once

namespace le {
struct PushConstant {
	vk::ShaderStageFlagBits stages{};
	std::uint32_t size{};
	std::array<std::byte, 128> data{};
	std::uint32_t offset{};
};
} // namespace le