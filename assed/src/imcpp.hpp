#pragma once
#include <imgui.h>
#include <kvf/rect.hpp>
#include <array>

namespace le::assed::imcpp {
inline auto drag_tex_rect(kvf::UvRect& uv, glm::ivec2 const size) -> bool {
	auto const tex_rect = static_cast<kvf::Rect<int>>(uv * size);
	auto left_right = std::array{tex_rect.lt.x, tex_rect.rb.x};
	auto top_bottom = std::array{tex_rect.lt.y, tex_rect.rb.y};
	auto modified = ImGui::DragInt2("left-right", left_right.data(), 1.0f, 0, size.x);
	modified |= ImGui::DragInt2("top-bottom", top_bottom.data(), 1.0f, 0, size.y);
	if (modified) { uv = kvf::UvRect{.lt = {left_right[0], top_bottom[0]}, .rb = {left_right[1], top_bottom[1]}} / size; }
	return modified;
}
} // namespace le::assed::imcpp
