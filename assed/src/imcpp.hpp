#pragma once
#include <imgui.h>
#include <klib/base_types.hpp>
#include <kvf/rect.hpp>
#include <kvf/time.hpp>
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

class Modal : public klib::Polymorphic {
  public:
	explicit Modal(std::string label, int const flags = 0) : m_label(std::move(label)), m_flags(flags) {}

	[[nodiscard]] auto is_open() const -> bool { return ImGui::IsPopupOpen(m_label.c_str()); }
	void set_open() { m_open = true; }
	void set_closed() { m_closed = true; }
	static void close_current() { ImGui::CloseCurrentPopup(); }

	void tick(kvf::Seconds dt);

  protected:
	virtual void update() = 0;

	[[nodiscard]] auto get_dt() const -> kvf::Seconds { return m_dt; }

	std::string m_label{};
	int m_flags{};

  private:
	kvf::Seconds m_dt{};
	bool m_open{};
	bool m_closed{};
};

class ErrorModal : public Modal {
  public:
	explicit ErrorModal(std::string title = "Error!") : Modal(std::move(title)) {}

	void update() override;

	std::string message{};
};
} // namespace le::assed::imcpp
