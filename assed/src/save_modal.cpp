#include <save_modal.hpp>
#include <filesystem>

namespace le::assed {
namespace fs = std::filesystem;

void SaveModal::set_open(std::string_view const uri) {
	uri_input.set_text(uri);
	m_set_open = true;
	m_overwrite = false;
}

auto SaveModal::update() -> Result {
	if (m_set_open) {
		m_set_open = false;
		ImGui::OpenPopup(title.c_str());
	}
	auto ret = Result::None;
	if (imcpp::begin_modal(title.c_str())) {
		ImGui::SetNextItemWidth(300.0f);
		uri_input.update("URI");
		ImGui::Separator();
		auto disabled = uri_input.as_view().empty();
		auto const path = fs::path{root_dir} / uri_input.as_view();
		if (fs::is_regular_file(path)) {
			ImGui::Checkbox("overwrite existing?", &m_overwrite);
			disabled |= !m_overwrite;
		}
		ImGui::BeginDisabled(disabled);
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
