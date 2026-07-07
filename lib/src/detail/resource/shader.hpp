#pragma once
#include "kvf/graphics_shader.hpp"
#include "le2d/resource/shader.hpp"
#include "le2d/vertex.hpp"
#include <array>

namespace le::detail {
class Shader : public IShader {
  public:
	explicit Shader(gsl::not_null<kvf::IRenderDevice*> render_device, std::span<vk::DescriptorSetLayout const> set_layouts)
		: m_render_device(render_device), m_set_layouts(set_layouts) {}

	[[nodiscard]] auto load(SpirV vertex, SpirV fragment) -> bool final {
		static constexpr auto bindings_v = std::array{
			vk::VertexInputBindingDescription2EXT{0, sizeof(Vertex), vk::VertexInputRate::eVertex, 1},
		};

		static constexpr auto attributes_v = std::array{
			vk::VertexInputAttributeDescription2EXT{0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)},
			vk::VertexInputAttributeDescription2EXT{1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color)},
			vk::VertexInputAttributeDescription2EXT{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
		};

		static constexpr auto input_v = kvf::GraphicsShaderInput{
			.bindings = bindings_v,
			.attributes = attributes_v,
		};

		auto const shader_ci = kvf::IGraphicsShader::CreateInfo{
			.code = {.vertex = vertex, .fragment = fragment},
			.input = input_v,
			.set_layouts = m_set_layouts,
		};
		m_shader = kvf::IGraphicsShader::create(m_render_device, shader_ci);
		return m_shader != nullptr;
	}

  private:
	[[nodiscard]] auto get_kvf_shader() const -> kvf::IGraphicsShader const& final { return *m_shader; }

	[[nodiscard]] auto is_ready() const -> bool final { return true; }

	gsl::not_null<kvf::IRenderDevice*> m_render_device;
	std::span<vk::DescriptorSetLayout const> m_set_layouts;

	std::unique_ptr<kvf::IGraphicsShader> m_shader{};
};
} // namespace le::detail
