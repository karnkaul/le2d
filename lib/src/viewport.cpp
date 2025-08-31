#include <kvf/aspect_resize.hpp>
#include <le2d/viewport.hpp>

namespace le::viewport {
auto Letterbox::scaled_size(glm::vec2 target_size) const -> glm::vec2 {
	if (!kvf::is_positive(world_size) || !kvf::is_positive(target_size)) { return target_size; }
	auto const self_ar = world_size.x / world_size.y;
	auto const target_ar = target_size.x / target_size.y;
	auto const resize_aspect = [&] {
		if (self_ar == target_ar) { return kvf::ResizeAspect::None; }
		if (self_ar < target_ar) { return kvf::ResizeAspect::FixHeight; }
		return kvf::ResizeAspect::FixWidth;
	}();
	return kvf::aspect_resize(target_size, world_size, resize_aspect);
}
} // namespace le::viewport
