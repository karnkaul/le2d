#include <imcpp.hpp>

namespace le::assed::imcpp {
void Modal::tick(kvf::Seconds const dt) {
	m_dt = dt;

	if (m_open) {
		ImGui::OpenPopup(m_label.c_str());
		m_open = false;
	}

	if (ImGui::BeginPopupModal(m_label.c_str(), nullptr, m_flags)) {
		update();
		if (m_closed) {
			close_current();
			m_closed = false;
		}
		ImGui::EndPopup();
	}
}

void ErrorModal::update() {
	ImGui::TextUnformatted(message.c_str());
	ImGui::Separator();
	if (ImGui::Button("Close")) { set_closed(); }
}
} // namespace le::assed::imcpp
