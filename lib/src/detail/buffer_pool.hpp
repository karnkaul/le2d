#pragma once
#include <kvf/buffered.hpp>
#include <kvf/render_device.hpp>
#include <kvf/vma.hpp>
#include <log.hpp>
#include <vulkan/vulkan_hash.hpp>
#include <unordered_map>

namespace le::detail {
class BufferPool {
  public:
	explicit BufferPool(gsl::not_null<kvf::RenderDevice*> render_device) : m_render_device(render_device) {}

	void next_frame() {
		for (auto& [_, list] : m_map) { list.at(std::size_t(m_render_device->get_frame_index())).next_index = 0; }
	}

	[[nodiscard]] auto allocate(vk::BufferUsageFlags usage, vk::DeviceSize size) -> kvf::vma::Buffer& {
		auto& list = m_map[usage].at(std::size_t(m_render_device->get_frame_index()));

		if (list.next_index < list.buffers.size()) {
			auto& ret = list.buffers.at(list.next_index);
			ret->resize(size);
			++list.next_index;
			return *ret;
		}

		static constexpr std::size_t warn_limit_v{5000};
		if (list.buffers.size() > warn_limit_v) { log::warn("BufferPool: {} buffers already allocated, possible leak?", list.buffers.size()); }

		auto const bci = kvf::vma::BufferCreateInfo{
			.usage = usage,
			.type = kvf::vma::BufferType::Host,
		};
		list.buffers.push_back(std::make_unique<kvf::vma::Buffer>(m_render_device, bci, size));
		list.next_index = list.buffers.size();
		return *list.buffers.back();
	}

  private:
	struct List {
		std::vector<std::unique_ptr<kvf::vma::Buffer>> buffers{};
		std::size_t next_index{};
	};

	struct Hasher {
		[[nodiscard]] auto operator()(vk::BufferUsageFlags usage) const -> std::size_t { return std::hash<vk::BufferUsageFlags>{}(usage); }
	};

	kvf::RenderDevice* m_render_device;

	std::unordered_map<vk::BufferUsageFlags, kvf::Buffered<List>, Hasher> m_map{};
};
} // namespace le::detail
