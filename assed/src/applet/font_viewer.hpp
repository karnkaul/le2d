#pragma once
#include <applet/applet.hpp>
#include <le2d/drawable/input_text.hpp>
#include <le2d/drawable/shape.hpp>
#include <le2d/resource/font.hpp>
#include <optional>

namespace le::assed {
class FontViewer : public Applet {
  public:
	static constexpr klib::CString name_v{"Font Viewer"};

	[[nodiscard]] auto get_name() const -> klib::CString final { return name_v; }

	explicit FontViewer(gsl::not_null<ServiceLocator const*> services);

  private:
	enum class Display : std::int8_t { Atlas, InputText, COUNT_ };

	auto consume_key(event::Key const& key) -> bool final;
	auto consume_codepoint(event::Codepoint codepoint) -> bool final;

	void tick(kvf::Seconds dt) final;
	void render(IRenderer& renderer) const final;

	void on_drop(FileDrop const& drop) final;

	void inspect();
	void inspect_display();
	void inspect_input_text();

	void try_load_font(Uri const& uri);

	void set_text_height(TextHeight height);
	void create_input_text();
	void set_display(Display display);

	std::unique_ptr<IFont> m_font{};
	IFontAtlas const* m_atlas{};
	TextHeight m_text_height{TextHeight::Default};

	drawable::Quad m_quad{};
	std::optional<drawable::InputText> m_input_text{};
	Display m_display{Display::Atlas};
};
} // namespace le::assed
