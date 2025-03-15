#include <imcpp.hpp>
#include <kvf/is_positive.hpp>

namespace le::assed::imcpp {
auto InputText::update(klib::CString const name, glm::vec2 const multi_size) -> bool {
	static constexpr auto flags_v = ImGuiInputTextFlags_CallbackResize;
	if (m_buffer.empty()) { m_buffer.resize(init_size_v, '\0'); }
	auto const cb = [](ImGuiInputTextCallbackData* data) { return static_cast<InputText*>(data->UserData)->on_callback(*data); };
	if (!kvf::is_positive(multi_size)) { return ImGui::InputText(name.c_str(), m_buffer.data(), m_buffer.size(), flags_v, cb, this); }
	return ImGui::InputTextMultiline(name.c_str(), m_buffer.data(), m_buffer.size(), {multi_size.x, multi_size.y}, flags_v, cb, this);
}

void InputText::set_text(std::string_view const text) {
	if (text.empty()) {
		m_buffer.clear();
		return;
	}
	m_buffer.resize(text.size() + 1, '\0');
	std::memcpy(m_buffer.data(), text.data(), text.size());
	m_buffer.at(text.size()) = '\0';
}

auto InputText::on_callback(ImGuiInputTextCallbackData& data) -> int {
	switch (data.EventFlag) {
	case ImGuiInputTextFlags_CallbackResize: resize_buffer(data); break;
	default: break;
	}
	return 0;
}

void InputText::resize_buffer(ImGuiInputTextCallbackData& data) {
	assert(!m_buffer.empty());
	m_buffer.resize(m_buffer.size() * 2);
	data.BufSize = static_cast<int>(m_buffer.size());
	data.Buf = m_buffer.data();
	data.BufDirty = true;
}
} // namespace le::assed::imcpp
