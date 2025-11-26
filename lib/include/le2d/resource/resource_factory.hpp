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
	[[nodiscard]] virtual auto create_shader() const -> std::unique_ptr<IShader> = 0;
	[[nodiscard]] virtual auto create_texture(TextureSampler const& sampler = {}) const -> std::unique_ptr<ITexture> = 0;
	[[nodiscard]] virtual auto create_tilesheet(TextureSampler const& sampler = {}) const -> std::unique_ptr<ITileSheet> = 0;
	[[nodiscard]] virtual auto create_font() const -> std::unique_ptr<IFont> = 0;
	[[nodiscard]] virtual auto create_audio_buffer() const -> std::unique_ptr<IAudioBuffer> = 0;
};
} // namespace le
