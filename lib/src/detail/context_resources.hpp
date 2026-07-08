#pragma once
#include "detail/render_resources.hpp"
#include "le2d/audio_mixer.hpp"
#include "le2d/render_pass.hpp"
#include "le2d/resource/factory.hpp"
#include <memory>

namespace le::detail {
class ContextResources {
  public:
	explicit ContextResources(gsl::not_null<kvf::IRenderDevice*> render_device, int sfx_sources);

	[[nodiscard]] auto create_render_pass(vk::SampleCountFlagBits samples) const -> std::unique_ptr<IRenderPass>;

	std::unique_ptr<IAudioMixer> audio_mixer{};
	std::unique_ptr<ShaderLayout> shader_layout{};
	std::unique_ptr<ISamplerFactory> sampler_factory{};
	std::unique_ptr<IResourceFactory> resource_factory{};
	std::unique_ptr<IRenderResources> render_resources{};
};
} // namespace le::detail
