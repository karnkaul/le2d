#pragma once
#include <vulkan/vulkan.hpp>

namespace le {
struct PipelineFixedState {
	vk::SampleCountFlagBits samples;
	vk::PrimitiveTopology topology;
	vk::PolygonMode polygon_mode;
};
} // namespace le
