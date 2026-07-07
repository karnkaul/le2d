#pragma once
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

	[[nodiscard]] virtual auto create_shader(SpirV vertex, SpirV fragment) const -> std::unique_ptr<IShader> = 0;

	[[nodiscard]] virtual auto create_texture(kvf::Bitmap bitmap = {}, TextureSampler const& sampler = {}) const -> std::unique_ptr<ITexture> = 0;

	[[nodiscard]] virtual auto create_tilesheet(kvf::Bitmap bitmap, TextureSampler const& sampler = {}) const -> std::unique_ptr<ITileSheet> = 0;

	/// \param font_bytes Copy of TTF / OTF data as bytes.
	/// \returns Concrete instance if successfully loaded.
	[[nodiscard]] virtual auto create_font(std::vector<std::byte> font_bytes) const -> std::unique_ptr<IFont> = 0;

	/// \param bytes Bytes to load.
	/// \param encoding Encoding of audio data, if known.
	/// \returns Concrete instance if successfully loaded.
	[[nodiscard]] virtual auto create_audio_buffer(std::span<std::byte const> bytes, std::optional<capo::Encoding> encoding = {}) const
		-> std::unique_ptr<IAudioBuffer> = 0;
};
} // namespace le
