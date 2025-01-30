#pragma once
#include <kvf/buffered.hpp>
#include <kvf/vma.hpp>
#include <unordered_map>

namespace le {
class BufferPool {
  public:
	explicit BufferPool(gsl::not_null<kvf::RenderDevice*> render_device);

	void next_frame();

	[[nodiscard]] auto allocate(vk::BufferUsageFlags usage, vk::DeviceSize size) -> kvf::vma::Buffer&;

  private:
	struct List {
		std::vector<std::unique_ptr<kvf::vma::Buffer>> buffers{};
		std::size_t next_index{};
	};

	struct Hasher {
		[[nodiscard]] auto operator()(vk::BufferUsageFlags usage) const -> std::size_t;
	};

	kvf::RenderDevice* m_render_device;

	std::unordered_map<vk::BufferUsageFlags, kvf::Buffered<List>, Hasher> m_map{};
};
} // namespace le
