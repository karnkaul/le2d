#pragma once
#include <imgui.h>
#include <klib/base_types.hpp>
#include <klib/c_string.hpp>
#include <kvf/rect.hpp>
#include <array>
#include <vector>

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

class InputText : public klib::Polymorphic {
  public:
	static constexpr std::size_t init_size_v{64};

	auto update(klib::CString name, glm::vec2 multi_size = {}) -> bool;
	void set_text(std::string_view text);

	[[nodiscard]] auto as_view() const -> std::string_view { return m_buffer.data(); }
	[[nodiscard]] auto as_span() const -> std::span<char const> { return m_buffer; }

  protected:
	auto on_callback(ImGuiInputTextCallbackData& data) -> int;

	void resize_buffer(ImGuiInputTextCallbackData& data);

	std::vector<char> m_buffer{};
};

[[nodiscard]] auto begin_modal(klib::CString label) -> bool;
} // namespace le::assed::imcpp
