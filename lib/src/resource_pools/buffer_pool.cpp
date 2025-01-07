#include <kvf/render_device.hpp>
#include <le2d/resource_pools/buffer_pool.hpp>
#include <log.hpp>
#include <vulkan/vulkan_hash.hpp>

namespace le {
namespace {
constexpr std::size_t warn_limit_v{5000};
} // namespace

auto BufferPool::Hasher::operator()(vk::BufferUsageFlags const usage) const -> std::size_t { return std::hash<vk::BufferUsageFlags>{}(usage); }

BufferPool::BufferPool(gsl::not_null<kvf::RenderDevice*> render_device) : m_render_device(render_device) {}

void BufferPool::next_frame() {
	for (auto& [_, list] : m_map) { list.at(std::size_t(m_render_device->get_frame_index())).next_index = 0; }
}

auto BufferPool::allocate(vk::BufferUsageFlags const usage, vk::DeviceSize const size) -> kvf::vma::Buffer& {
	auto& list = m_map[usage].at(std::size_t(m_render_device->get_frame_index()));

	if (list.next_index < list.buffers.size()) {
		auto& ret = list.buffers.at(list.next_index);
		ret->resize(size);
		++list.next_index;
		return *ret;
	}

	if (list.buffers.size() > warn_limit_v) { log::warn("BufferPool: {} buffers already allocated, possible leak?", list.buffers.size()); }

	auto const bci = kvf::vma::BufferCreateInfo{
		.usage = usage,
		.type = kvf::vma::BufferType::Host,
	};
	list.buffers.push_back(std::make_unique<kvf::vma::Buffer>(m_render_device, bci, size));
	list.next_index = list.buffers.size();
	return *list.buffers.back();
}
} // namespace le
