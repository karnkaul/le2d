#include "imcpp.hpp"
#include <array>
#include <ranges>

namespace le::assed {
namespace imcpp {
void MultiSelect::sync_to_selection() {
	for (auto const [index, entry] : std::views::enumerate(entries)) { selection.SetItemSelected(ImGuiID(index), entry.is_selected); }
}

void MultiSelect::update(klib::CString const label) {
	static constexpr auto w_flags_v = ImGuiChildFlags_FrameStyle;
	if (ImGui::BeginChild(label.c_str(), ImVec2{0.0f, ImGui::GetFontSize() * 8.0f}, w_flags_v)) {
		static constexpr auto ms_flags_v = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
		auto* ms_io = ImGui::BeginMultiSelect(ms_flags_v, selection.Size, int(entries.size()));
		selection.ApplyRequests(ms_io);
		for (auto [index, entry] : std::views::enumerate(entries)) {
			entry.is_selected = selection.Contains(ImGuiID(index));
			ImGui::SetNextItemSelectionUserData(ImGuiSelectionUserData(index));
			ImGui::Selectable(entry.label.c_str(), entry.is_selected);
		}
		ms_io = ImGui::EndMultiSelect();
		selection.ApplyRequests(ms_io);
	}
	ImGui::EndChild();
}
} // namespace imcpp

auto imcpp::drag_tex_rect(kvf::UvRect& uv, glm::ivec2 const size) -> bool {
	auto const tex_rect = static_cast<kvf::Rect<int>>(uv * size);
	auto left_right = std::array{tex_rect.lt.x, tex_rect.rb.x};
	auto top_bottom = std::array{tex_rect.lt.y, tex_rect.rb.y};
	auto modified = ImGui::DragInt2("left-right", left_right.data(), 1.0f, 0, size.x);
	modified |= ImGui::DragInt2("top-bottom", top_bottom.data(), 1.0f, 0, size.y);
	if (modified) { uv = kvf::UvRect{.lt = {left_right[0], top_bottom[0]}, .rb = {left_right[1], top_bottom[1]}} / size; }
	return modified;
}

auto imcpp::color_edit(klib::CString const label, kvf::Color& color) -> bool {
	auto vec = color.to_linear();
	if (ImGui::ColorEdit4(label.c_str(), &vec.x)) {
		color = kvf::Color::linear_to_srgb(vec);
		return true;
	}
	return false;
}

auto imcpp::begin_modal(klib::CString const label) -> bool {
	auto const center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});
	return ImGui::BeginPopupModal(label.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
}
} // namespace le::assed
