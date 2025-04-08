#pragma once
#include <imgui.h>
#include <klib/base_types.hpp>
#include <klib/c_string.hpp>
#include <kvf/color.hpp>
#include <kvf/rect.hpp>
#include <vector>

namespace le::assed::imcpp {
auto drag_tex_rect(kvf::UvRect& uv, glm::ivec2 size) -> bool;

auto color_edit(klib::CString label, kvf::Color& color) -> bool;

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

class MultiSelect {
  public:
	struct Entry {
		std::string label{};
		bool is_selected{};
	};

	void sync_to_selection();
	void update(klib::CString label);

	std::vector<Entry> entries{};
	ImGuiSelectionBasicStorage selection{};
};

[[nodiscard]] auto begin_modal(klib::CString label) -> bool;
} // namespace le::assed::imcpp
