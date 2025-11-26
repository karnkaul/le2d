#include "le2d/unprojector.hpp"

namespace le {
namespace {
[[nodiscard]] auto world_size(Viewport const& viewport, glm::vec2 const framebuffer_size) {
	if (auto const* letterbox = std::get_if<viewport::Letterbox>(&viewport)) { return letterbox->unproject_target_space(framebuffer_size); }
	return framebuffer_size;
}
} // namespace

Unprojector::Unprojector(Viewport const& viewport, Transform const& view, glm::vec2 const framebuffer_size)
	: Unprojector(view, world_size(viewport, framebuffer_size)) {}
} // namespace le
