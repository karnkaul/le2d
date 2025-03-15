#include <save_modal.hpp>

namespace le::assed {
auto SaveModal::update() -> Result {
	if (set_open) {
		set_open = false;
		ImGui::OpenPopup(title.c_str());
	}
	auto ret = Result::None;
	if (imcpp::begin_modal(title.c_str())) {
		ImGui::SetNextItemWidth(150.0f);
		uri.update("URI");
		ImGui::Separator();
		ImGui::BeginDisabled(uri.as_view().empty());
		if (ImGui::Button("Save")) {
			ret = Result::Save;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ret = Result::Cancel;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	return ret;
}
} // namespace le::assed
