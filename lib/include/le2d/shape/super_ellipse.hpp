#pragma once
#include <kvf/rect.hpp>
#include <le2d/render_instance.hpp>
#include <le2d/shape/shape.hpp>
#include <le2d/vertex_array.hpp>

namespace le {
struct SuperEllipseParams {
	glm::vec2 size{200.0f};
	kvf::Color color{kvf::white_v};
	float exponent{4.0f};
	std::int32_t resolution{128};
};

namespace shape {
class SuperEllipse {
  public:
	using Params = SuperEllipseParams;

	void create(Params const& params = {});

	[[nodiscard]] auto get_params() const -> Params const& { return m_params; }
	[[nodiscard]] auto get_size() const -> glm::vec2 { return get_params().size; }

	[[nodiscard]] auto get_vertex_array() const -> VertexArray const& { return m_verts; }

	[[nodiscard]] auto get_primitive() const -> Primitive;

	ITexture const* texture{};

  private:
	VertexArray m_verts{};
	Params m_params{};
};

static_assert(ShapeT<SuperEllipse>);
} // namespace shape
} // namespace le
