#pragma once
#include "kvf/kvf_fwd.hpp"
#include "le2d/resource/audio_buffer.hpp"
#include "le2d/resource/font.hpp"
#include "le2d/resource/shader.hpp"
#include "le2d/resource/texture.hpp"
#include <memory>

namespace le {
/// \brief Factory for IResource derived types.
class IResourceFactory : public klib::Polymorphic {
  public:
	using SpirV = IShader::SpirV;

	[[nodiscard]] virtual auto get_render_device() const -> kvf::IRenderDevice& = 0;

	/// \param vertex Vertex shader SPIR-V code.
	/// \param fragment Fragment shader SPIR-V code.
	/// \returns Concrete instance if successfully loaded.
	[[nodiscard]] virtual auto create_shader(SpirV vertex, SpirV fragment) const -> std::unique_ptr<IShader> = 0;

	/// \param bitmap Bitmap to write.
	/// \param sampler TextureSampler to use.
	/// \returns Concrete instance.
	[[nodiscard]] virtual auto create_texture(kvf::Bitmap bitmap, TextureSampler sampler = {}) const -> std::unique_ptr<ITexture> = 0;

	/// \brief Create a 1x1 white texture.
	/// \returns Concrete instance.
	[[nodiscard]] auto create_default_texture(TextureSampler sampler = {}) const -> std::unique_ptr<ITexture> { return create_texture({}, sampler); }

	/// \param bitmap Bitmap to write.
	/// \param sampler TextureSampler to use.
	/// \returns Concrete instance.
	[[nodiscard]] virtual auto create_tilesheet(kvf::Bitmap bitmap, TextureSampler sampler = {}) const -> std::unique_ptr<ITileSheet> = 0;

	/// \param font_bytes Copy of TTF / OTF data as bytes.
	/// \returns Concrete instance if successfully loaded.
	[[nodiscard]] virtual auto create_font(std::vector<std::byte> font_bytes) const -> std::unique_ptr<IFont> = 0;

	/// \param bytes Compressed audio bytes to decode.
	/// \param encoding Encoding of audio data, if known.
	/// \returns Concrete instance if successfully decoded.
	[[nodiscard]] virtual auto create_audio_buffer(std::span<std::byte const> bytes, std::optional<capo::Encoding> encoding = {}) const
		-> std::unique_ptr<IAudioBuffer> = 0;
};
} // namespace le
