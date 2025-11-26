#pragma once
#include "le2d/renderer.hpp"
#include <glm/vec2.hpp>
#include <klib/base_types.hpp>

namespace le {
/// \brief Opaque interface for 2D render pass, owns a multi-sampled color RenderTarget.
class IRenderPass : public klib::Polymorphic {
  public:
	static constexpr auto min_size_v{32};
	static constexpr auto max_size_v{4 * 4096};

	[[nodiscard]] virtual auto get_render_device() const -> kvf::RenderDevice& = 0;

	/// \returns Reference to latest RenderTarget.
	[[nodiscard]] virtual auto get_render_target() const -> kvf::RenderTarget const& = 0;

	/// \returns Multi-sampling count.
	[[nodiscard]] virtual auto get_samples() const -> vk::SampleCountFlagBits = 0;

	/// \returns RenderTarget as a texture. Must not outlive RenderPass.
	[[nodiscard]] virtual auto render_texture() const -> RenderTexture = 0;

	/// \brief Set clear color for next pass.
	/// \param color Clear color.
	virtual void set_clear_color(kvf::Color color) = 0;

	/// \brief Recreate RenderTargets with possibly different MSAA samples.
	virtual void recreate(vk::SampleCountFlagBits samples) = 0;

	/// \returns Concrete Renderer for this Render Pass instance.
	[[nodiscard]] virtual auto create_renderer() -> std::unique_ptr<IRenderer> = 0;
};
} // namespace le
