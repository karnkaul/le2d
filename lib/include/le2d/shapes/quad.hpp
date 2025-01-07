#pragma once
#include <kvf/rect.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/shape.hpp>

namespace le::shape {
class Quad {
  public:
	static constexpr auto rect_v = kvf::Rect<>::from_size(glm::vec2{200.0f});

	explicit Quad(kvf::Rect<> const& rect = rect_v) { set_rect(rect); }

	[[nodiscard]] auto get_rect() const -> kvf::Rect<>;
	void set_rect(kvf::Rect<> const& rect);

	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_rect().size(); }
	void set_size(glm::vec2 size) { set_rect(kvf::Rect<>::from_size(size, get_origin())); }

	[[nodiscard]] auto get_origin() const -> glm::vec2 { return get_rect().center(); }
	void set_origin(glm::vec2 origin) { set_rect(kvf::Rect<>::from_size(get_size(), origin)); }

	[[nodiscard]] auto primitive() const -> Primitive;

	ITexture const* texture{};

  private:
	std::array<Vertex, 4> m_vertices{};
};

static_assert(ShapeT<Quad>);
} // namespace le::shape
