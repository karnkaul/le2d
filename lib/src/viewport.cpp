#include <kvf/aspect_resize.hpp>
#include <le2d/viewport.hpp>
#include <optional>

namespace le::viewport {
namespace {
[[nodiscard]] constexpr auto get_resize_aspect(glm::vec2 const world_size, glm::vec2 const target_size) -> std::optional<kvf::ResizeAspect> {
	if (!kvf::is_positive(world_size) || !kvf::is_positive(target_size)) { return {}; }
	auto const self_ar = world_size.x / world_size.y;
	auto const target_ar = target_size.x / target_size.y;
	if (self_ar == target_ar) { return kvf::ResizeAspect::None; }
	if (self_ar < target_ar) { return kvf::ResizeAspect::FixHeight; }
	return kvf::ResizeAspect::FixWidth;
}
} // namespace

auto Letterbox::fill_target_space(glm::vec2 const target_size) const -> glm::vec2 {
	auto const resize_aspect = get_resize_aspect(world_size, target_size);
	if (!resize_aspect) { return target_size; }
	return kvf::aspect_resize(target_size, world_size, *resize_aspect);
}

auto Letterbox::unproject_target_space(glm::vec2 const target_size) const -> glm::vec2 {
	auto const resize_aspect = get_resize_aspect(world_size, target_size);
	if (!resize_aspect) { return target_size; }
	return kvf::aspect_resize(world_size, target_size, *resize_aspect);
}
} // namespace le::viewport
