#include <klib/hash_combine.hpp>
#include <le2d/shader_program.hpp>
#include <log.hpp>

namespace le {
namespace {
[[nodiscard]] auto compute_hash(SpirV const& vert, SpirV const& frag) {
	auto ret = std::size_t{};
	auto const compute = [&](SpirV const& spir_v) {
		for (auto const u32 : spir_v.code) { klib::hash_combine(ret, u32); }
	};
	compute(vert);
	compute(frag);
	return ret;
}
} // namespace

ShaderProgram::ShaderProgram(vk::Device device, SpirV const& vertex, SpirV const& fragment) { load(device, vertex, fragment); }

auto ShaderProgram::load(vk::Device device, SpirV const& vertex, SpirV const& fragment) -> bool {
	auto smci = std::array<vk::ShaderModuleCreateInfo, 2>{};
	smci[0].setCode(vertex.code);
	smci[1].setCode(fragment.code);
	auto vert = device.createShaderModuleUnique(smci[0]);
	auto frag = device.createShaderModuleUnique(smci[1]);
	if (!vert || !frag) { return false; }

	m_vertex = std::move(vert);
	m_fragment = std::move(frag);
	m_hash = compute_hash(vertex, fragment);

	return true;
}
} // namespace le
