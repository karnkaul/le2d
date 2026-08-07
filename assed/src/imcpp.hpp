#pragma once
#include "klib/base_types.hpp"
#include "klib/string/c_string.hpp"
#include "kvf/color.hpp"
#include "kvf/rect.hpp"
#include <imgui.h>
#include <vector>

namespace le::assed::imcpp {
auto drag_tex_rect(kvf::UvRect& uv, glm::ivec2 size) -> bool;

auto color_edit(klib::CString label, kvf::Color& color) -> bool;

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
