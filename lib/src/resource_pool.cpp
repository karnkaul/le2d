#include <common.hpp>
#include <kvf/util.hpp>
#include <le2d/resource_pool.hpp>

namespace le {
ResourcePool::ResourcePool(gsl::not_null<kvf::RenderDevice*> render_device)
	: buffers(render_device), pipelines(render_device), samplers(render_device), white_texture(render_device, white_bitmap_v),
	  m_blocker(render_device->get_device()) {}
} // namespace le
