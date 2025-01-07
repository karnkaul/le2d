#pragma once
#include <le2d/resource_pools/buffer_pool.hpp>
#include <le2d/resource_pools/pipeline_pool.hpp>
#include <le2d/resource_pools/sampler_pool.hpp>
#include <le2d/shader.hpp>
#include <le2d/texture.hpp>
#include <cstddef>
#include <vector>

namespace le {
class ResourcePool {
  public:
	explicit ResourcePool(gsl::not_null<kvf::RenderDevice*> render_device);

	void next_frame() { buffers.next_frame(); }

	BufferPool buffers;
	PipelinePool pipelines;
	SamplerPool samplers;

	Texture white_texture;
	Shader default_shader{};

	std::vector<std::byte> scratch_buffer{};

  private:
	kvf::DeviceBlock m_blocker;
};
} // namespace le
