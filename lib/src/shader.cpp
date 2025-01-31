#include <klib/hash_combine.hpp>
#include <le2d/shader.hpp>
#include <log.hpp>

namespace le {
Shader::Shader(IDataLoader const& data_loader, vk::Device device, Uri const& vertex, Uri const& fragment) { load(data_loader, device, vertex, fragment); }

auto Shader::load(IDataLoader const& data_loader, vk::Device const device, Uri const& vertex, Uri const& fragment) -> bool {
	auto spirv = std::vector<std::uint32_t>{};
	auto smci = vk::ShaderModuleCreateInfo{};

	if (!data_loader.load_spirv(spirv, vertex)) {
		log::error("Shader: failed to load Vertex Shader SPIR-V: '{}'", vertex.get_string());
		return false;
	}
	smci.setCode(spirv);
	auto vert = device.createShaderModuleUnique(smci);
	if (!vert) {
		log::error("Shader: failed to create Vertex Shader from: '{}'", vertex.get_string());
		return false;
	}

	if (!data_loader.load_spirv(spirv, fragment)) {
		log::error("Shader: failed to load Fragment Shader SPIR-V: '{}'", fragment.get_string());
		return false;
	}
	smci.setCode(spirv);
	auto frag = device.createShaderModuleUnique(smci);
	if (!frag) {
		log::error("Shader: failed to create Fragment Shader from: '{}'", fragment.get_string());
		return false;
	}

	m_vertex = std::move(vert);
	m_fragment = std::move(frag);
	m_hash = klib::make_combined_hash(vertex.get_hash(), fragment.get_hash());
	log::info("Shader: loaded '{}' / '{}'", vertex.get_string(), fragment.get_string());

	return true;
}
} // namespace le
