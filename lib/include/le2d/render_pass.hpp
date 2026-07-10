#pragma once
#include "kvf/color_bitmap.hpp"
#include "kvf/image_writer.hpp"
#include "kvf/kvf_fwd.hpp"
#include "le2d/renderer.hpp"
#include "le2d/resource/resource.hpp"
#include <glm/vec2.hpp>
#include <optional>

namespace le {
/// \brief Opaque interface for 2D render pass, owns a (possibly multi-sampled) color RenderTarget.
class IRenderPass : public IResource {
  public:
	static constexpr auto min_size_v{32};
	static constexpr auto max_size_v{4 * 4096};

	[[nodiscard]] virtual auto get_render_device() const -> kvf::IRenderDevice& = 0;

	/// \returns Reference to last RenderTarget.
	[[nodiscard]] virtual auto get_render_target() const -> kvf::RenderTarget const& = 0;

	/// \returns Multi-sampling count.
	[[nodiscard]] virtual auto get_samples() const -> vk::SampleCountFlagBits = 0;

	/// \param sampler Handle to Vulkan Sampler.
	/// \returns Reference to last RenderTarget as a bindable texture.
	[[nodiscard]] virtual auto render_texture() const -> IRenderTexture& = 0;

	/// \brief Set clear color for next pass.
	/// \param color Clear color.
	virtual void set_clear_color(kvf::Color color) = 0;

	/// \brief Recreate RenderTargets with possibly different MSAA samples.
	virtual void recreate(vk::SampleCountFlagBits samples) = 0;

	/// \returns Concrete Renderer for this Render Pass instance.
	[[nodiscard]] virtual auto create_renderer() -> std::unique_ptr<IRenderer> = 0;

	/// \brief Copy the last RenderTarget into memory as a raw RGBA buffer.
	/// Note: this operation is expensive.
	[[nodiscard]] virtual auto raw_screenshot(glm::ivec2 custom_size = {}) const -> std::optional<kvf::ColorBitmap> = 0;

	/// \brief Convert a raw_screenshot() result to a compressed image.
	/// \returns Empty vector on failure.
	/// Note: this operation is expensive.
	[[nodiscard]] auto compressed_screenshot(kvf::Encoding encoding, glm::ivec2 custom_size = {}) const -> std::vector<std::byte> {
		auto const bitmap = raw_screenshot(custom_size);
		if (!bitmap) { return {}; }
		return kvf::ImageWriter{.bitmap = bitmap->bitmap()}.write(encoding);
	}
};
} // namespace le
